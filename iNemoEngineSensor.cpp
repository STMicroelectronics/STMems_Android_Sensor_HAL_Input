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
#if (SENSOR_FUSION_ENABLE == 1)

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <linux/time.h>
#include <string.h>
#include "iNemoEngineSensor.h"

#define FETCH_FULL_EVENT_BEFORE_RETURN		0

/*****************************************************************************/
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
MagnSensor* iNemoEngineSensor::mag = NULL;
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
AccelSensor* iNemoEngineSensor::acc = NULL;
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
GyroSensor* iNemoEngineSensor::gyr = NULL;
#endif
int iNemoEngineSensor::status = 0;
int iNemoEngineSensor::mEnabled = 0;
int iNemoEngineSensor::startup_samples = DEFAULT_SAMPLES_TO_DISCARD;
int iNemoEngineSensor::samples_to_discard = DEFAULT_SAMPLES_TO_DISCARD;
int iNemoEngineSensor::DecimationBuffer[numSensors] = {0};
int iNemoEngineSensor::DecimationCount[numSensors] = {0};
int64_t iNemoEngineSensor::DelayBuffer[numSensors] = {0};
int64_t iNemoEngineSensor::gyroDelay_ms = GYR_DEFAULT_DELAY;

struct timespec old_time, new_time;


int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
	int64_t timeAft;
	int64_t timeBef;
	int64_t timeDiff;

	timeBef = (int64_t)((int64_t)(timeB_p->tv_sec * 1000000000L) + timeB_p->tv_nsec);
	timeAft = (int64_t)((int64_t)(timeA_p->tv_sec * 1000000000L) + timeA_p->tv_nsec);

	if (timeAft > timeBef)
		timeDiff = timeAft - timeBef;
	else
		timeDiff = timeBef - timeAft;

	return (int64_t)timeDiff;
}

iNemoEngineSensor::iNemoEngineSensor()
#if (!SENSORS_GYROSCOPE_ENABLE && SENSORS_VIRTUAL_GYROSCOPE_ENABLE)
        : SensorBase(NULL, SENSOR_DATANAME_ACCELEROMETER),
#else
        : SensorBase(NULL, SENSOR_DATANAME_GYROSCOPE),
