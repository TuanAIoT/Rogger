/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2021 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_intc_user.c
* Version      : CodeGenerator for RL78/G13 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F100LF
* Tool-Chain   : GCCRL78
* Description  : This file implements device driver for INTC module.
* Creation Date: 7/28/2024
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_intc.h"
/* Start user code for include. Do not edit comment generated here */
#include <DGC_A230_Define.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
uint16_t M1_Torque_Setting = TORQUE_LEVEL_8, M2_Torque_Setting = TORQUE_LEVEL_8;
uint8_t M1_Period_Count = 0, M2_Period_Count = 0;
extern uint8_t	M1_Timestamp, M2_Timestamp;

/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_intc0_interrupt
* Description  : This function is INTP0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_intc0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	if (P13_bit.no7 == 1)
	{
		M1_PWM = 1;					// Clear Trigger Pulse Output
		M2_PWM = 1;					// Clear M2 PWM Pulse Output
	}
	else
	{
		TDR02 = M1_Torque_Setting;			// Set Torque for Motor 1
		TDR06 = M2_Torque_Setting;			// Set Torque for Motor 1

		if (M1_Timestamp == DECELERATION_TIMESTAMP)
		{
			if (++M1_Period_Count == 6)
			{
				M1_Period_Count = 0;
			}
			if (M1_Period_Count < 2)
				R_TAU0_Channel2_Start();			// Start Timer Channel 2
		}
		else
		{
			R_TAU0_Channel2_Start();			// Start Timer Channel 2
		}

		if (M2_Timestamp == DECELERATION_TIMESTAMP)
		{
			if (++M2_Period_Count == 6)
			{
				M2_Period_Count = 0;
			}
			if (M2_Period_Count < 2)
				R_TAU0_Channel6_Start();			// Start Timer Channel 2
		}
		else
		{
			R_TAU0_Channel6_Start();			// Start Timer Channel 6
		}
	}
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_intc10_interrupt
* Description  : This function is INTP10 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_intc10_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
