/*
 * Copyright (C) 2013 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Alberto Marinoni - Motion MEMS Product Div.
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
#if (SENSORS_ACCELEROMETER_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <string.h>
#include "AccelSensor.h"

#define FETCH_FULL_EVENT_BEFORE_RETURN		0
#define MS2_TO_G(x)				(x / 9.8)

/*****************************************************************************/

sensors_vec_t  AccelSensor::dataBuffer;
int AccelSensor::mEnabled = 0;
int64_t AccelSensor::delayms = 0;
int AccelSensor::current_fullscale = 0;
int64_t AccelSensor::setDelayBuffer[numSensors] = {0};
int64_t AccelSensor::writeDelayBuffer[numSensors] = {0};
int AccelSensor::DecimationBuffer[numSensors] = {0};
int AccelSensor::DecimationCount = 0;
pthread_mutex_t AccelSensor::dataMutex;

AccelSensor::AccelSensor()
	: SensorBase(NULL, SENSOR_DATANAME_ACCELEROMETER),
	mInputReader(6),
	mHasPendingEvent(false)
{
	pthread_mutex_init(&dataMutex, NULL);

	memset(mPendingEvents, 0, sizeof(mPendingEvents));

	mPendingEvents[Acceleration].version = sizeof(sensors_event_t);
	mPendingEvents[Acceleration].sensor = ID_ACCELEROMETER;
	mPendingEvents[Acceleration].type = SENSOR_TYPE_ACCELEROMETER;
	mPendingEvents[Acceleration].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;

#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
	mPendingEvents[SignificantMotion].version = sizeof(sensors_event_t);
	mPendingEvents[SignificantMotion].sensor = ID_SIGNIFICANT_MOTION;
	mPendingEvents[SignificantMotion].type = SENSOR_TYPE_SIGNIFICANT_MOTION;
	mPendingEvents[SignificantMotion].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif

#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
	mPendingEvents[ActivityReco].version = sizeof(sensors_event_t);
	mPendingEvents[ActivityReco].sensor = ID_ACTIVITY_RECOGNIZER;
	mPendingEvents[ActivityReco].type = SENSOR_TYPE_ACTIVITY;
	mPendingEvents[ActivityReco].data[0] = 1.0f;
#endif

#if defined(STORE_CALIB_ACCEL_ENABLED)
	pStoreCalibration = StoreCalibration::getInstance();
#endif

	if (data_fd) {
		STLOGI("AccelSensor::AccelSensor accel_device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("AccelSensor::AccelSensor accel_device_sysfs_path:(%s) not found", sysfs_device_path);
	}

	data_raw[0] = data_raw[1] = data_raw[2] = 0.0;
}

AccelSensor::~AccelSensor()
{
	if (mEnabled) {
		enable(SENSORS_ACCELEROMETER_HANDLE, 0, 0);
	}
	pthread_mutex_destroy(&dataMutex);
}

#if !defined(NOT_SET_ACC_INITIAL_STATE)
int AccelSensor::setInitialState()
{
	struct input_absinfo absinfo_x;
	struct input_absinfo absinfo_y;
	struct input_absinfo absinfo_z;

	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_X), &absinfo_x) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Y), &absinfo_y) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Z), &absinfo_z))
	{
		mHasPendingEvent = true;
	}

	setFullScale(SENSORS_ACCELEROMETER_HANDLE, ACCEL_DEFAULT_FULLSCALE);
	DecimationCount = 0;

	return 0;
}
#endif

int AccelSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
		case SENSORS_ACCELEROMETER_HANDLE:
			what = Acceleration;
			break;
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
		case SENSORS_SIGNIFICANT_MOTION_HANDLE:
			what = SignificantMotion;
			break;
#endif
#if (SENSOR_FUSION_ENABLE == 1)
		case SENSORS_SENSOR_FUSION_HANDLE:
			what = iNemoAcceleration;
			break;
#endif
#if (MAG_CALIBRATION_ENABLE == 1)
		case SENSORS_MAGNETIC_FIELD_HANDLE:
			what = MagCalAcceleration;
			break;
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
		case SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE:
			what = GeoMagRotVectAcceleration;
			break;
#endif
#if (SENSOR_FUSION_ENABLE == 0)
  #if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
		case SENSORS_ORIENTATION_HANDLE:
			what = Orientation;
			break;
  #endif
  #if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
		case SENSORS_LINEAR_ACCELERATION_HANDLE:
			what = Linear_Accel;
			break;
  #endif
  #if (GEOMAG_GRAVITY_ENABLE == 1)
		case SENSORS_GRAVITY_HANDLE:
			what = Gravity_Accel;
			break;
  #endif
