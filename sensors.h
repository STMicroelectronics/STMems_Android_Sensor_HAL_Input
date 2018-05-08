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

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>
#include "configuration.h"

__BEGIN_DECLS

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define ID_BASE					(SENSORS_HANDLE_BASE+1)
#define ID_ACCELEROMETER			(ID_BASE+0)
#define ID_MAGNETIC_FIELD			(ID_BASE+1)
#define ID_ORIENTATION				(ID_BASE+2)
#define ID_GYROSCOPE				(ID_BASE+3)
#define ID_LIGHT				(ID_BASE+4)
#define ID_PRESSURE				(ID_BASE+5)
#define ID_TEMPERATURE				(ID_BASE+6)
#define ID_PROXIMITY				(ID_BASE+7)
#define ID_GRAVITY				(ID_BASE+8)
#define ID_LINEAR_ACCELERATION			(ID_BASE+9)
#define ID_ROTATION_VECTOR			(ID_BASE+10)
#define ID_GAME_ROTATION			(ID_BASE+11)
#define ID_UNCALIB_GYROSCOPE			(ID_BASE+12)
#define ID_SIGNIFICANT_MOTION			(ID_BASE+13)
#define ID_UNCALIB_MAGNETIC_FIELD		(ID_BASE+14)
#define ID_GEOMAG_ROTATION_VECTOR		(ID_BASE+15)
#define ID_SENSOR_FUSION			(ID_BASE+16)
#define ID_VIRTUAL_GYROSCOPE			(ID_BASE+17)
#define ID_TILT_DETECTOR			(ID_BASE+18)
#define ID_STEP_COUNTER				(ID_BASE+19)
#define ID_STEP_DETECTOR			(ID_BASE+20)
#define ID_SIGN_MOTION				(ID_BASE+21)
#define ID_ACTIVITY_RECOGNIZER			(ID_BASE+22)
#define ID_TAP					(ID_BASE+23)
#define ID_HUMIDITY				(ID_BASE+24)
#define ID_UNCALIB_ACCELEROMETER		(ID_BASE+25)

#define SENSORS_ACCELEROMETER_HANDLE		ID_ACCELEROMETER
#define SENSORS_MAGNETIC_FIELD_HANDLE		ID_MAGNETIC_FIELD
#define SENSORS_ORIENTATION_HANDLE		ID_ORIENTATION
#define SENSORS_LIGHT_HANDLE			ID_LIGHT
#define SENSORS_PROXIMITY_HANDLE		ID_PROXIMITY
#define SENSORS_GYROSCOPE_HANDLE		ID_GYROSCOPE
#define SENSORS_GRAVITY_HANDLE			ID_GRAVITY
#define SENSORS_LINEAR_ACCELERATION_HANDLE	ID_LINEAR_ACCELERATION
#define SENSORS_ROTATION_VECTOR_HANDLE		ID_ROTATION_VECTOR
#define SENSORS_PRESSURE_HANDLE			ID_PRESSURE
#define SENSORS_TEMPERATURE_HANDLE		ID_TEMPERATURE
#define SENSORS_GAME_ROTATION_HANDLE		ID_GAME_ROTATION
#define SENSORS_UNCALIB_GYROSCOPE_HANDLE	ID_UNCALIB_GYROSCOPE
#define SENSORS_SIGNIFICANT_MOTION_HANDLE	ID_SIGNIFICANT_MOTION
#define SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE	ID_UNCALIB_MAGNETIC_FIELD
#define SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE	ID_GEOMAG_ROTATION_VECTOR
#define SENSORS_SENSOR_FUSION_HANDLE		ID_SENSOR_FUSION
#define SENSORS_VIRTUAL_GYROSCOPE_HANDLE	ID_VIRTUAL_GYROSCOPE
#define SENSORS_TILT_DETECTOR_HANDLE		ID_TILT_DETECTOR
#define SENSORS_STEP_COUNTER_HANDLE		ID_STEP_COUNTER
#define SENSORS_STEP_DETECTOR_HANDLE		ID_STEP_DETECTOR
#define SENSORS_SIGN_MOTION_HANDLE		ID_SIGN_MOTION
#define SENSORS_ACTIVITY_RECOGNIZER_HANDLE	ID_ACTIVITY_RECOGNIZER
#define SENSORS_TAP_HANDLE			ID_TAP
#define SENSORS_HUMIDITY_HANDLE			ID_HUMIDITY
#define SENSORS_UNCALIB_ACCELEROMETER_HANDLE	ID_UNCALIB_ACCELEROMETER

#define SENSOR_TYPE_TAP				(SENSOR_TYPE_DEVICE_PRIVATE_BASE + 2)
#define SENSOR_TYPE_ACTIVITY			(SENSOR_TYPE_DEVICE_PRIVATE_BASE + 3)

#define SENSOR_STRING_TYPE_TAP			"com.st.tap"

#define TRACE_FUNCTION_START			ALOGD("%s:start at %lld", __func__, SensorBase::getTimestamp());
#define TRACE_FUNCTION_END			ALOGD("%s:end at %lld", __func__, SensorBase::getTimestamp());
/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/

#if (ANDROID_VERSION >= ANDROID_JBMR2)
#define SENSOR_POLL_DEVICE sensors_poll_device_1
#else
#define SENSOR_POLL_DEVICE sensors_poll_device_t
#endif

__END_DECLS

#endif  // ANDROID_SENSORS_H
