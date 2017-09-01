/*
 * Copyright (C) 2017 STMicroelectronics
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

#ifndef CONFIGURATION_LIS3DHH_H
#define CONFIGURATION_LIS3DHH_H

#define SENSORS_ACCELEROMETER_ENABLE	(1)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL		"LIS3DHH 3-axis Accelerometer"	// Label views in Android Applications
#define SENSOR_DATANAME_ACCELEROMETER	"lis3dhh"			// Name of input device: struct input_dev->name
#define ACCEL_DELAY_FILE_NAME		"device/pollrate_ms"		// name of sysfs file for setting the pollrate
#define ACCEL_ENABLE_FILE_NAME		"device/enable_device"		// name of sysfs file for enable/disable the sensor state
#define ACCEL_RANGE_FILE_NAME		"device/range"			// name of sysfs file for setting the full scale
#define ACCEL_MAX_RANGE			2.5*GRAVITY_EARTH		// Set Max Full-scale [m/s^2]
#define ACCEL_MAX_ODR			1100				// Set Max value of ODR [Hz] - driver uses delayed works (min tick 1 jiffies)
#define ACCEL_POWER_CONSUMPTION		0.033f				// Set sensor's power consumption [mA]
#define ACCEL_DEFAULT_FULLSCALE		2.5				// Set default full-scale (value depends on the driver sysfs file)
#define ACC_DEFAULT_DELAY		0.9				// 1/frequency (default: 10 -> 100 Hz) [ms]
/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL		EV_MSC

/* Event Type in accelerometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_ACCEL_X		MSC_SERIAL
#define EVENT_TYPE_ACCEL_Y		MSC_PULSELED
#define EVENT_TYPE_ACCEL_Z		MSC_GESTURE

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
 */

static short matrix_acc[3][3] = {
	{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 }
};

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A			(GRAVITY_EARTH/1000000.0f)	// 1000000ug = 9.86 m/s^2
#define CONVERT_A_X 			(CONVERT_A)
#define CONVERT_A_Y			(CONVERT_A)
#define CONVERT_A_Z			(CONVERT_A)

#define ACC_EVENT_HAS_TIMESTAMP		1

#endif	/*	CONFIGURATION_LIS3DHH_H	*/
