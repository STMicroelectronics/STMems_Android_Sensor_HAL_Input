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
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)

#ifndef ANDROID_MAGN_SENSOR_H
#define ANDROID_MAGN_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "configuration.h"
#include "sensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include "AccelSensor.h"

#if MAG_CALIBRATION_ENABLE == 1
extern "C"
{
	#include "STMagCalibration_API.h"
};
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
extern "C"
{
	#include "iNemoEngineGeoMagAPI.h"
};
#endif
/*****************************************************************************/

struct input_event;


class MagnSensor : public SensorBase
{
	enum {
		MagneticField = 0,
		UncalibMagneticField,
		iNemoMagnetic,
		GeoMagRotVect_Magnetic,
		Orientation,
		Gravity_Accel,
		Linear_Accel,
		VirtualGyro,
		numSensors
	};
	static int mEnabled;
	static int64_t delayms;
	static int current_fullscale;
	InputEventCircularReader mInputReader;
	sensors_event_t mPendingEvent[numSensors];
	bool mHasPendingEvent;
	int data_read;
	int setInitialState();
#if SENSOR_GEOMAG_ENABLE == 1
	int refFreq;
#endif
#if MAG_CALIBRATION_ENABLE == 1
	STMagCalibration_Input magCalibIn;
	STMagCalibration_Output magCalibOut;
#endif

private:
	static sensors_vec_t  dataBuffer;
	static int64_t setDelayBuffer[numSensors];
	static int64_t writeDelayBuffer[numSensors];
	static int DecimationBuffer[numSensors];
	static int DecimationCount[numSensors];
	sensors_vec_t mSensorsBufferedVectors[3];
	virtual bool setBufferData(sensors_vec_t *value);
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	static AccelSensor *acc;
#endif
#if (SENSOR_GEOMAG_ENABLE == 1)
	iNemoGeoMagSensorsData sData;
#endif
	float data_raw[3];
	float data_rot[3];
	sensors_vec_t data_calibrated;
	static pthread_mutex_t dataMutex;
	int64_t timestamp;

public:
	MagnSensor();
	virtual ~MagnSensor();
	virtual int readEvents(sensors_event_t* data, int count);
	virtual bool hasPendingEvents() const;
	virtual int setDelay(int32_t handle, int64_t ns);
	virtual int writeMinDelay(void);
	static void getMagDelay(int64_t *Mag_Delay_ms);
	virtual int setFullScale(int32_t handle, int value);
	virtual int enable(int32_t handle, int enabled, int type);
	virtual int getWhatFromHandle(int32_t handle);
	int64_t getDelayms() {
		return delayms;
	};
	static bool getBufferData(sensors_vec_t *lastBufferedValues);
	static int count_call_ecompass;
	static int freq;
};

#endif  /* ANDROID_MAGN_SENSOR_H */

#endif /* SENSORS_MAGNETIC_FIELD_ENABLE */
