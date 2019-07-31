/*
 * Copyright (C) 2012 STMicroelectronics
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
#if (SENSORS_GYROSCOPE_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <string.h>
#include <pthread.h>

#include "GyroSensor.h"

#define FETCH_FULL_EVENT_BEFORE_RETURN		0

/*****************************************************************************/
sensors_vec_t  GyroSensor::dataBuffer;
int GyroSensor::mEnabled = 0;
int64_t GyroSensor::delayms = 0;
int GyroSensor::startup_samples = DEFAULT_SAMPLES_TO_DISCARD;
int GyroSensor::current_fullscale = 0;
int GyroSensor::samples_to_discard = DEFAULT_SAMPLES_TO_DISCARD;
float GyroSensor::gbias_out[3] = {0};
int64_t GyroSensor::setDelayBuffer[numSensors] = {0};
int64_t GyroSensor::writeDelayBuffer[numSensors] = {0};
int GyroSensor::DecimationBuffer[numSensors] = {0};
int GyroSensor::DecimationCount[numSensors] = {0};
pthread_mutex_t GyroSensor::dataMutex;
#if (SENSORS_ACCELEROMETER_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
AccelSensor* GyroSensor::acc = NULL;
#endif

GyroSensor::GyroSensor()
	: SensorBase(NULL, SENSOR_DATANAME_GYROSCOPE),
	mInputReader(6),
	mHasPendingEvent(false)
{
	pthread_mutex_init(&dataMutex, NULL);

#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 0)
	mPendingEvent[Gyro].version = sizeof(sensors_event_t);
	mPendingEvent[Gyro].sensor = ID_GYROSCOPE;
	mPendingEvent[Gyro].type = SENSOR_TYPE_GYROSCOPE;
	mPendingEvent[Gyro].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;

  #if ((SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1))
	mPendingEvent[GyroUncalib].version = sizeof(sensors_event_t);
	mPendingEvent[GyroUncalib].sensor = ID_UNCALIB_GYROSCOPE;
	mPendingEvent[GyroUncalib].type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
	mPendingEvent[GyroUncalib].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;
  #endif
#endif

#if defined(STORE_CALIB_GYRO_ENABLED)
	pStoreCalibration = StoreCalibration::getInstance();
#endif

	if (data_fd) {
		STLOGI("GyroSensor::GyroSensor gyro_device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("GyroSensor::GyroSensor gyro_device_sysfs_path:(%s) not found", sysfs_device_path);
	}

	memset(data_raw, 0, sizeof(data_raw));

#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
	iNemoEngine_API_gbias_Initialization(false);
  #if (SENSORS_ACCELEROMETER_ENABLE == 1)
	acc = new AccelSensor();
  #endif
#endif
}

GyroSensor::~GyroSensor()
{
	if (mEnabled) {
		enable(SENSORS_GYROSCOPE_HANDLE, 0, 0);
	}
#if ((SENSORS_ACCELEROMETER_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1))
	acc->~AccelSensor();
#endif
	pthread_mutex_destroy(&dataMutex);
}

#if !defined(NOT_SET_GYRO_INITIAL_STATE)
int GyroSensor::setInitialState()
{
	struct input_absinfo absinfo_x;
	struct input_absinfo absinfo_y;
	struct input_absinfo absinfo_z;

	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_X), &absinfo_x) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Y), &absinfo_y) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_GYRO_Z), &absinfo_z))
	{
		mHasPendingEvent = true;
	}

	setFullScale(SENSORS_GYROSCOPE_HANDLE, GYRO_DEFAULT_FULLSCALE);
	startup_samples = samples_to_discard;
	memset(DecimationCount, 0, sizeof(DecimationCount));

	return 0;
}
#endif

int GyroSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
		case SENSORS_GYROSCOPE_HANDLE:
			what = Gyro;
			break;
#if ((SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1))
		case SENSORS_UNCALIB_GYROSCOPE_HANDLE:
			what = GyroUncalib;
			break;
#endif
#if (SENSOR_FUSION_ENABLE == 1)
		case SENSORS_SENSOR_FUSION_HANDLE:
			what = iNemoGyro;
			break;
