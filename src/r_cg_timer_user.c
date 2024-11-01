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
* File Name    : r_cg_timer_user.c
* Version      : CodeGenerator for RL78/G13 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F100LF
* Tool-Chain   : GCCRL78
* Description  : This file implements device driver for TAU module.
* Creation Date: 7/28/2024
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#include <DGC_A230_Define.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
extern uint8_t Digit_Count;
extern uint8_t Display_Mode;
extern uint8_t Mask;

uint16_t Tick_Count, M1_Tick_Count, M2_Tick_Count, Brake_Tick_Count, Disable_Tick_Count;
uint16_t M1_Position_Count, M2_Position_Count;

uint8_t	HMI_Flash_Count;
uint8_t HMI_Flash_Enable = 0;

extern uint16_t Key_Tick_Count;
extern uint8_t Braking_Time_Enable, Disable_Time_Enable;


extern uint8_t	M1_Timestamp, M2_Timestamp;
extern uint16_t Torque_Inc;
extern uint16_t M1_Torque_Setting, M2_Torque_Setting;

extern uint8_t	Flash_Timer_Counter, Pre_Flash_Type, Flash_Frequency_Config, Flash_Enable;
uint8_t Flash_Frequency_Count;

extern uint8_t  Courtesy_Light_Enable;
extern uint16_t Courtesy_Light_Time, Courtesy_Light_Count;
extern uint8_t Leaf_No1_State, Leaf_No2_State;

extern uint8_t Machine_State;

extern struct Device_Config_Parameters CFG_parameter;

extern uint8_t Gate_Open_Indicator_Mode;
extern uint8_t Gate_Open_Indicator_Flash_Count;

extern uint8_t Auto_Close_Gate_Command;
extern uint8_t Auto_Close_Gate_Timer_Enable;
extern uint16_t Auto_Close_Gate_Timer_Counter;

extern uint8_t Stop_Timer_Enable;
extern uint8_t Stop_Timer_Counter;

/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_tau0_channel0_interrupt
* Description  : This function is INTTM00 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	Key_Tick_Count += 1;
	if (++Digit_Count == 4)
	{
		Digit_Count = 0;
	}
	if (Display_Mode == NORMAL_MODE)
		{
		Check_Safety_Device_Status();
		Display_Status_Callback();
		}
	else
		{
		Display_Parameter_Callback();
		}

	if (Stop_Timer_Enable == 0x01)
	{
		if (++Stop_Timer_Counter == STOP_TIMER_PERIOD_NUMBER)
		{
			Stop_Timer_Enable = 0x00;
		}
	}
	/* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel1_interrupt
* Description  : This function is INTTM01 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel1_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	Tick_Count +=1;
	if ((Leaf_No1_State == GATE_STATE_OPENING) || (Leaf_No1_State == GATE_STATE_CLOSING))
	{
		M1_Tick_Count += 1;
		if ((M1_Timestamp == NORMAL_TIMESTAMP) || (M1_Timestamp == BOOST_TIMESTAMP))
		{
			M1_Position_Count += 3;
		}
		else
		{
			M1_Position_Count += 1;
		}
	}
	if ((Leaf_No2_State == GATE_STATE_OPENING) || (Leaf_No2_State == GATE_STATE_CLOSING))
	{
		M2_Tick_Count += 1;
		if ((M2_Timestamp == NORMAL_TIMESTAMP) || (M2_Timestamp == BOOST_TIMESTAMP))
		{
			M2_Position_Count += 3;
		}
		else
		{
			M2_Position_Count += 1;
		}
	}

	if (Disable_Time_Enable == 0x01)
	{
		if (++Disable_Tick_Count == 4)
		{
			Disable_Time_Enable = 0x00;
		}
	}

	if (Braking_Time_Enable == 0x01)
	{
		if (++Brake_Tick_Count == CFG_parameter.Braking_Time)
		{
			Braking_Time_Enable = 0x00;
		}
	}
	/*
	 * Control by state Revision 2024/03/13
	 */
	if (Machine_State != MACHINE_STATE_STOP)
		Timing_Motor_Machine_State();
	/*
	 * End of  control by state
	 */
	if (M1_Timestamp == STARTUP_TIMESTAMP)
	{
		M1_Torque_Setting -= Torque_Inc;
	}
	if (M2_Timestamp == STARTUP_TIMESTAMP)
	{
		M2_Torque_Setting -= Torque_Inc;
	}