#endif
	mPendingMask(0),
	mInputReader(4),
	mHasPendingEvent(false)
{
	memset(mPendingEvents, 0, sizeof(mPendingEvents));
	memset(mSensorsBufferedVectors, 0, sizeof(sensors_vec_t) * 3);
	memset(DecimationCount, 0, sizeof(DecimationCount));

#if (SENSORS_ORIENTATION_ENABLE == 1)
	mPendingEvents[Orientation].version = sizeof(sensors_event_t);
	mPendingEvents[Orientation].sensor = ID_ORIENTATION;
	mPendingEvents[Orientation].type = SENSOR_TYPE_ORIENTATION;
	mPendingEvents[Orientation].orientation.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif
#if (SENSORS_GRAVITY_ENABLE == 1)
	mPendingEvents[Gravity].version = sizeof(sensors_event_t);
	mPendingEvents[Gravity].sensor = ID_GRAVITY;
	mPendingEvents[Gravity].type = SENSOR_TYPE_GRAVITY;
	mPendingEvents[Gravity].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif
#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
	mPendingEvents[LinearAcceleration].version = sizeof(sensors_event_t);
	mPendingEvents[LinearAcceleration].sensor = ID_LINEAR_ACCELERATION;
	mPendingEvents[LinearAcceleration].type = SENSOR_TYPE_LINEAR_ACCELERATION;
	mPendingEvents[LinearAcceleration].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif
#if (SENSORS_ROTATION_VECTOR_ENABLE == 1)
	mPendingEvents[RotationMatrix].version = sizeof(sensors_event_t);
	mPendingEvents[RotationMatrix].sensor = ID_ROTATION_VECTOR;
	mPendingEvents[RotationMatrix].type = SENSOR_TYPE_ROTATION_VECTOR;
#endif
#if (SENSORS_GAME_ROTATION_ENABLE == 1)
	mPendingEvents[GameRotation].version = sizeof(sensors_event_t);
	mPendingEvents[GameRotation].sensor = ID_GAME_ROTATION;
	mPendingEvents[GameRotation].type = SENSOR_TYPE_GAME_ROTATION_VECTOR;
#endif
#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
  #if (SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1)
	mPendingEvents[UncalibGyro].version = sizeof(sensors_event_t);
	mPendingEvents[UncalibGyro].sensor = ID_UNCALIB_GYROSCOPE;
	mPendingEvents[UncalibGyro].type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
	mPendingEvents[UncalibGyro].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;
  #endif
	mPendingEvents[CalibGyro].version = sizeof(sensors_event_t);
	mPendingEvents[CalibGyro].sensor = ID_GYROSCOPE;
	mPendingEvents[CalibGyro].type = SENSOR_TYPE_GYROSCOPE;
	mPendingEvents[CalibGyro].gyro.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif

	if (data_fd) {
		STLOGI("iNemoSensor::iNemoSensor main driver device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("iNemoSensor::iNemoSensor main driver device_sysfs_path:(%s) not found", sysfs_device_path);
	}

	init_data_api.GbiasLearningMode = 2;
	init_data_api.ATime = -1;
	init_data_api.MTime = -1;
	init_data_api.PTime = -1;
	init_data_api.FrTime = -1;
	init_data_api.gbias_file = NULL;
	init_data_api.LocalEarthMagField = 50.0f;
	init_data_api.Gbias_threshold_magn = 1200e-6;
	init_data_api.Gbias_threshold_accel = 1200e-6;
	init_data_api.Gbias_threshold_gyro = 1200e-6;

	debug_init_data_api.accel_flag = 0;
	debug_init_data_api.magn_flag = 0;
	debug_init_data_api.gyro_flag = 0;

#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	init_data_api.Gbias_threshold_accel = ACC_GBIAS_THRESHOLD;
	debug_init_data_api.accel_flag = 1;
	iNemoEngineSensor::acc = new AccelSensor();
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	init_data_api.Gbias_threshold_magn = MAG_GBIAS_THRESHOLD;
	debug_init_data_api.magn_flag = 1;
	iNemoEngineSensor::mag = new MagnSensor();
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
	debug_init_data_api.gyro_flag = 1;
	init_data_api.Gbias_threshold_gyro = GYR_GBIAS_THRESHOLD;
	iNemoEngineSensor::gyr = new GyroSensor();
#endif
	if (iNemoEngine_API_Initialization(&init_data_api, &debug_init_data_api) < 0)
		STLOGE("iNemoSensor:: Failed to initialize iNemoEngineAPI library");
}

iNemoEngineSensor::~iNemoEngineSensor()
{
#if (SENSORS_GYROSCOPE_ENABLE == 1)
	iNemoEngineSensor::gyr->~GyroSensor();
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	iNemoEngineSensor::mag->~MagnSensor();
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	iNemoEngineSensor::acc->~AccelSensor();
#endif
}

int iNemoEngineSensor::setInitialState()
{
	startup_samples = samples_to_discard;

	return 0;
}

int iNemoEngineSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {
#if (SENSORS_ORIENTATION_ENABLE == 1)
		case SENSORS_ORIENTATION_HANDLE:
			what = Orientation;
			break;
#endif
#if (SENSORS_GRAVITY_ENABLE == 1)
		case SENSORS_GRAVITY_HANDLE:
			what = Gravity;
			break;
#endif
#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
		case SENSORS_LINEAR_ACCELERATION_HANDLE:
			what = LinearAcceleration;
			break;
#endif
#if (SENSORS_ROTATION_VECTOR_ENABLE == 1)
		case SENSORS_ROTATION_VECTOR_HANDLE:
			what = RotationMatrix;
			break;
#endif
#if (SENSORS_GAME_ROTATION_ENABLE == 1)
		case SENSORS_GAME_ROTATION_HANDLE:
			what = GameRotation;
			break;
#endif
#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
  #if (SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1)
		case SENSORS_UNCALIB_GYROSCOPE_HANDLE:
			what = UncalibGyro;
			break;
  #endif
		case SENSORS_GYROSCOPE_HANDLE:
			what = CalibGyro;
			break;
#endif
		default:
			what = -1;
			break;
	}

	return what;
}

