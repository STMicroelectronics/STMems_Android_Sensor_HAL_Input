/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Denis Ciocca - Motion MEMS Product Div.
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>
#include <utils/Atomic.h>

#include "SensorBase.h"
#include "configuration.h"
#include "sensors.h"

/*****************************************************************************/

SensorBase::SensorBase(const char* dev_name, const char* data_name)
	: dev_name(dev_name), data_name(data_name),
	dev_fd(-1), data_fd(-1)
{
	if(data_name)
		data_fd = openInput(data_name);
}

SensorBase::~SensorBase()
{
	if (data_fd >= 0)
		close(data_fd);

	if (dev_fd >= 0)
		close(dev_fd);
}

int SensorBase::open_device()
{
	if (dev_fd<0 && dev_name) {
		dev_fd = open(dev_name, O_RDONLY);
		STLOGE_IF(dev_fd < 0, "Couldn't open %s (%s)", dev_name, strerror(errno));
	}
	return 0;
}

int SensorBase::close_device()
{
	if (dev_fd >= 0) {
		close(dev_fd);
		dev_fd = -1;
	}
	return 0;
}

int SensorBase::getFd() const
{
	if (!data_name)
		return dev_fd;

	return data_fd;
}

int SensorBase::setDelay(int32_t  __attribute__((unused)) handle,
			 int64_t  __attribute__((unused)) ns)
{
	return 0;
}

bool SensorBase::hasPendingEvents() const
{
	return false;
}

int SensorBase::openInput(const char* inputDeviceName)
{
	int fd = -1;

	fd = getSysfsDevicePath(sysfs_device_path, inputDeviceName);
	sysfs_device_path_len = strlen(sysfs_device_path);

	return fd;
}


int SensorBase::getSysfsDevicePath(char* sysfs_path ,const char* inputDeviceName)
{
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;

	sysfs_path[0] ='\0';
	dir = opendir(dirname);

	if(dir == NULL)
		return -1;

	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';

	while((de = readdir(dir))) {
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;

		strcpy(filename, de->d_name);
		fd = open(devname, O_RDONLY);
		if (fd >= 0) {
			char name[80];
			if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
				name[0] = '\0';

			if (!strcmp(name, inputDeviceName)) {
				strcpy(sysfs_path,"/sys/class/input/");
				strcat(sysfs_path,filename);
				strcat(sysfs_path,"/device/");

				break;
			} else {
				close(fd);
				fd = -1;
			}
		}
	}

	closedir(dir);
	STLOGE_IF(fd < 0, "couldn't find sysfs path for device '%s' ", inputDeviceName);

	return fd;
}

int SensorBase::writeFullScale(int32_t handle, int value)
{
	int fd;
	int err;
	char buf[6];
	const char *className;

	switch(handle) {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		case SENSORS_ACCELEROMETER_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], ACCEL_RANGE_FILE_NAME);
			className = "AccelSensor::setFullScale()";
			break;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		case SENSORS_MAGNETIC_FIELD_HANDLE:
		case SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], MAGN_RANGE_FILE_NAME);
			className = "MagnSensor::setFullScale()";
			break;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		case SENSORS_GYROSCOPE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], GYRO_RANGE_FILE_NAME);
			className = "Gyro::setFullScale()";
			break;
#endif
		default:
			return -1;
	}


	fd = open(sysfs_device_path, O_RDWR);
	sprintf(buf,"%d", value);
	err = write(fd, buf, sizeof(buf));
	close(fd);

	if(err >= 0) {
		STLOGI("%s Set new full-scale to %d", className, value);
	} else {
		STLOGE("%s Failed to set Full-scale: %d - %s", className, value, sysfs_device_path);
	}

	return err >= 0 ? 0 : -1;
}

int SensorBase::writeEnable(int32_t handle, int enable)
{
	int fd;
	int err;
	char buf[6];
	const char *className;

	switch(handle) {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		case SENSORS_ACCELEROMETER_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], ACCEL_ENABLE_FILE_NAME);
			className = "AccelSensor::Enable(Accel)";
			break;
#endif
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
		case SENSORS_SIGNIFICANT_MOTION_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], SIGN_MOTION_ENABLE_FILE_NAME);
			className = "AccelSensor::Enable(SigMotion)";
			break;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		case SENSORS_MAGNETIC_FIELD_HANDLE:
		case SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], MAGN_ENABLE_FILE_NAME);
			className = "MagnSensor::Enable()";
			break;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		case SENSORS_GYROSCOPE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], GYRO_ENABLE_FILE_NAME);
			className = "GyroSensor::Enable()";
			break;
#endif
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
		case SENSORS_PRESSURE_HANDLE:
		case SENSORS_TEMPERATURE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], PRESS_ENABLE_FILE_NAME);
			className = "PressTempSensor::Enable()";
			break;
#endif
#if (SENSORS_TEMP_ENABLE == 1)
		case SENSORS_TEMPERATURE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], TEMP_ENABLE_FILE_NAME);
			className = "TempSensor::Enable()";
			break;
#endif
#if (SENSORS_TILT_ENABLE == 1)
		case SENSORS_TILT_DETECTOR_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], TILT_ENABLE_FILE_NAME);
			className = "TiltSensor::Enable()";
			break;
#endif
#if (SENSORS_STEP_COUNTER_ENABLE == 1)
		case SENSORS_STEP_COUNTER_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], STEP_C_ENABLE_FILE_NAME);
			className = "StepCounterSensor::Enable()";
			break;
#endif
#if (SENSORS_STEP_DETECTOR_ENABLE == 1)
		case SENSORS_STEP_DETECTOR_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], STEP_D_ENABLE_FILE_NAME);
			className = "StepDetectorSensor::Enable()";
			break;
