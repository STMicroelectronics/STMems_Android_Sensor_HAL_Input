/*
 * Copyright (C) 2016 STMicroelectronics
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

#ifndef CONFIGURATION_HTS221_H
#define CONFIGURATION_HTS221_H

#define SENSORS_HUMIDITY_ENABLE		(0)
#define SENSORS_TEMP_RH_ENABLE		(1)

#define SENSOR_HUMIDITY_LABEL		"HTS221 Humidity sensor"		// Label views in Android Applications
#define SENSOR_TEMP_LABEL		"HTS221 Temperature sensor"		// Label views in Android Applications
#define SENSOR_DATANAME_HUMIDITY	"hts221"				// Name of input device: struct input_dev->name
#define HUMIDITY_DELAY_FILE_NAME	"device/poll_ms"			// name of sysfs file for setting the pollrate
#define HUMIDITY_ENABLE_FILE_NAME	"device/enable_device"			// name of sysfs file for enable/disable the sensor state
#define HUMIDITY_DELAY_FILE_NAME	"device/poll_ms"			// name of sysfs file for setting the pollrate
#define HUMIDITY_MAX_RANGE		80					// Set Max Full-scale [Celsius]
#define TEMP_MAX_RANGE			80					// Set Max Full-scale [Celsius]
#define HUMIDITY_MAX_ODR		13					// Set Max value of ODR [Hz]
#define HUMIDITY_MIN_ODR		1					// Set Min value of ODR [Hz]
#define HUMIDITY_POWER_CONSUMPTION	0.015f					// Set sensor's power consumption [mA]
#define TEMP_MAX_ODR			HUMIDITY_MAX_ODR			// Set Max value of ODR [Hz]
#define TEMP_MIN_ODR			HUMIDITY_MIN_ODR			// Set Min value of ODR [Hz]
#define TEMP_POWER_CONSUMPTION		HUMIDITY_POWER_CONSUMPTION		// Set sensor's power consumption [mA]

/*****************************************************************************/
/* EVENT TYPE */
/*****************************************************************************/
#define EVENT_TYPE_HUMIDITY		MSC_PULSELED
#define EVENT_TYPE_TEMPERATURE		MSC_SERIAL

/*****************************************************************************/
/* DATA CONVERSION */
/*****************************************************************************/
#define CONVERT_RH			(1.0f/1000.0f)
#define CONVERT_TEMP			(1.0f/1000.0f)

#endif	/* CONFIGURATION_HTS221_H */
