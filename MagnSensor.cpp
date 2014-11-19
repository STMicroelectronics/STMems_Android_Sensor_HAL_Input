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
#if (SENSORS_MAGNETIC_FIELD_ENABLE == 1)

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

#include "MagnSensor.h"

#if (SENSOR_GEOMAG_ENABLE == 1)
#include "iNemoEngineGeoMagAPI.h"
#endif

#define FETCH_FULL_EVENT_BEFORE_RETURN		0
#define MS2_TO_MG(x)				(x*102.040816327f)
#define UT_TO_MGAUSS(x)				(x*10.0f)
#define MGAUSS_TO_UT(x)				(x/10.0f)

/*****************************************************************************/

sensors_vec_t  MagnSensor::dataBuffer;
int MagnSensor::freq = 0;
int MagnSensor::count_call_ecompass = 0;
int MagnSensor::mEnabled = 0;
int64_t MagnSensor::delayms = 0;
int MagnSensor::current_fullscale = 0;
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
AccelSensor* MagnSensor::acc = NULL;
#endif
int64_t MagnSensor::setDelayBuffer[numSensors] = {0};
int64_t MagnSensor::writeDelayBuffer[numSensors] = {0};
int MagnSensor::DecimationBuffer[numSensors] = {0};
int MagnSensor::DecimationCount[numSensors] = {0};
pthread_mutex_t MagnSensor::dataMutex;

