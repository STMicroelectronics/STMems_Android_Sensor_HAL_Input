/*
 * Copyright (C) 2016 STMicroelectronics
 * Lorenzo Bianconi - Motion MEMS Product Div.
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
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))

#ifndef ANDROID_HUMIDITY_SENSOR_H
#define ANDROID_HUMIDITY_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

class HumiditySensor : public SensorBase {
private:
	bool mEnabled;
	int current_fullscale;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvents[2];

	int64_t delayms;
	char device_sysfs_path_prs[PATH_MAX];
	int device_sysfs_path_prs_len;
	int writeSensorDelay(int handle);
public:
	HumiditySensor();
	~HumiditySensor();
	int readEvents(sensors_event_t* data, int count);
	bool hasPendingEvents() const { return false; }
	int setDelay(int32_t handle, int64_t ns);
	int setFullScale(int32_t handle, int value);
	int enable(int32_t handle, int enabled, int type);
	int getWhatFromHandle(int32_t handle) { return 0; }
};
#endif /* ANDROID_HUMIDITY_SENSOR_H */
#endif /* SENSORS_HUMIDITYURE_ENABLE || SENSORS_TEMP_RH_ENABLE */
