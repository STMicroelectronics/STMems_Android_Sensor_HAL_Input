/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Denis Ciocca - Motion MEMS Product Div.
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

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#define DELAY_OFF		(-1000000)
#define MSEC_TO_SEC(x)		((x) / 1000)
#define NSEC_TO_MSEC(x)		((x) / 1000000)
#define MSEC_TO_NSEC(x)		((x) * 1000000)
#define SEC_TO_MSEC(x)		((x) * 1000)
#define SEC_TO_NSEC(x)		(MSEC_TO_NSEC(SEC_TO_MSEC(x)))
/*****************************************************************************/

struct sensors_event_t;

class SensorBase {

protected:
	const char* dev_name;
	const char* data_name;
	char sysfs_device_path[PATH_MAX];
	int sysfs_device_path_len;
	int dev_fd;
	int data_fd;

	int openInput(const char* inputDeviceName);


	static int64_t timevalToNano(timeval const& t) {
		return t.tv_sec*1000000000LL + t.tv_usec*1000;
	}

	int open_device();
	int close_device();
	int getSysfsDevicePath(char* sysfs_path, const char* inputDeviceName);

public:
	SensorBase(const char* dev_name, const char* data_name);
	virtual ~SensorBase();

	virtual int readEvents(sensors_event_t* data, int count) = 0;
	virtual bool hasPendingEvents() const;
	virtual int getFd() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int enable(int32_t handle, int enabled, int type) = 0;
	virtual int writeFullScale(int32_t handle, int value);
	virtual int writeEnable(int32_t handle, int enable);
	virtual int writeDelay(int32_t handle, int64_t delay_ms);
	virtual int writeSysfsCommand(int32_t handle, const char *sysfsFilename, const char *dataFormat, int64_t param);
	virtual int getWhatFromHandle(int32_t handle) = 0;
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H