MagnSensor::MagnSensor()
	: SensorBase(NULL, SENSOR_DATANAME_MAGNETIC_FIELD),
	mInputReader(6),
	mHasPendingEvent(false)
{
	int err;

	pthread_mutex_init(&dataMutex, NULL);

#if SENSOR_GEOMAG_ENABLE == 1
	refFreq = (MAGN_MAX_ODR < GEOMAG_FREQUENCY) ? MAGN_MAX_ODR : GEOMAG_FREQUENCY;
#endif

	memset(mPendingEvent, 0, sizeof(mPendingEvent));
	memset(DecimationCount, 0, sizeof(DecimationCount));

	mPendingEvent[MagneticField].version = sizeof(sensors_event_t);
	mPendingEvent[MagneticField].sensor = ID_MAGNETIC_FIELD;
	mPendingEvent[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
	memset(mPendingEvent[MagneticField].data, 0, sizeof(mPendingEvent[MagneticField].data));
	mPendingEvent[MagneticField].magnetic.status = SENSOR_STATUS_UNRELIABLE;

#if (SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE == 1)
	mPendingEvent[UncalibMagneticField].version = sizeof(sensors_event_t);
	mPendingEvent[UncalibMagneticField].sensor = ID_UNCALIB_MAGNETIC_FIELD;
	mPendingEvent[UncalibMagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
	memset(mPendingEvent[UncalibMagneticField].data, 0, sizeof(mPendingEvent[UncalibMagneticField].data));
	mPendingEvent[UncalibMagneticField].magnetic.status = SENSOR_STATUS_UNRELIABLE;
#endif
#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
	mPendingEvent[Orientation].version = sizeof(sensors_event_t);
	mPendingEvent[Orientation].sensor = ID_ORIENTATION;
	mPendingEvent[Orientation].type = SENSOR_TYPE_ORIENTATION;
	memset(mPendingEvent[Orientation].data, 0, sizeof(mPendingEvent[Orientation].data));
	mPendingEvent[Orientation].orientation.status = SENSOR_STATUS_UNRELIABLE;
#endif
#if (GEOMAG_GRAVITY_ENABLE == 1)
	mPendingEvent[Gravity_Accel].version = sizeof(sensors_event_t);
	mPendingEvent[Gravity_Accel].sensor = ID_GRAVITY;
	mPendingEvent[Gravity_Accel].type = SENSOR_TYPE_GRAVITY;
	memset(mPendingEvent[Gravity_Accel].data, 0, sizeof(mPendingEvent[Gravity_Accel].data));
	mPendingEvent[Gravity_Accel].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif
#if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
	mPendingEvent[Linear_Accel].version = sizeof(sensors_event_t);
	mPendingEvent[Linear_Accel].sensor = ID_LINEAR_ACCELERATION;
	mPendingEvent[Linear_Accel].type = SENSOR_TYPE_LINEAR_ACCELERATION;
	memset(mPendingEvent[Linear_Accel].data, 0, sizeof(mPendingEvent[Linear_Accel].data));
	mPendingEvent[Linear_Accel].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
	mPendingEvent[GeoMagRotVect_Magnetic].version = sizeof(sensors_event_t);
	mPendingEvent[GeoMagRotVect_Magnetic].sensor = ID_GEOMAG_ROTATION_VECTOR;
	mPendingEvent[GeoMagRotVect_Magnetic].type = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
	memset(mPendingEvent[GeoMagRotVect_Magnetic].data, 0, sizeof(mPendingEvent[GeoMagRotVect_Magnetic].data));
	mPendingEvent[GeoMagRotVect_Magnetic].magnetic.status = SENSOR_STATUS_UNRELIABLE;
#endif

#if (SENSOR_GEOMAG_ENABLE == 1)
	memset(&sData, 0, sizeof(iNemoGeoMagSensorsData));
	iNemoEngine_GeoMag_API_Initialization(100);
#endif

	if (data_fd) {
		STLOGI("MagnSensor::MagnSensor magn_device_sysfs_path:(%s)", sysfs_device_path);
	} else {
		STLOGE("MagnSensor::MagnSensor magn_device_sysfs_path:(%s) not found", sysfs_device_path);
	}

	memset(data_raw, 0, sizeof(data_raw));

#if (SENSOR_GEOMAG_ENABLE == 1)
	acc = new AccelSensor();
#endif
}

MagnSensor::~MagnSensor() {
	if (mEnabled) {
		enable(SENSORS_MAGNETIC_FIELD_HANDLE, 0, 0);
		mEnabled = 0;
	}
	pthread_mutex_destroy(&dataMutex);
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
	acc->~AccelSensor();
#endif
}

#if !defined(NOT_SET_MAG_INITIAL_STATE)
int MagnSensor::setInitialState()
{
	struct input_absinfo absinfo_x;
	struct input_absinfo absinfo_y;
	struct input_absinfo absinfo_z;
	float value;

#if (MAG_CALIBRATION_ENABLE == 1)
	data_read = 0;
#endif

	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAG_X), &absinfo_x) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAG_Y), &absinfo_y) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_MAG_Z), &absinfo_z))
	{
		mHasPendingEvent = true;
	}

	setFullScale(SENSORS_MAGNETIC_FIELD_HANDLE, MAGN_DEFAULT_FULLSCALE);
	memset(DecimationCount, 0, sizeof(DecimationCount));

	return 0;
}
#endif

int MagnSensor::getWhatFromHandle(int32_t handle)
{
	int what = -1;

	switch(handle) {

		case SENSORS_MAGNETIC_FIELD_HANDLE:
			what = MagneticField;
			break;

#if (SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE == 1)
		case SENSORS_UNCALIB_MAGNETIC_FIELD_HANDLE:
			what = UncalibMagneticField;
			break;
#endif
#if (SENSOR_FUSION_ENABLE == 1)
		case SENSORS_SENSOR_FUSION_HANDLE:
			what = iNemoMagnetic;
			break;
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
		case SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE:
			what = GeoMagRotVect_Magnetic;
			break;
#endif
#if (MAG_CALIBRATION_ENABLE == 1)
	#if (GEOMAG_GRAVITY_ENABLE == 1)
		case SENSORS_GRAVITY_HANDLE:
			what = Gravity_Accel;
			break;
	#endif
	#if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
		case SENSORS_LINEAR_ACCELERATION_HANDLE:
			what = Linear_Accel;
			break;
	#endif
	#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
			case SENSORS_ORIENTATION_HANDLE:
				what = Orientation;
				break;
	#endif
	#if (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
			case SENSORS_VIRTUAL_GYROSCOPE_HANDLE:
				what = VirtualGyro;
				break;
	#endif
#endif
		default:
			what = -1;
	}

	return what;
}

