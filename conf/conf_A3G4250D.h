/*
 * Copyright (C) 2014 STMicroelectronics
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


#ifndef CONFIGURATION_A3G4250D_H
#define CONFIGURATION_A3G4250D_H

#define SENSORS_GYROSCOPE_ENABLE		(1)
#define SENSORS_UNCALIB_GYROSCOPE_ENABLE	(1 & SENSORS_GYROSCOPE_ENABLE & OS_VERSION_ENABLE)

/* GYROSCOPE SENSOR */
#define SENSOR_GYRO_LABEL		"A3G4250D 3-axis Gyroscope sensor"	// Label views in Android Applications
#define SENSOR_UNCALIB_GYRO_LABEL	"A3G4250D 3-axis Uncalibrated Gyroscope sensor"
#define SENSOR_DATANAME_GYROSCOPE	"a3g4250d_gyr"				// Name of input device: struct input_dev->name
#define GYRO_DELAY_FILE_NAME		"device/pollrate_ms"				// name of sysfs file for setting the pollrate
#define GYRO_ENABLE_FILE_NAME		"device/enable_device"				// name of sysfs file for enable/disable the sensor state
#define GYRO_RANGE_FILE_NAME		"device/range"					// name of sysfs file for setting the full scale
#define GYRO_MAX_RANGE			(245.0f*(float)M_PI/180.0f)		// Set Max Full-scale [rad/sec]
#define GYRO_MAX_ODR			200					// Set Max value of ODR [Hz]
#define GYRO_POWER_CONSUMPTION		6.1f					// Set sensor's power consumption [mA]
#define GYRO_DEFAULT_FULLSCALE		245					// Set default full-scale (value depends on the driver sysfs file)


/* INEMO_ENGINE SENSOR */
#define GYRO_DEFAULT_RANGE		245					// full scale set to 2000 DPS (value depends on the driver sysfs file)
#define GYR_DEFAULT_DELAY		10					// 1/frequency (default: 9.523809f -> 105 Hz) [ms]

/* SENSOR FUSION */
#define GYROSCOPE_ACTIVE		1					// Enable Gyroscope sensor in Sensor Fusion -> [0]:off, [1]:on
#define GYR_GBIAS_THRESHOLD		475e-6					// Set gyroscope gbias threshold [uT] - Default value for L3G4200D: 400e-6

/* GYROSCOPE STARTUP */
#define DEFAULT_SAMPLES_TO_DISCARD	(20)
#define GYRO_STARTUP_TIME_MS		(200)

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
/* Event Type in gyroscope sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_GYRO		EV_MSC

#define EVENT_TYPE_TIME_MSB		MSC_SCAN
#define EVENT_TYPE_TIME_LSB		MSC_MAX
#define EVENT_TYPE_GYRO_X		MSC_SERIAL
#define EVENT_TYPE_GYRO_Y		MSC_PULSELED
#define EVENT_TYPE_GYRO_Z		MSC_GESTURE

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
 *   GYROSCOPE:
 *
 *     board        gyr     | -1  0  0 |
 *   [x' y' z'] = [x y z] * |  1  0  0 |
 *                          |  0  -1 0 |
 *
*/

static short matrix_gyr[3][3] = {
				{ 1, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 1 }
				};

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of gyro data to SI units (radian/sec)
#define CONVERT_GYRO			((8.75f*((float)M_PI)) / 180000.0f)
#define CONVERT_GYRO_X			(CONVERT_GYRO)
#define CONVERT_GYRO_Y			(CONVERT_GYRO)
#define CONVERT_GYRO_Z			(CONVERT_GYRO)

#define GYRO_EVENT_HAS_TIMESTAMP		1

#endif	/*	CONFIGURATION_A3G4250D_H	*/