#endif
		default:
			what = -1;
	}

	return what;
}

int GyroSensor::enable(int32_t handle, int en, int  __attribute__((unused))type)
{
	int err = 0;
	int flags = en ? 1 : 0;
	int what = -1;
	int mEnabledPrev;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (flags) {
		mEnabled |= (1<<what);
		writeMinDelay();

		if (mEnabled == (1<<what)) {
#if ((GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1) && (SENSORS_ACCELEROMETER_ENABLE == 1))
			acc->enable(SENSORS_GYROSCOPE_HANDLE, flags, 1);
			iNemoEngine_API_gbias_enable(flags);
#endif
#if !defined(NOT_SET_GYRO_INITIAL_STATE)
			setInitialState();
#endif
			err = writeEnable(SENSORS_GYROSCOPE_HANDLE, flags);
			if(err >= 0)
				err = 0;
		}

	} else {
		mEnabledPrev = mEnabled;
		mEnabled &= ~(1<<what);

		if ((!mEnabled) && (mEnabledPrev)){
			err = writeEnable(SENSORS_GYROSCOPE_HANDLE, flags);
			if(err >= 0)
				err = 0;

#if ((GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1) && (SENSORS_ACCELEROMETER_ENABLE == 1))
			acc->enable(SENSORS_GYROSCOPE_HANDLE, flags, 1);
			STLOGD("GyroSensor::Acc OFF");
			iNemoEngine_API_gbias_enable(false);
#endif

		}
		//setDelay(handle, DELAY_OFF);
		if (mEnabled) {
			writeMinDelay();
		}
	}

	if(err >= 0 ) {
		STLOGD("GyroSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);
	} else {
		STLOGE("GyroSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);
	}

	return err;
}

bool GyroSensor::hasPendingEvents() const
{
	return mHasPendingEvent;
}

int GyroSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int what = -1;
	int err = 0;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);

	if(delay_ms == 0)
		return err;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
  #if (SENSORS_ACCELEROMETER_ENABLE == 1)
	if (delay_ns >= 10000000)
		acc->setDelay(SENSORS_GYROSCOPE_HANDLE, delay_ns);
	else
		acc->setDelay(SENSORS_GYROSCOPE_HANDLE, 10000000);

  #endif
#endif

	/**
	 * The handled sensor is disabled. Set 0 in its setDelayBuffer position
	 * and update decimation buffer.
	 */
	if (delay_ms == NSEC_TO_MSEC(DELAY_OFF))
		delay_ms = 0;

	// Min setDelay Definition
	setDelayBuffer[what] = delay_ms;

#if (DEBUG_POLL_RATE == 1)
	STLOGD("GyroSensor::setDelayBuffer[] = %lld, %lld, %lld", setDelayBuffer[0], setDelayBuffer[1], setDelayBuffer[2]);
#endif

	// Update sysfs
	if(mEnabled & 1<<what)
	{
		writeMinDelay();
	}

	return err;
}

int GyroSensor::writeMinDelay(void)
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

	if ((Min_delay_ms > 0) && (Min_delay_ms != delayms))
	{
		samples_to_discard = (int)(GYRO_STARTUP_TIME_MS/Min_delay_ms)+1;
		startup_samples = samples_to_discard;
		err = writeDelay(SENSORS_GYROSCOPE_HANDLE, Min_delay_ms);
		if(err >= 0) {
			err = 0;
			delayms = Min_delay_ms;
			memset(DecimationCount, 0, sizeof(DecimationCount));
#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
			iNemoEngine_API_gbias_set_frequency(1000.0f /
							(float)Min_delay_ms);
  #if (SENSORS_ACCELEROMETER_ENABLE == 1)
			if (Min_delay_ms >= 10)
				acc->setDelay(SENSORS_GYROSCOPE_HANDLE,(float)Min_delay_ms*1000000);
			else
				acc->setDelay(SENSORS_GYROSCOPE_HANDLE,10000000);
  #endif
#endif
		}
	}

	// Decimation Definition
	for(kk = 0; kk < numSensors; kk++)
	{
		if (kk == Gyro || kk == GyroUncalib)
			continue;

		if (delayms)
			DecimationBuffer[kk] = writeDelayBuffer[kk]/delayms;
		else
			DecimationBuffer[kk] = 0;
	}

