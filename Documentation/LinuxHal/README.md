Index
=====
	* Introduction
	* Configure and Build test_linux
	* Copyright


Introduction
=========

SensorHal Linux Documentation

The **test_linux** application is an example that demonstrates how to reuse the **SensorHAL** library code even in a Linux environment

This test_linux code examples searches the SensorHAL shared library(*) in the following paths of the target board:

>    "/tmp/mnt/SensorHAL.so"
>    "/system/vendor/lib64/hw/SensorHAL.so"
>    "/system/vendor/lib/hw/SensorHAL.so"

Once found the SensorHAL shared library the test_linux example application executes the dlopen (open_hal routine) and looks for the symbol library HAL_MODULE_INFO_SYM_AS_STR (defined as "HMI"); if library is found on the target, test_linux launches the hmi-> common.methods-> open method of the SensorHAL

The SensorHAL open method will search for all configured and supported MEMS sensor devices and add them to the list of managed sensors

At the end of the scanning procedure it will be possible to use the **poll_dev** and **hmi** structure methods to perform all common sensor operations, for examples:

#####  list of scanned ST MEMS sensors:
    sensor_num = hmi->get_sensors_list(hmi, &list);

#####  getting reference to a sensor:
    handle = get_sensor(list, <sensor_type>, &sensor);

#####  acrivate a sensor:
    sensor_activate(handle, SENSOR_ENABLE);

#####  deactivate a sensor:
    sensor_activate(handle, SENSOR_DISABLE);

#####  setting ODR frequency (in Hz):
    static int sensor_setdelay(int type, int odr)
    {
        int handle;
        int64_t delay;
        struct sensor_t *sensor = NULL;

        delay = 1000000000 / odr;
        handle = get_sensor(list, type, &sensor);
        if (handle != INVALID_HANDLE && sensor) {
            return poll_dev->setDelay(&poll_dev->v0, handle, delay);
        }

        return -1;
    }

All the native SensorHAL methods have been implemented in the test_linux application

The test_linux example application has the following menu options:

    usage: ./test_linux [OPTIONS]

    OPTIONS:
        --accodr:       Set Accelerometer ODR (default 104)
        --gyroodr:      Set Gyroscope ODR (default 104)
        --list:         Show sensor list
        --nsample:      Number of samples (default 10)
        --delay:        Set Delay time in ns
        --timeout:      Set timeout (s) for receive samples (default 1 s)
        --sensor:       Select sensor (default all)
        --logname:      Output log file (default log.txt)
        --version:      Print Version
        --help:         This help

NOTE: (*) SensorHAL library must becompiled for linux by using the Makefile provided

Configure and Build test_linux
========

To build the test_linux sample application set CROSS_COMPILE environment accordingly to you target board, follow an examples for raspberry pi zero target:

>   PATH=$PATH:/local/home/raspy/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
>   make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-


Copyright
========

Copyright (C) 2022 STMicroelectronics

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
