Important Notice
==================

DISCONTINUED (November 2022): the maintenance for this repository has been discontinued.

Please refer to:

https://github.com/STMicroelectronics/st-mems-android-linux-sensors-hal

for the up-to-date HAL repository.


---

The current repository contains the implementation of Android sensors HAL, which must be combined with the STMicroelectronics suite of input Linux kernel device drivers for MEMS sensors using __input API__ (LDD, https://github.com/STMicroelectronics/st-mems-android-linux-drivers-input/ ). This implementation of Android sensors HAL has become __obsolete__ for use in recent __Android__ systems.

Due to intrinsic limitations of the input framework, the drivers of this family are therefore limited in terms of performance and only the basic hardware embedded features of the MEMS sensors are supported.

The kernel support for sensors and Android has evolved and an improved solution for integrating ST MEMS sensors in Linux kernel now leverages on IIO API.
STMicroelectronics provides full support for MEMS sensors through a complete suite of Linux device drivers (LDD) leveraging on the IIO API/framework made available on GitHub:

https://github.com/STMicroelectronics/st-mems-android-linux-drivers-iio/

For recent Android systems and pure Linux systems, a brand new sensors HAL implementation has been designed accordingly to be combined with the above-mentioned LDD suite for IIO:

https://github.com/STMicroelectronics/st-mems-android-linux-sensors-hal

__We highly encourage and recommend that users upgrade to the suite of IIO drivers and sensors HAL when integrating our sensors in Linux kernel and Android systems. These upgrades are especially important for our most recent devices and for targets expected to run Android distributions.__

The designs for the latest Android versions using the Input HAL and the Input LDD will fail their Android CTS tests for sensors due to tightening requirements impacting high-speed data read and use of buffers/FIFO.

Index
=====
	* Introduction
	* Software architecture
	* Integration details
	* STMicroelectronics proprietary libraries
	* More information
	* Copyright


Introduction
=========
The ST Android sensor Hardware Abstraction Layer (*HAL*) defines a standard interface for ST sensors allowing Android to be agnostic about [lower-level driver implementations](https://github.com/STMicroelectronics/STMems_Linux_Input_drivers/tree/linux-3.10.y-gh) . The HAL library is packaged into modules (.so) file and loaded by the Android system at the appropriate time. For more information see [AOSP HAL Interface](https://source.android.com/devices/sensors/hal-interface.html)

ST sensors HAL is leaning on [Linux Input framework](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/input) to gather data from sensor device drivers and to forward samples to the Android Framework

Currently supported sensors are:

### Inertial Module Unit (IMU):

> ASM330LXH, LSM330, LSM330DLC, LSM6DS0,  LSM6DS3, LSM6DSM, LSM6DSL, LSM9DS0, LSM9DS1, LSM330D, ASM330LHH, ISM330DLC,
> LSM6DSO, LSM6DSOX, LSM6DSO32, LSM6DSO32X

### eCompass:

> LSM303AGR, LSM303AH, LSM303C, LSM303D, LSM303DLHC, ISM303DAC

### Accelerometer:

> AIS328DQ, LIS2DE, LIS2DH12, LIS2DS12, LIS2HH12, LIS3DH, LIS2DW12, LIS3DHH, IIS2DH, IIS3DHHC

### Gyroscope:

> A3G4250D, L3GD20, L3GD20H

### Magnetometer:

> LIS3MDL, LIS2MDL, IIS2MDC

### Pressure and Temperature:

> LPS22HB, LPS22HD, LPS25H, LPS33HW, LPS35HW, LPS22HH

### Humidity and Temperature:

> HTS221

Software architecture
===============
ST sensor HAL is written in *C++* language using object-oriented design. For each hw sensor there is a custom class file (*AccelSensor.cpp*, *MagnSensor.cpp*, *GyroSensor.cpp*, *PressSensor.cpp* and *HumiditySensor.cpp*) which extends the common base class (*SensorBase.cpp*).

Configuration parameters for each device (such as the name of the sensor, default full scale, names of the sysfs files, etc.) are specified in a dedicated file placed in the *conf* directory and named *conf_<sensor_name\>.h* (e.g. *conf_LSM6DSM.h*)

	/* ACCELEROMETER SENSOR */
	#define SENSOR_ACC_LABEL			"LSM6DSM 3-axis Accelerometer Sensor"
	#define SENSOR_DATANAME_ACCELEROMETER		"ST LSM6DSM Accelerometer Sensor"
	#define ACCEL_DELAY_FILE_NAME			"accel/polling_rate"
	#define ACCEL_ENABLE_FILE_NAME			"accel/enable"
	#define ACCEL_RANGE_FILE_NAME			"accel/scale"
	#define ACCEL_MAX_RANGE				8 * GRAVITY_EARTH
	#define ACCEL_MAX_ODR				200
	#define ACCEL_MIN_ODR				13
	#define ACCEL_POWER_CONSUMPTION			0.6f
	#define ACCEL_DEFAULT_FULLSCALE			4
	
	/* GYROSCOPE SENSOR */
	#define SENSOR_GYRO_LABEL			"LSM6DSM 3-axis Gyroscope sensor"
	#define SENSOR_DATANAME_GYROSCOPE		"ST LSM6DSM Gyroscope Sensor"
	#define GYRO_DELAY_FILE_NAME			"gyro/polling_rate"
	#define GYRO_ENABLE_FILE_NAME			"gyro/enable"
	#define GYRO_RANGE_FILE_NAME			"gyro/scale"
	#define GYRO_MAX_RANGE				(2000.0f * (float)M_PI/180.0f)
	#define GYRO_MAX_ODR				200
	#define GYRO_MIN_ODR				13
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

### Android

Copy the HAL source code into *<AOSP_DIR\>/hardware/STMicroelectronics/SensorHAL_Input* folder. During building process Android will include automatically the sensors HAL Android.mk.
In *<AOSP_DIR\>/device/<vendor\>/<board\>/device.mk* add package build information:

	PRODUCT_PACKAGES += sensors.{TARGET_BOARD_PLATFORM}

	Note: device.mk can not read $(TARGET_BOARD_PLATFORM) variable, read and replace the value from your BoardConfig.mk (e.g. PRODUCT_PACKAGES += sensors.msm8974 for Nexus 5)

  The Android.mk file defines the list of all supported devices. To enable a sensor put its name (e.g. *LSM6DSM*) in the *ENABLED_SENSORS* macro:

	ENABLED_SENSORS := LSM6DSM

In *<AOSP_DIR\>/device/<vendor\>/<board\>/ueventd.rc* add rules to access to input devices and sysfs:

	# Common input char devices
	/dev/input* 0666 system system

	# Input device (for pressure sensor)
	/sys/class/input/input* device/poll_period_ms 0666 system system
	/sys/class/input/input* device/enable_device 0666 system system

	# Input device (for imu sensor)
	/sys/class/input/input* accel/scale 0666 system system
	/sys/class/input/input* accel/polling_rate 0666 system system
	/sys/class/input/input* accel/enable 0666 system system
	/sys/class/input/input* accel/scale_avail 0666 system system
	/sys/class/input/input* accel/sampling_freq_avail 0666 system system
	/sys/class/input/input* accel/sampling_freq 0666 system system
	/sys/class/input/input* gyro/scale 0666 system system
	/sys/class/input/input* gyro/polling_rate 0666 system system
	/sys/class/input/input* gyro/enable 0666 system system
	/sys/class/input/input* gyro/scale_avail 0666 system system
	/sys/class/input/input* gyro/sampling_freq_avail 0666 system system
	/sys/class/input/input* gyro/sampling_freq 0666 system system
	/sys/class/input/input* temp/scale 0666 system system
	/sys/class/input/input* temp/polling_rate 0666 system system
	/sys/class/input/input* temp/enable 0666 system system
	/sys/class/input/input* temp/scale_avail 0666 system system
	/sys/class/input/input* temp/sampling_freq_avail 0666 system system
	/sys/class/input/input* temp/sampling_freq 0666 system system

To compile sensors HAL input just build AOSP source code from *$TOP* folder

	$ cd <AOSP_DIR>
	$ source build/envsetup.sh
	$ lunch <select target platform>
	$ make V=99

The compiled library will be placed in *<AOSP_DIR\>/out/target/product/<board\>/system/vendor/lib/hw/sensor.{TARGET_BOARD_PLATFORM}.so*

For more information on compiling an Android project, please consult the [AOSP website](https://source.android.com/source/requirements.html) 

### Linux

To build the sensors HAL for Linux only set CROSS_COMPILE environment accordingly to you target board, follow an examples for raspberry pi zero target:

>   PATH=$PATH:/local/home/raspy/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
>   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

The compiled library will be placed in local sensors HAL source code path.

STMicroelectronics proprietary libraries
================

ST proprietary libraries are used to define composite sensors based on hardware (accelerometer, gyroscope, magnetometer) or to provide sensor calibration

### SENSOR_FUSION:
> The ST Sensor Fusion library is a complete 9-axis/6-axis solution which combines the measurements from a 3-axis gyroscope, a 3-axis magnetometer and a 3-axis accelerometer to provide a robust absolute orientation vector and game orientation vector

### GEOMAG_FUSION:
> The ST GeoMag Fusion library is a complete 6-axis solution which combines the measurements from a 3-axis magnetometer and a 3-axis accelerometer to provide a robust geomagnetic orientation vector

### GBIAS:
> The ST Gbias Calibration library provides an efficient gyroscope bias runtime compensation

### MAGCALIB:
> The ST Magnetometer Calibration library provides an accurate magnetometer Hard Iron (HI) and Soft Iron (SI) runtime compensation

The *Android.mk* file enumerates ST libraries supported by the HAL. The *ENABLED_MODULES* variable is used to enable support for proprietary ST libraries

	ENABLED_MODULES := SENSOR_FUSION MAGCALIB GBIAS

The release of ST proprietary libraries is subject to signature of a License User Agreement (LUA); please contact an STMicroelectronics sales office and representatives for further information.


Copyright
========
Copyright (C) 2016-2020 STMicroelectronics

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
