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

#ifndef CONFIGURATION_SENSOR_LSM303D_H
#define CONFIGURATION_SENSOR_LSM303D_H

#define SENSORS_ACCELEROMETER_ENABLE			(1)
#define SENSORS_MAGNETIC_FIELD_ENABLE			(1)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE		(1 & SENSORS_MAGNETIC_FIELD_ENABLE & OS_VERSION_ENABLE)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL		"LSM303D 3-axis Accelerometer"	// Label views in Android Applications
#define SENSOR_DATANAME_ACCELEROMETER	"lsm303d_acc"				// Name of input device: struct input_dev->name
#define ACCEL_DELAY_FILE_NAME		"device/accelerometer/pollrate_ms"		// name of sysfs file for setting the pollrate
#define ACCEL_ENABLE_FILE_NAME		"device/accelerometer/enable_device"		// name of sysfs file for enable/disable the sensor state
#define ACCEL_RANGE_FILE_NAME		"device/accelerometer/full_scale"		// name of sysfs file for setting the full scale
#define ACCEL_MAX_RANGE			16*GRAVITY_EARTH			// Set Max Full-scale [m/s^2]
#define ACCEL_MAX_ODR			1000					// Set Max value of ODR [Hz]
#define ACCEL_POWER_CONSUMPTION		0.033f					// Set sensor's power consumption [mA]
#define ACCEL_DEFAULT_FULLSCALE		8					// Set default full-scale (value depends on the driver sysfs file)

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL		"LSM303D 3-axis Magnetometer Sensor"	// Label views in Android Applications
#define SENSOR_DATANAME_MAGNETIC_FIELD	"lsm303d_mag"				// Name of input device: struct input_dev->name
#define MAGN_DELAY_FILE_NAME		"device/magnetometer/pollrate_ms"		// name of sysfs file for setting the pollrate
#define MAGN_ENABLE_FILE_NAME		"device/magnetometer/enable_device"		// name of sysfs file for enable/disable the sensor state
#define MAGN_RANGE_FILE_NAME		"device/magnetometer/full_scale"		// name of sysfs file for setting the full scale
#define CALIBRATION_ENABLE		1					// Enable Calibration -> [0]:off, [1]:on
#define MAGN_MAX_RANGE			1200.0f					// Set Max Full-scale [uT]
#define MAGN_MAX_ODR			200					// Set Max value of ODR [Hz]
#define MAGN_POWER_CONSUMPTION		0.077f					// Set sensor's power consumption [mA]
#define MAGN_DEFAULT_FULLSCALE		8					// Set default full-scale (value depends on the driver sysfs file)

/* INEMO_ENGINE SENSOR */
#define ACC_DEFAULT_RANGE		4					// full scale set to 8g (value depends on the driver sysfs file)
#define MAG_DEFAULT_RANGE		8					// full scale set to +-2.5Gauss (value depends on the driver sysfs file)
#define ACC_DEFAULT_DELAY		10					// 1/frequency (default: 10 -> 100 Hz) [ms]
#define MAG_DEFAULT_DELAY		10					// 1/frequency (default: 10 -> 100 Hz) [ms]

/* SENSOR FUSION */
#define LOCAL_EARTH_MAGNETIC_FIELD	50.0f					// Value of local earth magnetic field [uT]
#define ACC_GBIAS_THRESHOLD		550e-6					// Set acceleration gbias threshold [uT] - Default value for LSM303D: 550e-6
#define MAG_GBIAS_THRESHOLD		1200e-6					// Set magnetometer gbias threshold [uT] - Default value for LSM303D: 1200e-6

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL		EV_ABS
#define EVENT_TYPE_MAG			EV_ABS

/* Event Type in accelerometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_ACCEL_X		ABS_X
#define EVENT_TYPE_ACCEL_Y		ABS_Y
#define EVENT_TYPE_ACCEL_Z		ABS_Z

/* Event Type in magnetometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_MAG_X		ABS_X
#define EVENT_TYPE_MAG_Y		ABS_Y
#define EVENT_TYPE_MAG_Z		ABS_Z

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
#define CONVERT_A			(GRAVITY_EARTH/1000000.0f)	// 1000mg = 9.86 m/s^2
#define CONVERT_A_X 			(CONVERT_A)
#define CONVERT_A_Y			(CONVERT_A)
#define CONVERT_A_Z			(CONVERT_A)

// conversion of magnetic data to uT units
#define CONVERT_M			(1.0f/10000.0f)		// 1uTESLA = 10000 uGauss
#define CONVERT_M_X			(CONVERT_M)
#define CONVERT_M_Y			(CONVERT_M)
#define CONVERT_M_Z			(CONVERT_M)

#endif /* CONFIGURATION_SENSOR_LSM303D_H */
