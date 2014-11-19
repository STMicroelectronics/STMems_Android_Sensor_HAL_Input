/*
 * Copyright (C) 2013 STMicroelectronics
 * Giuseppe Barba, Matteo Dameno, Ciocca Denis, Alberto Marinoni
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
#include <sys/stat.h>

#include <fstream>
using std::ifstream;

#include <iostream>
using std::cout;
using std::endl;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int StoreCalibration::instanceCount = 0;
StoreCalibration* StoreCalibration::single = NULL;
time_t StoreCalibration::oldMTime = 0;
int StoreCalibration::cal_file = 0;
int StoreCalibration::observer_fd = 0;
int StoreCalibration::watch_fd = 0;
bias_out_t StoreCalibration::bias;
bool StoreCalibration::is_changed = false;


int StoreCalibration::eventCheck(int fd)
{
	fd_set rfds;

	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);
	/* Wait until an event happens */
	return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

StoreCalibration::StoreCalibration()
{
	int rc = 0;

	readCalibrationFile();
	observer_fd = inotify_init();
	if (!observer_fd) {
		STLOGE("Error to start inotify!!!");
		return;
	}

	rc = pthread_create(&thread, NULL, StoreCalibration::checkChangesThread, NULL);
	if (rc) {
		STLOGE("Error to create pthread\n");
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
		STLOGE("Error while adding inotify watcher!!!");
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
							//LOG_I("Changes on Calibration file detected");

							readCalibrationFile();
						}
					} else if (event->mask & (IN_DELETE)) {
						//LOG_I("Calibration file deleted");

						pthread_mutex_lock(&lock);
						memset(bias, 0, sizeof(bias));
						is_changed = true;
						pthread_mutex_unlock(&lock);
					} else {
						//LOG_I("Event not used %d", event->mask);
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
	int n;

	LOG_I("Read calibration file");
	fin.open(concat(CAL_DIR,CAL_FILE));

	pthread_mutex_lock(&lock);
	memset(bias, 0, sizeof(bias));
	bias[ACCELEROMETER_GAIN][XAxis] = 1.0;
	bias[ACCELEROMETER_GAIN][YAxis] = 1.0;
	bias[ACCELEROMETER_GAIN][ZAxis] = 1.0;
	if (!fin.good()) {
		LOG_I("Calibration File is not present! %s", concat(CAL_DIR,CAL_FILE));
	} else {
		LOG_I("Calibration File is present!");
		while (!fin.eof())
		{
			fin.getline(buf, MAX_CHARS_PER_LINE);
			n = 0;
			const char* token[MAX_TOKENS_PER_LINE] = {};
			token[0] = strtok(buf, DELIMITER);
			if (token[0]) // zero if line is blank
			{
			        for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
				{
					token[n] = strtok(0, DELIMITER);
					if (!token[n]) break;
				}
#if (SENSORS_GYROSCOPE_ENABLE == 1)
				if(!strcmp(GYRO_TOKEN, token[0])) {
					bias[GYROSCOPE][XAxis] = atof(token[1]);
					bias[GYROSCOPE][YAxis] = atof(token[2]);
					bias[GYROSCOPE][ZAxis] = atof(token[3]);
					LOG_I("Gyro bias %f:%f:%f", bias[GYROSCOPE][XAxis],
							bias[GYROSCOPE][YAxis],
							bias[GYROSCOPE][ZAxis]);
				}
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
				if(!strcmp(ACC_BIAS_TOKEN, token[0])) {
					
					bias[ACCELEROMETER_BIAS][XAxis] = atof(token[1]);
					bias[ACCELEROMETER_BIAS][YAxis] = atof(token[2]);
					bias[ACCELEROMETER_BIAS][ZAxis] = atof(token[3]);
					LOG_I("Acc bias %f:%f:%f", bias[ACCELEROMETER_BIAS][XAxis],
							bias[ACCELEROMETER_BIAS][YAxis],
							bias[ACCELEROMETER_BIAS][ZAxis]);
				}
				if(!strcmp(ACC_GAIN_TOKEN, token[0])) {
					
					bias[ACCELEROMETER_GAIN][XAxis] = atof(token[1]);
					bias[ACCELEROMETER_GAIN][YAxis] = atof(token[2]);
					bias[ACCELEROMETER_GAIN][ZAxis] = atof(token[3]);
					LOG_I("Acc Sensitivity %f:%f:%f",
							bias[ACCELEROMETER_GAIN][XAxis],
							bias[ACCELEROMETER_GAIN][YAxis],
							bias[ACCELEROMETER_GAIN][ZAxis]);
				}
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
				if(!strcmp(MAG_TOKEN, token[0])) {
					bias[MAGNETOMETER][XAxis] = atof(token[1]);
					bias[MAGNETOMETER][YAxis] = atof(token[2]);
					bias[MAGNETOMETER][ZAxis] = atof(token[3]);
					LOG_I("Mag bias %f:%f:%f", bias[MAGNETOMETER][XAxis],
							bias[MAGNETOMETER][YAxis],
							bias[MAGNETOMETER][ZAxis]);
				}
#endif
			}
		}
	}
	is_changed = true;
	pthread_mutex_unlock(&lock);
	fin.close();
}

float StoreCalibration::getCalibration(int sensorId, int axis)
{
	if ((sensorId < NUM_OF_SENSORS) && (axis < NUM_OF_AXIS)) {
		//LOG_I("get calibration[%d][%d] = %f", sensorId, axis, bias[sensorId][axis]);
		return bias[sensorId][axis];
	} else {
		STLOGE("Unvalid argument to getCalibration");
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