#endif
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
		case SENSORS_VIRTUAL_GYROSCOPE_HANDLE:
			what = VirtualGyro;
			break;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		case SENSORS_GYROSCOPE_HANDLE:
			what = Gbias;
			break;
#endif
#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
		case SENSORS_ACTIVITY_RECOGNIZER_HANDLE:
			what = ActivityReco;
			break;
#endif
		default:
			what = -1;
	}

	return what;
}

int AccelSensor::enable(int32_t handle, int en, int  __attribute__((unused))type)
{
	int err = 0, errSM1 = 0, errSM2 = 0;
	int flags = en ? 1 : 0;
	int what = -1;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (flags) {
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
		if (what == SignificantMotion) {
			//enable sysfs state machine
			if(!(mEnabled & (1<<Acceleration)))
				errSM1 = writeSysfsCommand(SENSORS_SIGNIFICANT_MOTION_HANDLE,
								SIGN_MOTION_POLL_EN_FILE_NAME,
								"%lld" ,0);

			errSM2 = writeEnable(SENSORS_SIGNIFICANT_MOTION_HANDLE,
								SIGN_MOTION_ENABLE_VALUE);

		}
		if (what == Acceleration) {
			errSM2 = writeSysfsCommand(SENSORS_ACCELEROMETER_HANDLE,
								SIGN_MOTION_POLL_EN_FILE_NAME,
								"%lld", 1);
		}
#endif
		mEnabled |= (1<<what);
		writeMinDelay();

		if ((mEnabled == (1<<what)) && (errSM1 >= 0) && (errSM2 >= 0)) {
#if !defined(NOT_SET_ACC_INITIAL_STATE)
			setInitialState();
#endif
			err = writeEnable(SENSORS_ACCELEROMETER_HANDLE, flags);	// Enable Accelerometer
		}


	} else {
		int tmp = mEnabled;
		mEnabled &= ~(1<<what);
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
		if (what == SignificantMotion)
			errSM1 = writeEnable(SENSORS_SIGNIFICANT_MOTION_HANDLE,
								SIGN_MOTION_DISABLE_VALUE);
		if (what == Acceleration)
			errSM2 = writeSysfsCommand(SENSORS_ACCELEROMETER_HANDLE,
								SIGN_MOTION_POLL_EN_FILE_NAME,
								"%lld", 0);
#endif
		if((!mEnabled) && (tmp != 0)) {
			err = writeEnable(SENSORS_ACCELEROMETER_HANDLE, flags);
		}
		if ( (errSM1 < 0) || (errSM2 < 0) ){
			err = -1;
			mEnabled = tmp;
		}
		//setDelay(handle, DELAY_OFF);
		if (mEnabled) {
			writeMinDelay();
		}
	}

	if(err >= 0 ) {
		STLOGD("AccelSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);
	} else {
		STLOGE("AccelSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);
	}

	return err;
}

bool AccelSensor::hasPendingEvents() const
{
	return mHasPendingEvent;
}

int AccelSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int err = 0;
	int kk;
	int what = -1;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);

	if(delay_ms == 0)
		return err;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (mEnabled & (1<<SignificantMotion))
	{
		delay_ms = ACC_DEFAULT_DELAY;
	}

	/**
	 * The handled sensor is disabled. Set 0 in its setDelayBuffer position
	 * and update decimation buffer.
	 */
	if (delay_ms == NSEC_TO_MSEC(DELAY_OFF))
		delay_ms = 0;

	// Min setDelay Definition
	setDelayBuffer[what] = delay_ms;

#if (DEBUG_POLL_RATE == 1)
	STLOGD("AccSensor::setDelayBuffer[] = %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld",
						setDelayBuffer[0], setDelayBuffer[1],
						setDelayBuffer[2], setDelayBuffer[3],
						setDelayBuffer[4], setDelayBuffer[5],
						setDelayBuffer[6], setDelayBuffer[7],
						setDelayBuffer[8], setDelayBuffer[9],
						setDelayBuffer[10]);
#endif

	// Update sysfs
	if(mEnabled & 1 << what)
	{
		writeMinDelay();
	}

	return err;
}

