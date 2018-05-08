/*
 * Copyright (C) 2012 STMicroelectronics
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
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))

#ifndef ANDROID_PRESS_SENSOR_H
#define ANDROID_PRESS_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

class PressSensor : public SensorBase {
private:
	enum {
		Pressure = 0,
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
		pressChan = 0,
		tempChan
	};
	int64_t delayms;

	//char device_sysfs_path_prs[PATH_MAX];
	//int device_sysfs_path_prs_len;
	int writeSensorDelay(int handle);
public:
	PressSensor();
	virtual ~PressSensor();
	virtual int readEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents() const { return mHasPendingEvent; }
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_PRESS_SENSOR_H
#endif /* SENSORS_TEMP_PRESS_ENABLE | SENSORS_PRESSURE_ENABLE */
