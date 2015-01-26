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


#ifndef CONFIGURATION_FUSION_H
#define CONFIGURATION_FUSION_H

#define REAL_9AXIS_AVAILABLE			(SENSORS_ACCELEROMETER_ENABLE && SENSORS_MAGNETIC_FIELD_ENABLE && SENSORS_GYROSCOPE_ENABLE)
#define REAL_6AXIS_AVAILABLE			(SENSORS_ACCELEROMETER_ENABLE && SENSORS_GYROSCOPE_ENABLE && !SENSORS_MAGNETIC_FIELD_ENABLE)
#define VIRTUAL_9AXIS_AVAILABLE			(SENSORS_ACCELEROMETER_ENABLE && SENSORS_MAGNETIC_FIELD_ENABLE && SENSORS_VIRTUAL_GYROSCOPE_ENABLE)
#define SENSORS_ORIENTATION_ENABLE		(1 && (REAL_9AXIS_AVAILABLE || VIRTUAL_9AXIS_AVAILABLE) && SENSOR_FUSION_MODULE_PRESENT)
#define SENSORS_GRAVITY_ENABLE			(1 && (REAL_9AXIS_AVAILABLE || VIRTUAL_9AXIS_AVAILABLE || REAL_6AXIS_AVAILABLE) && SENSOR_FUSION_MODULE_PRESENT)
#define SENSORS_LINEAR_ACCELERATION_ENABLE	(1 && (REAL_9AXIS_AVAILABLE || VIRTUAL_9AXIS_AVAILABLE  || REAL_6AXIS_AVAILABLE) && SENSOR_FUSION_MODULE_PRESENT)
#define SENSORS_ROTATION_VECTOR_ENABLE		(1 && (REAL_9AXIS_AVAILABLE || VIRTUAL_9AXIS_AVAILABLE) && SENSOR_FUSION_MODULE_PRESENT)
#define SENSORS_GAME_ROTATION_ENABLE		(1 && SENSORS_ACCELEROMETER_ENABLE && (SENSORS_GYROSCOPE_ENABLE || SENSORS_VIRTUAL_GYROSCOPE_ENABLE) && OS_VERSION_ENABLE && SENSOR_FUSION_MODULE_PRESENT)
#define SENSOR_FUSION_ENABLE			(SENSOR_FUSION_MODULE_PRESENT && (SENSORS_ORIENTATION_ENABLE || SENSORS_GRAVITY_ENABLE || SENSORS_LINEAR_ACCELERATION_ENABLE || SENSORS_ROTATION_VECTOR_ENABLE || SENSORS_GAME_ROTATION_ENABLE))
#define FUSION_6AXIS				(SENSORS_GYROSCOPE_ENABLE && SENSORS_ACCELEROMETER_ENABLE)
#define FUSION_9AXIS				(FUSION_6AXIS && SENSORS_MAGNETIC_FIELD_ENABLE)


#if (FUSION_6AXIS && !FUSION_9AXIS)
  #define FUSION_CONSUMPTION 			(GYRO_POWER_CONSUMPTION  + ACCEL_POWER_CONSUMPTION)
#else
  #if (FUSION_9AXIS)
    #define FUSION_CONSUMPTION 			(GYRO_POWER_CONSUMPTION + MAGN_POWER_CONSUMPTION + ACCEL_POWER_CONSUMPTION)
  #else
    #define FUSION_CONSUMPTION 			(MAGN_POWER_CONSUMPTION + ACCEL_POWER_CONSUMPTION)
  #endif
#endif

#define ORIENTATION_POWER_CONSUMPTION 		(FUSION_CONSUMPTION)
#define GRAVITY_POWER_CONSUMPTION 		(FUSION_CONSUMPTION)
#define LINEAR_ACCEL_POWER_CONSUMPTION 		(FUSION_CONSUMPTION)
#define ROT_VEC_POWER_CONSUMPTION 		(FUSION_CONSUMPTION)
#define GEOMAG_FREQUENCY			(100)
#define FUSION_MAX_ODR				(100)
#define FUSION_MIN_ODR				(1)
#define ORIENTATION_MAX_ODR			(100)

#endif /* CONFIGURATION_FUSION_H */
