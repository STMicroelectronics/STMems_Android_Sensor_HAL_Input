
/******************** (C) COPYRIGHT 2014 STMicroelectronics ********************
*
* File Name	: lsm303agr.h
* Authors	: AMS - Motion Mems Division - Application Team
*		: Matteo Dameno (matteo.dameno@st.com)
*		: Denis Ciocca (denis.ciocca@st.com)
*		: Both authors are willing to be considered the contact
*		: and update points for the driver.
* Version	: V.1.0.0
* Date		: 2014/Sep/14
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

#ifndef	__LSM303AGR_ACC_H__
#define	__LSM303AGR_ACC_H__

#define	LSM303AGR_ACC_DEV_NAME		"lsm303agr_acc"

/************************************************/
/* 	Output data: ug				*/
/*	Sysfs enable: enable_device		*/
/*	Sysfs odr: pollrate_ms			*/
/*	Sysfs full scale: range			*/
/************************************************/

#define	LSM303AGR_ACC_MIN_POLL_PERIOD_MS		(1)

#ifdef __KERNEL__
/* I2C slave address */
#define LSM303AGR_ACC_I2C_SAD			(0x29)

/* Accelerometer Sensor Full Scale */
#define	LSM303AGR_ACC_FS_MSK			(0x20)
#define LSM303AGR_ACC_G_2G			(0x00)
#define LSM303AGR_ACC_G_8G			(0x20)


#endif	/* __KERNEL__ */

#endif	/* __LSM303AGR_ACC_H__ */
