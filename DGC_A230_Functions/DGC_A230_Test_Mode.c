/*
 * DGC_A230_Test_Mode.c
 *
 *  Created on: Nov 22, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

#include "string.h"

#include <DGC_A230_Define.h>
#include <Common.h>

extern uint8_t 	Display_Mode;
extern char 	Parameter_Name[2];
extern char 	Parameter_Value[2];
extern uint8_t	HMI_Flash_Count;
extern uint16_t Key_Tick_Count;
extern uint8_t 	Mask;
extern uint8_t	CFG_Parameters_Value[128];

uint8_t Check_Safety_and_Limit_Switch (void)
{
	uint8_t Alarm_And_Limit_Error_Code = 0x00;
	uint8_t Limit_Switch_Activated_Count;

	if (E_STOP)
	{
		return (EMEGENCY_STOP_CONTACT_IS_OPENED);
	}

	if ((COS1) && (CFG_Parameters_Value[73] != 0x00))
	{
		return (SENSING_EDGE_COS1_CONTACT_IS_OPENED);
	}

	if ((COS2) && (CFG_Parameters_Value[74] != 0x00))
	{
		return (SENSING_EDGE_COS2_CONTACT_IS_OPENED);
	}

	if ((FT1) && (!((CFG_Parameters_Value[50] == 0x00) && (CFG_Parameters_Value[51] == 0x00))))
	{
		return (PHOTOCELL_FT1_CONTACT_IS_OPENED);
	}

	if ((FT2) && (!((CFG_Parameters_Value[53] == 0x00) && (CFG_Parameters_Value[54] == 0x00))))
	{
		return (PHOTOCELL_FT2_CONTACT_IS_OPENED);
	}

	Limit_Switch_Activated_Count = 0;
	if (FCC2_IN && (CFG_Parameters_Value[72] == 0x01))
	{
		Alarm_And_Limit_Error_Code = MOTOR_2_CLOSED_LIMIT_SWITCH_IS_ACTIVATED;
		Limit_Switch_Activated_Count += 1;
	}
	if (FCA2_IN && (CFG_Parameters_Value[72] != 0x00))
	{
		Alarm_And_Limit_Error_Code = MOTOR_2_OPEN_LIMIT_SWITCH_IS_ACTIVATED;
		Limit_Switch_Activated_Count += 1;
	}
	if (FCC1_IN && (CFG_Parameters_Value[72] == 0x01))
	{
		Alarm_And_Limit_Error_Code = MOTOR_1_CLOSED_LIMIT_SWITCH_IS_ACTIVATED;
		Limit_Switch_Activated_Count += 1;
	}
	if (FCA1_IN && (CFG_Parameters_Value[72] != 0x00))
	{
		Alarm_And_Limit_Error_Code = MOTOR_1_OPEN_LIMIT_SWITCH_IS_ACTIVATED;
		Limit_Switch_Activated_Count += 1;
	}

	if ((FCC1_IN && (CFG_Parameters_Value[72] == 0x01) && (CFG_Parameters_Value[70] == 0x01))
			|| (FCC1_IN && FCC2_IN && (CFG_Parameters_Value[72] == 0x01) && (CFG_Parameters_Value[70] == 0x02)))
	{
		Alarm_And_Limit_Error_Code = GATE_CLOSED_LIMIT_SWITCH_IS_ACTIVATED;
	}

	if ((FCA1_IN && (CFG_Parameters_Value[72] != 0x00) && (CFG_Parameters_Value[70] == 0x01))
			|| (FCA1_IN && FCA2_IN && (CFG_Parameters_Value[72] != 0x00) && (CFG_Parameters_Value[70] == 0x02)))
	{
		Alarm_And_Limit_Error_Code = GATE_OPEN_LIMIT_SWITCH_IS_ACTIVATED;
	}

	if (Limit_Switch_Activated_Count >= 3)
	{
		Alarm_And_Limit_Error_Code = MORE_THAN_THREE_LIMIT_SW_IS_ACTIVATED;
	}

	return (Alarm_And_Limit_Error_Code);
}
void Test_Command_Process ()
{
	uint8_t working_done = 0;
	uint8_t Alarm_And_Limit_Error_Code = 0x00;

	Display_Mode = TEST_MODE;

	memcpy(&Parameter_Name, "  ", 2);
	convert_binary_to_ascii(Alarm_And_Limit_Error_Code, &Parameter_Value[0], &Parameter_Value[1]);

	Key_Tick_Count = 0;				// Reset Tick count

	while ((working_done == 0) && (Key_Tick_Count < 2000))
	{
		Alarm_And_Limit_Error_Code = Check_Safety_and_Limit_Switch();
		convert_binary_to_ascii(Alarm_And_Limit_Error_Code, &Parameter_Value[0], &Parameter_Value[1]);

		if (!TEST_KEY)										// Test_Key is pressed
		{
			while(!TEST_KEY);								// Do nothing while Test_Key is still pressed
			working_done = 1;
		}

		if (!ORO_IN)
		{
			while(!ORO_IN);
			Key_Tick_Count = 0;								// Reset Tick count
			memcpy(&Parameter_Name, CLOCK_COMMAND, 2);		// Indicate that is Open Command activated
			while (Key_Tick_Count < 1000);					// Do nothing in 5 second
			memcpy(&Parameter_Name, "  ", 2);				// Clear Command Indicator
			Key_Tick_Count = 0;								// Reset Tick count
		}

		if (!AP_IN)
		{
			while(!AP_IN);
			Key_Tick_Count = 0;								// Reset Tick count
			memcpy(&Parameter_Name, OPEN_COMMAND, 2);		// Indicate that is Open Command activated
			while (Key_Tick_Count < 1000);					// Do nothing in 5 second
			memcpy(&Parameter_Name, "  ", 2);				// Clear Command Indicator
			Key_Tick_Count = 0;								// Reset Tick count
		}

		if (!CH_IN)
		{
			while(!CH_IN);
			Key_Tick_Count = 0;								// Reset Tick count
			memcpy(&Parameter_Name, CLOSE_COMMAND, 2);		// Indicate that is Open Command activated
			while (Key_Tick_Count < 1000);					// Do nothing in 5 second
			memcpy(&Parameter_Name, "  ", 2);				// Clear Command Indicator
			Key_Tick_Count = 0;								// Reset Tick count
		}

		if (!PP_IN)
		{
			while(!PP_IN);
			Key_Tick_Count = 0;								// Reset Tick count
			memcpy(&Parameter_Name, STEP_COMMAND, 2);		// Indicate that is Open Command activated
			while (Key_Tick_Count < 1000);					// Do nothing in 5 second
			memcpy(&Parameter_Name, "  ", 2);				// Clear Command Indicator
			Key_Tick_Count = 0;								// Reset Tick count
		}

		if (!PED_IN)
		{
			while(!PED_IN);
			Key_Tick_Count = 0;								// Reset Tick count
			memcpy(&Parameter_Name, PATIAL_COMMAND, 2);		// Indicate that is Open Command activated
			while (Key_Tick_Count < 1000);					// Do nothing in 5 second
			memcpy(&Parameter_Name, "  ", 2);				// Clear Command Indicator
			Key_Tick_Count = 0;								// Reset Tick count
		}
	}

	Display_Mode = NORMAL_MODE;
}

