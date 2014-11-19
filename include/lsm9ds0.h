/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
*
* File Name          : lsm9ds0.h
* Authors            : MSH - C&I BU - Application Team
*		     : Matteo Dameno (matteo.dameno@st.com)
*		     : Denis Ciocca (denis.ciocca@st.com)
* Version            : V.1.0.0
* Date               : 2012/Aug/21
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
********************************************************************************
********************************************************************************
* REVISON HISTORY
*
* VERSION	| DATE		| AUTHORS	  | DESCRIPTION
* 1.0		| 2010/May/02	| Carmine Iascone | First Release
* 1.1.3		| 2011/Jun/24	| Matteo Dameno	  | Corrects ODR Bug
* 1.1.4		| 2011/Sep/02	| Matteo Dameno	  | SMB Bus Mng,
*		|		|		  | forces BDU setting
* 1.1.5		| 2011/Sep/24	| Matteo Dameno	  | Introduces FIFO Feat.
* 1.1.5.2	| 2011/Nov/11	| Matteo Dameno	  | enable gpio_int to be
*		|		|		  | passed as parameter at
*		|		|		  | module loading time;
*		|		|		  | corrects polling
*		|		|		  | bug at end of probing;
* 1.1.5.3	| 2011/Dec/20	| Matteo Dameno	  | corrects error in
*		|		|		  | I2C SADROOT; Modifies
*		|		|		  | resume suspend func.
* 1.1.5.4	| 2012/Jan/09	| Matteo Dameno	  | moved under input/misc;
* 1.1.5.5	| 2012/Mar/30	| Matteo Dameno	  | moved watermark, use_smbus,
*		|		|		  | fifomode @ struct foo_status
*		|		|		  | sysfs range input format
*		|		|		  | changed to decimal
* 1.2		| 2012/Jul/10	| Denis Ciocca	  | input_poll_dev removal
* 1.2.1		| 2012/Jul/10	| Denis Ciocca	  | added high resolution timers
*******************************************************************************/

#ifndef	__LSM9DS0_H__
#define	__LSM9DS0_H__

#define	LSM9DS0_DEV_NAME	"lsm9ds0_acc_mag"	/* i2c system name */
#define	LSM9DS0_ACC_DEV_NAME	"lsm9ds0_acc"		/* Input file name */
#define	LSM9DS0_MAG_DEV_NAME	"lsm9ds0_mag"		/* Input file name */
#define LSM9DS0_GYR_DEV_NAME	"lsm9ds0_gyr"		/* Input file name */

#define LSM9DS0_SAD0L_ACC_MAG		(0x02)
#define LSM9DS0_SAD0H_ACC_MAG		(0x01)
#define LSM9DS0_SAD0L_GYR		(0x00)
#define LSM9DS0_SAD0H_GYR		(0x01)

/************************************************/
/* 	Output data			 	*/
/*************************************************
accelerometer: ug
magnetometer: ugauss
gyroscope: udps
*************************************************/

/************************************************/
/* 	sysfs data			 	*/
/*************************************************
accelerometer:
	- pollrate->ms
	- fullscale->g
magnetometer:
	- pollrate->ms
	- fullscale->gauss
gyroscope:
	- pollrate->ms
	- fullscale->dps
*************************************************/

#define LSM9DS0_ACC_MAG_I2C_SADROOT	(0x07)

/* I2C address if gyr SA0 pin to GND */
#define LSM9DS0_ACC_MAG_I2C_SAD_L	((LSM9DS0_ACC_MAG_I2C_SADROOT<<2)| \
							LSM9DS0_SAD0L_ACC_MAG)
/* I2C address if gyr SA0 pin to Vdd */
#define LSM9DS0_ACC_MAG_I2C_SAD_H	((LSM9DS0_ACC_MAG_I2C_SADROOT<<2)| \
							LSM9DS0_SAD0H_ACC_MAG)

/************************************************/
/* 	Accelerometer section defines	 	*/
/************************************************/

