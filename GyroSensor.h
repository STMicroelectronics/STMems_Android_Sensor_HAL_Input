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
#if (SENSORS_GYROSCOPE_ENABLE == 1)

#ifndef ANDROID_GYRO_SENSOR_H
#define ANDROID_GYRO_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include "AccelSensor.h"

#if defined(STORE_CALIB_GYRO_ENABLED)
#include "StoreCalibration.h"
#endif

#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
extern "C"
{
#include <iNemoEngineAPI_gbias_estimation.h>
};
#endif

/*****************************************************************************/

struct input_event;

class GyroSensor : public SensorBase
{
	enum {
		Gyro = 0,
		GyroUncalib,
		iNemoGyro,
		numSensors
	};
	static int mEnabled;
	static int64_t delayms;
	static int current_fullscale;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvent[numSensors];
	bool mHasPendingEvent;
	int setInitialState();

private:
	static int startup_samples;
	static int samples_to_discard;
	static sensors_vec_t  dataBuffer;
	static int64_t setDelayBuffer[numSensors];
	static int64_t writeDelayBuffer[numSensors];
	static int DecimationBuffer[numSensors];
	static int DecimationCount[numSensors];
	virtual bool setBufferData(sensors_vec_t *value);
	static float gbias_out[3];
	float data_raw[3];
	float data_rot[3];
	static pthread_mutex_t dataMutex;
	int64_t timestamp;
#if defined(STORE_CALIB_GYRO_ENABLED)
	StoreCalibration *pStoreCalibration;
#endif
#if ((SENSORS_ACCELEROMETER_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1))
	static AccelSensor *acc;
	float data_acc[3];
#endif

public:
	GyroSensor();
	virtual ~GyroSensor();
	virtual int readEvents(sensors_event_t *data, int count);
	virtual bool hasPendingEvents() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int writeMinDelay(void);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	static bool getBufferData(sensors_vec_t *lastBufferedValues);
	static void getGyroDelay(int64_t *Gyro_Delay_ms);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_GYRO_SENSOR_H

#endif /* SENSORS_GYROSCOPE_ENABLE */
