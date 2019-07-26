/*
 * Copyright (C) 2019 STMicroelectronics
 * Matteo Dameno, Denis Ciocca, Alberto Marinoni - Motion MEMS Product Div.
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

#include "configuration.h"
#if (SENSORS_TEMP_ENABLE == 1)

#ifndef ANDROID_TEMP_SENSOR_H
#define ANDROID_TEMP_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

class TempSensor : public SensorBase {
private:
	enum {
		Temperature,
		numSensors
	};
	static unsigned int mEnabled;
	static int current_fullscale;
	uint32_t mPendingMask;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvents[numSensors];
	bool mHasPendingEvent;

	int setInitialState();

	enum channelid{
		tempChan = 0
	};
	int64_t delayms;

	//char device_sysfs_path_prs[PATH_MAX];
	//int device_sysfs_path_prs_len;
	int writeSensorDelay(int handle);
public:
	TempSensor();
	virtual ~TempSensor();
	virtual int readEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents() const { return mHasPendingEvent; }
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_TEMP_SENSOR_H
#endif /* SENSORS_TEMP_ENABLE */
