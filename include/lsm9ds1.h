
/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
*
* File Name          : lsm9ds1_mag.h
* Authors	     : AMS - Motion Mems Division - Application Team
*		     : Giuseppe Barba (giuseppe.barba@st.com)
*		     : Matteo Dameno (matteo.dameno@st.com)
*		     : Denis Ciocca (denis.ciocca@st.com)
* Version            : V.1.0.0
* Date               : 2014/Feb/11
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
********************************************************************************/

#ifndef	__LSM9DS1_H__
#define	__LSM9DS1_H__

#define LSM9DS1_ACC_GYR_DEV_NAME	"lsm9ds1_acc_gyr"
#define LSM9DS1_MAG_DEV_NAME		"lsm9ds1_mag"
#define LSM9DS1_ACC_DEV_NAME		"lsm9ds1_acc"
#define LSM9DS1_GYR_DEV_NAME    	"lsm9ds1_gyr"


/************************************************/
/* 	Output data			 	*/
/*************************************************
magnetometer: raw
*************************************************/

/************************************************/
/* 	sysfs data			 	*/
/*************************************************
magnetometer:
	- pollrate->ms
	- fullscale->gauss
*************************************************/

/* Magnetometer Sensor Full Scale */
#define LSM9DS1_MAG_FS_MASK		(0x60)
#define LSM9DS1_MAG_FS_4G		(0x00)	/* Full scale 4 Gauss */
#define LSM9DS1_MAG_FS_8G		(0x20)	/* Full scale 8 Gauss */
#define LSM9DS1_MAG_FS_10G		(0x40)	/* Full scale 10 Gauss */
#define LSM9DS1_MAG_FS_16G		(0x60)	/* Full scale 16 Gauss */

#ifdef	__KERNEL__

#define MAG_ENABLE_ON_INPUT_OPEN 	0
#define LSM9DS1_MAG_MIN_POLL_PERIOD_MS	13
#define LSM9DS1_INT_M_GPIO_DEF		(-EINVAL)
#define LSM9DS1_M_POLL_INTERVAL_DEF	(100)


/**********************************************/
/* 	Accelerometer section defines	 	*/
/**********************************************/
#define LSM9DS1_ACC_MIN_POLL_PERIOD_MS	1

/* Accelerometer Sensor Full Scale */
#define LSM9DS1_ACC_FS_MASK		(0x18)
#define LSM9DS1_ACC_FS_2G 		(0x00)	/* Full scale 2g */
#define LSM9DS1_ACC_FS_4G 		(0x08)	/* Full scale 4g */
#define LSM9DS1_ACC_FS_8G 		(0x10)	/* Full scale 8g */

/* Accelerometer Anti-Aliasing Filter */
#define LSM9DS1_ACC_BW_408		(0X00)
#define LSM9DS1_ACC_BW_211		(0X01)
#define LSM9DS1_ACC_BW_105		(0X02)
#define LSM9DS1_ACC_BW_50		(0X03)
#define LSM9DS1_ACC_BW_MASK		(0X03)

#define LSM9DS1_INT1_GPIO_DEF		(-EINVAL)
#define LSM9DS1_INT2_GPIO_DEF		(-EINVAL)

#define LSM9DS1_ACC_ODR_OFF		(0x00)
#define LSM9DS1_ACC_ODR_MASK		(0xE0)
#define LSM9DS1_ACC_ODR_10		(0x20)
#define LSM9DS1_ACC_ODR_50		(0x40)
#define LSM9DS1_ACC_ODR_119		(0x60)
#define LSM9DS1_ACC_ODR_238		(0x80)
#define LSM9DS1_ACC_ODR_476		(0xA0)
#define LSM9DS1_ACC_ODR_952		(0xC0)

/**********************************************/
/* 	Gyroscope section defines	 	*/
/**********************************************/
#define LSM9DS1_GYR_MIN_POLL_PERIOD_MS	1

#define LSM9DS1_GYR_FS_MASK		(0x18)
#define LSM9DS1_GYR_FS_245DPS		(0x00)
#define LSM9DS1_GYR_FS_500DPS		(0x08)
#define LSM9DS1_GYR_FS_2000DPS		(0x18)

#define LSM9DS1_GYR_ODR_OFF		(0x00)
#define LSM9DS1_GYR_ODR_MASK		(0xE0)
#define LSM9DS1_GYR_ODR_14_9		(0x20)
#define LSM9DS1_GYR_ODR_59_5		(0x40)
#define LSM9DS1_GYR_ODR_119		(0x60)
#define LSM9DS1_GYR_ODR_238		(0x80)
#define LSM9DS1_GYR_ODR_476		(0xA0)
#define LSM9DS1_GYR_ODR_952		(0xC0)

#define LSM9DS1_GYR_BW_0		(0x00)
#define LSM9DS1_GYR_BW_1		(0x01)
#define LSM9DS1_GYR_BW_2		(0x02)
#define LSM9DS1_GYR_BW_3		(0x03)

#define LSM9DS1_GYR_POLL_INTERVAL_DEF	(100)
#define LSM9DS1_ACC_POLL_INTERVAL_DEF	(100)

struct lsm9ds1_mag_platform_data {
	unsigned int poll_interval;
	unsigned int min_interval;
	u8 fs_range;
	short rot_matrix[3][3];
	int gpio_int_m;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
#ifdef CONFIG_OF
	struct device_node	*of_node;
#endif
};

struct lsm9ds1_acc_platform_data {
	unsigned int poll_interval;
	unsigned int min_interval;
	u8 fs_range;
	u8 aa_filter_bandwidth;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
};

struct lsm9ds1_gyr_platform_data {
	unsigned int poll_interval;
	unsigned int min_interval;
	u8 fs_range;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
};

struct lsm9ds1_acc_gyr_main_platform_data {
	int gpio_int1;
	int gpio_int2;
	short rot_matrix[3][3];
	struct lsm9ds1_acc_platform_data *pdata_acc;
	struct lsm9ds1_gyr_platform_data *pdata_gyr;
#ifdef CONFIG_OF
	struct device_node	*of_node;
#endif
};

#endif	/* __KERNEL__ */
#endif	/* __LSM9DS1_H__ */
