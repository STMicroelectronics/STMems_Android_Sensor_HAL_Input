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

#ifndef ST_STORE_CALIBRATION_H
#define ST_STORE_CALIBRATION_H

#include "configuration.h"

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <time.h>
#include <sys/inotify.h>
#include <pthread.h>


#include "sensors.h"
#include "SensorBase.h"

#define DEBUG_STORE_CALIBRATION	1

#define LOG_I(...)	{if (DEBUG_STORE_CALIBRATION) STLOGI(__VA_ARGS__);}while(0);
#define concat(first, second) first second

#define CAL_FILE 			"calibration.txt" //TODO: put this into configuration.h
#define CAL_DIR				"/data/data/com.st.mems.st_calibrationtool/files/"
#define OBS_MASK 			(IN_MODIFY | IN_DELETE | IN_CREATE)
#define NUM_OF_SENSORS			4
#define NUM_OF_AXIS			3
#define THD_SLEEP_USEC			3000000
#define EVENT_SIZE			(sizeof(struct inotify_event))
#define MAX_CHARS_PER_LINE		512
#define DELIMITER			" "
#define MAX_TOKENS_PER_LINE		4
#define ACC_BIAS_TOKEN			"AccBias"
#define ACC_GAIN_TOKEN			"AccSens"
#define GYRO_TOKEN			"Gyro"
#define MAG_TOKEN			"Mag"
#define EVENT_BUF_SIZE			512

typedef float bias_out_t[NUM_OF_SENSORS][NUM_OF_AXIS];

class StoreCalibration {
private:
	StoreCalibration();
	static void* checkChangesThread(void *arg);
	static void readCalibrationFile();
	static int eventCheck(int fd);

	static int instanceCount;
	static StoreCalibration *single;
	static bias_out_t bias;
	static time_t oldMTime;
	static int observer_fd;
	static int watch_fd;
	static int cal_file;
	pthread_t thread;
	static bool is_changed;

public:
	static StoreCalibration* getInstance();
	~StoreCalibration() {
		instanceCount--;
		if (instanceCount == 0) {
			inotify_rm_watch(observer_fd, watch_fd);
		}
	}
	float getCalibration(int sensorId, int axis);
	bool isChanged();
	enum {
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		GYROSCOPE = 0,
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		ACCELEROMETER_BIAS,
		ACCELEROMETER_GAIN,
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		MAGNETOMETER,
#endif
	};

	enum {
		XAxis = 0,
		YAxis,
		ZAxis,
	};
};

#endif /* ST_STORE_CALIBRATION_H */