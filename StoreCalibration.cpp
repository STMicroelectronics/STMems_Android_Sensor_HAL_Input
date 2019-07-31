/*
 * Copyright (C) 2015 STMicroelectronics
 * Giuseppe Barba <giuseppe.barba@st.com>
 * Motion MEMS Product Div.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define CLASS_NAME "StoreCalibration"
#include <sys/stat.h>
#include "StoreCalibration.h"
#include <fcntl.h>
#include <cutils/log.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#if defined(STORE_CALIB_ENABLED)
#include <private/android_filesystem_config.h>

#include <fstream>
using std::ifstream;

#include <iostream>
using std::cout;
using std::endl;

#define PACKAGES_LIST_FILE  "/data/system/packages.list"


static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int StoreCalibration::instanceCount = 0;
StoreCalibration* StoreCalibration::single = NULL;
time_t StoreCalibration::oldMTime = 0;
int StoreCalibration::cal_file = 0;
int StoreCalibration::observer_fd = 0;
int StoreCalibration::watch_fd = 0;
calib_out_t StoreCalibration::calibration;
bool StoreCalibration::is_changed = false;

static const struct sensor_spec_t {
	uint8_t id;
	char token[9];
	float reset_value;
} sensor_spec[] = {
	{
		StoreCalibration::GYROSCOPE_BIAS,
		"GyroBias",
		0.0,
	},
	{
		StoreCalibration::GYROSCOPE_SENS,
		"GyroSens",
		1.0,
	},
	{
		StoreCalibration::ACCELEROMETER_BIAS,
		"AccBias",
		0.0,
	},
	{
		StoreCalibration::ACCELEROMETER_SENS,
		"AccSens",
		1.0,
	},
	{
		StoreCalibration::MAGNETOMETER_BIAS,
		"MagBias",
		0.0,
	},
	{
		StoreCalibration::MAGNETOMETER_SENS,
		"MagSens",
		1.0,
	},
};

int StoreCalibration::eventCheck(int fd)
{
	fd_set rfds;

	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);
	/* Wait until an event happens */

	return select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

StoreCalibration::StoreCalibration()
{
	int rc = 0;

	readCalibrationFile();
	observer_fd = inotify_init();
	if (!observer_fd) {
		ALOGE("Error to start inotify!!!");
		return;
	}

	rc = pthread_create(&thread, NULL,
			    StoreCalibration::checkChangesThread, NULL);
	if (rc) {
		ALOGE("Error to create pthread\n");
	}
}

StoreCalibration* StoreCalibration::getInstance()
{
	if(instanceCount == 0)
	{
		    single = new StoreCalibration();
		    instanceCount = 1;

		    return single;
	} else {
		    instanceCount++;

		    return single;
	}
}

void* StoreCalibration::checkChangesThread(void  __attribute__((unused))*arg)
{
	char event_buf[EVENT_BUF_SIZE];
	struct inotify_event* event;
	int num_bytes = 0;
	int event_pos, i;
	int event_size;
	struct stat s;

	while (stat(CAL_DIR, &s)<0) {
		/* sleep until the calibration directory is present */
		usleep(THD_SLEEP_USEC);
	}

	watch_fd = inotify_add_watch(observer_fd, CAL_DIR, OBS_MASK);
	if (!watch_fd) {
		ALOGE("Error while adding inotify watcher!!!");
		pthread_exit(NULL);
		return NULL;
	}

	while (true) {
		if (eventCheck(observer_fd)) {
			num_bytes = read(observer_fd, event_buf, EVENT_BUF_SIZE);
			event_pos = 0;
			while (num_bytes >= (int)sizeof(*event)) {
				event = (struct inotify_event *)(event_buf + event_pos);
				if (event->len) {
					if (event->mask & (IN_MODIFY /*| IN_CREATE*/)) {

						if (!strcmp(event->name, CAL_FILE)) {
							ALOGI("Changes on Calibration file detected");
							readCalibrationFile();
						}
					} else if (event->mask & (IN_DELETE)) {
						ALOGI("Calibration file deleted");

						pthread_mutex_lock(&lock);
						memset(calibration, 0, sizeof(calibration));
						is_changed = true;
						pthread_mutex_unlock(&lock);
					} else {
						ALOGI("Event not used %d", event->mask);
					}
				}
				event_size = sizeof(*event) + event->len;
				num_bytes -= event_size;
				event_pos += event_size;
			}
		}
	}
	pthread_exit(NULL);
	return NULL;
}

void StoreCalibration::readCalibrationFile()
{
	ifstream fin;
	char buf[MAX_CHARS_PER_LINE];
	const char* token[MAX_TOKENS_PER_LINE] = {};
	int n, i;

	fin.open(concat(CAL_DIR,CAL_FILE));
	pthread_mutex_lock(&lock);
	for (n = 0; n < SPEC_NUM; n++)
		for (i = 0; i < NumAxis; i++)
			calibration[sensor_spec[n].id][i] = sensor_spec[n].reset_value;

	if (!fin.good()) {
		ALOGI("Calibration File is not present! %s", concat(CAL_DIR,CAL_FILE));
	} else {
		ALOGI("Calibration File is present!");
		while (!fin.eof())
		{
			fin.getline(buf, MAX_CHARS_PER_LINE);

			token[0] = strtok(buf, DELIMITER);
			if (token[0]) // zero if line is blank
			{
			        for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
				{
					token[n] = strtok(0, DELIMITER);
					if (!token[n]) break;
				}
				for (n = 0; n < SPEC_NUM; n++)
				{
					if(strcmp(sensor_spec[n].token, token[0]) == 0)
						break;
				}
				if (n == SPEC_NUM)
					break;

				calibration[sensor_spec[n].id][XAxis] = atof(token[1]);
				calibration[sensor_spec[n].id][YAxis] = atof(token[2]);
				calibration[sensor_spec[n].id][ZAxis] = atof(token[3]);
				ALOGD("Sensor %s = %f\t%f\t%f",
					sensor_spec[n].token,
					calibration[sensor_spec[n].id][XAxis],
					calibration[sensor_spec[n].id][YAxis],
					calibration[sensor_spec[n].id][ZAxis]);
			}
		}
		is_changed = true;
	}

	pthread_mutex_unlock(&lock);
	fin.close();
}

float StoreCalibration::getCalibration(int sensorId, int axis)
{
	if ((sensorId < NUM_OF_SENSORS) && (axis < NUM_OF_AXIS)) {
		return calibration[sensorId][axis];
	} else {
		ALOGE("Invalid argument to getCalibration");
		return 0;
	}
}

bool StoreCalibration::isChanged()
{
	bool temp;

	pthread_mutex_lock(&lock);
	temp = is_changed;
	is_changed = false;
	pthread_mutex_unlock(&lock);

	return temp;
}

#endif /* STORE_CALIB_ENABLED */
