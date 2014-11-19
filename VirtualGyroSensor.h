/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Alberto Marinoni, Giuseppe Barba
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

#include "configuration.h"
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)

#ifndef ANDROID_VIRTUAL_GYRO_SENSOR_H
#define ANDROID_VIRTUAL_GYRO_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>


#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include "MagnSensor.h"
#include "AccelSensor.h"

extern "C"
{
	#include "iNemoEngineGeoMagAPI.h"
};

/*****************************************************************************/

struct input_event;

class VirtualGyroSensor : public SensorBase
{
	enum {
		Acceleration = 0,
		MagneticField,
		VirtualGyro,
		iNemoGyro,
		numSensors
	};
	static int mEnabled;
	static int64_t delayms;
	static int current_fullscale;
	sensors_event_t mPendingEvent[numSensors];
	int setInitialState();
	InputEventCircularReader mInputReader;
	bool mHasPendingEvent;
	sensors_vec_t mSensorsBufferedVectors[2];

private:
	static int startup_samples;
	static int samples_to_discard;
	static sensors_vec_t dataBuffer;
	static int64_t MagDelay_ms;
	static int64_t setDelayBuffer[numSensors];
	static int DecimationBuffer[numSensors];
	static int DecimationCount;
	virtual bool setBufferData(sensors_vec_t *value);

	float gyro[3];
	MagnSensor *mag;
	AccelSensor *acc;
	static pthread_mutex_t dataMutex;

public:
	VirtualGyroSensor();
	virtual ~VirtualGyroSensor();
	virtual int readEvents(sensors_event_t *data, int count);
	virtual bool hasPendingEvents() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual void updateDecimations(int64_t Delay_ms);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	static bool getBufferData(sensors_vec_t *lastBufferedValues);
	static void getGyroDelay(int64_t *Gyro_Delay_ms);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_VIRTUAL_GYRO_SENSOR_H

#endif /* SENSORS_GYROSCOPE_ENABLE */
