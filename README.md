STMicroelectronics Sensor HAL - Input Framework

This HAL support multiple kind of STMicroelectronics MEMS sensors.
The Android.mk file has the list of supported sensors that is updated
every time a new sensor support is added.
To enable a sensor put its name (e.g. LSM6DS3) in the ENABLED_SENSORS, e.g.:

    ENABLED_SENSORS := LSM6DS3  LIS3MDL

The ENABLED_MODULES variable is used to enable support for proprietary STM
libraries like ones listed into Android.mk file.

To build the code, first of all it needs to load Android environment:

    cd <ANDROID_ROOT>
    <ANDROID_ROOT>/ source build/envsetup
    <ANDROID_ROOT>/ lunch #select target platform
    <ANDROID_ROOT>/ export TOP=`pwd`
    <ANDROID_ROOT>/ cd <ST_SENSOR_HAL>
    <ST_SENSOR_HAL>/ mm

