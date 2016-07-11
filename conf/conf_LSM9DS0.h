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

#ifndef CONFIGURATION_LSM9DS0_H
#define CONFIGURATION_LSM9DS0_H

#define SENSORS_ACCELEROMETER_ENABLE		(1)
#define SENSORS_GYROSCOPE_ENABLE		(1)
#define SENSORS_MAGNETIC_FIELD_ENABLE		(1)
#define SENSORS_UNCALIB_GYROSCOPE_ENABLE	(1 & SENSORS_GYROSCOPE_ENABLE & OS_VERSION_ENABLE)
#define SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE	(1 & SENSORS_MAGNETIC_FIELD_ENABLE & OS_VERSION_ENABLE)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL		"LSM9DS0 3-axis Accelerometer Sensor"	// Label views in Android Applications
#define SENSOR_DATANAME_ACCELEROMETER	"lsm9ds0_acc"			// Name of input device: struct input_dev->name
#define ACCEL_DELAY_FILE_NAME		"device/accelerometer/pollrate_ms"		// name of sysfs file for setting the pollrate
#define ACCEL_ENABLE_FILE_NAME		"device/accelerometer/enable_device"		// name of sysfs file for enable/disable the sensor state
#define ACCEL_RANGE_FILE_NAME		"device/accelerometer/full_scale"		// name of sysfs file for setting the full scale
#define ACCEL_MAX_RANGE			16*GRAVITY_EARTH			// Set Max Full-scale [m/s^2]
#define ACCEL_MAX_ODR			100					// Set Max value of ODR [Hz]
#define ACCEL_POWER_CONSUMPTION		0.033f					// Set sensor's power consumption [mA]
#define ACCEL_DEFAULT_FULLSCALE		4					// Set default full-scale (value depends on the driver sysfs file)

/* MAGNETOMETER SENSOR */
#define SENSOR_MAGN_LABEL		"LSM9DS0 3-axis Magnetometer Sensor"	// Label views in Android Applications
#define SENSOR_DATANAME_MAGNETIC_FIELD	"lsm9ds0_mag"			// Name of input device: struct input_dev->name
#define MAGN_DELAY_FILE_NAME		"device/magnetometer/pollrate_ms"		// name of sysfs file for setting the pollrate
#define MAGN_ENABLE_FILE_NAME		"device/magnetometer/enable_device"		// name of sysfs file for enable/disable the sensor state
#define MAGN_RANGE_FILE_NAME		"device/magnetometer/full_scale"		// name of sysfs file for setting the full scale
#define MAGN_MAX_RANGE			1200.0f					// Set Max Full-scale [uT]
#define MAGN_MAX_ODR			100					// Set Max value of ODR [Hz]
#define MAGN_POWER_CONSUMPTION		0.077f					// Set sensor's power consumption [mA]
#define MAGN_DEFAULT_FULLSCALE		8					// Set default full-scale (value depends on the driver sysfs file)

/* GYROSCOPE SENSOR */
#define SENSOR_GYRO_LABEL		"LSM9DS0 3-axis Gyroscope sensor"	// Label views in Android Applications
#define SENSOR_DATANAME_GYROSCOPE	"lsm9ds0_gyr"			// Name of input device: struct input_dev->name
#define GYRO_DELAY_FILE_NAME		"device/pollrate_ms"				// name of sysfs file for setting the pollrate
#define GYRO_ENABLE_FILE_NAME		"device/enable_device"				// name of sysfs file for enable/disable the sensor state
#define GYRO_RANGE_FILE_NAME		"device/range"					// name of sysfs file for setting the full scale
#define GYRO_MAX_RANGE			(2000.0f*(float)M_PI/180.0f)		// Set Max Full-scale [rad/sec]
#define GYRO_MAX_ODR			200					// Set Max value of ODR [Hz]
#define GYRO_POWER_CONSUMPTION		6.1f					// Set sensor's power consumption [mA]
#define GYRO_DEFAULT_FULLSCALE		2000					// Set default full-scale (value depends on the driver sysfs file)

