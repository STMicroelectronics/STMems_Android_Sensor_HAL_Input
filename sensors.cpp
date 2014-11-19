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

#include <hardware/sensors.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <linux/input.h>
#include <cutils/log.h>

#include "sensors.h"
#include "configuration.h"

#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
#include "MagnSensor.h"
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
#include "GyroSensor.h"
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
#include "AccelSensor.h"
#endif
#if (SENSOR_FUSION_ENABLE == 1)
#include "iNemoEngineSensor.h"
#endif
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
#include "PressSensor.h"
#endif
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
#include "VirtualGyroSensor.h"
#endif
#if (SENSORS_TILT_ENABLE == 1)
#include "TiltSensor.h"
#endif
#if (SENSORS_STEP_COUNTER_ENABLE == 1)
#include "StepCounterSensor.h"
#endif
#if (SENSORS_STEP_DETECTOR_ENABLE == 1)
#include "StepDetectorSensor.h"
#endif
#if (SENSORS_SIGN_MOTION_ENABLE == 1)
#include "SignMotionSensor.h"
#endif
#if (SENSORS_TAP_ENABLE == 1)
#include "TapSensor.h"
#endif
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
#include "HumiditySensor.h"
#endif


/*****************************************************************************/

#define DELAY_OUT_TIME			0x7FFFFFFF

#define LIGHT_SENSOR_POLLTIME		2000000000
#define FREQUENCY_TO_USECONDS(x)		(1000000 / x)

/*****************************************************************************/

