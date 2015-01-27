/*
 * Copyright (C) 2012 STMicroelectronics
 * Giuseppe Barba, Alberto Marinoni - Motion MEMS Product Div.
 * Copyright (C) 2015 The Android Open Source Project
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
#if (SENSORS_STEP_DETECTOR_ENABLE == 1)

#ifndef ANDROID_STEP_DETECTOR_SENSOR_H
#define ANDROID_STEP_DETECTOR_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

struct input_event;

class StepDetectorSensor : public SensorBase
{
	static int mEnabled;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvent;

private:
	static pthread_mutex_t dataMutex;
	int64_t timestamp;
	int32_t steps;

public:
	StepDetectorSensor();
	virtual ~StepDetectorSensor();
	virtual int readEvents(sensors_event_t *data, int count);
	virtual bool hasPendingEvents() const;
	virtual int getWhatFromHandle(int32_t handle);
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int enable(int32_t handle, int enabled, int type);
	static bool getBufferData(sensors_vec_t *lastBufferedValues);
};

#endif  // ANDROID_STEP_DETECTOR_SENSOR_H

#endif
