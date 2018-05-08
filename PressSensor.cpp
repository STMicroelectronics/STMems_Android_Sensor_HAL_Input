/*
 * Copyright (C) 2012 STMicroelectronics
 * Matteo Dameno, Denis Ciocca, Alberto Marinoni - Motion MEMS Product Div.
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

#include "configuration.h"
#if ((SENSORS_PRESSURE_ENABLE == 1) || (SENSORS_TEMP_PRESS_ENABLE == 1))

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "PressSensor.h"

int PressSensor::current_fullscale = 0;
int unsigned PressSensor::mEnabled = 0;

PressSensor::PressSensor() :
	SensorBase(NULL, SENSOR_DATANAME_BAROMETER),
	mPendingMask(0),
	mInputReader(4),
	mHasPendingEvent(false)
{
	memset(mPendingEvents, 0, sizeof(mPendingEvents));

	mPendingEvents[Pressure].version = sizeof(sensors_event_t);
	mPendingEvents[Pressure].sensor = ID_PRESSURE;
	mPendingEvents[Pressure].type = SENSOR_TYPE_PRESSURE;

#if (SENSORS_TEMP_PRESS_ENABLE == 1)
	mPendingEvents[Temperature].version = sizeof(sensors_event_t);
	mPendingEvents[Temperature].sensor = ID_TEMPERATURE;
	mPendingEvents[Temperature].type = SENSOR_TYPE_TEMPERATURE;
#endif

	if (data_fd) {
		STLOGI("PressSensor::PressSensor press_device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("PressSensor::PressSensor press_device_sysfs_path:(%s) not found", sysfs_device_path);
	}
}

PressSensor::~PressSensor()
{
	if (mEnabled) {
		enable(SENSORS_PRESSURE_HANDLE, 0, 0);
#if (SENSORS_TEMP_PRESS_ENABLE == 1)
		enable(SENSORS_TEMPERATURE_HANDLE, 0, 0);
#endif
	}
}

int PressSensor::setInitialState()
{
	struct input_absinfo absinfo_pressure;
	struct input_absinfo absinfo_temperature;
	float value;

	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PRESSURE), &absinfo_pressure) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_TEMPERATURE), &absinfo_temperature))
	{
		value = absinfo_temperature.value;
		mPendingEvents[Pressure].data[tempChan] = TEMPERATURE_OFFSET + value * CONVERT_TEMP;
		mPendingEvents[Temperature].temperature = TEMPERATURE_OFFSET + value * CONVERT_TEMP;

		value = absinfo_pressure.value;
		mPendingEvents[Pressure].data[pressChan] = value * CONVERT_PRESS;
		mHasPendingEvent = true;
	}

	return 0;
}

int PressSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
		case SENSORS_PRESSURE_HANDLE:
			what = Pressure;
			break;
#if (SENSORS_TEMP_PRESS_ENABLE == 1)
		case SENSORS_TEMPERATURE_HANDLE:
			what = Temperature;
			break;
#endif
		default:
			what = -1;
	}

	return what;
}

int PressSensor::writeSensorDelay(int handle)
{
	int err = writeDelay(handle, delayms);

	return err >= 0 ? 0 : err;
}

int PressSensor::enable(int32_t handle, int en, int type __attribute__((unused)))
{
	int err = 0;
	int what = -1;
	static int enabled = 0;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if(en) {
		err = writeSensorDelay(handle);
		if (err < 0)
			return err;

		if(mEnabled == 0) {
			enabled = 1;
			err = writeEnable(SENSORS_PRESSURE_HANDLE, 1);
		}
		if(err >= 0) {
			mEnabled |= (1<<what);
			err = 0;
			enabled = 0;
		}
	} else {
		int tmp = mEnabled;
		mEnabled &= ~(1<<what);
		if((mEnabled == 0) && (tmp != 0))
			err = writeEnable(SENSORS_PRESSURE_HANDLE, 0);

		if(err < 0)
			mEnabled |= (1<<what);
		else
			err = 0;
	}

	if(enabled == 1)
		setInitialState();

	return err;
}

int PressSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int err = -1, what;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);

	if (delay_ms == 0)
		return err;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	delayms = delay_ms;

	if (mEnabled & (1 << what))
		err = writeSensorDelay(handle);

	return err;
}

int PressSensor::setFullScale(int32_t __attribute__((unused))handle, int value)
{
	int err = -1;

	if(value <= 0)
		return err;
	else
		err = 0;

	if(value != current_fullscale)
	{
		err = writeFullScale(SENSORS_PRESSURE_HANDLE, value);
		if(err >= 0) {
			err = 0;
			current_fullscale = value;
		}
	}
	return err;
}

int PressSensor::readEvents(sensors_event_t* data, int count)
{
	static float lastTempValue = 0.0f;

#if DEBUG_PRESSURE_SENSOR == 1
	STLOGD("PressSensor::readEvents (count=%d)",count);
#endif

	if (count < 1)
		return -EINVAL;

	if (mHasPendingEvent) {
		mHasPendingEvent = false;
	}

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

	int numEventReceived = 0;
	input_event const* event;

	while (count && mInputReader.readEvent(&event)) {
#if DEBUG_PRESSURE_SENSOR == 1
		STLOGD("PressSensor::readEvents (count=%d),type(%d)",count,event->type);
#endif

		if (event->type == EV_MSC) {
			float value = (float) event->value;

			if (event->code == EVENT_TYPE_PRESSURE) {
				mPendingEvents[Pressure].data[pressChan] = value * CONVERT_PRESS;
				mPendingEvents[Pressure].data[tempChan] = TEMPERATURE_OFFSET + lastTempValue * CONVERT_TEMP;
			}
#if (SENSORS_TEMP_PRESS_ENABLE == 1)
			else if (event->code == EVENT_TYPE_TEMPERATURE) {
				lastTempValue = value;
				mPendingEvents[Temperature].temperature = TEMPERATURE_OFFSET + value * CONVERT_TEMP;
			}
#endif
			else {
				STLOGE("PressSensor: unknown event code (type=%d, code=%d)", event->type,event->code);
			}
		} else if (event->type == EV_SYN) {
			if(mEnabled & (1<<Pressure))
				mPendingMask |= 1<<Pressure;
			if(mEnabled & (1<<Temperature))
				mPendingMask |= 1<<Temperature;

			int64_t time = timevalToNano(event->time);
			for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
				if (mPendingMask & (1<<j)) {
					mPendingMask &= ~(1<<j);
					mPendingEvents[j].timestamp = time;
					if (mEnabled & (1<<j)) {
						*data++ = mPendingEvents[j];
						count--;
						numEventReceived++;
					}
				}
			}
		} else {
			STLOGE("PressSensor: unknown event type (type=%d, code=%d)", event->type, event->code);
		}
		mInputReader.next();
	}

	return numEventReceived;
}

#endif /* SENSORS_PRESSURE_ENABLE */
