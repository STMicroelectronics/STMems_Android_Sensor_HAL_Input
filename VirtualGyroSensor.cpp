/*
 * Copyright (C) 2014 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Alberto Marinoni, Giuseppe Barba
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
#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <string.h>

#include "VirtualGyroSensor.h"

#define DEG_TO_RAD(x)		(x * 3.14f / 180.0f)

/****************************************************************************/

sensors_vec_t  VirtualGyroSensor::dataBuffer;
int VirtualGyroSensor::mEnabled = 0;
int64_t VirtualGyroSensor::delayms = 0;
int VirtualGyroSensor::startup_samples = 0;
int VirtualGyroSensor::current_fullscale = 0;
int VirtualGyroSensor::samples_to_discard = 0;
int64_t VirtualGyroSensor::MagDelay_ms = MAG_DEFAULT_DELAY;
int64_t VirtualGyroSensor::setDelayBuffer[numSensors] = {0};
int VirtualGyroSensor::DecimationBuffer[numSensors] = {0};
int VirtualGyroSensor::DecimationCount = 0;
pthread_mutex_t VirtualGyroSensor::dataMutex;

VirtualGyroSensor::VirtualGyroSensor()
	: SensorBase(NULL, SENSOR_DATANAME_MAGNETIC_FIELD),
	mInputReader(4),
	mHasPendingEvent(false)
{
	pthread_mutex_init(&dataMutex, NULL);

	memset(mPendingEvent, 0, sizeof(mPendingEvent));
	mPendingEvent[VirtualGyro].version = sizeof(sensors_event_t);
	mPendingEvent[VirtualGyro].sensor = ID_VIRTUAL_GYROSCOPE;
	mPendingEvent[VirtualGyro].type = SENSOR_TYPE_GYROSCOPE;
	mPendingEvent[VirtualGyro].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;
	memset(gyro, 0, sizeof(gyro));

	if (data_fd) {
		STLOGI("VirtualGyroSensor::VirtualGyroSensor main driver"
			" device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("VirtualGyroSensor::VirtualGyroSensor main driver"
			" device_sysfs_path:(%s) not found", sysfs_device_path);
	}

	/* GeoMag library execution here only if GeoMag sensors are disabled */
	mag = new MagnSensor();
	acc = new AccelSensor();
#if (SENSOR_GEOMAG_ENABLE == 0)
	iNemoEngine_GeoMag_API_Initialization(100);
#endif
}

VirtualGyroSensor::~VirtualGyroSensor()
{
	if (mEnabled) {
		enable(SENSORS_VIRTUAL_GYROSCOPE_HANDLE, 0, 0);
	}
	pthread_mutex_destroy(&dataMutex);
	acc->~SensorBase();
	mag->~SensorBase();
}

int VirtualGyroSensor::setInitialState()
{
	mHasPendingEvent = true;
	setFullScale(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
		     VIRTUAL_GYRO_DEFAULT_FULLSCALE);
	startup_samples = samples_to_discard;

	return 0;
}

int VirtualGyroSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
		case SENSORS_VIRTUAL_GYROSCOPE_HANDLE:
			what = VirtualGyro;
			break;
			#if SENSOR_FUSION_ENABLE == 1
		case SENSORS_SENSOR_FUSION_HANDLE:
			what = iNemoGyro;
			break;
			#endif
		default:
			what = -1;
	}

	return what;
}