int iNemoEngineSensor::enable(int32_t handle, int en, int  __attribute__((unused))type)
{
	int err = 0;
	int what = -1;
	static int enabled = 0;

#if (SENSORS_GYROSCOPE_ENABLE == 1)
	if (iNemoEngineSensor::gyr->getFd() <= 0)
		return -1;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
	if (handle != SENSORS_GAME_ROTATION_HANDLE) {
		if (iNemoEngineSensor::mag->getFd() <= 0)
			return -1;
	}
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	if (iNemoEngineSensor::acc->getFd() <= 0)
		return -1;
#endif

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if(en) {
		if(mEnabled == 0) {
			enabled = 1;

#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
			if (handle != SENSORS_GAME_ROTATION_HANDLE) {
				iNemoEngineSensor::mag->enable(SENSORS_SENSOR_FUSION_HANDLE, 1, 1);
				iNemoEngineSensor::mag->setFullScale(SENSORS_SENSOR_FUSION_HANDLE, MAG_DEFAULT_RANGE);
			}
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
			iNemoEngineSensor::gyr->enable(SENSORS_SENSOR_FUSION_HANDLE, 1, 1);
			iNemoEngineSensor::gyr->setFullScale(SENSORS_SENSOR_FUSION_HANDLE, GYRO_DEFAULT_RANGE);
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			iNemoEngineSensor::acc->enable(SENSORS_SENSOR_FUSION_HANDLE, 1, 1);
			iNemoEngineSensor::acc->setFullScale(SENSORS_SENSOR_FUSION_HANDLE, ACC_DEFAULT_RANGE);
#endif

		}
		mEnabled |= (1<<what);
	} else {
		int tmp = mEnabled;
		mEnabled &= ~(1<<what);
		if((mEnabled == 0)&&(tmp != 0)) {
#if (SENSORS_GYROSCOPE_ENABLE == 1)
			iNemoEngineSensor::gyr->setFullScale(SENSORS_SENSOR_FUSION_HANDLE, GYRO_DEFAULT_RANGE);
			iNemoEngineSensor::gyr->enable(SENSORS_SENSOR_FUSION_HANDLE, 0, 1);
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
			if (handle != SENSORS_GAME_ROTATION_HANDLE) {
				iNemoEngineSensor::mag->setFullScale(SENSORS_SENSOR_FUSION_HANDLE, MAG_DEFAULT_RANGE);
				iNemoEngineSensor::mag->enable(SENSORS_SENSOR_FUSION_HANDLE, 0, 1);
			}
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			iNemoEngineSensor::acc->enable(SENSORS_SENSOR_FUSION_HANDLE, 0, 1);
#endif
		}
		setDelay(handle, DELAY_OFF);
	}

	if ((handle == SENSORS_GAME_ROTATION_HANDLE) ||
		(handle == SENSORS_GYROSCOPE_HANDLE) ||
		(handle == SENSORS_UNCALIB_GYROSCOPE_HANDLE))
	{
		if ((mEnabled & (1<<GameRotation)) ||
		    (mEnabled & (1<<UncalibGyro)) ||
		    (mEnabled & (1<<CalibGyro)))
		{
				iNemoEngine_API_enable6X(true);
#if (DEBUG_INEMO_SENSOR == 1)
				STLOGD( "iNemoEngineSensor: Fusion 6axis enabled");
#endif
		} else {
				iNemoEngine_API_enable6X(false);
#if (DEBUG_INEMO_SENSOR == 1)
				STLOGD( "iNemoEngineSensor: Fusion 6axis disabled");
#endif
		}
	} else {
		if ((mEnabled & (1<<Orientation)) ||
			(mEnabled & (1<<Gravity)) ||
			(mEnabled & (1<<LinearAcceleration)) ||
			(mEnabled & (1<<RotationMatrix)))
		{
			iNemoEngine_API_enable9X(true);
#if (DEBUG_INEMO_SENSOR == 1)
			STLOGD( "iNemoEngineSensor: Fusion 9axis enabled");
#endif
		} else {
			iNemoEngine_API_enable9X(false);
#if (DEBUG_INEMO_SENSOR == 1)
			STLOGD( "iNemoEngineSensor: Fusion 9axis disabled");
#endif
		}
	}

	if (enabled) {
		enabled = 0;
		setInitialState();
		clock_gettime(CLOCK_MONOTONIC, &old_time);
	}

	return err;
}

bool iNemoEngineSensor::hasPendingEvents() const
{
	return mHasPendingEvent;
}

int iNemoEngineSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int err;
	int what = -1;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);
	int64_t gyr_delay_ms;
	int64_t mag_delay_ms;
	int64_t acc_delay_ms;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (delay_ms == NSEC_TO_MSEC(DELAY_OFF)) {
		delay_ms = 0;
		gyr_delay_ms = 0;
		mag_delay_ms = 0;
		acc_delay_ms = 0;
	}
	else{
#if (SENSORS_GYROSCOPE_ENABLE == 1)
		gyr_delay_ms = (int64_t)MSEC_TO_NSEC(GYR_DEFAULT_DELAY);
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		mag_delay_ms = (int64_t)MSEC_TO_NSEC(MAG_DEFAULT_DELAY);
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		acc_delay_ms = (int64_t)MSEC_TO_NSEC(ACC_DEFAULT_DELAY);
#endif
	}


#if (SENSORS_GYROSCOPE_ENABLE == 1)
		err = iNemoEngineSensor::gyr->setDelay(SENSORS_SENSOR_FUSION_HANDLE, gyr_delay_ms);
		if(err < 0)
			return -1;
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
		if (handle != SENSORS_GAME_ROTATION_HANDLE) {
			err = iNemoEngineSensor::mag->setDelay(SENSORS_SENSOR_FUSION_HANDLE, mag_delay_ms);
			if(err < 0)
				return -1;
		}
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
		err = iNemoEngineSensor::acc->setDelay(SENSORS_SENSOR_FUSION_HANDLE, acc_delay_ms);
		if(err < 0)
			return -1;
#endif
	/**
	 * The handled sensor is disabled. Set 0 in its setDelayBuffer position
	 * and update decimation buffer.
	 */

	DelayBuffer[what] = delay_ms;
	updateDecimations(gyroDelay_ms);

	return 0;
}

void iNemoEngineSensor::updateDecimations(int64_t delayms)
{
	int kk;

	if (delayms) {
		samples_to_discard = (int)(GYRO_STARTUP_TIME_MS/delayms)+1;
		startup_samples = samples_to_discard;
	}

	// Decimation Definition
	for(kk = 0; kk < numSensors; kk++)
	{
		if (delayms)
			DecimationBuffer[kk] = DelayBuffer[kk]/delayms;
		else
			DecimationBuffer[kk] = 0;
	}

	memset(DecimationCount, 0, sizeof(DecimationCount));

#if (DEBUG_POLL_RATE == 1)
	STLOGD("iNemo::Gyro Delay = %lld", delayms);
	STLOGD("iNemo::DelayBuffer = %lld, %lld, %lld", DelayBuffer[3], DelayBuffer[4], DelayBuffer[5]);
	STLOGD("iNemo::DelayBuffer = %lld, %lld, %lld", DelayBuffer[6], DelayBuffer[7], DelayBuffer[8]);
	STLOGD("iNemo::DelayBuffer = %lld", DelayBuffer[9]);
	STLOGD("iNemo::DecimationBuffer = %d, %d, %d", DecimationBuffer[3], DecimationBuffer[4], DecimationBuffer[5]);
	STLOGD("iNemo::DecimationBuffer = %d, %d, %d", DecimationBuffer[6], DecimationBuffer[7], DecimationBuffer[8]);
	STLOGD("iNemo::DecimationBuffer = %d", DecimationBuffer[9]);
#endif
}

