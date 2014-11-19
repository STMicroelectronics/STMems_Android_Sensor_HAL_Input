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
#if (SENSORS_PRESSURE_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "PressTempSensor.h"
#include SENSOR_PRESS_INCLUDE_FILE_NAME

#define FETCH_FULL_EVENT_BEFORE_RETURN		0

/*****************************************************************************/

int64_t PressTempSensor::delayms = 0;
int PressTempSensor::current_fullscale = 0;
int unsigned PressTempSensor::mEnabled = 0;

PressTempSensor::PressTempSensor()
	: SensorBase(NULL, SENSOR_DATANAME_BAROMETER),
	mPendingMask(0),
	mInputReader(4),
	mHasPendingEvent(false)
{
	memset(mPendingEvents, 0, sizeof(mPendingEvents));

	mPendingEvents[Pressure].version = sizeof(sensors_event_t);
	mPendingEvents[Pressure].sensor = ID_PRESSURE;
	mPendingEvents[Pressure].type = SENSOR_TYPE_PRESSURE;

#if (SENSORS_TEMPERATURE_ENABLE == 1)
	mPendingEvents[Temperature].version = sizeof(sensors_event_t);
	mPendingEvents[Temperature].sensor = ID_TEMPERATURE;
	mPendingEvents[Temperature].type = SENSOR_TYPE_TEMPERATURE;
#endif

	if (data_fd) {
		STLOGI("PressTempSensor::PressTempSensor press_device_sysfs_path:(%s)", sysfs_device_path);
#if PRESS_COMPENSATION_ENABLE == 1
		initCompensationAlgo();
#endif
	} else {
		STLOGE("PressTempSensor::PressTempSensor press_device_sysfs_path:(%s) not found", sysfs_device_path);
	}
}

PressTempSensor::~PressTempSensor()
{
	if (mEnabled) {
		enable(SENSORS_PRESSURE_HANDLE, 0, 0);
#if (SENSORS_TEMPERATURE_ENABLE == 1)
		enable(SENSORS_TEMPERATURE_HANDLE, 0, 0);
#endif
	}
}

int PressTempSensor::setInitialState()
{
	struct input_absinfo absinfo_pressure;
	struct input_absinfo absinfo_temperature;
	float pressureCompansationOffset = 0.0f;
	float value;

	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PRESSURE), &absinfo_pressure) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_TEMPERATURE), &absinfo_temperature))
	{
		value = absinfo_temperature.value;
		mPendingEvents[Pressure].data[tempChan] = TEMPERATURE_OFFSET + value * CONVERT_TEMP;
		mPendingEvents[Temperature].temperature = TEMPERATURE_OFFSET + value * CONVERT_TEMP;

#if PRESS_COMPENSATION_ENABLE == 1
		pressureCompansationOffset = (float)LPS331AP_getPressureCompensationOffSet(absinfo_temperature.value);
#endif

		value = absinfo_pressure.value;
		value = value + pressureCompansationOffset;
		mPendingEvents[Pressure].data[pressChan] = value * CONVERT_PRESS;
		mHasPendingEvent = true;
	}

	return 0;
}

int PressTempSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
		case SENSORS_PRESSURE_HANDLE:
			what = Pressure;
			break;
#if (SENSORS_TEMPERATURE_ENABLE == 1)
		case SENSORS_TEMPERATURE_HANDLE:
			what = Temperature;
			break;
#endif
		default:
			what = -1;
	}

	return what;
}

