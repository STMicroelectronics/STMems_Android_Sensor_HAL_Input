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


#ifndef CONFIGURATION_VGYRO_H
#define CONFIGURATION_VGYRO_H

#define SENSORS_VIRTUAL_GYROSCOPE_ENABLE	(1 && SENSORS_ACCELEROMETER_ENABLE && SENSORS_MAGNETIC_FIELD_ENABLE && !SENSORS_GYROSCOPE_ENABLE && GEOMAG_FUSION_MODULE_PRESENT)
#define SENSORS_UNCALIB_GYROSCOPE_ENABLE	(0)

#define SENSOR_VIRTUAL_GYRO_LABEL		"STMicroelectronics VGyroscope sensor"
#define VIRTUAL_GYRO_MAX_RANGE			(2000)
#define VIRTUAL_GYRO_POWER_CONSUMPTION		(ACCEL_POWER_CONSUMPTION + MAGN_POWER_CONSUMPTION)
#define VIRTUAL_GYRO_MAX_ODR			100
#define VIRTUAL_GYRO_DEFAULT_FULLSCALE		2000
#define VGYRO_DEFAULT_DELAY			10

/* GYROSCOPE STARTUP */
#define DEFAULT_SAMPLES_TO_DISCARD		(0)
#define GYRO_STARTUP_TIME_MS			(0)

#endif /* CONFIGURATION_VGYRO_H */
