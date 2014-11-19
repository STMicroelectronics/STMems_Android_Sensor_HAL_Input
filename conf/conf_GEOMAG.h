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


#ifndef CONFIGURATION_GEOMAG_H
#define CONFIGURATION_GEOMAG_H

#define SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE	(1 & GEOMAG_FUSION_MODULE_PRESENT & SENSORS_MAGNETIC_FIELD_ENABLE && SENSORS_ACCELEROMETER_ENABLE && OS_VERSION_ENABLE)
#define GEOMAG_COMPASS_ORIENTATION_ENABLE	(0 & GEOMAG_FUSION_MODULE_PRESENT & SENSORS_MAGNETIC_FIELD_ENABLE & SENSORS_ACCELEROMETER_ENABLE)
#define GEOMAG_LINEAR_ACCELERATION_ENABLE	(0 & GEOMAG_FUSION_MODULE_PRESENT & SENSORS_MAGNETIC_FIELD_ENABLE & SENSORS_ACCELEROMETER_ENABLE)
#define GEOMAG_GRAVITY_ENABLE				(0 & GEOMAG_FUSION_MODULE_PRESENT & SENSORS_MAGNETIC_FIELD_ENABLE & SENSORS_ACCELEROMETER_ENABLE)
#define SENSOR_GEOMAG_ENABLE				(SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE || GEOMAG_COMPASS_ORIENTATION_ENABLE || GEOMAG_LINEAR_ACCELERATION_ENABLE || GEOMAG_GRAVITY_ENABLE)

#define GEOMAG_FREQUENCY			(100)

#endif /* CONFIGURATION_GEOMAG_H */
