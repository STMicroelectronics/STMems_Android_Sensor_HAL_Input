/*
 * Copyright (C) 2015 STMicroelectronics
 * Armando Visconti - Motion MEMS Product Div.
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

#ifndef CONFIGURATION_SENSOR_LSM303AGR_H
#define CONFIGURATION_SENSOR_LSM303AGR_H

#define SENSORS_ACCELEROMETER_ENABLE		(1)
#define SENSORS_MAGNETIC_FIELD_ENABLE		(1)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE	(1 & SENSORS_MAGNETIC_FIELD_ENABLE \
							& OS_VERSION_ENABLE)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL		"LSM303AGR 3-axis Accelerometer"
#define SENSOR_DATANAME_ACCELEROMETER	"lsm303agr_acc"
#define ACCEL_DELAY_FILE_NAME		"device/pollrate_ms"
#define ACCEL_ENABLE_FILE_NAME		"device/enable_device"
#define ACCEL_RANGE_FILE_NAME		"device/range"
#define ACCEL_MAX_RANGE			8*GRAVITY_EARTH
#define ACCEL_MAX_ODR			100
#define ACCEL_POWER_CONSUMPTION		0.033f
#define ACCEL_DEFAULT_FULLSCALE		4

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL		"LSM303AGR 3-axis Magnetometer Sensor"
#define SENSOR_DATANAME_MAGNETIC_FIELD	"lsm303agr_mag"
#define MAGN_DELAY_FILE_NAME		"device/pollrate_ms"
#define MAGN_ENABLE_FILE_NAME		"device/enable_device"
#define MAGN_RANGE_FILE_NAME		"device/full_scale"
#define CALIBRATION_ENABLE		1
#define MAGN_MAX_RANGE			1600.0f
#define MAGN_MAX_ODR			100
#define MAGN_POWER_CONSUMPTION		0.077f
#define MAGN_DEFAULT_FULLSCALE		50
#define MAGN_MIN_ODR			25

/* INEMO_ENGINE SENSOR */
#define ACC_DEFAULT_RANGE		4
#define MAG_DEFAULT_RANGE		50
#define ACC_DEFAULT_DELAY		100
#define MAG_DEFAULT_DELAY		100

/* SENSOR FUSION */
#define LOCAL_EARTH_MAGNETIC_FIELD	50.0f
#define ACC_GBIAS_THRESHOLD		550e-6
#define MAG_GBIAS_THRESHOLD		1200e-6

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL		EV_MSC
#define EVENT_TYPE_MAG			EV_MSC

/* Event Type in accelerometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_ACCEL_X		MSC_SERIAL
#define EVENT_TYPE_ACCEL_Y		MSC_PULSELED
#define EVENT_TYPE_ACCEL_Z		MSC_GESTURE

/* Event Type in magnetometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_MAG_X		MSC_SERIAL
#define EVENT_TYPE_MAG_Y		MSC_PULSELED
#define EVENT_TYPE_MAG_Z		MSC_GESTURE

#define EVENT_TYPE_TIME_MSB		MSC_SCAN
#define EVENT_TYPE_TIME_LSB		MSC_MAX

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
 *   ACCELEROMETER:
 *
 *     board        acc     |  0  1  0 |
 *   [x' y' z'] = [x y z] * |  1  0  0 |
 *                          |  0  0 -1 |
 *
 *
 *   MAGNETOMETER:
 *
 *     board        mag     |  1  0  0 |
 *   [x' y' z'] = [x y z] * |  0  0  1 |
 *                          |  0  0  1 |
 *
*/

static short matrix_acc[3][3] = {
				{ 1, 0, 0 },
				{ 0, -1, 0 },
				{ 0, 0, 1 }
				};

static short matrix_mag[3][3] = {
				{ 1, 0, 0 },
				{ 0, -1, 0 },
				{ 0, 0, 1 }
				};

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A			(GRAVITY_EARTH / 1000000.0f)
#define CONVERT_A_X 			(CONVERT_A)
#define CONVERT_A_Y			(CONVERT_A)
#define CONVERT_A_Z			(CONVERT_A)

#define CONVERT_M			(1.0f/10000.0f)// 1uTESLA = 10 mGauss
#define CONVERT_M_X			(CONVERT_M)
#define CONVERT_M_Y			(CONVERT_M)
#define CONVERT_M_Z			(CONVERT_M)

#define MAG_EVENT_HAS_TIMESTAMP		1
#define ACC_EVENT_HAS_TIMESTAMP		1
#define NOT_SET_ACC_INITIAL_STATE	1
#define NOT_SET_MAG_INITIAL_STATE	1

#endif /* CONFIGURATION_SENSOR_LSM303AGR_H */