int VirtualGyroSensor::enable(int32_t handle, int en, int type)
{
	int err = 0;
	int flags = en ? 1 : 0;
	int what = -1;
	int mEnabledPrev;

#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	if (VirtualGyroSensor::acc->getFd() <= 0)
		return -1;
#endif

#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	if (VirtualGyroSensor::mag->getFd() <= 0)
		return -1;
#endif

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (flags) {
		if (!mEnabled) {
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
 			mag->enable(SENSORS_VIRTUAL_GYROSCOPE_HANDLE, flags, 1);
 			mag->setFullScale(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
 							MAG_DEFAULT_RANGE);
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
 			acc->enable(SENSORS_VIRTUAL_GYROSCOPE_HANDLE, flags, 1);
 			acc->setFullScale(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
 							ACC_DEFAULT_RANGE);
			setInitialState();
#endif
		}
		mEnabled |= (1 << what);
	} else {

		mEnabledPrev = mEnabled;
		mEnabled &= ~(1 << what);
		if (!mEnabled && mEnabledPrev) {
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
  			mag->setFullScale(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
  							MAG_DEFAULT_RANGE);
 			mag->enable(SENSORS_VIRTUAL_GYROSCOPE_HANDLE, flags, 1);
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
  			acc->setFullScale(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
  							ACC_DEFAULT_RANGE);
 			acc->enable(SENSORS_VIRTUAL_GYROSCOPE_HANDLE, flags, 1);
#endif
		}
		setDelay(handle, DELAY_OFF);
	}

	if(err >= 0 ) {
		STLOGD("VirtualGyroSensor::enable(%d), handle: %d, what: %d,"
				" mEnabled: %x",flags, handle, what, mEnabled);
	} else {
		STLOGE("VirtualGyroSensor::enable(%d), handle: %d, what: %d,"
				" mEnabled: %x",flags, handle, what, mEnabled);
	}

	return err;
}

bool VirtualGyroSensor::hasPendingEvents() const
{
	return mHasPendingEvent;
}

int VirtualGyroSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int what = -1, err;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);
	int64_t Min_delay_ms = 0;

	if(delay_ms == 0)
		return -1;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	/**
	 * The handled sensor is disabled. Set 0 in its setDelayBuffer position
	 * and update decimation buffer.
	 */
	if (delay_ms == NSEC_TO_MSEC(DELAY_OFF))
	{
		delay_ms = 0;
		Min_delay_ms = 0;
	}
	else {
		if (delay_ms < VGYRO_DEFAULT_DELAY)
			Min_delay_ms = delay_ms;
		else
			Min_delay_ms = VGYRO_DEFAULT_DELAY;
	}
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	err = mag->setDelay(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
			    (int64_t)MSEC_TO_NSEC(Min_delay_ms));
	if(err < 0)
		return -1;
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	err = acc->setDelay(SENSORS_VIRTUAL_GYROSCOPE_HANDLE,
			    (int64_t)MSEC_TO_NSEC(Min_delay_ms));
	if(err < 0)
		return -1;
#endif

	/** Min setDelay Definition */
	setDelayBuffer[what] = delay_ms;
	updateDecimations(MagDelay_ms);

	return 0;
}

void VirtualGyroSensor::updateDecimations(int64_t delayms)
{
	int kk;

	if (delayms) {
		samples_to_discard = (int)(GYRO_STARTUP_TIME_MS/delayms)+1;
		startup_samples = samples_to_discard;
	}

	/** Decimation Definition */
	for(kk = 0; kk < numSensors; kk++)
	{
		if (delayms)
			DecimationBuffer[kk] = setDelayBuffer[kk] / delayms;
		else
			DecimationBuffer[kk] = 0;
	}

#if (DEBUG_POLL_RATE == 1)
	STLOGD("VirtualGyroSensor::setDelayBuffer[] = %lld, %lld, %lld",
			setDelayBuffer[0], setDelayBuffer[1], setDelayBuffer[2]);
	STLOGD("VirtualGyroSensor::delayms = %lld, "
			"mEnabled = %d", delayms, mEnabled);
	STLOGD("VirtualGyroSensor::samples_to_discard = %d",
			samples_to_discard);
	STLOGD("VirtualGyroSensor::DecimationBuffer = %d, %d, %d",
	       DecimationBuffer[0], DecimationBuffer[1], DecimationBuffer[2]);
#endif

}


void VirtualGyroSensor::getGyroDelay(int64_t *Gyro_Delay_ms)
{
	*Gyro_Delay_ms = delayms;
}


int VirtualGyroSensor::setFullScale(int32_t handle, int value)
{
	if(value <= 0)
		return -1;

	if(value != current_fullscale)
	{
		/** The full scale value is not used right now. */
		current_fullscale = value;
	}

	return 0;
}