int MagnSensor::enable(int32_t handle, int en, int __attribute__((unused))type)
{
	int err = 0;
	int flags = en ? 1 : 0;
	int what = -1;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

	if (flags) {
		mEnabled |= (1<<what);
		writeMinDelay();

		if (mEnabled == (1<<what)) {
#if !defined(NOT_SET_MAG_INITIAL_STATE)
			setInitialState();
#endif
			err = writeEnable(SENSORS_MAGNETIC_FIELD_HANDLE, flags);
			if(err >= 0) {
				err = 0;
			}
		}

#if (MAG_CALIBRATION_ENABLE == 1)
		ST_MagCalibration_API_Init(CALIBRATION_PERIOD_MS);
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
		if (what == GeoMagRotVect_Magnetic)
			acc->enable(SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE, flags, 3);
#endif
#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
		if (what == Orientation)
			acc->enable(SENSORS_ORIENTATION_HANDLE, flags, 4);
#endif
#if (GEOMAG_GRAVITY_ENABLE == 1)
		if (what == Gravity_Accel)
			acc->enable(SENSORS_GRAVITY_HANDLE, flags, 5);
#endif
#if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
		if (what == Linear_Accel)
			acc->enable(SENSORS_LINEAR_ACCELERATION_HANDLE, flags, 6);
#endif
#endif /* MAG_CALIBRATION_ENABLE */
	} else {
		mEnabled &= ~(1<<what);

		if (!mEnabled) {
			err = writeEnable(SENSORS_MAGNETIC_FIELD_HANDLE, flags);
			if(err >= 0)
				err = 0;
		}

#if (MAG_CALIBRATION_ENABLE == 1)
		ST_MagCalibration_API_DeInit(CALIBRATION_PERIOD_MS);
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
		if (what == GeoMagRotVect_Magnetic)
			acc->enable(SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE, flags, 3);
#endif
#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
		if (what == Orientation)
			acc->enable(SENSORS_ORIENTATION_HANDLE, flags, 4);
#endif
#if (GEOMAG_GRAVITY_ENABLE == 1)
		if (what == Gravity_Accel)
			acc->enable(SENSORS_GRAVITY_HANDLE, flags, 5);
#endif
#if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
		if (what == Linear_Accel)
			acc->enable(SENSORS_ORIENTATION_HANDLE, flags, 6);
#endif
#endif /* MAG_CALIBRATION_ENABLE */

		if (mEnabled) {
			writeMinDelay();
		}
	}

	if(err >= 0 )
		STLOGD("MagSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);
	else
		STLOGE("MagSensor::enable(%d), handle: %d, what: %d, mEnabled: %x",
						flags, handle, what, mEnabled);

	return err;
}

bool MagnSensor::hasPendingEvents() const
{
	return mHasPendingEvent;
}

int MagnSensor::setDelay(int32_t handle, int64_t delay_ns)
{
	int err = 0;
	int kk;
	int what = -1;
	int64_t delay_ms = NSEC_TO_MSEC(delay_ns);
	int64_t Min_delay_ms = 0;

	if(delay_ms == 0)
		return err;

	what = getWhatFromHandle(handle);
	if (what < 0)
		return what;

#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
	if (what == GeoMagRotVect_Magnetic)
		acc->setDelay(SENSORS_GEOMAG_ROTATION_VECTOR_HANDLE, SEC_TO_NSEC(1.0f / refFreq));
#endif
#if (MAG_CALIBRATION_ENABLE == 1)
#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
	if (what == Orientation)
		acc->setDelay(SENSORS_ORIENTATION_HANDLE, SEC_TO_NSEC(1.0f / refFreq));
#endif
#if (GEOMAG_GRAVITY_ENABLE == 1)
	if (what == Gravity_Accel)
		acc->setDelay(SENSORS_GRAVITY_HANDLE, SEC_TO_NSEC(1.0f / refFreq));
#endif
#if (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1)
	if (what == Linear_Accel)
		acc->setDelay(SENSORS_LINEAR_ACCELERATION_HANDLE, SEC_TO_NSEC(1.0f / refFreq));
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
	STLOGD("MagSensor::setDelayBuffer[] = %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld",
				setDelayBuffer[0], setDelayBuffer[1], setDelayBuffer[2],
				setDelayBuffer[3], setDelayBuffer[4], setDelayBuffer[5],
				setDelayBuffer[6], setDelayBuffer[7]);
	STLOGD("MagSensor::Requested_delay_ms = %lld", delay_ms);
#endif

	// Update sysfs
	if(mEnabled & 1<<what)
	{
		writeMinDelay();
	}

	return err;
}

int MagnSensor::writeMinDelay(void)
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
			if ((writeDelayBuffer[kk] != 0) && (writeDelayBuffer[kk] < Min_delay_ms))
				Min_delay_ms = writeDelayBuffer[kk];
		} else
			Min_delay_ms = writeDelayBuffer[kk];
	}