int AccelSensor::writeMinDelay(void)
{
	int err = 0;
	int kk;
	int64_t Min_delay_ms = 0;

	for(kk = 0; kk < numSensors; kk++)
	{
		if ((mEnabled & 1<<kk) != 0)
		{
			writeDelayBuffer[kk] = setDelayBuffer[kk];
		}
		else
			writeDelayBuffer[kk] = 0;
	}

	// Min setDelay Definition
	for(kk = 0; kk < numSensors; kk++)
	{
		if (Min_delay_ms != 0) {
			if ((writeDelayBuffer[kk] != 0) && (writeDelayBuffer[kk] <= Min_delay_ms))
				Min_delay_ms = writeDelayBuffer[kk];
		} else
			Min_delay_ms = writeDelayBuffer[kk];
	}

#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
	if ((mEnabled & (1 << ActivityReco)) &&
		(Min_delay_ms > (1000 / ACTIVITY_RECOGNIZER_ODR)))
			Min_delay_ms = 1000 / ACTIVITY_RECOGNIZER_ODR;
#endif

	if ((Min_delay_ms > 0) && (Min_delay_ms != delayms))
	{
		err = writeDelay(SENSORS_ACCELEROMETER_HANDLE, Min_delay_ms);
		if(err >= 0) {
			err = 0;
			delayms = Min_delay_ms;
			DecimationCount = 0;
		}
	}

	// Decimation Definition
	for(kk = 0; kk < numSensors; kk++)
	{
		if (kk == Acceleration || kk == Gbias)
			continue;

		if (delayms)
			DecimationBuffer[kk] = writeDelayBuffer[kk]/delayms;
		else
			DecimationBuffer[kk] = 0;
	}

#if (DEBUG_POLL_RATE == 1)
	STLOGD("AccSensor::writeDelayBuffer[] = %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld",
						writeDelayBuffer[0], writeDelayBuffer[1],
						writeDelayBuffer[2], writeDelayBuffer[3],
						writeDelayBuffer[4], writeDelayBuffer[5],
						writeDelayBuffer[6], writeDelayBuffer[7],
						writeDelayBuffer[8], writeDelayBuffer[9],
						writeDelayBuffer[10]);
	STLOGD("AccSensor::Min_delay_ms = %lld, delayms = %lld, mEnabled = %d",
						Min_delay_ms, delayms, mEnabled);
	STLOGD("AccSensor::DecimationBuffer = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
						DecimationBuffer[0], DecimationBuffer[1],
						DecimationBuffer[2], DecimationBuffer[3],
						DecimationBuffer[4], DecimationBuffer[5],
						DecimationBuffer[6], DecimationBuffer[7],
						DecimationBuffer[8], DecimationBuffer[9],
						DecimationBuffer[10]);
#endif

	return err;
}

void AccelSensor::getAccDelay(int64_t *Acc_Delay_ms)
{
	*Acc_Delay_ms = delayms;

	return;
}


int AccelSensor::setFullScale(int32_t  __attribute__((unused))handle, int value)
{
	int err = -1;

	if(value <= 0)
		return err;
	else
		err = 0;

	if(value != current_fullscale)
	{
		err = writeFullScale(SENSORS_ACCELEROMETER_HANDLE, value);
		if(err >= 0) {
			err = 0;
			current_fullscale = value;
		}
	}
	return err;
}

int AccelSensor::readEvents(sensors_event_t* data, int count)
{
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

#if (FETCH_FULL_EVENT_BEFORE_RETURN)
	again:
#endif
	while (count && mInputReader.readEvent(&event)) {
		if (event->type == EVENT_TYPE_ACCEL) {
#if (DEBUG_ACCELEROMETER == 1)
	STLOGD("AccelSensor::readEvents (event_code=%d)", event->code);
#endif
			float value = (float) event->value;
			if (event->code == EVENT_TYPE_ACCEL_X) {
				data_raw[0] = value * CONVERT_A_X;
			}
			else if (event->code == EVENT_TYPE_ACCEL_Y) {
				data_raw[1] = value * CONVERT_A_Y;
			}
			else if (event->code == EVENT_TYPE_ACCEL_Z) {
				data_raw[2] = value * CONVERT_A_Z;
			}
#if defined(ACC_EVENT_HAS_TIMESTAMP)
			else if (event->code == EVENT_TYPE_TIME_MSB) {
				timestamp = ((int64_t)(event->value)) << 32;
			}
			else if (event->code == EVENT_TYPE_TIME_LSB) {
				timestamp |= (uint32_t)(event->value);
			}
#endif
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
			else if (event->code == EVENT_TYPE_SIGNIFICANT_MOTION) {

				if(mEnabled & (1<<SignificantMotion)) {
					mPendingEvents[SignificantMotion].data[0] = value;
					mPendingEvents[SignificantMotion].timestamp = timevalToNano(event->time);
					mPendingMask |= 1<<SignificantMotion;
  #if (DEBUG_ACCELEROMETER == 1)
					STLOGD("AccelSensor::SignificantMotion event type (type = %d, code = %d)",
							event->type, event->code);
  #endif
				}
			}
#endif
			else
				STLOGE("AccelSensor: unknown event code (type = %d, code = %d)",
							event->type, event->code);
		} else if (event->type == EV_SYN) {
			data_rot[0] = data_raw[0]*matrix_acc[0][0] +
					data_raw[1]*matrix_acc[1][0] +
					data_raw[2]*matrix_acc[2][0];
			data_rot[1] = data_raw[0]*matrix_acc[0][1] +
					data_raw[1]*matrix_acc[1][1] +
					data_raw[2]*matrix_acc[2][1];
			data_rot[2] = data_raw[0]*matrix_acc[0][2] +
					data_raw[1]*matrix_acc[1][2] +
					data_raw[2]*matrix_acc[2][2];
#if defined(STORE_CALIB_ACCEL_ENABLED)
			data_rot[0] -= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_BIAS,
					StoreCalibration::XAxis);
			data_rot[1] -= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_BIAS,
					StoreCalibration::YAxis);
			data_rot[2] -= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_BIAS,
					StoreCalibration::ZAxis);
			data_rot[0] *= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_SENS,
					StoreCalibration::XAxis);
			data_rot[1] *= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_SENS,
					StoreCalibration::YAxis);
			data_rot[2] *= pStoreCalibration->getCalibration(
					StoreCalibration::ACCELEROMETER_SENS,
					StoreCalibration::ZAxis);
