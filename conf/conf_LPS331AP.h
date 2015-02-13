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

#ifndef CONFIGURATION_LPS331AP_H
#define CONFIGURATION_LPS331AP_H

#define SENSORS_PRESSURE_ENABLE		(1)
#define SENSORS_TEMPERATURE_ENABLE	(0 & SENSORS_PRESSURE_ENABLE)

/* PRESSURE SENSOR */
#define SENSOR_PRESS_LABEL		"LPS331AP Pressure sensor"		// Label views in Android Applications
#define SENSOR_TEMP_LABEL		"LPS331AP Temperature sensor"		// Label views in Android Applications
#define SENSOR_PRESS_INCLUDE_FILE_NAME	"lps331ap.h"				// Name of driver header file (include subfolder)
#define SENSOR_DATANAME_BAROMETER	LPS331AP_PRS_DEV_NAME			// Name of input device: struct input_dev->name
#define PRESS_DELAY_FILE_NAME		"device/poll_period_ms"			// name of sysfs file for setting the pollrate
#define PRESS_ENABLE_FILE_NAME		"device/enable_device"				// name of sysfs file for enable/disable the sensor state
#define PRESS_RANGE_FILE_NAME		"device/range"					// name of sysfs file for setting the full scale
#define PRESS_COMPENSATION_ENABLE	(1 & SENSORS_TEMPERATURE_ENABLE)	// Enable Pressure Compensation -> [0]:off, [1]:on
#define PRESS_MAX_RANGE			1260					// Set Max Full-scale [hPa]
#define TEMP_MAX_RANGE			80					// Set Max Full-scale [Celsius]
#define PRESS_TEMP_MAX_ODR		25					// Set Max value of ODR [Hz]
#define PRESS_TEMP_POWER_CONSUMPTION	0.015f					// Set sensor's power consumption [mA]

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/

/* Event Type in pressure sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_PRESSURE		ABS_PR

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of pressure data to SI units (hPa, 1hPa=1mbar)
#define CONVERT_PRESS			(1.0f/4096.0f)		/* sensit. = 4096 LSB/mbar */


// conversion of temperature data to SI units (radian/sec)
#define CONVERT_TEMP			(1.0f/480.0f)		/* sensit. = 480 LSB/degrC */

#endif	/*	CONFIGURATION_LPS331AP_H	*/