#if (DEBUG_POLL_RATE == 1)
	STLOGD("GyroSensor::writeDelayBuffer[] = %lld, %lld, %lld", writeDelayBuffer[0], writeDelayBuffer[1], writeDelayBuffer[2]);
	STLOGD("GyroSensor::Min_delay_ms = %lld, delayms = %lld, mEnabled = %d", Min_delay_ms, delayms, mEnabled);
	STLOGD("GyroSensor::samples_to_discard = %d", samples_to_discard);
	STLOGD("GyroSensor::DecimationBuffer = %d, %d, %d", DecimationBuffer[0], DecimationBuffer[1], DecimationBuffer[2]);
#endif

	return err;

}


void GyroSensor::getGyroDelay(int64_t *Gyro_Delay_ms)
{
	*Gyro_Delay_ms = delayms;

	return;
}


int GyroSensor::setFullScale(int32_t  __attribute__((unused))handle, int value)
{
	int err = -1;

	if(value <= 0)
		return err;
	else
		err = 0;

	if(value != current_fullscale)
	{
		err = writeFullScale(SENSORS_GYROSCOPE_HANDLE, value);
		if(err >= 0) {
			err = 0;
			current_fullscale = value;
		}
	}
	return err;
}

int GyroSensor::readEvents(sensors_event_t* data, int count)
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

		if (event->type == EVENT_TYPE_GYRO) {

#if (DEBUG_GYROSCOPE == 1)
	STLOGD("GyroSensor::readEvents (event_code=%d)", event->code);
#endif

			float value = (float) event->value;
			if (event->code == EVENT_TYPE_GYRO_X) {
				data_raw[0] = value * CONVERT_GYRO_X;
			}
			else if (event->code == EVENT_TYPE_GYRO_Y) {
				data_raw[1] = value * CONVERT_GYRO_Y;
			}
			else if (event->code == EVENT_TYPE_GYRO_Z) {
				data_raw[2] = value * CONVERT_GYRO_Z;
			}
#if defined(GYRO_EVENT_HAS_TIMESTAMP)
			else if (event->code == EVENT_TYPE_TIME_MSB) {
				timestamp = ((int64_t)(event->value)) << 32;
			}
			else if (event->code == EVENT_TYPE_TIME_LSB) {
				timestamp |= (uint32_t)(event->value);
			}
#endif
			else {
				STLOGE("GyroSensor: unknown event code (type = %d, code = %d)", event->type, event->code);
			}
		} else if (event->type == EV_SYN) {

			if (startup_samples) {
				startup_samples--;

#if (DEBUG_GYROSCOPE == 1)
				STLOGD("GyroSensor::Start-up samples = %d", startup_samples);
#endif
				goto no_data;
			}

			data_rot[0] = data_raw[0]*matrix_gyr[0][0] +
					data_raw[1]*matrix_gyr[1][0] +
					data_raw[2]*matrix_gyr[2][0];
			data_rot[1] = data_raw[0]*matrix_gyr[0][1] +
					data_raw[1]*matrix_gyr[1][1] +
					data_raw[2]*matrix_gyr[2][1];
			data_rot[2] = data_raw[0]*matrix_gyr[0][2] +
					data_raw[1]*matrix_gyr[1][2] +
					data_raw[2]*matrix_gyr[2][2];

#if defined(STORE_CALIB_GYRO_ENABLED)
			data_rot[0] -= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_BIAS,
					StoreCalibration::XAxis);
			data_rot[1] -= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_BIAS,
					StoreCalibration::YAxis);
			data_rot[2] -= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_BIAS,
					StoreCalibration::ZAxis);
			data_rot[0] *= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_SENS,
					StoreCalibration::XAxis);
			data_rot[1] *= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_SENS,
					StoreCalibration::YAxis);
			data_rot[2] *= pStoreCalibration->getCalibration(
					StoreCalibration::GYROSCOPE_SENS,
					StoreCalibration::ZAxis);