#if (MAG_CALIBRATION_ENABLE == 1)
	if(Min_delay_ms > CALIBRATION_PERIOD_MS)
		Min_delay_ms = CALIBRATION_PERIOD_MS;
#endif
#if ((SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1) ||\
     (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1) ||\
	 (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1) ||\
	 (GEOMAG_GRAVITY_ENABLE == 1))
	if ((mEnabled & 1<<GeoMagRotVect_Magnetic) || (mEnabled & 1<<Orientation)
			 || (mEnabled & 1<<Linear_Accel) || (mEnabled & 1<<Gravity_Accel)){
		if(Min_delay_ms > (1000.0f / refFreq))
			Min_delay_ms = 1000.0f / refFreq;
	}
#endif

	if ((Min_delay_ms > 0) && (Min_delay_ms != delayms))
	{
		err = writeDelay(SENSORS_MAGNETIC_FIELD_HANDLE, Min_delay_ms);
		if(err >= 0) {
			err = 0;
			delayms = Min_delay_ms;
			freq = 1000.0f / Min_delay_ms;
#if (MAG_CALIBRATION_ENABLE == 1)
			count_call_ecompass = freq / CALIBRATION_FREQUENCY;
#endif
			memset(DecimationCount, 0, sizeof(DecimationCount));
		}
	}

	// Decimation Definition
	for(kk = 0; kk < numSensors; kk++)
	{
		if (kk == MagneticField || kk == UncalibMagneticField)
			continue;

		if (delayms)
			DecimationBuffer[kk] = writeDelayBuffer[kk]/delayms;
		else
			DecimationBuffer[kk] = 0;
	}

#if (DEBUG_POLL_RATE == 1)
	STLOGD("MagSensor::writeDelayBuffer[] = %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld",
				writeDelayBuffer[0], writeDelayBuffer[1], writeDelayBuffer[2],
				writeDelayBuffer[3], writeDelayBuffer[4], writeDelayBuffer[5],
				writeDelayBuffer[6], writeDelayBuffer[7]);
	STLOGD("MagSensor::Min_delay_ms = %lld, delayms = %lld, mEnabled = %d",
				Min_delay_ms, delayms, mEnabled);
	STLOGD("MagSensor::DecimationBuffer = %d, %d, %d, %d, %d, %d, %d, %d",
				DecimationBuffer[0], DecimationBuffer[1], DecimationBuffer[2],
				DecimationBuffer[3], DecimationBuffer[4], DecimationBuffer[5],
				DecimationBuffer[6], DecimationBuffer[7]);
	STLOGD("MagSensor::count_call_ecompass = %d", count_call_ecompass);
