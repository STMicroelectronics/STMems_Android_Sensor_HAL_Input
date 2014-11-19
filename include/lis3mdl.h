/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
*
* File Name          : lis3mdl.h
* Authors            : MSH - C&I BU - Application Team
*		     : Matteo Dameno (matteo.dameno@st.com)
*		     : Denis Ciocca (denis.ciocca@st.com)
* Version            : V.1.0.1
* Date               : 2012/May/07
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
Version History.

Revision 1-0-1 2012/05/04
 first revision
*******************************************************************************/

#ifndef	__LIS3MDL_H__
#define	__LIS3MDL_H__

#define	LIS3MDL_DEV_NAME	"lis3mdl_mag"


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
#define LIS3MDL_MAG_FS_MASK	(0x60)
#define LIS3MDL_MAG_FS_4G	(0x00)	/* Full scale 4 gauss */
#define LIS3MDL_MAG_FS_8G	(0x20)	/* Full scale 8 gauss */
#define LIS3MDL_MAG_FS_10G	(0x40)	/* Full scale 10 gauss */


#ifdef	__KERNEL__

#define DEFAULT_INT1_GPIO		(-EINVAL)
#define DEFAULT_INT2_GPIO		(-EINVAL)

#define LIS3MDL_MAG_MIN_POLL_PERIOD_MS	5


struct lis3mdl_mag_platform_data {

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
#endif	/* __LIS3MDL_H__ */
