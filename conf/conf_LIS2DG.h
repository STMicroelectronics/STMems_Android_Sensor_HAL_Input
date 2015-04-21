/*
 * Copyright (C) 2014 STMicroelectronics
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

#ifndef CONFIGURATION_LIS2DG_H
#define CONFIGURATION_LIS2DG_H

#define SENSORS_ACCELEROMETER_ENABLE		(1)

#define SENSORS_TILT_ENABLE			(1)
#define SENSORS_STEP_COUNTER_ENABLE		(1)
#define SENSORS_STEP_DETECTOR_ENABLE		(1)
#define SENSORS_SIGN_MOTION_ENABLE		(1)
#define SENSORS_TAP_ENABLE			(1)

/* ACCELEROMETER SENSOR */
#define SENSOR_ACC_LABEL			"LIS2DG 3-axis Accelerometer Sensor"	// Label views in Android Applications
#define SENSOR_ACC_INCLUDE_FILE_NAME		"lis2dg.h"								// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_ACCELEROMETER		"ST LIS2DG Accelerometer Sensor"				// Name of input device: struct input_dev->name
#define ACCEL_DELAY_FILE_NAME			"accel/polling_rate"						// name of sysfs file for setting the pollrate
#define ACCEL_ENABLE_FILE_NAME			"accel/enable"							// name of sysfs file for enable/disable the sensor state
#define ACCEL_RANGE_FILE_NAME			"accel/range"							// name of sysfs file for setting the full scale
#define ACCEL_MAX_RANGE				8 * GRAVITY_EARTH									// Set Max Full-scale [m/s^2]
#define ACCEL_MAX_ODR				200					// Set Max value of ODR [Hz]
#define ACCEL_POWER_CONSUMPTION			0.6f				// Set sensor's power consumption [mA]
#define ACCEL_DEFAULT_FULLSCALE			4					// Set default full-scale (value depends on the driver sysfs file)

/* TILT SENSOR */
#define SENSOR_TILT_LABEL			"LIS2DG Tilt Detector sensor"	// Label views in Android Applications
#define SENSOR_TILT_INCLUDE_FILE_NAME		"lis2dg.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_TILT			"ST LIS2DG Tilt Sensor"		// Name of input device: struct input_dev->name
#define TILT_ENABLE_FILE_NAME			"tilt/enable"		// name of sysfs file for enable/disable the sensor state
#define TILT_POWER_CONSUMPTION			ACCEL_POWER_CONSUMPTION

/* STEP COUNTER SENSOR */
#define SENSOR_STEP_C_LABEL			"LIS2DG Step Counter sensor"	// Label views in Android Applications
#define SENSOR_STEP_C_INCLUDE_FILE_NAME		"lis2dg.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_STEP_C			"ST LIS2DG Step Counter Sensor"		// Name of input device: struct input_dev->name
#define STEP_C_ENABLE_FILE_NAME			"step_c/enable"		// name of sysfs file for enable/disable the sensor state
#define STEP_C_DELAY_FILE_NAME			"step_c/max_delivery_rate"
#define STEP_C_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* STEP DETECTOR SENSOR */
#define SENSOR_STEP_D_LABEL			"LIS2DG Step Detector Sensor"	// Label views in Android Applications
#define SENSOR_STEP_D_INCLUDE_FILE_NAME		"lis2dg.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_STEP_D			"ST LIS2DG Step Detector Sensor"		// Name of input device: struct input_dev->name
#define STEP_D_ENABLE_FILE_NAME			"step_d/enable"		// name of sysfs file for enable/disable the sensor state
#define STEP_D_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* SIGNIFICANT MOTION SENSOR */
#define SENSOR_SIGN_M_LABEL			"LIS2DG Significant Motion Sensor"	// Label views in Android Applications
#define SENSOR_SIGN_M_INCLUDE_FILE_NAME		"lis2dg.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_SIGN_M			"ST LIS2DG Significant Motion Sensor"		// Name of input device: struct input_dev->name
#define SIGN_M_ENABLE_FILE_NAME			"sign_m/enable"		// name of sysfs file for enable/disable the sensor state
#define SIGN_M_POWER_CONSUMPTION		ACCEL_POWER_CONSUMPTION

/* STEP TAP SENSOR */
#define SENSOR_TAP_LABEL			"LIS2DG Tap Sensor"	// Label views in Android Applications
#define SENSOR_TAP_INCLUDE_FILE_NAME		"lis2dg.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_TAP			"ST LIS2DG Step Detector Sensor"		// Name of input device: struct input_dev->name
#define TAP_ENABLE_FILE_NAME			"tap/enable"		// name of sysfs file for enable/disable the sensor state
#define TAP_POWER_CONSUMPTION			ACCEL_POWER_CONSUMPTION

/* INEMO_ENGINE SENSOR */
#define ACC_DEFAULT_RANGE			8					// full scale set to 8g (value depends on the driver sysfs file)
#define ACC_DEFAULT_DELAY			10					// 1/frequency (default: 10 -> 100 Hz) [ms]

/* SENSOR FUSION */
#define ACC_GBIAS_THRESHOLD			550e-6					// Set acceleration gbias threshold

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_ACCEL			EV_MSC
#define EVENT_TYPE_TILT				EV_MSC
#define EVENT_TYPE_STEP_C			EV_MSC
#define EVENT_TYPE_STEP_D			EV_MSC
#define EVENT_TYPE_SIGN_M			EV_MSC
#define EVENT_TYPE_TAP				EV_MSC

#define EVENT_TYPE_TIME_MSB			MSC_SCAN
#define EVENT_TYPE_TIME_LSB			MSC_MAX

/* Event Type in accelerometer sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_ACCEL_X			MSC_SERIAL
#define EVENT_TYPE_ACCEL_Y			MSC_PULSELED
#define EVENT_TYPE_ACCEL_Z			MSC_GESTURE

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

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of acceleration data to SI units (m/s^2)
#define CONVERT_A				(GRAVITY_EARTH / 1000000.0f)
#define CONVERT_A_X 				(CONVERT_A)
#define CONVERT_A_Y				(CONVERT_A)
#define CONVERT_A_Z				(CONVERT_A)

#define ACC_EVENT_HAS_TIMESTAMP			1
#define INPUT_EVENT_HAS_TIMESTAMP		1
#define NOT_SET_ACC_INITIAL_STATE		1
#endif	/*	CONFIGURATION_LIS2DG_H	*/
