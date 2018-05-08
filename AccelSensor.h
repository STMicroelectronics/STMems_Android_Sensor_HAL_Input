/*
 * Copyright (C) 2013 STMicroelectronics
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
#if (SENSORS_ACCELEROMETER_ENABLE == 1)

#ifndef ANDROID_ACC_SENSOR_H
#define ANDROID_ACC_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"

#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
extern "C"
{
	#include "ActivityRecoLib.h"
};
#endif
#if (ACCEL_CALIBRATION_ENABLE == 1)
extern "C"
{
	#include "STAccCalibration_API.h"
};
#endif

/*****************************************************************************/

struct input_event;

class AccelSensor : public SensorBase {
	enum {
		Acceleration = 0,
		AccelUncalib,
		SignificantMotion,
		iNemoAcceleration,
		MagCalAcceleration,
		GeoMagRotVectAcceleration,
		Orientation,
		Gravity_Accel,
		Linear_Accel,
		VirtualGyro,
		Gbias,
		ActivityReco,
		numSensors
	};
	static int mEnabled;
	static int64_t delayms;
	static int current_fullscale;
	InputEventCircularReader mInputReader;
	uint32_t mPendingMask;
	sensors_event_t mPendingEvents[numSensors];
	bool mHasPendingEvent;
	int setInitialState();
#if ACCEL_CALIBRATION_ENABLE == 1
	STAccCalibration_Input accCalibIn;
	STAccCalibration_Output accCalibOut;
#endif

private:
	static sensors_vec_t  dataBuffer;
	static int64_t setDelayBuffer[numSensors];
	static int64_t writeDelayBuffer[numSensors];
	static int DecimationBuffer[numSensors];
	static int DecimationCount;
	virtual bool setBufferData(sensors_vec_t *value);
	float data_raw[3];
	float data_rot[3];
	sensors_vec_t data_calibrated;
	static pthread_mutex_t dataMutex;
	int64_t timestamp;

public:
	AccelSensor();
	virtual ~AccelSensor();
	virtual int readEvents(sensors_event_t *data, int count);
	virtual bool hasPendingEvents() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int writeMinDelay(void);
	static void getAccDelay(int64_t *Acc_Delay_ms);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	static bool getBufferData(sensors_vec_t *lastBufferedValues);
	virtual int getWhatFromHandle(int32_t handle);
};

#endif  // ANDROID_ACCEL_SENSOR_H

#endif /* SENSORS_ACCELEROMETER_ENABLE */