int iNemoEngineSensor::readEvents(sensors_event_t *data, int count)
{
	static int cont = 0;
	iNemoSensorsData sdata;
	int64_t timeElapsed;
	int64_t newGyroDelay_ms = GYR_DEFAULT_DELAY;
	int err;
	int numEventReceived = 0;
	input_event const* event;

#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
	float gbias[3];
#endif

	if (count < 1)
		return -EINVAL;

	if (mHasPendingEvent) {
		mHasPendingEvent = false;
	}

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

#if (FETCH_FULL_EVENT_BEFORE_RETURN)
	again:
#endif

	while (count && mInputReader.readEvent(&event)) {
#if (defined(GYRO_EVENT_HAS_TIMESTAMP) || defined(ACC_EVENT_HAS_TIMESTAMP))
  #if (!SENSORS_GYROSCOPE_ENABLE && SENSORS_VIRTUAL_GYROSCOPE_ENABLE)
		if (event->type == EVENT_TYPE_ACCEL) {
  #else
		if (event->type == EVENT_TYPE_GYRO) {
  #endif
			if (event->code == EVENT_TYPE_TIME_MSB) {
				timestamp = ((int64_t)(event->value)) << 32;
			}
			else if (event->code == EVENT_TYPE_TIME_LSB) {
				timestamp |= (uint32_t)(event->value);
			}
		}
#endif
		if(event->type == EV_SYN) {

			if (startup_samples) {
				startup_samples--;
#if (DEBUG_INEMO_SENSOR == 1)
				STLOGD("iNemo::Start-up samples = %d", startup_samples);
#endif
				goto no_data;
			}
#if !(defined(GYRO_EVENT_HAS_TIMESTAMP) || defined(ACC_EVENT_HAS_TIMESTAMP))
	timestamp = timevalToNano(event->time);
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
	iNemoEngineSensor::gyr->getGyroDelay(&newGyroDelay_ms);
#else
	iNemoEngineSensor::acc->getAccDelay(&newGyroDelay_ms);
#endif

	if((newGyroDelay_ms != gyroDelay_ms) && mEnabled) {
		updateDecimations(newGyroDelay_ms);
		gyroDelay_ms = newGyroDelay_ms;
	}

#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			AccelSensor::getBufferData(&mSensorsBufferedVectors[Acceleration]);
#endif
#if (SENSORS_GYROSCOPE_ENABLE == 1)
			GyroSensor::getBufferData(&mSensorsBufferedVectors[AngularSpeed]);
#endif
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)
			MagnSensor::getBufferData(&mSensorsBufferedVectors[MagneticField]);
#else
			/* Constant Magnetometer module is passed to the Library when Mag is disabled */
			mSensorsBufferedVectors[MagneticField].v[0] = 0.0f;
			mSensorsBufferedVectors[MagneticField].v[1] = 0.7f;
			mSensorsBufferedVectors[MagneticField].v[2] = 0.7f;
#endif

			if (mEnabled & ((1<<Orientation) | (1<<Gravity) |
					(1<<LinearAcceleration) | (1<<GameRotation) |
					(1<<UncalibGyro) | (1<<RotationMatrix) |
					(1<<CalibGyro))) {
				/** Copy accelerometer data [m/s^2] */
				memcpy(sdata.accel, mSensorsBufferedVectors[Acceleration].v, sizeof(float) * 3);

				/** Copy magnetometer data [uT] */
				memcpy(sdata.magn, mSensorsBufferedVectors[MagneticField].v, sizeof(float) * 3);

				/** Copy gyroscope data [rad/sec] */
				memcpy(sdata.gyro, mSensorsBufferedVectors[AngularSpeed].v, sizeof(float) * 3);

#if (DEBUG_INEMO_SENSOR == 1)
				STLOGD("Acc_x=%f [m/s^2], Acc_y=%f [m/s^2], Acc_z=%f [m/s^2]", sdata.accel[0], sdata.accel[1], sdata.accel[2]);
				STLOGD("Mag_x=%f [uT], Mag_y=%f [uT], Mag_z=%f [uT]", sdata.magn[0], sdata.magn[1], sdata.magn[2]);
				STLOGD("Gyr_x=%f [rad/sec], Gyr_y=%f [rad/sec], Gyr_z=%f [rad/sec]", sdata.gyro[0], sdata.gyro[1], sdata.gyro[2]);
#endif
				clock_gettime(CLOCK_MONOTONIC, &new_time);
				timeElapsed = timespecDiff(&new_time, &old_time);
				if (timeElapsed > (3 * MSEC_TO_NSEC(GYR_DEFAULT_DELAY)))
					timeElapsed = MSEC_TO_NSEC(GYR_DEFAULT_DELAY);

				iNemoEngine_API_Run(timeElapsed, &sdata);
				old_time = new_time;
#if (SENSORS_ORIENTATION_ENABLE == 1)
				DecimationCount[Orientation]++;
				if(mEnabled & (1<<Orientation) && (DecimationCount[Orientation] >= DecimationBuffer[Orientation])) {
					DecimationCount[Orientation] = 0;
					err = iNemoEngine_API_Get_Euler_Angles(mPendingEvents[Orientation].data);
					if (err != 0) {
						goto no_data;
					}

					mPendingEvents[Orientation].orientation.status = mSensorsBufferedVectors[MagneticField].status;
					mPendingMask |= 1<<Orientation;
  #if (DEBUG_INEMO_SENSOR == 1)
					STLOGD("time =  %lld, menabled = %d, orientation = %f", timeElapsed, mEnabled, mPendingEvents[Orientation].data);
  #endif
				}
#endif
#if (SENSORS_GRAVITY_ENABLE == 1)
				DecimationCount[Gravity]++;
				if(mEnabled & (1<<Gravity) && (DecimationCount[Gravity] >= DecimationBuffer[Gravity])) {
					DecimationCount[Gravity] = 0;
					err = iNemoEngine_API_Get_Gravity(mPendingEvents[Gravity].data);
					if (err != 0)
						goto no_data;

					mPendingMask |= 1<<Gravity;
				}
#endif
#if (SENSORS_LINEAR_ACCELERATION_ENABLE == 1)
				DecimationCount[LinearAcceleration]++;
				if(mEnabled & (1<<LinearAcceleration) && (DecimationCount[LinearAcceleration] >= DecimationBuffer[LinearAcceleration])) {
					DecimationCount[LinearAcceleration] = 0;
					err = iNemoEngine_API_Get_Linear_Acceleration(mPendingEvents[LinearAcceleration].data);
					if (err != 0)
						goto no_data;

					mPendingMask |= 1<<LinearAcceleration;
				}
#endif
#if (SENSORS_ROTATION_VECTOR_ENABLE == 1)
				DecimationCount[RotationMatrix]++;
				if(mEnabled & (1<<RotationMatrix) && (DecimationCount[RotationMatrix] >= DecimationBuffer[RotationMatrix])) {
					DecimationCount[RotationMatrix] = 0;
					err = iNemoEngine_API_Get_Quaternion(mPendingEvents[RotationMatrix].data);
					if (err != 0)
						goto no_data;

					mPendingEvents[RotationMatrix].data[4] = -1;
					mPendingMask |= 1<<RotationMatrix;
				}
#endif
#if (SENSORS_GAME_ROTATION_ENABLE == 1)
				DecimationCount[GameRotation]++;
				if(mEnabled & (1<<GameRotation) && (DecimationCount[GameRotation] >= DecimationBuffer[GameRotation])) {
					DecimationCount[GameRotation] = 0;
					err = iNemoEngine_API_Get_6X_Quaternion(mPendingEvents[GameRotation].data);
					if (err != 0)
						goto no_data;

					mPendingMask |= 1<<GameRotation;
				}
#endif
#if (GYROSCOPE_GBIAS_ESTIMATION_FUSION == 1)
  #if (SENSORS_UNCALIB_GYROSCOPE_ENABLE == 1)
				DecimationCount[UncalibGyro]++;
				if(mEnabled & (1<<UncalibGyro) && (DecimationCount[UncalibGyro] >= DecimationBuffer[UncalibGyro])) {
					DecimationCount[UncalibGyro] = 0;

					err = iNemoEngine_API_Get_Gbias(gbias);
					if (err != 0)
						goto no_data;

					int i;
					for (i = 0 ; i < 3; i++) {
						mPendingEvents[UncalibGyro].uncalibrated_gyro.uncalib[i] = sdata.gyro[i];
						mPendingEvents[UncalibGyro].uncalibrated_gyro.bias[i] = gbias[i];
					}
					mPendingMask |= 1<<UncalibGyro;
				}
  #endif
				DecimationCount[CalibGyro]++;
				if(mEnabled & (1<<CalibGyro) && (DecimationCount[CalibGyro] >= DecimationBuffer[CalibGyro])) {
					DecimationCount[CalibGyro] = 0;
					err = iNemoEngine_API_Get_Gbias(gbias);
					if (err != 0)
						goto no_data;

					int i;
					for (i = 0 ; i < 3; i++) {
						mPendingEvents[CalibGyro].data[i] = sdata.gyro[i] - gbias[i];
					}
					mPendingMask |= 1<<CalibGyro;
				}
#endif
			}

no_data:
			for (int j=0 ; count && mPendingMask && j<numSensors ; j++) {
				if (mPendingMask & (1<<j)) {
					mPendingMask &= ~(1<<j);
					mPendingEvents[j].timestamp = timestamp;
					if (mEnabled & (1<<j)) {
						*data++ = mPendingEvents[j];
						count--;
						numEventReceived++;
					}
				}
			}
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

#endif /* SENSOR_FUSION_ENABLE */
