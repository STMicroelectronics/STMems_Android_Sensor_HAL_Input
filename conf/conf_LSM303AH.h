/*
 * Copyright (C) 2015 STMicroelectronics
 * Giuseppe Barba - Motion MEMS Product Div.
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

#ifndef CONFIGURATION_LSM303AH_H
#define CONFIGURATION_LSM303AH_H

#define SENSORS_ACCELEROMETER_ENABLE		(1)
#define SENSORS_MAGNETIC_FIELD_ENABLE		(1)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE	(1 & SENSORS_MAGNETIC_FIELD_ENABLE & OS_VERSION_ENABLE)

#define SENSORS_TILT_ENABLE			(1)
#define SENSORS_STEP_COUNTER_ENABLE		(1)
#define SENSORS_STEP_DETECTOR_ENABLE		(1)
#define SENSORS_SIGN_MOTION_ENABLE		(1)
#define SENSORS_TAP_ENABLE			(1)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL			"LSM303AH 3-axis Accelerometer Sensor"
#define SENSOR_DATANAME_ACCELEROMETER		"lsm303ah_acc_accel"
#define ACCEL_DELAY_FILE_NAME			"accel/polling_rate"
#define ACCEL_ENABLE_FILE_NAME			"accel/enable"
#define ACCEL_RANGE_FILE_NAME			"accel/scale"
#define ACCEL_MAX_RANGE				8 * GRAVITY_EARTH
#define ACCEL_MAX_ODR				200
#define ACCEL_POWER_CONSUMPTION			0.6f
#define ACCEL_DEFAULT_FULLSCALE			4

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL			"LSM303AH 3-axis Magnetometer Sensor"
#define SENSOR_UNCALIB_MAGN_LABEL		"LSM303AH 3-axis Uncalibrated Magnetometer sensor"
#define SENSOR_DATANAME_MAGNETIC_FIELD		"ST LSM303AH Magnetometer Sensor"
#define MAGN_DELAY_FILE_NAME			"magn/polling_rate"
#define MAGN_ENABLE_FILE_NAME			"magn/enable"
#define MAGN_RANGE_FILE_NAME			"magn/range"
#define CALIBRATION_ENABLE			1
#define MAGN_MAX_RANGE				1000.0f
#define MAGN_MAX_ODR				100
#define MAGN_MIN_ODR			25
#define MAGN_POWER_CONSUMPTION			0.077f
#define MAGN_DEFAULT_FULLSCALE			50

/* TILT SENSOR */
#define SENSOR_TILT_LABEL			"LSM303AH Tilt Detector sensor"
#define SENSOR_DATANAME_TILT			"lsm303ah_acc_tilt"
#define TILT_ENABLE_FILE_NAME			"tilt/enable"
#define TILT_POWER_CONSUMPTION			ACCEL_POWER_CONSUMPTION

/* STEP COUNTER SENSOR */
#define SENSOR_STEP_C_LABEL			"LSM303AH Step Counter sensor"
#define SENSOR_DATANAME_STEP_C			"lsm303ah_acc_step_c"
#define STEP_C_ENABLE_FILE_NAME			"step_c/enable"
#define STEP_C_DELAY_FILE_NAME			"step_c/max_delivery_rate"
#define STEP_C_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* STEP DETECTOR SENSOR */
#define SENSOR_STEP_D_LABEL			"LSM303AH Step Detector Sensor"
#define SENSOR_DATANAME_STEP_D			"lsm303ah_acc_step_d"
#define STEP_D_ENABLE_FILE_NAME			"step_d/enable"
#define STEP_D_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* SIGNIFICANT MOTION SENSOR */
#define SENSOR_SIGN_M_LABEL			"LSM303AH Significant Motion Sensor"
#define SENSOR_DATANAME_SIGN_M			"lsm303ah_acc_sign_m"
#define SIGN_M_ENABLE_FILE_NAME			"sign_m/enable"
#define SIGN_M_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* STEP TAP SENSOR */
#define SENSOR_TAP_LABEL			"LSM303AH Tap Sensor"
#define SENSOR_DATANAME_TAP			"lsm303ah_acc_tap"
#define TAP_ENABLE_FILE_NAME			"tap/enable"
#define TAP_POWER_CONSUMPTION			ACCEL_POWER_CONSUMPTION

