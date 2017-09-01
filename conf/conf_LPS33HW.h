/*
 * Copyright (C) 2017 STMicroelectronics
 * Lorenzo Bianconi <lorenzo.bianconi@st.com> - Motion MEMS Product Div.
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

#ifndef CONFIGURATION_LPS33HW_H
#define CONFIGURATION_LPS33HW_H

#define SENSORS_PRESSURE_ENABLE		(1)
#define SENSORS_TEMP_PRESS_ENABLE	(1)

/* PRESSURE SENSOR */
#define SENSOR_PRESS_LABEL		"LPS33HW Pressure sensor"		// Label views in Android Applications
#define SENSOR_TEMP_LABEL		"LPS33HW Temperature sensor"		// Label views in Android Applications
#define SENSOR_DATANAME_BAROMETER	"lps33hw"				// Name of input device: struct input_dev->name
#define PRESS_DELAY_FILE_NAME		"device/poll_period_ms"			// name of sysfs file for setting the pollrate
#define PRESS_ENABLE_FILE_NAME		"device/enable_device"			// name of sysfs file for enable/disable the sensor state
#define PRESS_RANGE_FILE_NAME		"device/range"				// name of sysfs file for setting the full scale
#define PRESS_MAX_RANGE			1260					// Set Max Full-scale [hPa]
#define TEMP_MAX_RANGE			80					// Set Max Full-scale [Celsius]
#define TEMPERATURE_OFFSET		0
#define PRESS_MAX_ODR			75					// Set Max value of ODR [Hz]
#define PRESS_MIN_ODR			1					// Set Min value of ODR [Hz]
#define PRESS_POWER_CONSUMPTION		0.015f					// Set sensor's power consumption [mA]
#define TEMP_MAX_ODR			PRESS_MAX_ODR				// Set Max value of ODR [Hz]
#define TEMP_MIN_ODR			PRESS_MIN_ODR				// Set Min value of ODR [Hz]
#define TEMP_POWER_CONSUMPTION		PRESS_POWER_CONSUMPTION			// Set sensor's power consumption [mA]

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/

/* Event Type in pressure sensor: see input_set_abs_params() function in your input driver */
#define EVENT_TYPE_PRESSURE		MSC_SERIAL
#define EVENT_TYPE_TEMPERATURE		MSC_PULSELED

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
// conversion of pressure data to SI units (hPa, 1hPa=1mbar)
#define CONVERT_PRESS			(1.0f/4096.0f)

// conversion of temperature data to SI units
#define CONVERT_TEMP			(1.0f/100.0f)

#endif	/* CONFIGURATION_LPS33HW_H */
