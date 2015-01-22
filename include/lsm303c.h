
/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
*
* File Name	: lsm303c.h
* Authors	: AMS - Motion Mems Division - Application Team
*		: Matteo Dameno (matteo.dameno@st.com)
*		: Denis Ciocca (denis.ciocca@st.com)
* Version	: V.1.0.3
* Date		: 2013/Dec/18
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
/******************************************************************************/

#ifndef	__LSM303C_H__
#define	__LSM303C_H__


#define LSM303C_ACC_DEV_NAME			"lsm303c_acc"
#define	LSM303C_MAG_DEV_NAME			"lsm303c_mag"


#ifdef __KERNEL__

/* to set gpios numb connected to interrupt pins,
 * the unused ones have to be set to -EINVAL
 */
#define LSM303C_ACC_DEFAULT_INT1_GPIO		(-EINVAL)

#define LSM303C_MAG_DEFAULT_INT1_GPIO		(-EINVAL)

/* Accelerometer Sensor Full Scale */
#define LSM303C_ACC_FS_MASK			(0x30)
#define LSM303C_ACC_FS_2G			(0x00)
#define LSM303C_ACC_FS_4G			(0x20)
#define LSM303C_ACC_FS_8G			(0x30)

/* Magnetometer Sensor Full Scale */
#define LSM303C_MAG_FS_MASK			(0x60)
#define LSM303C_MAG_FS_4G			(0x00)	/* Full scale 4 G */
#define LSM303C_MAG_FS_8G			(0x20)	/* Full scale 8 G */
#define LSM303C_MAG_FS_10G			(0x40)	/* Full scale 10 G */
#define LSM303C_MAG_FS_16G			(0x60)	/* Full scale 16 G */

#define LSM303C_ACC_MIN_POLL_PERIOD_MS		2
#define LSM303C_MAG_MIN_POLL_PERIOD_MS		13


struct lsm303c_acc_platform_data {
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

	/* set gpio_int[1] to choose gpio pin number or to -EINVAL
	 * if leaved unconnected
	 */
	int gpio_int1;
};

struct lsm303c_mag_platform_data {

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

#endif	/* __KERNEL__ */

#endif	/* __LSM303C_H__ */



