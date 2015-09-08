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

#ifndef _STORE_CALIBRATION_H
#define _STORE_CALIBRATION_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <time.h>
#include <sys/inotify.h>
#include <pthread.h>
#include "configuration.h"

#if defined(STORE_CALIB_ENABLED)
#define concat(first, second)		first second
#define PACKAGENAME			"com.st.mems.st_gyrocal"
#define CAL_FILE 			"calibration.txt"
#define CAL_DIR				"/data/"
#define OBS_MASK 			(IN_MODIFY | IN_DELETE | IN_CREATE)
#define NUM_OF_SENSORS			6
#define NUM_OF_AXIS			3
#define THD_SLEEP_USEC			3000000
#define EVENT_SIZE			(sizeof(struct inotify_event))
#define MAX_CHARS_PER_LINE		512
#define DELIMITER			" "
#define MAX_TOKENS_PER_LINE		4
#define EVENT_BUF_SIZE			512

typedef float calib_out_t[NUM_OF_SENSORS][NUM_OF_AXIS];

typedef struct {
    uid_t  uid;
    char   isDebuggable;
    char   dataDir[PATH_MAX];
    char   seinfo[PATH_MAX];
} PackageInfo;

class StoreCalibration {
private:
	StoreCalibration();
	static void* checkChangesThread(void *arg);
	static void readCalibrationFile();
	static int eventCheck(int fd);

	static int instanceCount;
	static StoreCalibration *single;
	static calib_out_t calibration;
	static time_t oldMTime;
	static int observer_fd;
	static int watch_fd;
	static int cal_file;
	pthread_t thread;
	static bool is_changed;

public:
	enum {
		GYROSCOPE_BIAS = 0,
		GYROSCOPE_SENS,
		ACCELEROMETER_BIAS,
		ACCELEROMETER_SENS,
		MAGNETOMETER_BIAS,
		MAGNETOMETER_SENS,
		SPEC_NUM,
	};

	enum {
		XAxis = 0,
		YAxis,
		ZAxis,
		NumAxis,
	};

	static StoreCalibration* getInstance();
	~StoreCalibration() {
		instanceCount--;
		if (instanceCount == 0) {
			inotify_rm_watch(observer_fd, watch_fd);
		}
	}
	float getCalibration(int sensorId, int axis);
	bool isChanged();
};
#endif /* STORE_CALIB_ENABLED */
#endif /* _STORE_CALIBRATION_H */