#endif
#if !defined(ACC_EVENT_HAS_TIMESTAMP)
			timestamp = timevalToNano(event->time);

#endif

			DecimationCount++;

			if ((mEnabled & (1<<Acceleration)) &&
			   (DecimationCount >= DecimationBuffer[Acceleration])) {
				DecimationCount = 0;

				memcpy(mPendingEvents[Acceleration].data, data_rot, sizeof(float) * 3);
				mPendingEvents[Acceleration].timestamp = timestamp;
				mPendingMask |= 1<<Acceleration;
			}

#if (SENSORS_ACTIVITY_RECOGNIZER_ENABLE == 1)
			if (mEnabled & (1<<ActivityReco))
			{
				int activity_changed = 0;

				mPendingEvents[ActivityReco].data[0] =
						(float)ActivityRecognizerFunction(
							-MS2_TO_G(data_rot[0]),
							-MS2_TO_G(data_rot[1]),
							-MS2_TO_G(data_rot[2]),
							&activity_changed);
				if (activity_changed != 0) {
#if (DEBUG_ACTIVITY_RECO == 1)
					ALOGD("ActivityRecognizerSensor::readEvents, activity = %d",
					      mPendingEvents[ActivityReco].data[0]);
#endif

					mPendingEvents[ActivityReco].timestamp = timestamp;
					mPendingMask |= 1<<ActivityReco;
				}
			}
#endif

			if (mEnabled & ((1<<iNemoAcceleration) | (1<<MagCalAcceleration) |
				(1<<GeoMagRotVectAcceleration) | (1<<Orientation) |
				(1<<Linear_Accel) | (1<<Gravity_Accel) | (1<<Gbias) |
				(1<<VirtualGyro)))
			{
				sensors_vec_t sData;
				memcpy(sData.v, data_rot, sizeof(data_rot));
				setBufferData(&sData);
			}

#if (DEBUG_ACCELEROMETER == 1)
			STLOGD("AccelSensor(Acceleration)::readEvents (time = %lld), count(%d), received(%d)",
						mPendingEvents[Acceleration].timestamp,
						count, numEventReceived);
#endif
		} else {
			STLOGE("AccelSensor: unknown event type (type = %d, code = %d)", event->type, event->code);
		}

		for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
			if (mPendingMask & (1<<j)) {
				mPendingMask &= ~(1<<j);
#if (SENSORS_SIGNIFICANT_MOTION_ENABLE == 1)
				if((j == SignificantMotion) && mPendingEvents[j].data[0] == 0.0f)
					enable(SENSORS_SIGNIFICANT_MOTION_HANDLE, 0, 0);
#endif
				if (mEnabled & (1<<j)) {
					*data++ = mPendingEvents[j];
					count--;
					numEventReceived++;
				}
			}
		}
		mInputReader.next();
	}

#if (FETCH_FULL_EVENT_BEFORE_RETURN)
	/* if we didn't read a complete event, see if we can fill and
	try again instead of returning with nothing and redoing poll. */
	if (numEventReceived == 0 && (mEnabled > 0)) {
		n = mInputReader.fill(data_fd);
		if (n)
			goto again;
	}
#endif

	return numEventReceived;
}

bool AccelSensor::setBufferData(sensors_vec_t *value)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(&dataBuffer, value, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

	return true;
}

bool AccelSensor::getBufferData(sensors_vec_t *lastBufferedValues)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(lastBufferedValues, &dataBuffer, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

#if (DEBUG_ACCELEROMETER == 1)
	STLOGD("AccelSensor: getBufferData got values: x:(%f),y:(%f), z:(%f).",
					lastBufferedValues->x, lastBufferedValues->y,
					lastBufferedValues->z);
#endif

	return true;
}

#endif /* SENSORS_ACCELEROMETER_ENABLE */