#endif

#if !defined(GYRO_EVENT_HAS_TIMESTAMP)
			timestamp = timevalToNano(event->time);
#endif

#if !(GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
			memset(gbias_out, 0, sizeof(gbias_out));
#if (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1)
			int bias_meas;
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			sensors_vec_t tmp_data_acc;
			AccelSensor::getBufferData(&tmp_data_acc);
			memcpy(data_acc, tmp_data_acc.v, sizeof(float) * 3);
#else
			memset(data_acc, 0, sizeof(data_acc));
#endif
			iNemoEngine_API_gbias_Run(data_acc, data_rot);
			iNemoEngine_API_Get_gbias(gbias_out);
#endif
			DecimationCount[Gyro]++;
			if(mEnabled & (1<<Gyro) && (DecimationCount[Gyro] >= DecimationBuffer[Gyro])) {
				DecimationCount[Gyro] = 0;
				mPendingEvent[Gyro].data[0] = data_rot[0] - gbias_out[0];
				mPendingEvent[Gyro].data[1] = data_rot[1] - gbias_out[1];
				mPendingEvent[Gyro].data[2] = data_rot[2] - gbias_out[2];
				mPendingEvent[Gyro].timestamp = timestamp;
				mPendingEvent[Gyro].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;

				*data++ = mPendingEvent[Gyro];
				count--;
				numEventReceived++;
			}

  #if ((SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1) && (GYROSCOPE_GBIAS_ESTIMATION_STANDALONE == 1))
			DecimationCount[GyroUncalib]++;
			if(mEnabled & (1<<GyroUncalib) && (DecimationCount[GyroUncalib] >= DecimationBuffer[GyroUncalib])) {
				DecimationCount[GyroUncalib] = 0;
				mPendingEvent[GyroUncalib].uncalibrated_gyro.uncalib[0] = data_rot[0];
				mPendingEvent[GyroUncalib].uncalibrated_gyro.uncalib[1] = data_rot[1];
				mPendingEvent[GyroUncalib].uncalibrated_gyro.uncalib[2] = data_rot[2];
				mPendingEvent[GyroUncalib].uncalibrated_gyro.bias[0] = gbias_out[0];
				mPendingEvent[GyroUncalib].uncalibrated_gyro.bias[1] = gbias_out[1];
				mPendingEvent[GyroUncalib].uncalibrated_gyro.bias[2] = gbias_out[2];
				mPendingEvent[GyroUncalib].timestamp = timestamp;
				mPendingEvent[GyroUncalib].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;

				*data++ = mPendingEvent[GyroUncalib];
				count--;
				numEventReceived++;
			}
  #endif
#endif
			if(mEnabled & (1<<iNemoGyro)) {
				sensors_vec_t sData;
				sData.x = data_rot[0] - gbias_out[0];
				sData.y = data_rot[1] - gbias_out[1];
				sData.z = data_rot[2] - gbias_out[2];
				setBufferData(&sData);
			}

#if (DEBUG_GYROSCOPE == 1)
			STLOGD("GyroSensor::readEvents (time = %lld), count(%d), received(%d)",
						mPendingEvent[Gyro].timestamp,
						count, numEventReceived);
#endif

		} else {
			STLOGE("GyroSensor: unknown event (type=%d, code=%d)",
						event->type, event->code);
		}
no_data:
		mInputReader.next();
	}

#if (FETCH_FULL_EVENT_BEFORE_RETURN)
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

bool GyroSensor::setBufferData(sensors_vec_t *value)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(&dataBuffer, value, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

	return true;
}

bool GyroSensor::getBufferData(sensors_vec_t *lastBufferedValues)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(lastBufferedValues, &dataBuffer, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

#if (DEBUG_GYROSCOPE == 1)
	STLOGD("GyroSensor: getBufferData got values: x:(%f),y:(%f), z:(%f).",
						lastBufferedValues->x,
						lastBufferedValues->y,
						lastBufferedValues->z);
#endif

	return true;
}

#endif /* SENSORS_GYROSCOPE_ENABLE */
