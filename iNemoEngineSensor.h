/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Alberto Marinoni - Motion MEMS Product Div.
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
#if (SENSOR_FUSION_ENABLE == 1)

#ifndef ANDROID_INEMOENGINE_SENSOR_H
#define ANDROID_INEMOENGINE_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

#if (SENSORS_ACCELEROMETER_ENABLE == 1)
#include "AccelSensor.h"
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
#include "GyroSensor.h"
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
#include "MagnSensor.h"
#endif

extern "C"
{
	#include "iNemoEngineProAPI.h"
};

/*****************************************************************************/

struct input_event;

class iNemoEngineSensor : public SensorBase
{
	enum {
		Acceleration = 0,
		MagneticField,
		AngularSpeed,
		Orientation,
		Gravity,
		LinearAcceleration,
		RotationMatrix,
		GameRotation,
		UncalibGyro,
		CalibGyro,
		numSensors
	};

	int initialized;
	static int mEnabled;
	uint32_t mPendingMask;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvents[numSensors];
	bool mHasPendingEvent;
	int setInitialState();

private:
	static int startup_samples;
	static int samples_to_discard;
	sensors_vec_t mSensorsBufferedVectors[3];
	iNemoInitData init_data_api;
	iNemoDebugInitData debug_init_data_api;

#if (SENSORS_GYROSCOPE_ENABLE == 1)
	char devices_sysfs_path_gyr[PATH_MAX];
	int devices_sysfs_path_gyr_len;
	static GyroSensor *gyr;
#endif
#if(SENSORS_ACCELEROMETER_ENABLE == 1)
	char devices_sysfs_path_acc[PATH_MAX];
	int devices_sysfs_path_acc_len;
	static AccelSensor *acc;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	char devices_sysfs_path_mag[PATH_MAX];
	int devices_sysfs_path_mag_len;
	static MagnSensor *mag;
#endif
	static int status;
	static int64_t gyroDelay_ms;
	static int64_t DelayBuffer[numSensors];
	static int DecimationBuffer[numSensors];
	static int DecimationCount[numSensors];

	int64_t timestamp;

public:
	iNemoEngineSensor();
	virtual ~iNemoEngineSensor();
	virtual int readEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int enable(int32_t handle, int enabled, int type);
	virtual void updateDecimations(int64_t Delay_ms);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_INEMOENGINE_SENSOR_H

#endif /* SENSOR_FUSION_ENABLE */