/* The SENSORS Module */
static const struct sensor_t sSensorList[] = {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	{
		SENSOR_ACC_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_ACCELEROMETER_HANDLE,
		SENSOR_TYPE_ACCELEROMETER,
		ACCEL_MAX_RANGE,
		0.0f,
		ACCEL_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(ACCEL_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_ACCELEROMETER,
		"",
		FREQUENCY_TO_USECONDS(ACCEL_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	{
		SENSOR_MAGN_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_MAGNETIC_FIELD_HANDLE,
		SENSOR_TYPE_MAGNETIC_FIELD,
		MAGN_MAX_RANGE,
		0.0f,
		MAGN_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(MAGN_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_MAGNETIC_FIELD,
		"",
		FREQUENCY_TO_USECONDS(MAGN_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
	{
		SENSOR_GYRO_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_GYROSCOPE_HANDLE,
		SENSOR_TYPE_GYROSCOPE,
		GYRO_MAX_RANGE,
		0.0f,
		GYRO_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(GYRO_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GYROSCOPE,
		"",
		FREQUENCY_TO_USECONDS(GYRO_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if ((SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1) && ((GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1) || (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)))
	{
		SENSOR_GYRO_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_UNCALIB_GYROSCOPE_HANDLE,
		SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
		GYRO_MAX_RANGE,
		0.0f,
		UNCALIB_GYRO_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(GYRO_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED,
		"",
		FREQUENCY_TO_USECONDS(GYRO_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
		{
		SENSOR_SIGNIFICANT_MOTION_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_SIGNIFICANT_MOTION_HANDLE,
		SENSOR_TYPE_SIGNIFICANT_MOTION,
		1.0f,
		0.0f,
		ACCEL_POWER_CONSUMPTION,
		-1,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_SIGNIFICANT_MOTION,
		"",
		-1,
		SENSOR_FLAG_ONE_SHOT_MODE,
#endif
#endif
		{ }
	},
#endif
#if ((SENSORS_ORIENTATION_ENABLE == 1) || (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1))
	{
  #if (SENSORS_ORIENTATION_ENABLE == 1)
		"iNemoEngine Orientation sensor",
  #else
    #if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
		"iNemoEngineGeoMag Orientation sensor",
    #else
		"STMicroelectronics Orientation sensor",
    #endif
  #endif
		"STMicroelectronics",
		1,
		SENSORS_ORIENTATION_HANDLE,
		SENSOR_TYPE_ORIENTATION,
		360.0f,
		0.0f,
		ORIENTATION_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(ORIENTATION_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_ORIENTATION,
		"",
		FREQUENCY_TO_USECONDS(ORIENTATION_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_GRAVITY_ENABLE == 1)
	{
		"iNemoEngine Gravity sensor",
		"STMicroelectronics",
		1,
		SENSORS_GRAVITY_HANDLE,
		SENSOR_TYPE_GRAVITY,
		GRAVITY_EARTH,
		0.0f,
		GRAVITY_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(FUSION_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GRAVITY,
		"",
		FREQUENCY_TO_USECONDS(FUSION_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
	{
		"iNemoEngine Linear Acceleration sensor",
		"STMicroelectronics",
		1,
		SENSORS_LINEAR_ACCELERATION_HANDLE,
		SENSOR_TYPE_LINEAR_ACCELERATION,
		ACCEL_MAX_RANGE-GRAVITY_EARTH,
		0.0f,
		LINEAR_ACCEL_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(FUSION_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_LINEAR_ACCELERATION,
		"",
		FREQUENCY_TO_USECONDS(FUSION_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_ROTATION_VECTOR_ENABLE == 1)
	{
		"iNemoEngine Rotation_Vector sensor",
		"STMicroelectronics",
		1,
		SENSORS_ROTATION_VECTOR_HANDLE,
		SENSOR_TYPE_ROTATION_VECTOR,
#if (!SENSORS_GYROSCOPE_ENABLE && SENSORS_VIRTUAL_GYROSCOPE_ENABLE)
		VIRTUAL_GYRO_MAX_RANGE,
#else
		GYRO_MAX_RANGE,
#endif
		0.0f,
		ROT_VEC_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(FUSION_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_ROTATION_VECTOR,
		"",
		FREQUENCY_TO_USECONDS(FUSION_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_GAME_ROTATION_ENABLE == 1)
	{
		"iNemoEngine Game Rotation sensor",
		"STMicroelectronics",
		1,
		SENSORS_GAME_ROTATION_HANDLE,
		SENSOR_TYPE_GAME_ROTATION_VECTOR,
#if (!SENSORS_GYROSCOPE_ENABLE && SENSORS_VIRTUAL_GYROSCOPE_ENABLE)
		VIRTUAL_GYRO_MAX_RANGE,
#else
		GYRO_MAX_RANGE,
#endif
		0.0f,
		GYRO_POWER_CONSUMPTION+ACCEL_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(FUSION_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR,
		"",
		FREQUENCY_TO_USECONDS(FUSION_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_PRESSURE_ENABLE == 1)
	{
		SENSOR_PRESS_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_PRESSURE_HANDLE,
		SENSOR_TYPE_PRESSURE,
		PRESS_MAX_RANGE,
		0.0f,
		PRESS_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(PRESS_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_PRESSURE,
		"",
		FREQUENCY_TO_USECONDS(PRESS_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_TEMPERATURE_ENABLE == 1)
	{
		SENSOR_TEMP_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_TEMPERATURE_HANDLE,
		SENSOR_TYPE_TEMPERATURE,
		TEMP_MAX_RANGE,
		0.0f,
		TEMP_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(TEMP_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_TEMPERATURE,
		"",
		FREQUENCY_TO_USECONDS(TEMP_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE == 1)
	{
		SENSOR_MAGN_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE,
		SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
		MAGN_MAX_RANGE,
		0.0f,
		MAGN_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(MAGN_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
		"",
		FREQUENCY_TO_USECONDS(MAGN_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
	{
		"iNemoEngine Geomagnetic Rotation Vector sensor",
		"STMicroelectronics",
		1,
		SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE,
		SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
		1.0f,
		0.0f,
		MAGN_POWER_CONSUMPTION+ACCEL_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(MAGN_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
		"",
		FREQUENCY_TO_USECONDS(MAGN_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
	{
		SENSOR_VIRTUAL_GYRO_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
		SENSOR_TYPE_GYROSCOPE,
		VIRTUAL_GYRO_MAX_RANGE,
		0.0f,
		VIRTUAL_GYRO_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(VIRTUAL_GYRO_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_GYROSCOPE,
		"",
		FREQUENCY_TO_USECONDS(VIRTUAL_GYRO_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_TILT_ENABLE == 1)
	{
		SENSOR_TILT_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_TILT_DETECTOR_HANDLE,
		SENSOR_TYPE_TILT_DETECTOR,
		1.0f,
		1.0f,
		TILT_POWER_CONSUMPTION,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_TILT_DETECTOR,
		"",
		0,
		SENSOR_FLAG_SPECIAL_REPORTING_MODE,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_STEP_COUNTER_ENABLE == 1)
	{
		SENSOR_STEP_C_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_STEP_COUNTER_HANDLE,
		SENSOR_TYPE_STEP_COUNTER,
		65535.0f,
		1.0f,
		STEP_C_POWER_CONSUMPTION,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_STEP_COUNTER,
		"",
		0,
		SENSOR_FLAG_ON_CHANGE_MODE,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_STEP_DETECTOR_ENABLE == 1)
	{
		SENSOR_STEP_D_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_STEP_DETECTOR_HANDLE,
		SENSOR_TYPE_STEP_DETECTOR,
		1.0f,
		1.0f,
		STEP_D_POWER_CONSUMPTION,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_STEP_DETECTOR,
		"",
		0,
		SENSOR_FLAG_SPECIAL_REPORTING_MODE,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_SIGN_MOTION_ENABLE == 1)
	{
		SENSOR_SIGN_M_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_SIGN_MOTION_HANDLE,
		SENSOR_TYPE_SIGNIFICANT_MOTION,
		1.0f,
		1.0f,
		SIGN_M_POWER_CONSUMPTION,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_SIGNIFICANT_MOTION,
		"",
		0,
		SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_TAP_ENABLE == 1)
	{
		SENSOR_TAP_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_TAP_HANDLE,
		SENSOR_TYPE_TAP,
		10.0f,
		1.0f,
		TAP_POWER_CONSUMPTION,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_TAP,
		"",
		0,
		SENSOR_FLAG_ON_CHANGE_MODE,
#endif
#endif
		{ }
	},
#endif

#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
	{
		SENSOR_ACTIVITY_RECOGNIZERO_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_ACTIVITY_RECOGNIZER_HANDLE,
		SENSOR_TYPE_ACTIVITY,
		10.0f,
		1.0f,
		SENSORS_ACTIVITY_RECOGNIZER_POWER,
		0,
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_ACTIVITY,
		"",
		0,
		SENSOR_FLAG_ON_CHANGE_MODE,
#endif
#endif
		{ }
	},
#endif
#if (SENSORS_HUMIDITY_ENABLE == 1)
	{
		SENSOR_HUMIDITY_LABEL,
		"STMicroelectronics",
		1,
		SENSORS_HUMIDITY_HANDLE,
		SENSOR_TYPE_RELATIVE_HUMIDITY,
		HUMIDITY_MAX_RANGE,
		0.0f,
		HUMIDITY_POWER_CONSUMPTION,
		FREQUENCY_TO_USECONDS(HUMIDITY_MAX_ODR),
#if (ANDROID_VERSION >= ANDROID_KK)
		0,
		0,
#if (ANDROID_VERSION >= ANDROID_L)
		SENSOR_STRING_TYPE_RELATIVE_HUMIDITY,
		"",
		FREQUENCY_TO_USECONDS(HUMIDITY_MIN_ODR),
		SENSOR_FLAG_CONTINUOUS_MODE,
#endif
#endif
		{ }
	},
#endif
};


static int open_sensors(const struct hw_module_t* module, const char* id, struct hw_device_t** device);

void get_ref(sensors_module_t *sm);

static struct hw_module_methods_t sensors_module_methods = {
	open: open_sensors
};

int sensors__get_sensors_list(struct sensors_module_t __attribute__((unused))*module,
						struct sensor_t const** list)
{
	*list = sSensorList;
	return ARRAY_SIZE(sSensorList);
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
	common: {
		tag: HARDWARE_MODULE_TAG,
		version_major: 1,
		version_minor: 0,
		id: SENSORS_HARDWARE_MODULE_ID,
		name: "STMicroelectronics Sensor module",
		author: "STMicroelectronics",
		methods: &sensors_module_methods,
		dso: NULL,
		reserved: { },
	},
	get_sensors_list: sensors__get_sensors_list,
#if (ANDROID_VERSION >= ANDROID_M)
	set_operation_mode: NULL,
#endif
}	
;

void get_ref(sensors_module_t __attribute__((unused))*sm) {
	sm = &HAL_MODULE_INFO_SYM;
	return;
}

struct sensors_poll_context_t {
	struct SENSOR_POLL_DEVICE device;

	sensors_poll_context_t();
	~sensors_poll_context_t();
	int activate(int handle, int enabled);
	int setDelay(int handle, int64_t ns);
	int pollEvents(sensors_event_t* data, int count);
#if (ANDROID_VERSION >= ANDROID_JBMR2)
	int batch(int sensor_handle, int flags, int64_t sampling_period_ns,
										int64_t max_report_latency_ns);
	int flush(int sensor_handle);
#endif
private:
	enum {
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		gyro = 0,
#endif
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
		virtual_gyro,
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		accel,
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		magn,
#endif
#if (SENSOR_FUSION_ENABLE == 1)
		inemo,
#endif
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
		press,
#endif
#if(SENSORS_TILT_ENABLE == 1)
		tilt,
#endif
#if(SENSORS_STEP_COUNTER_ENABLE == 1)
		step_c,
#endif
#if(SENSORS_STEP_DETECTOR_ENABLE == 1)
		step_d,
#endif
#if(SENSORS_SIGN_MOTION_ENABLE == 1)
		sign_m,
#endif
#if(SENSORS_TAP_ENABLE == 1)
		tap,
#endif
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
		humidity,
#endif
		numSensorDrivers,
		numFds,
	};

	struct pollfd mPollFds[numFds];

#if (ANDROID_VERSION >= ANDROID_JBMR2)
	static const size_t flushFD = numFds - 1;
	int mWriteFlushPipe;
#endif
	SensorBase* mSensors[numSensorDrivers];

	int handleToDriver(int handle) const
	{
		switch (handle) {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			case SENSORS_ACCELEROMETER_HANDLE:
				return accel;
#endif

#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
			case SENSORS_MAGNETIC_FIELD_HANDLE:
				return magn;
#endif

#if ((SENSORS_ORIENTATION_ENABLE == 1) || (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1))
			case SENSORS_ORIENTATION_HANDLE:
#if (SENSORS_ORIENTATION_ENABLE == 1)
				return inemo;
#else
				return magn;
#endif
#endif

#if (SENSORS_GRAVITY_ENABLE == 1)
			case SENSORS_GRAVITY_HANDLE:
#if (SENSOR_FUSION_ENABLE == 1)
				return inemo;
#else
				return magn;
#endif
#endif

#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
			case SENSORS_LINEAR_ACCELERATION_HANDLE:
#if (SENSOR_FUSION_ENABLE == 1)
				return inemo;
#else
				return magn;
#endif
#endif

#if (SENSORS_ROTATION_VECTOR_ENABLE == 1)
			case SENSORS_ROTATION_VECTOR_HANDLE:
				return inemo;
#endif
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
			case SENSORS_PRESSURE_HANDLE:
			case SENSORS_TEMPERATURE_HANDLE:
				return press;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
			case SENSORS_GYROSCOPE_HANDLE:
#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
				return inemo;
#else
				return gyro;
#endif
#endif
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
			case SENSORS_VIRTUAL_GYROSCOPE_HANDLE:
				return virtual_gyro;
#endif
#if (SENSORS_GAME_ROTATION_ENABLE == 1) && (SENSOR_FUSION_ENABLE == 1)
			case SENSORS_GAME_ROTATION_HANDLE:
				return inemo;
#endif
#if (SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1)
			case SENSORS_UNCALIB_GYROSCOPE_HANDLE:
#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
				return inemo;
#else
				return gyro;
#endif
#endif
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
			case SENSORS_SIGNIFICANT_MOTION_HANDLE:
				return accel;
#endif
#if SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE
			case SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE:
				return magn;
#endif
#if SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE
			case SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE:
				return magn;
#endif
#if SENSORS_TILT_ENABLE
			case SENSORS_TILT_DETECTOR_HANDLE:
				return tilt;
#endif
#if SENSORS_STEP_COUNTER_ENABLE
			case SENSORS_STEP_COUNTER_HANDLE:
				return step_c;
#endif
#if SENSORS_STEP_DETECTOR_ENABLE
			case SENSORS_STEP_DETECTOR_HANDLE:
				return step_d;
#endif
#if SENSORS_SIGN_MOTION_ENABLE
			case SENSORS_SIGN_MOTION_HANDLE:
				return sign_m;
#endif
#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
			case SENSORS_ACTIVITY_RECOGNIZER_HANDLE:
				return accel;
#endif
#if (SENSORS_TAP_ENABLE == 1)
			case SENSORS_TAP_HANDLE:
				return tap;
#endif
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
			case SENSORS_TEMPERATURE_HANDLE:
			case SENSORS_HUMIDITY_HANDLE:
				return humidity;
#endif
		}
		return -EINVAL;
	}
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	mSensors[magn] = new MagnSensor();
	mPollFds[magn].fd = mSensors[magn]->getFd();
	mPollFds[magn].events = POLLIN;
	mPollFds[magn].revents = 0;
#endif

#if (SENSORS_GYROSCOPE_ENABLE == 1)
	mSensors[gyro] = new GyroSensor();
	mPollFds[gyro].fd = mSensors[gyro]->getFd();
	mPollFds[gyro].events = POLLIN;
	mPollFds[gyro].revents = 0;
#endif

#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
	mSensors[virtual_gyro] = new VirtualGyroSensor();
	mPollFds[virtual_gyro].fd = mSensors[virtual_gyro]->getFd();
	mPollFds[virtual_gyro].events = POLLIN;
	mPollFds[virtual_gyro].revents = 0;
#endif

#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	mSensors[accel] = new AccelSensor();
	mPollFds[accel].fd = mSensors[accel]->getFd();
	mPollFds[accel].events = POLLIN;
	mPollFds[accel].revents = 0;
#endif

#if (SENSOR_FUSION_ENABLE == 1)
	mSensors[inemo] = new iNemoEngineSensor();
	mPollFds[inemo].fd = mSensors[inemo]->getFd();
	mPollFds[inemo].events = POLLIN;
	mPollFds[inemo].revents = 0;
#endif

#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
	mSensors[press] = new PressSensor();
	mPollFds[press].fd = mSensors[press]->getFd();
	mPollFds[press].events = POLLIN;
	mPollFds[press].revents = 0;
#endif

#if (SENSORS_TILT_ENABLE == 1)
	mSensors[tilt] = new TiltSensor();
	mPollFds[tilt].fd = mSensors[tilt]->getFd();
	mPollFds[tilt].events = POLLIN;
	mPollFds[tilt].revents = 0;
#endif

#if (SENSORS_STEP_COUNTER_ENABLE == 1)
	mSensors[step_c] = new StepCounterSensor();
	mPollFds[step_c].fd = mSensors[step_c]->getFd();
	mPollFds[step_c].events = POLLIN;
	mPollFds[step_c].revents = 0;
#endif

#if (SENSORS_STEP_DETECTOR_ENABLE == 1)
	mSensors[step_d] = new StepDetectorSensor();
	mPollFds[step_d].fd = mSensors[step_d]->getFd();
	mPollFds[step_d].events = POLLIN;
	mPollFds[step_d].revents = 0;
#endif

#if (SENSORS_SIGN_MOTION_ENABLE == 1)
	mSensors[sign_m] = new SignMotionSensor();
	mPollFds[sign_m].fd = mSensors[sign_m]->getFd();
	mPollFds[sign_m].events = POLLIN;
	mPollFds[sign_m].revents = 0;
#endif

#if (SENSORS_TAP_ENABLE == 1)
	mSensors[tap] = new TapSensor();
	mPollFds[tap].fd = mSensors[tap]->getFd();
	mPollFds[tap].events = POLLIN;
	mPollFds[tap].revents = 0;
#endif

#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
	mSensors[humidity] = new HumiditySensor();
	mPollFds[humidity].fd = mSensors[humidity]->getFd();
	mPollFds[humidity].events = POLLIN;
	mPollFds[humidity].revents = 0;
#endif

#if (ANDROID_VERSION >= ANDROID_JBMR2)
	int FlushFds[2];
	int err = pipe(FlushFds);
	if (err < 0) {
		ALOGE("Failed to create pipe for flush sensor.");
	} else {
		fcntl(FlushFds[0], F_SETFL, O_NONBLOCK);
		fcntl(FlushFds[1], F_SETFL, O_NONBLOCK);
		mWriteFlushPipe = FlushFds[1];
		mPollFds[flushFD].fd = FlushFds[0];
		mPollFds[flushFD].events = POLLIN;
		mPollFds[flushFD].revents = 0;
	}
#endif
}

sensors_poll_context_t::~sensors_poll_context_t()
{
	for (int i=0 ; i<numSensorDrivers ; i++) {
		delete mSensors[i];
	}

#if (ANDROID_VERSION >= ANDROID_JBMR2)
	close(mPollFds[flushFD].fd);
	close(mWriteFlushPipe);
#endif
}

int sensors_poll_context_t::activate(int handle, int enabled)
{
	int index = handleToDriver(handle);
	if(index < 0)
		return index;

	int err =  mSensors[index]->enable(handle, enabled, 0);
	return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns)
{
	int index = handleToDriver(handle);
	if(index < 0)
		return index;

	return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
	int nbEvents = 0;
	int n = 0;

	do {
		if (count) {
			n = poll(mPollFds, numFds, nbEvents ? 0 : -1);
			if (n < 0) {
				STLOGE("poll() failed (%s)", strerror(errno));
				return -errno;
			}
		}
		for (int i=0 ; count && i<numSensorDrivers ; i++) {
			SensorBase* const sensor(mSensors[i]);
			if((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents()))
			{
				int nb = sensor->readEvents(data, count);
				if (nb < count) {
					mPollFds[i].revents = 0;
				}
				count -= nb;
				nbEvents += nb;
				data += nb;
			}
		}
#if (ANDROID_VERSION >= ANDROID_JBMR2)
		if(mPollFds[flushFD].revents & POLLIN && count) {
			if (read(mPollFds[flushFD].fd, data, sizeof(struct sensors_event_t)) > 0) {
				count--;
				nbEvents++;
				data++;
			}
			mPollFds[flushFD].revents = 0;
		}
#endif
	} while (n && count);

	return nbEvents;
}

#if (ANDROID_VERSION >= ANDROID_JBMR2)
int sensors_poll_context_t::batch(int sensor_handle, int __attribute__((unused))flags,
						int64_t sampling_period_ns,
				  int64_t __attribute__((unused))max_report_latency_ns)
{
	this->setDelay(sensor_handle, sampling_period_ns);

	return 0;
}

int sensors_poll_context_t::flush(int sensor_handle)
{
	sensors_event_t flush_event;
	int err;

	flush_event.timestamp = 0;
	flush_event.meta_data.sensor = sensor_handle;
	flush_event.meta_data.what = META_DATA_FLUSH_COMPLETE;
	flush_event.type = SENSOR_TYPE_META_DATA;
	flush_event.version = META_DATA_VERSION;
	flush_event.sensor = 0;

	err = write(mWriteFlushPipe, &flush_event, sizeof(flush_event));
	if (err < 0) {
		ALOGE("Failed to write flush data. %d", err);
		return -EINVAL;
	}

	return 0;
}
#endif
/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	if(ctx) {
		delete ctx;
	}
	return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev, int handle, int enabled)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev, int handle, int64_t ns)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev, sensors_event_t* data, int count)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->pollEvents(data, count);
}

#if (ANDROID_VERSION >= ANDROID_JBMR2)
static int poll__batch(struct sensors_poll_device_1* dev, int sensor_handle, int flags,
						int64_t sampling_period_ns, int64_t max_report_latency_ns)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->batch(sensor_handle, flags, sampling_period_ns, max_report_latency_ns);
}

static int poll__flush(struct sensors_poll_device_1* dev, int sensor_handle)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	return ctx->flush(sensor_handle);
}
#endif
/*****************************************************************************/

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t* module,
			const char __attribute__((unused))*id,
			struct hw_device_t** device)
{
	int status = -EINVAL;
	sensors_poll_context_t *dev = new sensors_poll_context_t();

	memset(&dev->device, 0, sizeof(SENSOR_POLL_DEVICE));

	dev->device.common.tag		= HARDWARE_DEVICE_TAG;

#if (ANDROID_VERSION > ANDROID_KK)
	dev->device.common.version	= SENSORS_DEVICE_API_VERSION_1_3;
#else
#if (ANDROID_VERSION > ANDROID_JBMR2)
	dev->device.common.version	= SENSORS_DEVICE_API_VERSION_1_1;
#else
#if (ANDROID_VERSION == ANDROID_JBMR2)
	dev->device.common.version	= SENSORS_DEVICE_API_VERSION_1_0;
#else
	dev->device.common.version	= 0;
#endif
#endif
#endif
	dev->device.common.module	= const_cast<hw_module_t*>(module);
	dev->device.common.close	= poll__close;
	dev->device.activate		= poll__activate;
	dev->device.setDelay		= poll__setDelay;
	dev->device.poll			= poll__poll;
#if (ANDROID_VERSION >= ANDROID_JBMR2)
	dev->device.batch			= poll__batch;
	dev->device.flush			= poll__flush;
#endif

	*device = &dev->device.common;
	status = 0;

	return status;
}