#endif

	return err;

}

void MagnSensor::getMagDelay(int64_t *Mag_Delay_ms)
{
	*Mag_Delay_ms = delayms;

	return;
}

int MagnSensor::setFullScale(int32_t __attribute__((unused))handle, int value)
{
	int err = -1;

	if(value <= 0)
		return err;
	else
		err = 0;

	if(value != current_fullscale)
	{
		err = writeFullScale(SENSORS_MAGNETIC_FIELD_HANDLE, value);
		if(err >= 0) {
			err = 0;
			current_fullscale = value;
		}
	}
	return err;
}

int MagnSensor::readEvents(sensors_event_t *data, int count)
{
	int err;
	float MagOffset[3];

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

		if (event->type == EVENT_TYPE_MAG) {
			float value = (float) event->value;

			if (event->code == EVENT_TYPE_MAG_X) {
				data_raw[0] = value * CONVERT_M_X;
			} else if (event->code == EVENT_TYPE_MAG_Y) {
				data_raw[1] = value * CONVERT_M_Y;
			} else if (event->code == EVENT_TYPE_MAG_Z) {
				data_raw[2] = value * CONVERT_M_Z;
			}
#if defined(MAG_EVENT_HAS_TIMESTAMP)
			else if (event->code == EVENT_TYPE_TIME_MSB) {
				timestamp = ((int64_t)(event->value)) << 32;
			}
			else if (event->code == EVENT_TYPE_TIME_LSB) {
				timestamp |= (uint32_t)(event->value);
			}
#endif
			else {
				STLOGE("MagnSensor: unknown event code (type = %d, code = %d)", event->type, event->code);
			}
		} else if (event->type == EV_SYN) {
			data_rot[0] =	data_raw[0] * matrix_mag[0][0] +
					data_raw[1] * matrix_mag[1][0] +
					data_raw[2] * matrix_mag[2][0];
			data_rot[1] = 	data_raw[0] * matrix_mag[0][1] +
					data_raw[1] * matrix_mag[1][1] +
					data_raw[2] * matrix_mag[2][1];
			data_rot[2] = 	data_raw[0] * matrix_mag[0][2] +
					data_raw[1] * matrix_mag[1][2] +
					data_raw[2] * matrix_mag[2][2];

#if !defined(MAG_EVENT_HAS_TIMESTAMP)
			timestamp = timevalToNano(event->time);
#endif
#if (SENSORS_ACCELEROMETER_ENABLE == 1)
			AccelSensor::getBufferData(&mSensorsBufferedVectors[ID_ACCELEROMETER]);
#endif /* SENSORS_ACCELEROMETER_ENABLE */
#if (MAG_CALIBRATION_ENABLE == 1)
			magCalibIn.timestamp = timestamp;
			magCalibIn.mag_raw[0] = data_rot[0];
			magCalibIn.mag_raw[1] = data_rot[1];
			magCalibIn.mag_raw[2] = data_rot[2];

			ST_MagCalibration_API_Run(&magCalibOut, &magCalibIn);
#if (DEBUG_CALIBRATION == 1)
				STLOGD("Calibration MagData [uT] -> raw_x:%f raw_y:%f raw_z:%f",
					data_rot[0], data_rot[1], data_rot[2]);
				STLOGD("Calibration MagData [uT] -> uncal_x:%f uncal_y:%f uncal_z:%f",
					magCalibOut.mag_cal[0], magCalibOut.mag_cal[1],
					magCalibOut.mag_cal[2]);
#endif /* DEBUG_CALIBRATION */
#endif /* MAG_CALIBRATION_ENABLE */
			if (mEnabled & ((1 << MagneticField) |
						(1 << UncalibMagneticField) |
						(1 << GeoMagRotVect_Magnetic) |
						(1 << Orientation) |
						(1 << Linear_Accel) |
						(1 << Gravity_Accel) |
						(1 << iNemoMagnetic) |
						(1 << VirtualGyro))) {
				/**
				 * Get and apply Hard Iron calibration to raw mag data
				 */
#if (MAG_CALIBRATION_ENABLE == 1)
				data_calibrated.v[0] = magCalibOut.mag_cal[0];
				data_calibrated.v[1] = magCalibOut.mag_cal[1];
				data_calibrated.v[2] = magCalibOut.mag_cal[2];
				data_calibrated.status = magCalibOut.accuracy;
				MagOffset[0] = magCalibOut.offset[0];
				MagOffset[1] = magCalibOut.offset[1];
				MagOffset[2] = magCalibOut.offset[2];

#if (DEBUG_MAGNETOMETER == 1)
				STLOGD("MagnSensor::MagCalibData: %f, %f, %f", data_calibrated.v[0], data_calibrated.v[1], data_calibrated.v[2]);
#endif
#else
				/**
				 * No calibration is available!
				 */
				memcpy(data_calibrated.v, data_rot, sizeof(data_calibrated.v));
				data_calibrated.status = SENSOR_STATUS_UNRELIABLE;
#endif

#if ((SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1) ||\
	 (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1) ||\
	 (GEOMAG_LINEAR_ACCELERATION_ENABLE == 1) ||\
	 (GEOMAG_GRAVITY_ENABLE == 1))
				memcpy(sData.accel,
				       mSensorsBufferedVectors[ID_ACCELEROMETER].v,
							sizeof(sData.accel));
				memcpy(sData.magn, data_calibrated.v,
							sizeof(data_calibrated.v));
				iNemoEngine_GeoMag_API_Run(MagnSensor::delayms, &sData);
#endif
				DecimationCount[MagneticField]++;
				if((mEnabled & (1<<MagneticField)) && (DecimationCount[MagneticField] >= DecimationBuffer[MagneticField])) {
					DecimationCount[MagneticField] = 0;
					mPendingEvent[MagneticField].magnetic.status =
							data_calibrated.status;
					memcpy(mPendingEvent[MagneticField].data,
							data_calibrated.v,
							sizeof(data_calibrated.v));
					mPendingEvent[MagneticField].timestamp = timestamp;
					*data++ = mPendingEvent[MagneticField];
					count--;
					numEventReceived++;
				}
#if (SENSORS_UNCALIB_MAGNETIC_FIELD_ENABLE == 1)
				DecimationCount[UncalibMagneticField]++;
				if((mEnabled & (1<<UncalibMagneticField)) && (DecimationCount[UncalibMagneticField] >= DecimationBuffer[UncalibMagneticField])) {
					DecimationCount[UncalibMagneticField] = 0;
					mPendingEvent[UncalibMagneticField].magnetic.status = 
							data_calibrated.status;
					memcpy(mPendingEvent[UncalibMagneticField].uncalibrated_magnetic.uncalib,
							data_rot, sizeof(data_rot));
					memcpy(mPendingEvent[UncalibMagneticField].uncalibrated_magnetic.bias,
							MagOffset, sizeof(MagOffset));
					mPendingEvent[UncalibMagneticField].timestamp = timestamp;
					*data++ = mPendingEvent[UncalibMagneticField];
					count--;
					numEventReceived++;
				}
#endif
#if (SENSORS_GEOMAG_ROTATION_VECTOR_ENABLE == 1)
				DecimationCount[GeoMagRotVect_Magnetic]++;
				if((mEnabled & (1<<GeoMagRotVect_Magnetic)) && (DecimationCount[GeoMagRotVect_Magnetic] >= DecimationBuffer[GeoMagRotVect_Magnetic])) {
					DecimationCount[GeoMagRotVect_Magnetic] = 0;

					err = iNemoEngine_GeoMag_API_Get_Quaternion(mPendingEvent[GeoMagRotVect_Magnetic].data);
					if (err == 0) {
						mPendingEvent[GeoMagRotVect_Magnetic].magnetic.status =
							data_calibrated.status;
						mPendingEvent[GeoMagRotVect_Magnetic].data[4] = -1;
						mPendingEvent[GeoMagRotVect_Magnetic].timestamp = timestamp;
						*data++ = mPendingEvent[GeoMagRotVect_Magnetic];
						count--;
						numEventReceived++;
					}
				}
#endif
#if ((GEOMAG_LINEAR_ACCELERATION_ENABLE == 1))
				DecimationCount[Linear_Accel]++;
				if((mEnabled & (1<<Linear_Accel)) && (DecimationCount[Linear_Accel] >= DecimationBuffer[Linear_Accel])) {
					DecimationCount[Linear_Accel] = 0;
					err = iNemoEngine_GeoMag_API_Get_LinAcc(mPendingEvent[Linear_Accel].data);
					if (err == 0) {
						mPendingEvent[Linear_Accel].timestamp = timestamp;
						*data++ = mPendingEvent[Linear_Accel];
						count--;
						numEventReceived++;
					}
				}
#endif
#if ((GEOMAG_GRAVITY_ENABLE == 1))
				DecimationCount[Gravity_Accel]++;
				if((mEnabled & (1<<Gravity_Accel)) && (DecimationCount[Gravity_Accel] >= DecimationBuffer[Gravity_Accel])) {
					DecimationCount[Gravity_Accel] = 0;
					err = iNemoEngine_GeoMag_API_Get_Gravity(mPendingEvent[Gravity_Accel].data);
					if (err == 0) {
						mPendingEvent[Gravity_Accel].timestamp = timestamp;
						*data++ = mPendingEvent[Gravity_Accel];
						count--;
						numEventReceived++;
					}
				}
#endif
#if (GEOMAG_COMPASS_ORIENTATION_ENABLE == 1)
				DecimationCount[Orientation]++;
				if((mEnabled & (1<<Orientation)) && (DecimationCount[Orientation] >= DecimationBuffer[Orientation])) {
					DecimationCount[Orientation] = 0;
					err = iNemoEngine_GeoMag_API_Get_Hpr(mPendingEvent[Orientation].data);
					if (err == 0) {
						mPendingEvent[Orientation].orientation.status =
							data_calibrated.status;
						mPendingEvent[Orientation].timestamp = timestamp;
						*data++ = mPendingEvent[Orientation];
						count--;
						numEventReceived++;
					}
				}
#endif
#if (SENSOR_FUSION_ENABLE == 1) || \
    (SENSORS_VIRTUAL_GYROSCOPE_ENABLE == 1)
				if(mEnabled & ((1<<iNemoMagnetic) |
					       (1<<VirtualGyro)))
					setBufferData(&data_calibrated);
#endif
#if DEBUG_MAGNETOMETER == 1
				STLOGD("MagnSensor::readEvents (time = %lld),"
						"count(%d), received(%d)",
						mPendingEvent[MagneticField].timestamp,
						count, numEventReceived);
#endif
			}
		} else
			STLOGE("MagnSensor: unknown event (type = %d, code = %d)",
							event->type, event->code);

		mInputReader.next();
	}
#if FETCH_FULL_EVENT_BEFORE_RETURN
	/**
	 * if we didn't read a complete event, see if we can fill and
	 * try again instead of returning with nothing and redoing poll.
	 */
	if (numEventReceived == 0 && mEnabled != 0) {
		n = mInputReader.fill(data_fd);
		if (n)
			goto again;
	}
#endif
	return numEventReceived;
}

bool MagnSensor::setBufferData(sensors_vec_t *value)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(&dataBuffer, value, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

	return true;
}

bool MagnSensor::getBufferData(sensors_vec_t *lastBufferedValues)
{
	pthread_mutex_lock(&dataMutex);
	memcpy(lastBufferedValues, &dataBuffer, sizeof(sensors_vec_t));
	pthread_mutex_unlock(&dataMutex);

	return true;
}

#endif /* SENSORS_MAGNETIC_FIELD_ENABLE */
