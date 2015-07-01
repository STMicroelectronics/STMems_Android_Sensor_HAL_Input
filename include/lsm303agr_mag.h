
/******************** (C) COPYRIGHT 2014 STMicroelectronics ********************
*
* File Name	: lsm303agr_mag.h
* Authors	: AMS - Motion Mems Division - Application Team
*		: Armando Visconti (armando.visconti@st.com)
* Version	: V.1.0.0
* Date		: 2015/Jun/21
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

#ifndef	__LSM303AGR_MAG_H__
#define	__LSM303AGR_MAG_H__

#define	LSM303AGR_MAG_DEV_NAME		"lsm303agr_mag"

#ifdef __KERNEL__

struct lsm303agr_mag_platform_data {
	int (*init)(void);
	void (*exit)(void);
};

#endif	/* __KERNEL__ */

#endif	/* __LSM303AGR_MAG_H__ */