int VirtualGyroSensor::readEvents(sensors_event_t* data, int count)
{
	int numEventReceived = 0, deltatime = 0;
	input_event const* event;
	iNemoGeoMagSensorsData sdata;
	static int64_t pre_time = -1;
	int64_t cur_time = 0;
	float tmp[3];
	int64_t newMagDelay_ms = MAG_DEFAULT_DELAY;

	if (count < 1)
		return -EINVAL;

	if (mHasPendingEvent) {
		mHasPendingEvent = false;
	}

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

	while (count && mInputReader.readEvent(&event)) {
		if (event->type == EV_SYN) {
			if (startup_samples) {
				startup_samples--;

#if (DEBUG_VIRTUAL_GYROSCOPE == 1)
				STLOGD("VirtualGyroSensor::Start-up samples = %d",
				       startup_samples);
#endif
				goto no_data;
			}


			VirtualGyroSensor::mag->getMagDelay(&newMagDelay_ms);
			if((newMagDelay_ms != MagDelay_ms) && mEnabled) {
				updateDecimations(newMagDelay_ms);
				MagDelay_ms = newMagDelay_ms;
			}


#if (SENSOR_GEOMAG_ENABLE == 0)
			/* GeoMag library execution here only if GeoMag sensors are disabled */
			AccelSensor::getBufferData(&mSensorsBufferedVectors[Acceleration]);
			MagnSensor::getBufferData(&mSensorsBufferedVectors[MagneticField]);

			/** Copy accelerometer data [m/s^2] */
			memcpy(sdata.accel, mSensorsBufferedVectors[Acceleration].v, sizeof(float) * 3);

			/** Copy magnetometer data [uT] */
			memcpy(sdata.magn, mSensorsBufferedVectors[MagneticField].v, sizeof(float) * 3);

			cur_time = timevalToNano(event->time);
			if (pre_time > 0)
				deltatime = (int)NSEC_TO_MSEC(cur_time - pre_time);
			pre_time = cur_time;
			deltatime = (deltatime == 0) ? mag->getDelayms() : deltatime;

			iNemoEngine_GeoMag_API_Run(deltatime, &sdata);
#endif
			iNemoEngine_GeoMag_API_Get_VirtualGyro(gyro);

			DecimationCount++;

			if(mEnabled & (1<<VirtualGyro) && (DecimationCount
				>= DecimationBuffer[VirtualGyro])) {
				/** Downsample VirtualGyro output */
				DecimationCount = 0;
				mPendingEvent[VirtualGyro].data[0] = gyro[0];
				mPendingEvent[VirtualGyro].data[1] = gyro[1];
				mPendingEvent[VirtualGyro].data[2] = gyro[2];
				mPendingEvent[VirtualGyro].timestamp =
						timevalToNano(event->time);
				mPendingEvent[VirtualGyro].gyro.status =
						SENSOR_STATUS_ACCURACY_HIGH;

				*data++ = mPendingEvent[VirtualGyro];
				count--;
				numEventReceived++;
			}

			if(mEnabled & (1 << iNemoGyro)) {
				/** Save axis data for iNemo library */
				sensors_vec_t sData;
				sData.x = gyro[0];
				sData.y = gyro[1];
				sData.z = gyro[2];
				setBufferData(&sData);
			}

#if (DEBUG_VIRTUAL_GYROSCOPE == 1)
			STLOGD("VirtualGyroSensor::readEvents (time = %lld),"
			" count(%d), received(%d)",
				mPendingEvent[VirtualGyro].timestamp, count,
				numEventReceived);
#endif
		}
no_data:
		mInputReader.next();
	}
	return numEventReceived;
}

bool VirtualGyroSensor::setBufferData(sensors_vec_t *value)
{
	pthread_mutex_lock(&dataMutex);
	dataBuffer.x = value->x;
	dataBuffer.y = value->y;
	dataBuffer.z = value->z;
	pthread_mutex_unlock(&dataMutex);

	return true;
}

bool VirtualGyroSensor::getBufferData(sensors_vec_t *lastBufferedValues)
{
	pthread_mutex_lock(&dataMutex);
	lastBufferedValues->x = dataBuffer.x;
	lastBufferedValues->y = dataBuffer.y;
	lastBufferedValues->z = dataBuffer.z;
	pthread_mutex_unlock(&dataMutex);
#if (DEBUG_VIRTUAL_GYROSCOPE == 1)
	STLOGD("VirtualGyroSensor: getBufferData got values: x:(%f),"
		"y:(%f), z:(%f).", lastBufferedValues->x,
		lastBufferedValues->y, lastBufferedValues->z);
#endif
	return true;
}

#endif /* SENSORS_GYROSCOPE_ENABLE */
