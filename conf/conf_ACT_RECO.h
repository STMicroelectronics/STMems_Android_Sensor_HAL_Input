/*
 * Copyright (C) 2015 STMicroelectronics
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

#ifndef CONFIGURATION_ACTIVITY_RECO_H
#define CONFIGURATION_ACTIVITY_RECO_H

#define SENSORS_ACTIVITY_RECOGNIZER_ENABLE	(1 && ACTIVITY_RECOGNIZER_MODULE_PRESENT && SENSORS_ACCELEROMETER_ENABLE)
#define SENSORS_ACTIVITY_RECOGNIZER_POWER	ACCEL_POWER_CONSUMPTION
#define SENSOR_ACTIVITY_RECOGNIZERO_LABEL	"Activity Recognition"
#define SENSOR_STRING_TYPE_ACTIVITY		"com.st.activity"
#define ACTIVITY_RECOGNIZER_ODR			16

#endif