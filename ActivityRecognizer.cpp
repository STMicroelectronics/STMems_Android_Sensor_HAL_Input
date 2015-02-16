/*
 * Copyright (C) 2015 STMicroelectronics
 * Alberto Marinoni, Giuseppe Barba
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

#include "configuration.h"
#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>

#include "ActivityRecognizer.h"
#define ACC_DATA_ODR		16

/****************************************************************************/
ActivityRecognizerSensor::ActivityRecognizerSensor()
	: SensorBase(NULL, SENSOR_DATANAME_ACCELEROMETER),
	mInputReader(4)
{
	mEnabled = 0;

	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = ID_ACTIVITY_RECOGNIZER;
	mPendingEvent.type = SENSOR_TYPE_ACTIVITY;
	mPendingEvent.data[0] = 1.0f;

	acc = new AccelSensor();
}

ActivityRecognizerSensor::~ActivityRecognizerSensor()
{
	if (mEnabled) {
		acc->enable(SENSORS_ACTIVITY_RECOGNIZER_HANDLE, 0, 0);
		acc->~AccelSensor();
	}
}

int ActivityRecognizerSensor::getWhatFromHandle(int32_t __attribute__((unused))handle)
{
	return 0;
}

int ActivityRecognizerSensor::enable(int32_t handle, int en, int __attribute__((unused))type)
{
	int err = 0;
	int flags = en ? 1 : 0;
	int mEnabledPrev;

	if (flags) {
		if (!mEnabled) {
 			acc->enable(SENSORS_ACTIVITY_RECOGNIZER_HANDLE, 1, 1);
			acc->setDelay(SENSORS_ACTIVITY_RECOGNIZER_HANDLE,
						1000000000LL / ACC_DATA_ODR);
		}
		mEnabled = 1;
	} else {

		mEnabledPrev = mEnabled;
		mEnabled = 0;
		if (!mEnabled && mEnabledPrev) {
  			acc->enable(SENSORS_ACTIVITY_RECOGNIZER_HANDLE, 0, 1);
			acc->setDelay(SENSORS_ACTIVITY_RECOGNIZER_HANDLE, DELAY_OFF);
		}
		
	}

	if(err >= 0 ) {
		STLOGD("ActivityRecognizerSensor::enable(%d), handle: %d,"
				" mEnabled: %x",flags, handle, mEnabled);
	} else {
		STLOGE("ActivityRecognizerSensor::enable(%d), handle: %d,"
				" mEnabled: %x",flags, handle, mEnabled);
	}

	return err;
}

bool ActivityRecognizerSensor::hasPendingEvents() const
{
	return false;
}

int ActivityRecognizerSensor::setDelay(int32_t __attribute__((unused))handle,
				       int64_t __attribute__((unused))delay_ns)
{
	return 0;
}

int ActivityRecognizerSensor::readEvents(sensors_event_t* data, int count)
{
	int numEventReceived = 0;
	sensors_vec_t acc_data;
	input_event const* event;
	int act;
	int activity_changed = 0;

	if (!mEnabled)
		return 0;

	if (count < 1)
		return -EINVAL;


	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

	while (count && mInputReader.readEvent(&event)) {
		if (event->type == EVENT_TYPE_ACCEL) {
#if defined(ACC_EVENT_HAS_TIMESTAMP)
			if (event->code == EVENT_TYPE_TIME_MSB) {
				timestamp = ((int64_t)(event->value)) << 32;
			}
			else if (event->code == EVENT_TYPE_TIME_LSB) {
				timestamp |= (uint32_t)(event->value);
			}
#endif
		} else if (event->type == EV_SYN) {
			acc->getBufferData(&acc_data);
#if (DEBUG_ACTIVITY_RECO == 1)
			ALOGD("ActivityRecognizerSensor::readEvents, accell data =\t%f\t%f\t%f",
							acc_data.x,
							acc_data.y,
							acc_data.z);
#endif
			act = ActivityRecognizerFunction(acc_data.x,
							acc_data.y,
							acc_data.z,
							&activity_changed);
#if (DEBUG_ACTIVITY_RECO == 1)
			ALOGD("ActivityRecognizerSensor::readEvents, activity = %d", act);
#endif
			if (activity_changed == 0)
				return 0;

			ALOGD("ActivityRecognizerSensor::readEvents, activity = %d", act);

#if !defined(ACC_EVENT_HAS_TIMESTAMP)
			timestamp = timevalToNano(event->time);
#endif
			mPendingEvent.timestamp = timestamp;
			mPendingEvent.data[0] = (float)act;
			memcpy(data, &mPendingEvent, sizeof(mPendingEvent));
			data++;
			count--;
			numEventReceived++;
		}
		mInputReader.next();
	}
	return numEventReceived;
}
#endif /* SENSORS_ACTIVITY_RECOGNIZER_ENABLE */
 
