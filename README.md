Index
=====
	* Introduction
	* Software architecture
	* Integration details
	* STM propertary libraries
	* More information
	* Copyright


Introduction
=========
The STM Android sensor Hardware Abstraction Layer (*HAL*) defines a standard interface for STM sensors allowing Android to be agnostic about lower-level driver implementations (XXX link github). HAL library is packaged into modules (.so) file and loaded by the Android system at the appropriate time. For more information see [AOSP HAL Interface](https://source.android.com/devices/sensors/hal-interface.html) 

STM Sensor HAL is leaning on [Linux Input framework](https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/input) to gather data from sensor device drivers and forwards samples to Android Framework

Currently supported sensors are:

### Inertial Module Unit (IMU):

> asm330lxh, lsm330, lsm330dlc, lsm6ds0,  lsm6ds3, lsm6dsm, lsm9ds0, lsm9ds1, lsm330d

### ECompass:

> lsm303agr, lsm303ah, lsm303c, lsm303d, lsm303dlhc, lsm303a

### Accelerometer:

> ais328dq, lis2dg, lis2dh12, lis2ds12, lis2hh, lis2hh12, lis3dh

### Gyroscope:

> a3g4250d, l3gd20, l3gd20h

### Magnetometer:

> lis3mdl

### Pressure and Temperature:

> lps22hb, lps25h, lps331ap


Software architecture
===============
STM Sensor HAL is written in *C++* language using the object-oriented approach, for each hw sensor there is a custom class file (*AccelSensor.cpp*, *MagnSensor.cpp*, *GyroSensor.cpp* and *PressTempSensor.cpp*) which extends the common base class (*SensorBase.cpp*).

Configuration parameters for each device (such as the name of the sensor, default full scale, names of the sysfs files, etc.) are specified in a dedicated file placed in *conf* directory and named *conf_<sensor_name\>.h* (e.g. *conf_LSM6DSM.h*)


Integration details
=============

The Android.mk file defines the list of all supported devices. To enable a sensor put its name (e.g. *LSM6DSM*) in the *ENABLED_SENSORS* macro:

	ENABLED_SENSORS := LSM6DSM

Before to build the code Android environment must be configured using Android Open Source Project (AOSP) toolchain:

    $ cd <ANDROID_ROOT>
    $ source build/envsetup.sh
    $ lunch <select target platform>

From now on Android utilities can be used to compile STM Sensor HAL

    $ export TOP=$(pwd)
    $ cd <ST_SENSOR_HAL>
    $ mm -B
    
For more information how to compile Android project please refer to [AOSP website](https://source.android.com/source/requirements.html) 


STM propertary libraries
================

STM propertary libraries are used to define composite sensor based on hardware ones (accelerometer, gyroscope, magnetometer) or to provide sensor calibration

### SENSOR_FUSION:
> STM Sensor Fusion library is a complete 9-axis/6-axis solution which combines the measurements from 3-axis gyroscope, 3-axis magnetometer and 3-axis accelerometer to provide a robust absolute orientation vector and game orientation vector

### GEOMAG_FUSION:
>  STM GeoMag Fusion library is a complete 6-axis solution which combines the measurements from 3-axis magnetometer and 3-axis accelerometer to provide a robust geomagnetic orientation vector

### GBIAS:
> STM Gbias Calibration library provides an efficient gyroscope bias runtime compensation

### MAGCALIB:
> STM Magnetometer Calibration library provides an accurate magnetometer Hard Iron (HI) and Soft Iron (SI) runtime compensation

*ENABLED_MODULES* variable is used to enable support for proprietary STM libraries

	ENABLED_MODULES := SENSOR_FUSION MAGCALIB GBIAS

STM propertary libraries are available under License User Agreement (LUA)


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