/*
 * Copyright (C) 2018 STMicroelectronics AMG/MSD Product Div.
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

#ifndef CONFIGURATION_IIS2MDC_H
#define CONFIGURATION_IIS2MDC_H

#define SENSORS_MAGNETIC_FIELD_ENABLE		(1)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE	(1 & SENSORS_MAGNETIC_FIELD_ENABLE & OS_VERSION_ENABLE)

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL			"IIS2MDC 3-axis Magnetometer Sensor"
#define SENSOR_UNCALIB_MAGN_LABEL		"IIS2MDC 3-axis Uncalibrated Magnetometer sensor"
#define SENSOR_DATANAME_MAGNETIC_FIELD		"ST IIS2MDC Magnetometer Sensor"
#define MAGN_DELAY_FILE_NAME			"magn/polling_rate"
#define MAGN_ENABLE_FILE_NAME			"magn/enable"
#define MAGN_RANGE_FILE_NAME			"magn/range"
#define CALIBRATION_ENABLE			1
#define MAGN_MAX_RANGE				1000.0f
#define MAGN_MAX_ODR				100
#define MAGN_MIN_ODR			25
#define MAGN_POWER_CONSUMPTION			0.077f
#define MAGN_DEFAULT_FULLSCALE			50

/* INEMO_ENGINE SENSOR */
#define MAG_DEFAULT_RANGE		8					// full scale set to +-2.5Gauss (value depends on the driver sysfs file)
#define MAG_DEFAULT_DELAY		12					// 1/frequency (default: 10 -> 100 Hz) [ms]

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_MAG				EV_MSC

#define EVENT_TYPE_TIME_MSB			MSC_SCAN
#define EVENT_TYPE_TIME_LSB			MSC_MAX

#define EVENT_TYPE_MAG_X			MSC_SERIAL
#define EVENT_TYPE_MAG_Y			MSC_PULSELED
#define EVENT_TYPE_MAG_Z			MSC_GESTURE

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
static short matrix_mag[3][3] = {
				{ 1, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 1 }
				};

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
#define CONVERT_M				(1.0f/10000.0f)
#define CONVERT_M_X				(CONVERT_M)
#define CONVERT_M_Y				(CONVERT_M)
#define CONVERT_M_Z				(CONVERT_M)

#define INPUT_EVENT_HAS_TIMESTAMP		1
#define NOT_SET_MAG_INITIAL_STATE		1
#define MAG_EVENT_HAS_TIMESTAMP			1

#endif	/*	CONFIGURATION_IIS2MDC_H	*/
