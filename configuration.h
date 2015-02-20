/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Alberto Marinoni, Giuseppe Barba
 * Motion MEMS Product Div.
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


/*
 * Library Version: 2.0.0
 */

#ifndef CONFIGURATION_HAL_H
#define CONFIGURATION_HAL_H

/* ANDROID API VERSION */
#define ANDROID_ICS				(14)
#define ANDROIS_JB				(16)
#define ANDROIS_JBMR2				(18)
#define ANDROIS_KK				(19)
#define ANDROIS_L				(21)

#if (ANDROID_VERSION >= ANDROID_JBMR2)
  #define OS_VERSION_ENABLE			(1)
#else
  #define OS_VERSION_ENABLE			(0)
#endif

#if defined(LSM330D)
  #include "conf_LSM330D.h"
#endif
#if defined(LSM330DLC)
  #include "conf_LSM330DLC.h"
#endif
#if defined(LSM330)
  #include "conf_LSM330.h"
#endif
#if defined(LIS3MDL)
  #include "conf_LIS3MDL.h"
#endif
#if defined(LSM6DS0)
  #include "conf_LSM6DS0.h"
#endif
#if defined(LSM6DS3)
  #include "conf_LSM6DS3.h"
#endif
#if defined(LSM9DS0)
  #include "conf_LSM9DS0.h"
#endif
#if defined(LSM9DS1)
  #include "conf_LSM9DS1.h"
#endif
#if defined(LSM303C)
  #include "conf_LSM303C.h"
#endif
#if defined(LSM303D)
  #include "conf_LSM303D.h"
#endif
#if defined(LSM303DLHC)
  #include "conf_LSM303DLHC.h"
#endif
#if defined(L3GD20H)
  #include "conf_L3GD20H.h"
#endif
#if defined(L3GD20)
  #include "conf_L3GD20.h"
#endif
#if defined(LPS331AP)
  #include "conf_LPS331AP.h"
#endif
#if defined(SENSOR_FUSION)
  #include "conf_FUSION.h"
#else
  #include "conf_COMPASS.h"
#endif
#if defined(V_GYRO)
  #include "conf_VGYRO.h"
#endif
#if defined(GEOMAG_FUSION)
  #include "conf_GEOMAG.h"
#endif
#if defined(GBIAS)
  #include "conf_GBIAS.h"
#endif
#if defined(MAGCALIB)
  #include "conf_MAGCALIB.h"
#endif
#if defined(ACT_RECO)
  #include "conf_ACT_RECO.h"
#endif

#ifdef SENSORS_ORIENTATION_ENABLE
 #if (SENSORS_ORIENTATION_ENABLE ==1)
  #define SENSORS_COMPASS_ORIENTATION_ENABLE (0)
  #define GEOMAG_COMPASS_ORIENTATION_ENABLE (0)
 #endif
#else
 #ifdef GEOMAG_COMPASS_ORIENTATION_ENABLE
  #if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
    #define SENSORS_COMPASS_ORIENTATION_ENABLE (0)
  #endif
 #endif
#endif

#if (SENSORS_GRAVITY_ENABLE == 1)
#define GEOMAG_GRAVITY_ENABLE (0)
#endif

#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
#define GEOMAG_LINEAR_ACCELERATION_ENABLE (0)
#endif

#ifndef SENSORS_GYROSCOPE_ENABLE
  #define SENSORS_GYROSCOPE_ENABLE 		(0)
#endif

#ifndef SENSORS_ACCELEROMETER_ENABLE
  #define SENSORS_ACCELEROMETER_ENABLE 		(0)
#endif

#ifndef SENSORS_MAGNETIC_FIELD_ENABLE
  #define SENSORS_MAGNETIC_FIELD_ENABLE 	(0)
#endif

#ifndef SENSORS_PRESSURE_ENABLE
  #define SENSORS_PRESSURE_ENABLE 		(0)
#endif

/* Sensors power consumption */
#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
  #define UNCALIB_GYRO_POWER_CONSUMPTION 	(GYRO_POWER_CONSUMPTION + ACCEL_POWER_CONSUMPTION)
#else
  #define UNCALIB_GYRO_POWER_CONSUMPTION 	(GYRO_POWER_CONSUMPTION + MAGN_POWER_CONSUMPTION + ACCEL_POWER_CONSUMPTION)
#endif

/* DEBUG INFORMATION */
#define DEBUG_ACCELEROMETER			(0)
#define DEBUG_MAGNETOMETER			(0)
#define DEBUG_GYROSCOPE				(0)
#define DEBUG_INEMO_SENSOR			(0)
#define DEBUG_PRESSURE_SENSOR			(0)
#define DEBUG_CALIBRATION			(0)
#define DEBUG_MAG_SI_COMPENSATION		(0)
#define DEBUG_VIRTUAL_GYROSCOPE			(0)
#define DEBUG_TILT				(0)
#define DEBUG_STEP_C				(0)
#define DEBUG_STEP_D				(0)
#define DEBUG_SIGN_M				(0)
#define DEBUG_POLL_RATE				(0)
#define DEBUG_ACTIVITY_RECO                    (0)

#if (ANDROID_VERSION >= ANDROID_JB)
  #define STLOGI(...)				ALOGI(__VA_ARGS__)
  #define STLOGE(...)				ALOGE(__VA_ARGS__)
  #define STLOGD(...)				ALOGD(__VA_ARGS__)
  #define STLOGD_IF(...)			ALOGD_IF(__VA_ARGS__)
  #define STLOGE_IF(...)			ALOGE_IF(__VA_ARGS__)
#else
  #define STLOGI(...)				LOGI(__VA_ARGS__)
  #define STLOGE(...)				LOGE(__VA_ARGS__)
  #define STLOGD(...)				LOGD(__VA_ARGS__)
  #define STLOGD_IF(...)			LOGD_IF(__VA_ARGS__)
  #define STLOGE_IF(...)			LOGE_IF(__VA_ARGS__)
#endif

#endif	/*	CONFIGURATION_HAL_H	*/