/* INEMO_ENGINE SENSOR */
#define ACC_DEFAULT_RANGE			8
#define ACC_DEFAULT_DELAY			10

/* SENSOR FUSION */
#define ACC_GBIAS_THRESHOLD			550e-6

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL			EV_MSC
#define EVENT_TYPE_MAG				EV_MSC
#define EVENT_TYPE_TILT				EV_MSC
#define EVENT_TYPE_STEP_C			EV_MSC
#define EVENT_TYPE_STEP_D			EV_MSC
#define EVENT_TYPE_SIGN_M			EV_MSC
#define EVENT_TYPE_TAP				EV_MSC

#define EVENT_TYPE_TIME_MSB			MSC_SCAN
#define EVENT_TYPE_TIME_LSB			MSC_MAX

#define EVENT_TYPE_ACCEL_X			MSC_SERIAL
#define EVENT_TYPE_ACCEL_Y			MSC_PULSELED
#define EVENT_TYPE_ACCEL_Z			MSC_GESTURE

#define EVENT_TYPE_MAG_X			MSC_SERIAL
#define EVENT_TYPE_MAG_Y			MSC_PULSELED
#define EVENT_TYPE_MAG_Z			MSC_GESTURE

#define EVENT_TYPE_TILT_DATA			MSC_SERIAL
#define EVENT_TYPE_STEP_C_DATA			MSC_SERIAL
#define EVENT_TYPE_STEP_D_DATA			MSC_SERIAL
#define EVENT_TYPE_SIGN_M_DATA			MSC_SERIAL
#define EVENT_TYPE_TAP_DATA			MSC_SERIAL

/*****************************************************************************/
/* AXIS MAPPING */
/*****************************************************************************/

/* In this section you must define the axis mapping for individuate one only coordinate system ENU
 *
 * Example:
 *                                                 y'     /| z'
 *                                                  ^   /
 *                                                  |  /       You must define this coordinate system (reference system of board)
 *                                                  | /        in accordance to definition of the axis
 *                                                  |/         definition in sensors.h file
 *   +----------------------------------------------+---------> x'
 *   |          ^ x                                 |
 *   |          |                       ^ z         |
 *   |          |                       |           |
 *   |    +-----+---> y                 |           |
 *   |    | ACC |             <---+-----+           |
 *   |    |     |             x   | GYR |           |
 *   |    +-----+                 |     |           |
 *   |   /                        +-----+           |
 *   | |/       y ^  /| z              /            |
 *   |  z         | /                |/             |
 *   |            |/                   y            |
 *   |      +-----+---> x                           |
 *   |      | MAG |                                 |
 *   |      |     |                                 |
 *   |      +-----+                                 |
 *   |                                        BOARD |
 *   +----------------------------------------------+
 *
 *
 *   ACCELEROMETER:
 *
 *     board        acc     |  0  1  0 |
 *   [x' y' z'] = [x y z] * |  1  0  0 |
 *                          |  0  0 -1 |
 *
*/
static short matrix_acc[3][3] = {
				{ 1, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 1 }
				};

static short matrix_mag[3][3] = {
				{ 1, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 1 }
				};

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A				(GRAVITY_EARTH / 1000000.0f)
#define CONVERT_A_X 				(CONVERT_A)
#define CONVERT_A_Y				(CONVERT_A)
#define CONVERT_A_Z				(CONVERT_A)

#define CONVERT_M				(1.0f/10000.0f)
#define CONVERT_M_X				(CONVERT_M)
#define CONVERT_M_Y				(CONVERT_M)
#define CONVERT_M_Z				(CONVERT_M)

#define ACC_EVENT_HAS_TIMESTAMP			1
#define INPUT_EVENT_HAS_TIMESTAMP		1
#define NOT_SET_MAG_INITIAL_STATE		1
#define MAG_EVENT_HAS_TIMESTAMP			1

#endif	/*	CONFIGURATION_LSM303AH_H	*/
