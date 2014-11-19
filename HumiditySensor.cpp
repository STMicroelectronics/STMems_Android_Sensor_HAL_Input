/*
 * Copyright (C) 2016 STMicroelectronics
 * Lorenzo Bianconi - Motion MEMS Product Div.
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
#if ((SENSORS_HUMIDITY_ENABLE == 1) || (SENSORS_TEMP_RH_ENABLE == 1))

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "HumiditySensor.h"

HumiditySensor::HumiditySensor() :
	SensorBase(NULL, SENSOR_DATANAME_HUMIDITY),
	mEnabled(false),
	current_fullscale(0),
	mInputReader(4)
{
	memset(mPendingEvents, 0, sizeof(mPendingEvents));
	/* humidity sensor */
	mPendingEvents[0].version = sizeof(sensors_event_t);
	mPendingEvents[0].sensor = ID_HUMIDITY;
	mPendingEvents[0].type = SENSOR_TYPE_RELATIVE_HUMIDITY;
	/* temperature sensor */
	mPendingEvents[1].version = sizeof(sensors_event_t);
	mPendingEvents[1].sensor = ID_TEMPERATURE;
	mPendingEvents[1].type = SENSOR_TYPE_TEMPERATURE;
}

HumiditySensor::~HumiditySensor()
{
	if (mEnabled)
		enable(SENSORS_HUMIDITY_HANDLE, 0, 0);
}

int HumiditySensor::writeSensorDelay(int handle)
{
	int err = writeDelay(handle, delayms);

	return err >= 0 ? 0 : err;
}

int HumiditySensor::enable(int32_t handle, int en, int type)
{
	int err = 0;

	if (en) {
		err = writeSensorDelay(SENSORS_HUMIDITY_HANDLE);
		if (err < 0)
			return err;

		if (!mEnabled)
			err = writeEnable(SENSORS_HUMIDITY_HANDLE, 1);

		if (err >= 0) {
			err = 0;
			mEnabled = true;
		}
	} else {
		err = writeEnable(SENSORS_HUMIDITY_HANDLE, 0);
		if (err < 0)
			return err;

		mEnabled = false;
		err = 0;
	}

	return err;
}

int HumiditySensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);
	int err = 0;

	if (delay_ms == 0)
		return -EINVAL;

	delayms = delay_ms;
	if (mEnabled)
		err = writeSensorDelay(SENSORS_HUMIDITY_HANDLE);

	return err;
}

int HumiditySensor::setFullScale(int32_t __attribute__((unused))handle, int value)
{
	int err = 0;

	if (value <= 0)
		return -EINVAL;

	if (value != current_fullscale) {
		err = writeFullScale(SENSORS_HUMIDITY_HANDLE, value);
		if (err >= 0) {
			err = 0;
			current_fullscale = value;
		}
	}
	return err;
}

int HumiditySensor::readEvents(sensors_event_t* data, int count)
{
#if DEBUG_HUMIDITY_SENSOR == 1
	STLOGD("HumiditySensor::readEvents (count=%d)",count);
#endif

	if (count < 1)
		return -EINVAL;

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

	int numEventReceived = 0;
	input_event const* event;

	while (count && mInputReader.readEvent(&event)) {
#if DEBUG_HUMIDITY_SENSOR == 1
		STLOGD("HumiditySensor::readEvents (count=%d),type(%d)",count,event->type);
#endif

		if (event->type == EV_MSC) {
			float value = (float) event->value;
#if SENSORS_HUMIDITY_ENABLE == 1
			if (event->code == EVENT_TYPE_HUMIDITY) {
				mPendingEvents[0].relative_humidity = value * CONVERT_RH;
				mPendingEvents[0].timestamp = timevalToNano(event->time);
				*data++ = mPendingEvents[0];
				count--;
				numEventReceived++;
			}
#endif
#if SENSORS_TEMP_RH_ENABLE == 1
			if (event->code == EVENT_TYPE_TEMPERATURE) {
				mPendingEvents[1].temperature = value * CONVERT_TEMP;
				mPendingEvents[1].timestamp = timevalToNano(event->time);
				*data++ = mPendingEvents[1];
				count--;
				numEventReceived++;
			}
#endif
		} else if (event->type != EV_SYN) {
			STLOGE("HumiditySensor: unknown event type (type=%d, code=%d)",
			       event->type, event->code);
		}
		mInputReader.next();
	}

	return numEventReceived;
}
#endif /* SENSORS_HUMIDITY_ENABLE */

