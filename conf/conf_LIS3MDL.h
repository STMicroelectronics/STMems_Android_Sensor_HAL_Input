/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Giuseppe Barba - Motion MEMS Product Div.
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

#ifndef CONFIGURATION_LIS3MDL_H
#define CONFIGURATION_LIS3MDL_H

#define SENSORS_MAGNETIC_FIELD_ENABLE			(1)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE		(1 & SENSORS_MAGNETIC_FIELD_ENABLE & OS_VERSION_ENABLE)

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL		"LIS3MDL 3-axis Magnetometer Sensor"	// Label views in Android Applications
#define SENSOR_UNCALIB_MAGN_LABEL	"LIS3MDL 3-axis Uncalibrated Magnetometer sensor"
#define SENSOR_DATANAME_MAGNETIC_FIELD	"lis3mdl_mag"				// Name of input device: struct input_dev->name
#define MAGN_DELAY_FILE_NAME		"device/pollrate_ms"			// name of sysfs file for setting the pollrate
#define MAGN_ENABLE_FILE_NAME		"device/enable_device"			// name of sysfs file for enable/disable the sensor state
#define MAGN_RANGE_FILE_NAME		"device/full_scale"			// name of sysfs file for setting the full scale
#define CALIBRATION_ENABLE		1					// Enable Calibration -> [0]:off, [1]:on
#define MAGN_MAX_RANGE			1000.0f					// Set Max Full-scale [uT]
#define MAGN_MAX_ODR			80					// Set Max value of ODR [Hz]
#define MAGN_POWER_CONSUMPTION		0.077f					// Set sensor's power consumption [mA]
#define MAGN_DEFAULT_FULLSCALE		8					// Set default full-scale (value depends on the driver sysfs file)

/* INEMO_ENGINE SENSOR */
#define MAG_DEFAULT_RANGE		8					// full scale set to +-2.5Gauss (value depends on the driver sysfs file)
#define MAG_DEFAULT_DELAY		12					// 1/frequency (default: 10 -> 100 Hz) [ms]

/* SENSOR FUSION */
#define LOCAL_EARTH_MAGNETIC_FIELD	50.0f					// Value of local earth magnetic field [uT]
#define MAGNETOMETER_ACTIVE		1					// Enable Magnetometer sensor in Sensor Fusion -> [0]:off, [1]:on
#define MAG_GBIAS_THRESHOLD		1471e-6					// Set magnetometer gbias threshold [uT] - Default value for LSM303DLHC: 1200e-6

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
/* Event Type in magnetometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_MAG			EV_MSC

#define EVENT_TYPE_MAG_X		MSC_SERIAL
#define EVENT_TYPE_MAG_Y		MSC_PULSELED
#define EVENT_TYPE_MAG_Z		MSC_GESTURE
#define EVENT_TYPE_TIME_MSB		MSC_SCAN
#define EVENT_TYPE_TIME_LSB		MSC_MAX
#define MAG_EVENT_HAS_TIMESTAMP		1
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
 *   MAGNETOMETER:
 *
 *     board        mag     |  1  0  0 |
 *   [x' y' z'] = [x y z] * |  0  0  1 |
 *                          |  0  0  1 |
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
// conversion of magnetic data to uT units
#define CONVERT_M			(1.0f/10000.0f)
#define CONVERT_M_X			(CONVERT_M)
#define CONVERT_M_Y			(CONVERT_M)
#define CONVERT_M_Z			(CONVERT_M)



#endif	/*	CONFIGURATION_LIS3MDL_H	*/