int PressTempSensor::enable(int32_t handle, int en, int type)
{
	int err = 0;
	int what = -1;
	static int enabled = 0;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if(en) {
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

bool PressTempSensor::hasPendingEvents() const {
	return mHasPendingEvent;
}

int PressTempSensor::setDelay(int32_t __attribute__((unused))handle, int64_t delay_ns)
{
	int err = -1;
	int64_t delay_ms = 0;

	if((mEnabled & (1<<Pressure)) || (mEnabled & (1<<Temperature)))
	{
		delay_ms = delay_ns/1000000;
	}

	if(delay_ms == 0)
		return err;

	if(delay_ms != delayms)
	{
		err = writeDelay(SENSORS_PRESSURE_HANDLE, delay_ms);
		if(err >= 0) {
			err = 0;
			delayms = delay_ms;
		}
	}
	return err;
}

int PressTempSensor::setFullScale(int32_t __attribute__((unused))handle, int value)
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

int PressTempSensor::readEvents(sensors_event_t* data, int count)
{
	static int cont = 0;
	static float lastTempValue = 0.0f;
	float pressureCompansationOffset = 0.0f;


#if DEBUG_PRESSURE_SENSOR == 1
	STLOGD("PressTempSensor::readEvents (count=%d)",count);
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

#if FETCH_FULL_EVENT_BEFORE_RETURN
	again:
#endif

	while (count && mInputReader.readEvent(&event)) {
		
#if DEBUG_PRESSURE_SENSOR == 1
		STLOGD("PressTempSensor::readEvents (count=%d),type(%d)",count,event->type);
#endif

		if (event->type == EV_ABS) {
			float value = (float) event->value;

			if (event->code == EVENT_TYPE_PRESSURE) {

#if PRESS_COMPENSATION_ENABLE == 1
				pressureCompansationOffset = (float)LPS331AP_getPressureCompensationOffSet( lastTempValue );
#endif
				value = value + pressureCompansationOffset;
				mPendingEvents[Pressure].data[pressChan] = value * CONVERT_PRESS;
				mPendingEvents[Pressure].data[tempChan] = TEMPERATURE_OFFSET + lastTempValue * CONVERT_TEMP;
			}
#if (SENSORS_TEMPERATURE_ENABLE == 1)
			else if (event->code == EVENT_TYPE_TEMPERATURE) {
				lastTempValue = value;
				mPendingEvents[Temperature].temperature = TEMPERATURE_OFFSET + value * CONVERT_TEMP;
			}
#endif
			else {
				STLOGE("PressTempSensor: unknown event code (type=%d, code=%d)", event->type,event->code);
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
			STLOGE("PressTempSensor: unknown event type (type=%d, code=%d)", event->type, event->code);
		}
		mInputReader.next();
	}

#if FETCH_FULL_EVENT_BEFORE_RETURN
	/* if we didn't read a complete event, see if we can fill and
	try again instead of returning with nothing and redoing poll. */
	if (numEventReceived == 0 && mEnabled == 1) {
		n = mInputReader.fill(data_fd);
		if (n)
			goto again;
	}
#endif

	return numEventReceived;
}

#if PRESS_COMPENSATION_ENABLE == 1
int PressTempSensor::initCompensationAlgo(void)
{
	FILE *fd_param;
	char device_sysfs_path_prs[PATH_MAX];
	strcpy(device_sysfs_path_prs, sysfs_device_path);
	strcat(device_sysfs_path_prs, PRESS_COMPENSATION_FILE_NAME);
	if ((fd_param = fopen(device_sysfs_path_prs, "r")) == NULL ) {
		/* ERROR HANDLING; you can check errno variable to see what went wrong */
		STLOGE("PressTempSensor:Error Occured while opening %s File.!!\n", device_sysfs_path_prs);
		return -1;
	} else {

#if DEBUG_PRESSURE_SENSOR == 1
		STLOGD("PressTempSensor:File %s Opened Successfully !!\n", device_sysfs_path_prs);
#endif

		fscanf(fd_param, "%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
		&calvalues.TSL, &calvalues.TSH,
		&calvalues.TCV1, &calvalues.TCV2, &calvalues.TCV3,
		&calvalues.TCS1, &calvalues.TCS2, &calvalues.TCS3,
		&calvalues.digGain);

		STLOGI("PressTempSensor:Acquired Pressure Compansation Values: TSL = 0x%04x, TSH = 0x%04x, TCV1 = 0x%04x, TCV2 = 0x%04x, TCV3 = 0x%04x, TCS1 = 0x%04x, TCS2 = 0x%04x, TCS3 = 0x%04x, DGAIN = 0x%04x\n", calvalues.TSL, calvalues.TSH, calvalues.TCV1, calvalues.TCV2, calvalues.TCV3, calvalues.TCS1, calvalues.TCS2, calvalues.TCS3, calvalues.digGain);

		fclose(fd_param);
		LPS331AP_CalculateAccuracyQuadr(calvalues);
		return 0;
	}
}
#endif

#endif /* SENSORS_PRESSURE_ENABLE */
