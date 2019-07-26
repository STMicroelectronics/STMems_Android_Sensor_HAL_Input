Index
=====
	* Introduction
	* Software architecture
	* Integration details
	* STM proprietary libraries
	* More information
	* Copyright


Introduction
=========
The STM Android sensor Hardware Abstraction Layer (*HAL*) defines a standard interface for STM sensors allowing Android to be agnostic about [lower-level driver implementations](https://github.com/STMicroelectronics/STMems_Linux_Input_drivers/tree/linux-3.10.y-gh) . The HAL library is packaged into modules (.so) file and loaded by the Android system at the appropriate time. For more information see [AOSP HAL Interface](https://source.android.com/devices/sensors/hal-interface.html) 

STM Sensor HAL is leaning on [Linux Input framework](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/input) to gather data from sensor device drivers and to forward samples to the Android Framework

Currently supported sensors are:

### Inertial Module Unit (IMU):

> ASM330LXH, LSM330, LSM330DLC, LSM6DS0,  LSM6DS3, LSM6DSM, LSM6DSL, LSM9DS0, LSM9DS1, LSM330D, ASM330LHH, ISM330DLC

### eCompass:

> LSM303AGR, LSM303AH, LSM303C, LSM303D, LSM303DLHC, ISM303DAC

### Accelerometer:

> AIS328DQ, LIS2DE, LIS2DH12, LIS2DS12, LIS2HH12, LIS3DH, LIS2DW12, LIS3DHH, IIS2DH, IIS3DHHC

### Gyroscope:

> A3G4250D, L3GD20, L3GD20H

### Magnetometer:

> LIS3MDL, LIS2MDL, IIS2MDC

### Pressure and Temperature:

> LPS22HB, LPS22HD, LPS25H, LPS33HW, LPS35HW

### Humidity and Temperature:

> HTS221

Software architecture
===============
STM Sensor HAL is written in *C++* language using object-oriented design. For each hw sensor there is a custom class file (*AccelSensor.cpp*, *MagnSensor.cpp*, *GyroSensor.cpp*, *PressSensor.cpp* and *HumiditySensor.cpp*) which extends the common base class (*SensorBase.cpp*).

Configuration parameters for each device (such as the name of the sensor, default full scale, names of the sysfs files, etc.) are specified in a dedicated file placed in the *conf* directory and named *conf_<sensor_name\>.h* (e.g. *conf_LSM6DSM.h*)

	/* ACCELEROMETER SENSOR */
	#define SENSOR_ACC_LABEL				"LSM6DSM 3-axis Accelerometer Sensor"
	#define SENSOR_DATANAME_ACCELEROMETER	"ST LSM6DSM Accelerometer Sensor"
	#define ACCEL_DELAY_FILE_NAME			"accel/polling_rate"
	#define ACCEL_ENABLE_FILE_NAME			"accel/enable"
	#define ACCEL_RANGE_FILE_NAME			"accel/scale"
	#define ACCEL_MAX_RANGE					8 * GRAVITY_EARTH
	#define ACCEL_MAX_ODR					200
	#define ACCEL_MIN_ODR					13
	#define ACCEL_POWER_CONSUMPTION			0.6f
	#define ACCEL_DEFAULT_FULLSCALE			4
	
	/* GYROSCOPE SENSOR */
	#define SENSOR_GYRO_LABEL				"LSM6DSM 3-axis Gyroscope sensor"
	#define SENSOR_DATANAME_GYROSCOPE		"ST LSM6DSM Gyroscope Sensor"
	#define GYRO_DELAY_FILE_NAME			"gyro/polling_rate"
	#define GYRO_ENABLE_FILE_NAME			"gyro/enable"
	#define GYRO_RANGE_FILE_NAME			"gyro/scale"
	#define GYRO_MAX_RANGE					(2000.0f * (float)M_PI/180.0f)
	#define GYRO_MAX_ODR					200
	#define GYRO_MIN_ODR					13
	#define GYRO_POWER_CONSUMPTION			4.0f
	#define GYRO_DEFAULT_FULLSCALE			2000
	
...

	#define EVENT_TYPE_ACCEL			EV_MSC
	#define EVENT_TYPE_GYRO				EV_MSC
	
	#define EVENT_TYPE_TIME_MSB			MSC_SCAN
	#define EVENT_TYPE_TIME_LSB			MSC_MAX
	
	#define EVENT_TYPE_ACCEL_X			MSC_SERIAL
	#define EVENT_TYPE_ACCEL_Y			MSC_PULSELED
	#define EVENT_TYPE_ACCEL_Z			MSC_GESTURE
	
	#define EVENT_TYPE_GYRO_X			MSC_SERIAL
	#define EVENT_TYPE_GYRO_Y			MSC_PULSELED
	#define EVENT_TYPE_GYRO_Z			MSC_GESTURE
	
...
		
	/* In this section the user must define the axis mapping to individuate only one coordinate system ENU
	 *
	 * Example:
	 *                                                 y'     /| z'
	 *                                                  ^   /
	 *                                                  |  / 
	 *                                                  | /
	 *                                                  |/ 
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
	 *   GYROSCOPE:
	 *
	 *     board        gyr     | -1  0  0 |
	 *   [x' y' z'] = [x y z] * |  1  0  0 |
	 *                          |  0  -1 0 |
	 *
	*/
	static short matrix_acc[3][3] = {
					{ 0, 1, 0 },
					{ -1, 0, 0 },
					{ 0, 0, 1 }
					};
	
	static short matrix_gyr[3][3] = {
					{ 0, 1, 0 },
					{ -1, 0, 0 },
					{ 0, 0, 1 }
					};
	
	

Integration details
=============

Copy the HAL source code into *<AOSP_DIR\>/hardware/STMicroelectronics/SensorHAL_Input* folder. During building process Android will include automatically the SensorHAL Android.mk.
In *<AOSP_DIR\>/device/<vendor\>/<board\>/device.mk* add package build information:

	PRODUCT_PACKAGES += sensors.{TARGET_BOARD_PLATFORM}

	Note: device.mk can not read $(TARGET_BOARD_PLATFORM) variable, read and replace the value from your BoardConfig.mk (e.g. PRODUCT_PACKAGES += sensors.msm8974 for Nexus 5)

  The Android.mk file defines the list of all supported devices. To enable a sensor put its name (e.g. *LSM6DSM*) in the *ENABLED_SENSORS* macro:

	ENABLED_SENSORS := LSM6DSM

To compile SensorHAL_Input just build AOSP source code from *$TOP* folder

	$ cd <AOSP_DIR>
	$ source build/envsetup.sh
	$ lunch <select target platform>
	$ make V=99

The compiled library will be placed in *<AOSP_DIR\>/out/target/product/<board\>/system/vendor/lib/hw/sensor.{TARGET_BOARD_PLATFORM}.so*

For more information on compiling an Android project, please consult the [AOSP website](https://source.android.com/source/requirements.html) 


STM proprietary libraries
================

STM proprietary libraries are used to define composite sensors based on hardware (accelerometer, gyroscope, magnetometer) or to provide sensor calibration

### SENSOR_FUSION:
> The STM Sensor Fusion library is a complete 9-axis/6-axis solution which combines the measurements from a 3-axis gyroscope, a 3-axis magnetometer and a 3-axis accelerometer to provide a robust absolute orientation vector and game orientation vector

### GEOMAG_FUSION:
> The STM GeoMag Fusion library is a complete 6-axis solution which combines the measurements from a 3-axis magnetometer and a 3-axis accelerometer to provide a robust geomagnetic orientation vector

### GBIAS:
> The STM Gbias Calibration library provides an efficient gyroscope bias runtime compensation

### MAGCALIB:
> The STM Magnetometer Calibration library provides an accurate magnetometer Hard Iron (HI) and Soft Iron (SI) runtime compensation

The *Android.mk* file enumerates STM libraries supported by the HAL. The *ENABLED_MODULES* variable is used to enable support for proprietary STM libraries

	ENABLED_MODULES := SENSOR_FUSION MAGCALIB GBIAS

The release of STM proprietary libraries is subject to signature of a License User Agreement (LUA); please contact an STMicroelectronics sales office and representatives for further information.


Copyright
========
Copyright (C) 2016 STMicroelectronics

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
