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
* Copyright (C) 2011, 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_main.c
* Version      : CodeGenerator for RL78/G13 V2.05.07.02 [17 Nov 2023]
* Device(s)    : R5F100LF
* Tool-Chain   : GCCRL78
* Description  : This file implements main function.
* Creation Date: 30/09/2024
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_pfdl.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_serial.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#include "string.h"
#include <DGC_A230_Define.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */

uint8_t Display_Mode = NORMAL_MODE;
extern char Parameter_Name[2];
extern char Parameter_Value[2];
extern uint8_t	HMI_Flash_Count;
extern uint8_t Mask;
extern uint8_t HMI_Flash_Enable;
extern struct Device_Config_Parameters CFG_parameter;
extern uint8_t Last_Open_Command_State, Last_Close_Command_State, Last_Clock_Command_State;

uint8_t Power_failure;
//code

uint8_t Power_failure_gate_1;
uint8_t Power_failure_gate_2;
//code

/* End user code. Do not edit comment generated here */
void R_MAIN_UserInit(void);

/***********************************************************************************************************************
* Function Name: main
* Description  : This function implements main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void main(void)
{
    R_MAIN_UserInit();
    /* Start user code. Do not edit comment generated here */
    memcpy(&Parameter_Name[0], "--", sizeof("--"));
	memcpy(&Parameter_Value[0], "--", sizeof("--"));

	Display_Mode = PARA_MODE;

	R_TAU0_Channel0_Start();					// Start Timer channel 0 for display
	R_TAU0_Channel4_Start();					// Start Timer channel 4 for display

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	while (HMI_Flash_Count <1);

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;

    R_FDL_Create();

    if ((!LEFT_KEY)&&(!RIGHT_KEY))					// if Left_Key and Right_Key are pressed
    	Reset_Factory_Default_Parameters(0x00);		// Reset Factory Default Parameters

    Read_Parameters_From_Flash_Memory();			// Read Configuration Parameters

    Calculate_CFG_Parameters();						// Calculate Working parameters

//    //admin
//    Power_failure = POWER_FAILURE_STATE;
//    //admin
    //code
    Power_failure = POWER_GOOD_STATE;
	Power_failure_gate_1 = POWER_FAILURE_STATE;
	Power_failure_gate_2 = POWER_FAILURE_STATE;
//    if (CFG_parameter.Auto_Close_After_Power == 0x01)
//////    	Auto_Power_Failure_Recovery_Process();
//    	Auto_Power_Failure_Recovery_Process_Test();
	//code

    Last_Open_Command_State = COMMAND_RELEASE_STATE;
    Last_Close_Command_State = COMMAND_RELEASE_STATE;
    Last_Clock_Command_State = COMMAND_RELEASE_STATE;

    Normal_Mode();									// Normal working

    while (1U)
    {
    	;
    }

    /* End user code. Do not edit comment generated here */
}


/***********************************************************************************************************************
* Function Name: R_MAIN_UserInit
* Description  : This function adds user code before implementing main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_MAIN_UserInit(void)
{
    /* Start user code. Do not edit comment generated here */
    EI();
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