//	if (M1_Timestamp == FINISHED_TIMESTAMP)
//	{
//		M1_Torque_Setting += Torque_Dec;
//	}
//	if (M2_Timestamp == FINISHED_TIMESTAMP)
//	{
//		M2_Torque_Setting += Torque_Dec;
//	}
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel2_interrupt
* Description  : This function is INTTM02 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel2_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	R_TAU0_Channel2_Stop();		// Stop Timer Channel 2
	M1_PWM = 0;					// Set Trigger Pulse Output
//	R_TAU0_Channel3_Start();	// Start Timer Channel 3 for M1 PWM Pulse width
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel3_interrupt
* Description  : This function is INTTM03 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel3_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
//	M1_PWM = 1;					// Clear Trigger Pulse Output
//	R_TAU0_Channel3_Stop();		// Stop Timer Channel 3
   /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel4_interrupt
* Description  : This function is INTTM04 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel4_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	if (HMI_Flash_Enable == 1)
	{
		Mask = ~Mask;
		HMI_Flash_Count += 1;
	}

	if (Pre_Flash_Type == FLASH_ENABLE_TIMER)
	{
		Flash_Timer_Counter += 1;
	}

	if (Flash_Enable == 1)
	{
		if (Flash_Frequency_Config == FLASH_LIGHT_FREQUENCY_RAPIDLY)
		{
			FLASH = ~FLASH;
		}
		else if (Flash_Frequency_Config == FLASH_LIGHT_FREQUENCY_SLOWLY)
		{
			if (++Flash_Frequency_Count == 3)
			{
				Flash_Frequency_Count = 0;
				FLASH = ~FLASH;
			}
		}
		else if (Flash_Frequency_Config == FLASH_LIGHT_FREQUENCY_RECOVERY)	// Recovery Mode
		{
			if ((++Flash_Frequency_Count == 3) && (FLASH == 0))
			{
				Flash_Frequency_Count = 0;
				FLASH = 1;
			}
			if ((Flash_Frequency_Count == 6) && (FLASH == 1))
			{
				Flash_Frequency_Count = 0;
				FLASH = 0;
			}
		}
	}

	if (Courtesy_Light_Enable == 1)
	{
		if (++Courtesy_Light_Count == Courtesy_Light_Time)
		{
			Courtesy_Light_Enable = 0;
			COURTESY = 0;
		}
	}

	switch (Gate_Open_Indicator_Mode)
	{
		case GATE_OPEN_INDICATOR_MODE_1:
		{
			if (++Gate_Open_Indicator_Flash_Count == 3)
			{
				Gate_Open_Indicator_Flash_Count = 0;
				SC_EN = ~SC_EN;
			}
		} break;

		case GATE_OPEN_INDICATOR_MODE_2:
		{
			SC_EN = ~SC_EN;
		} break;

		case GATE_OPEN_INDICATOR_MODE_3:
		{
			if (++Gate_Open_Indicator_Flash_Count == 30)
			{
				Gate_Open_Indicator_Flash_Count = 0;
			}
			if ((Gate_Open_Indicator_Flash_Count == 0) || (Gate_Open_Indicator_Flash_Count == 2))
			{
				SC_EN = 1;
			}
			else
			{
				SC_EN = 0;
			}
		} break;

		case GATE_OPEN_INDICATOR_MODE_0:
		default:
		{
			// Do nothing
		} break;
	}

	if (Auto_Close_Gate_Timer_Enable == 1)
	{
		if (++Auto_Close_Gate_Timer_Counter == CFG_parameter.Automatic_Closing_Time)
		{
			Auto_Close_Gate_Command = AUTO_CLOSE_GATE_COMMAND_ON;
		}
	}
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel6_interrupt
* Description  : This function is INTTM06 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel6_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	R_TAU0_Channel6_Stop();		// Stop Timer Channel 6
	M2_PWM = 0;					// Set M2 PWM Pulse Output
//	R_TAU0_Channel7_Start();	// Start Timer Channel 7 for M2 PWM Pulse width
   /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel7_interrupt
* Description  : This function is INTTM07 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void r_tau0_channel7_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
//	M2_PWM = 1;					// Clear M2 PWM Pulse Output
//	R_TAU0_Channel7_Stop();		// Stop Timer Channel 3
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