/* Accelerometer Sensor Full Scale */
#define	LSM9DS0_ACC_FS_MASK	(0x18)
#define LSM9DS0_ACC_FS_2G 	(0x00)	/* Full scale 2g */
#define LSM9DS0_ACC_FS_4G 	(0x08)	/* Full scale 4g */
#define LSM9DS0_ACC_FS_8G 	(0x10)	/* Full scale 8g */
#define LSM9DS0_ACC_FS_16G	(0x18)	/* Full scale 16g */

/* Accelerometer Anti-Aliasing Filter */
#define ANTI_ALIASING_773	(0X00)
#define ANTI_ALIASING_362	(0X40)
#define ANTI_ALIASING_194	(0X80)
#define ANTI_ALIASING_50	(0XC0)

/************************************************/
/* 	Magnetometer section defines	 	*/
/************************************************/

/* Magnetometer Sensor Full Scale */
#define LSM9DS0_MAG_FS_MASK	(0x60)
#define LSM9DS0_MAG_FS_2G	(0x00)	/* Full scale 2 gauss */
#define LSM9DS0_MAG_FS_4G	(0x20)	/* Full scale 4 gauss */
#define LSM9DS0_MAG_FS_8G	(0x40)	/* Full scale 8 gauss */
#define LSM9DS0_MAG_FS_12G	(0x60)	/* Full scale 12 gauss */

/************************************************/
/* 	Gyroscope section defines	 	*/
/************************************************/

#define LSM9DS0_GYR_I2C_SADROOT	(0x35)

/* I2C address if gyr SA0 pin to GND */
#define LSM9DS0_GYR_I2C_SAD_L		((LSM9DS0_GYR_I2C_SADROOT<<1)| \
							LSM9DS0_SAD0L_GYR)
/* I2C address if gyr SA0 pin to Vdd */
#define LSM9DS0_GYR_I2C_SAD_H		((LSM9DS0_GYR_I2C_SADROOT<<1)| \
							LSM9DS0_SAD0H_GYR)

#ifdef	__KERNEL__

/* to set gpios numb connected to gyro interrupt pins,
 * the unused ones havew to be set to -EINVAL
 */
#define DEFAULT_INT1_GPIO		(-EINVAL)
#define DEFAULT_INT2_GPIO		(-EINVAL)

#define	LSM9DS0_ACC_MIN_POLL_PERIOD_MS	1
#define LSM9DS0_MAG_MIN_POLL_PERIOD_MS	5

#define LSM9DS0_GYR_DEFAULT_INT1_GPIO	(-EINVAL)
#define LSM9DS0_GYR_DEFAULT_INT2_GPIO	(-EINVAL)

#define LSM9DS0_GYR_MIN_POLL_PERIOD_MS	2

#define LSM9DS0_GYR_FS_250DPS		(0x00)
#define LSM9DS0_GYR_FS_500DPS		(0x10)
#define LSM9DS0_GYR_FS_2000DPS		(0x30)

struct lsm9ds0_acc_platform_data {
	
	unsigned int poll_interval;
	unsigned int min_interval;

	u8 fs_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	u8 aa_filter_bandwidth;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);

	int gpio_int1;
	int gpio_int2;
};

struct lsm9ds0_mag_platform_data {

	unsigned int poll_interval;
	unsigned int min_interval;

	u8 fs_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
};

struct lsm9ds0_gyr_platform_data {
	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
	unsigned int poll_interval;
	unsigned int min_interval;

	u8 fs_range;

	/* gpio ports for interrupt pads */
	int gpio_int1;
	int gpio_int2;		/* int for fifo */

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;
};

struct lsm9ds0_main_platform_data {
	
	struct lsm9ds0_acc_platform_data *pdata_acc;
	struct lsm9ds0_mag_platform_data *pdata_mag;
};

#endif	/* __KERNEL__ */
#endif	/* __LSM9DS0_H__ */
