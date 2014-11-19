
/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
*
* File Name          : lsm330dlc_sysfs.h
* Authors            : MH - C&I BU - Application Team
*		     : Matteo Dameno (matteo.dameno@st.com)
*		     : Carmine Iascone (carmine.iascone@st.com)
* Version            : V.1.0.12
* Date               : 2012/Feb/09
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
*******************************************************************************/
/*******************************************************************************
Version History.

 Revision 1.0.10: 2011/Aug/16
  merges release 1.0.10 acc + 1.1.5.3 gyr
 Revision 1.0.11: 2012/Jan/09
  moved under input/misc
 Revision 1.0.12: 2012/Feb/29
  renamed field g_range to fs_range in lsm330dlc_acc_platform_data
  replaced defines SA0L and SA0H with LSM330DLC_SAD0x
*******************************************************************************/

#ifndef	__LSM330DLC_H__
#define	__LSM330DLC_H__


#define	LSM330DLC_ACC_DEV_NAME		"lsm330dlc_acc"
#define LSM330DLC_GYR_DEV_NAME		"lsm330dlc_gyr"

/************************************************/
/*	Accelerometer section defines		*/
/************************************************/

#define	LSM330DLC_ACC_MIN_POLL_PERIOD_MS	1


/************************************************/
/*	Gyroscope section defines		*/
/************************************************/

#define LSM330DLC_GYR_ENABLED	1
#define LSM330DLC_GYR_DISABLED	0
#define LSM330DLC_GYR_MIN_POLL_PERIOD_MS	2


#ifdef	__KERNEL__

#define LSM330DLC_SAD0L			(0x00)
#define LSM330DLC_SAD0H			(0x01)

/* gyro section */
#define LSM330DLC_GYR_I2C_SADROOT		(0x35)

/* I2C address if gyr SA0 pin to GND */
#define LSM330DLC_GYR_I2C_SAD_L		((LSM330DLC_GYR_I2C_SADROOT<<1)|\
							LSM330DLC_SAD0L)
/* I2C address if gyr SA0 pin to Vdd */
#define LSM330DLC_GYR_I2C_SAD_H		((LSM330DLC_GYR_I2C_SADROOT<<1)|\
							LSM330DLC_SAD0H)
/* to set gpios numb connected to gyro interrupt pins,
 * the unused ones have to be set to -EINVAL
 */
#define LSM330DLC_GYR_DEFAULT_INT1_GPIO	(-EINVAL)
#define LSM330DLC_GYR_DEFAULT_INT2_GPIO	(-EINVAL)

/* Gyroscope Sensor Full Scale */
#define LSM330DLC_GYR_FS_250DPS		(0x00)
#define LSM330DLC_GYR_FS_500DPS		(0x10)
#define LSM330DLC_GYR_FS_2000DPS		(0x30)



/* acc section */
#define LSM330DLC_ACC_I2C_SADROOT		(0x0C)
/* I2C address if acc SA0 pin to GND */
#define LSM330DLC_ACC_I2C_SAD_L		((LSM330DLC_ACC_I2C_SADROOT<<1)| \
							LSM330DLC_SAD0L)
/* I2C address if acc SA0 pin to Vdd */
#define LSM330DLC_ACC_I2C_SAD_H		((LSM330DLC_ACC_I2C_SADROOT<<1)| \
							LSM330DLC_SAD0H)

/* to set gpios numb connected to gyro interrupt pins,
 * the unused ones havew to be set to -EINVAL
 */
#define LSM330DLC_ACC_DEFAULT_INT1_GPIO	(-EINVAL)
#define LSM330DLC_ACC_DEFAULT_INT2_GPIO	(-EINVAL)

/* Accelerometer Sensor Full Scale */
#define	LSM330DLC_ACC_FS_MASK		(0x30)
#define LSM330DLC_ACC_G_2G		(0x00)
#define LSM330DLC_ACC_G_4G		(0x10)
#define LSM330DLC_ACC_G_8G		(0x20)
#define LSM330DLC_ACC_G_16G		(0x30)



struct lsm330dlc_acc_platform_data {
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

	/* set gpio_int[1,2] either to the choosen gpio pin number or to -EINVAL
	 * if leaved unconnected
	 */
	int gpio_int1;
	int gpio_int2;
};

struct lsm330dlc_gyr_platform_data {
	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);

	unsigned int poll_interval;
	unsigned int min_interval;

	u8 fs_range;

	/* fifo related */
	u8 watermark;
	u8 fifomode;

	/* gpio ports for interrupt pads */
	int gpio_int1;
	int gpio_int2;		/* int for fifo */

	/* axis mapping */
	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;
};
#endif	/* __KERNEL__ */

#endif	/* __LSM330DLC_H__ */