#endif
#if (SENSORS_SIGN_MOTION_ENABLE == 1)
		case SENSORS_SIGN_MOTION_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], SIGN_M_ENABLE_FILE_NAME);
			className = "SignMotionSensor::Enable()";
			break;
#endif
#if (SENSORS_TAP_ENABLE == 1)
		case SENSORS_TAP_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], TAP_ENABLE_FILE_NAME);
			className = "TapSensor::Enable()";
			break;
#endif
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
		case SENSORS_TEMPERATURE_HANDLE:
		case SENSORS_HUMIDITY_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], HUMIDITY_ENABLE_FILE_NAME);
			className = "HimiditySensor::Enable()";
			break;
#endif
		default:
			return -1;
	}

	fd = open(sysfs_device_path, O_RDWR);
	sprintf(buf,"%d", enable);
	err = write(fd, buf, sizeof(buf));
	close(fd);

	if(err > 0) {
		STLOGI("%s Set enable to %d", className, enable);
	} else {
		STLOGE("%s Failed to set enable: %d - %s", className, enable, sysfs_device_path);
	}

	return err > 0 ? 0 : -1;
}

int SensorBase::writeDelay(int32_t handle, int64_t delay_ms)
{
	int fd;
	int err;
	char buf[8];
	const char *className;

	STLOGD( "SensorBase: setDelay handle = %d", handle);

	switch(handle) {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		case SENSORS_ACCELEROMETER_HANDLE:
		case SENSORS_SIGNIFICANT_MOTION_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], ACCEL_DELAY_FILE_NAME);
			className = "AccelSensor::Delay()";
			break;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		case SENSORS_MAGNETIC_FIELD_HANDLE:
		case SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], MAGN_DELAY_FILE_NAME);
			className = "MagnSensor::Delay()";
			break;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		case SENSORS_GYROSCOPE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], GYRO_DELAY_FILE_NAME);
			className = "Gyro::Delay()";
			break;
#endif
#if (SENSORS_STEP_COUNTER_ENABLE == 1)
		case SENSORS_STEP_COUNTER_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], STEP_C_DELAY_FILE_NAME);
			className = "StepCounterSensor::Delay()";
			break;
#endif
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))
		case SENSORS_PRESSURE_HANDLE:
		case SENSORS_TEMPERATURE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], PRESS_DELAY_FILE_NAME);
			className = "PressTempSensor::Delay()";
			break;
#endif
#if (SENSORS_TEMP_ENABLE == 1)
		case SENSORS_TEMPERATURE_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], TEMP_DELAY_FILE_NAME);
			className = "TempSensor::Enable()";
			break;
#endif
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))
		case SENSORS_TEMPERATURE_HANDLE:
		case SENSORS_HUMIDITY_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], HUMIDITY_DELAY_FILE_NAME);
			className = "HimiditySensor::Delay()";
			break;
#endif
		default:
			return -1;
	}

	fd = open(sysfs_device_path, O_RDWR);
	if (!fd) {
		STLOGE("%s Failed to open device: %s", className, sysfs_device_path);
		return -1;
	}

	sprintf(buf,"%lld", (long long)delay_ms);
	err = write(fd, buf, sizeof(buf));
	close(fd);

	if(err > 0) {
		STLOGI("%s Set delay to %lld [ms]", className, (long long)delay_ms);
	} else {
		STLOGE("%s Failed to set delay: %lld [ms] - %s", className, (long long)delay_ms, sysfs_device_path);
	}

        return err > 0 ? 0 : -1;
}

int SensorBase::writeSysfsCommand(int32_t handle, const char *sysfsFilename, const char *dataFormat, int64_t param)
{
	int fd;
	int err;
	char buf[8];
	const char *className;

	char formatstring1[50] = "%s Set %s to ";
	char formatstring2[100] = "%s Failed to set %s: ";

	switch(handle) {
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		case SENSORS_ACCELEROMETER_HANDLE:
		case SENSORS_SIGNIFICANT_MOTION_HANDLE:
			strcpy(&sysfs_device_path[sysfs_device_path_len], sysfsFilename);
			className = "AccelSensor::Command()";
			break;
#endif
		default:
			return -1;
	}

	fd = open(sysfs_device_path, O_RDWR);
	sprintf(buf, dataFormat, param);
	err = write(fd, buf, sizeof(buf));
	close(fd);


	strcat(formatstring1, dataFormat);
	strcat(formatstring2, dataFormat);
	strcat(formatstring2, " - %s");

	if(err > 0) {
		STLOGI(formatstring1, className, sysfsFilename, param);
	} else {
		STLOGE(formatstring2, className, sysfsFilename, param, sysfs_device_path);
	}

        return err > 0 ? 0 : -1;
}

/*
 * The following functions are the same used by Android to retrieve system
 * boot time (system/core/libutils/SystemClock.cpp).
 * These function are here reported to be consistent with the
 * timestamp check used into CTS tests.
 */
enum {
	SYSTEM_TIME_REALTIME = 0,  // system-wide realtime clock
	SYSTEM_TIME_MONOTONIC = 1, // monotonic time since unspecified starting point
	SYSTEM_TIME_PROCESS = 2,   // high-resolution per-process clock
	SYSTEM_TIME_THREAD = 3,    // high-resolution per-thread clock
	SYSTEM_TIME_BOOTTIME = 4   // same as SYSTEM_TIME_MONOTONIC, but including CPU suspend time
};

int64_t systemTime(int clock)
{
	static const clockid_t clocks[] = {
		CLOCK_REALTIME,
		CLOCK_MONOTONIC,
		CLOCK_PROCESS_CPUTIME_ID,
		CLOCK_THREAD_CPUTIME_ID,
		CLOCK_BOOTTIME
	};
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	clock_gettime(clocks[clock], &t);
	return int64_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}