/* INEMO_ENGINE SENSOR */
#define GYRO_DEFAULT_RANGE		2000					// full scale set to 2000 DPS (value depends on the driver sysfs file)
#define ACC_DEFAULT_RANGE		4					// full scale set to 8g (value depends on the driver sysfs file)
#define MAG_DEFAULT_RANGE		8					// full scale set to +-2.5Gauss (value depends on the driver sysfs file)
#define ACC_DEFAULT_DELAY		10					// 1/frequency (default: 10 -> 100 Hz) [ms]
#define MAG_DEFAULT_DELAY		10					// 1/frequency (default: 10 -> 100 Hz) [ms]
#define GYR_DEFAULT_DELAY		10					// 1/frequency (default: 9.523809f -> 105 Hz) [ms]

/* SENSOR FUSION */
#define LOCAL_EARTH_MAGNETIC_FIELD	50.0f					// Value of local earth magnetic field [uT]
#define MAG_GBIAS_THRESHOLD		1471e-6					// Set magnetometer gbias threshold [uT] - Default value for LSM303DLHC: 1200e-6
#define GYR_GBIAS_THRESHOLD		1491e-6					// Set gyroscope gbias threshold [uT] - Default value for L3G4200D: 400e-6
#define ACC_GBIAS_THRESHOLD		765e-6					// Set acceleration bias threshold

/* GYROSCOPE STARTUP */
#define DEFAULT_SAMPLES_TO_DISCARD	(20)
#define GYRO_STARTUP_TIME_MS		(200)

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL		EV_MSC
#define EVENT_TYPE_MAG			EV_MSC
#define EVENT_TYPE_GYRO			EV_MSC

/* Event Type in accelerometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_ACCEL_X		MSC_SERIAL
#define EVENT_TYPE_ACCEL_Y		MSC_PULSELED
#define EVENT_TYPE_ACCEL_Z		MSC_GESTURE

/* Event Type in magnetometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_MAG_X		MSC_SERIAL
#define EVENT_TYPE_MAG_Y		MSC_PULSELED
#define EVENT_TYPE_MAG_Z		MSC_GESTURE

/* Event Type in gyroscope sensor: see input_set_abs_params() function in your input driver */
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
 *   ACCELEROMETER:
 *
 *     board        acc     |  0  1  0 |
 *   [x' y' z'] = [x y z] * |  1  0  0 |
 *                          |  0  0 -1 |
 *
 *   MAGNETOMETER:
 *
 *     board        mag     |  1  0  0 |
 *   [x' y' z'] = [x y z] * |  0  0  1 |
 *                          |  0  0  1 |
 *
 *   GYROSCOPE:
 *
 *     board        gyr     | -1  0  0 |
 *   [x' y' z'] = [x y z] * |  1  0  0 |
 *                          |  0  -1 0 |
 *
*/
static short matrix_acc[3][3] = {
				{ -1, 0, 0 },
				{ 0, -1, 0 },
				{ 0, 0, 1 }
				};

static short matrix_mag[3][3] = {
				{ -1, 0, 0 },
				{ 0, -1, 0 },
				{ 0, 0, -1 }
				};

static short matrix_gyr[3][3] = {
				{ -1, 0, 0 },
				{ 0, -1, 0 },
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
#define CONVERT_M			(0.0001f)		// 1uTESLA = 10 mGauss
#define CONVERT_M_X			(CONVERT_M)
#define CONVERT_M_Y			(CONVERT_M)
#define CONVERT_M_Z			(CONVERT_M)

/**
 * Conversion of gyro data to SI units (radian/sec)
 * FS = 2000dps, sensitivity conversion applied into the driver.
 * Driver output as udps
 */
#define DPS2RAD				((float)M_PI/180.0f)
#define G_SENSITIVITY			(1.0f) //Already applied into the driver
#define CONVERT_GYRO			(DPS2RAD * (G_SENSITIVITY / (1000.0f * 1000.0f)))
#define CONVERT_GYRO_X			(CONVERT_GYRO)
#define CONVERT_GYRO_Y			(CONVERT_GYRO)
#define CONVERT_GYRO_Z			(CONVERT_GYRO)

#endif	/*	CONFIGURATION_LSM9DS0_H	*/
