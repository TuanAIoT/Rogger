/*
 * DGC_A230_Motor_Functions.c
 *
 *  Created on: Nov 22, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

#include "string.h"
#include <DGC_A230_Define.h>
#include <Common.h>

extern uint16_t M1_Torque_Setting, M2_Torque_Setting;
extern uint8_t	M1_Timestamp, M2_Timestamp;
extern uint16_t Torque_Inc, Torque_Dec;

uint16_t M1_Time_T1, M1_Time_T2, M1_Time_T3, M1_Time_T4, M1_Time_T5;
uint16_t M2_Time_T1, M2_Time_T2, M2_Time_T3, M2_Time_T4, M2_Time_T5;

extern uint8_t Leaf_No1_State, Leaf_No2_State;
extern struct Device_Config_Parameters CFG_parameter;
extern uint16_t Tick_Count, M1_Tick_Count, M2_Tick_Count, Brake_Tick_Count, Disable_Tick_Count;
extern uint16_t M1_Time_Position, M2_Time_Position;				// Gate position in time
extern uint16_t M1_Last_Time_Position, M2_Last_Time_Position;		// Last Gate Position in time

extern uint8_t Power_failure;
extern char Parameter_Name[2];
extern char Parameter_Value[2];
extern uint8_t	HMI_Flash_Count;
extern uint8_t Mask;
extern uint8_t HMI_Flash_Enable;

extern uint8_t Display_Mode;

uint8_t	Flash_Timer_Counter, Pre_Flash_Type, Flash_Frequency_Config, Flash_Enable = 0;
extern uint8_t Flash_Frequency_Count;

uint8_t Machine_State = MACHINE_STATE_STOP;
uint8_t Braking_Time_Enable = 0x00, Disable_Time_Enable = 0x00;

void Motor1_Disable(void)
{
	M1_FF = 0;
	M1_RR = 0;
//	delay_ms(10);
//	M1_PWM = 1;
}

void Motor1_Open_Enable(void)
{
	M1_FF = 1;
//	delay_ms(20);
}

void Motor1_Close_Enable(void)
{
	M1_RR = 1;
//	delay_ms(20);
}

void Motor2_Disable(void)
{
	M2_FF = 0;
	M2_RR = 0;
//	delay_ms(10);
//	M2_PWM = 1;
}

void Motor2_Open_Enable(void)
{
	M2_FF = 1;
//	delay_ms(20);
}

void Motor2_Close_Enable(void)
{
	M2_RR = 1;
//	delay_ms(20);
}

void Calculate_Torque_Parameter (void)
{
	Torque_Inc = 0x0000;

	if(CFG_parameter.Motor_Start_Time > 0)
	{
		M1_Torque_Setting = TORQUE_LEVEL_1;
		M2_Torque_Setting = TORQUE_LEVEL_1;
		if (CFG_parameter.Motor_Maximum_Boost_Time > 0)
			Torque_Inc = (TORQUE_LEVEL_1 - CFG_parameter.Motor_Torque_Boots) / CFG_parameter.Motor_Start_Time;
		else
			Torque_Inc = (TORQUE_LEVEL_1 - CFG_parameter.Motor_Torque_Maneuver) / CFG_parameter.Motor_Start_Time;
	}
	else
	{
		if (CFG_parameter.Motor_Maximum_Boost_Time > 0)
		{
			M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
		}
		else
		{
			M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		}
	}

	if (CFG_parameter.Motor_Finish_Time > 0)
		Torque_Dec = (TORQUE_LEVEL_1 - CFG_parameter.Motor_Torque_Decelaration) / CFG_parameter.Motor_Finish_Time;
	else
		Torque_Dec = 0x0000;

}

void Setting_Courtesy_Light_and_Flash_Light(uint8_t Moving_Direction)
{
	if(CFG_parameter.Courtery_Light_Mode == 1)	// PULSE 1500ms
	{
		COURTESY = 1;
		delay_ms(1500);
		COURTESY = 0;
	}
	else if (CFG_parameter.Courtery_Light_Mode == 2)	// ACTIVE
	{
		COURTESY = 1;
	}

	if (CFG_parameter.Pre_Flashing == 0x00)
	{
		FLASH = 1;
		Pre_Flash_Type = FLASH_DISABLE_TIMER;
	}
	else if (CFG_parameter.Pre_Flashing < 10)
	{
		FLASH = 1;
		Pre_Flash_Type = FLASH_ENABLE_TIMER;
		Flash_Timer_Counter = 0;
		while (Flash_Timer_Counter < (CFG_parameter.Pre_Flashing * 2)); 	// Flashing warning signal for 1 to 10 seconds prior to every maneuver
		Pre_Flash_Type = FLASH_DISABLE_TIMER;
		FLASH = 0;
	}
	else if (Moving_Direction == CLOSE_MOVING_DIRECTION)
	{
		FLASH = 1;
		Pre_Flash_Type = FLASH_ENABLE_TIMER;
		Flash_Timer_Counter = 0;
		while (Flash_Timer_Counter < 10);									// Flashing warning signal 5 second prior to closing maneuver
		Pre_Flash_Type = FLASH_DISABLE_TIMER;
		FLASH = 0;
	}

	if (CFG_parameter.Flashing_Light_Frequency == 0x00)
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_EXTERNAL;
	else if (CFG_parameter.Flashing_Light_Frequency == 0x01)
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_SLOWLY;
	else if (Moving_Direction == CLOSE_MOVING_DIRECTION)
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RAPIDLY;
	else
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_SLOWLY;

	Flash_Enable = 1;
	Flash_Frequency_Count = 0;

}

void Prepare_To_Moving(uint8_t Moving_Direction)
{
	uint16_t M1_Running_Time, M2_Running_Time;

	Calculate_Torque_Parameter();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		M1_Running_Time = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time - M1_Time_Position;
		M2_Running_Time = CFG_parameter.M2_Maneuver_Time + CFG_parameter.M2_Deceleration_Time - M2_Time_Position;
	}
	else
	{
		M1_Running_Time = M1_Time_Position;
		M2_Running_Time = M2_Time_Position;
	}

	/* Calculate Time point for time stamp*/
	M1_Time_T1 = CFG_parameter.Motor_Start_Time;
	M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time;
	M1_Time_T3 = M1_Time_T2 + (M1_Running_Time / 100) * (100 - CFG_parameter.M1_Deceleration_Space);
	M1_Time_T4 = M1_Time_T3 + (M1_Running_Time / 100) * CFG_parameter.M1_Deceleration_Space;
	M1_Time_T5 = M1_Time_T4 + CFG_parameter.Motor_Finish_Time;

	M2_Time_T1 = CFG_parameter.Motor_Start_Time;
	M2_Time_T2 = M2_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time;
	M2_Time_T3 = M2_Time_T2 + (M2_Running_Time / 100) * (100 - CFG_parameter.M2_Deceleration_Space);
	M2_Time_T4 = M2_Time_T3 + (M2_Running_Time / 100) * CFG_parameter.M2_Deceleration_Space;
	M2_Time_T5 = M2_Time_T4 + CFG_parameter.Motor_Finish_Time;

	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
}

//uint8_t Open_Gate (uint8_t NumOfGate)
//{
//	if ((M1_Tick_Count > M1_Time_T5) || (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))
//	{
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		Motor1_Disable();
//		Leaf_No1_State = GATE_STATE_OPENED;
//	}
//
//	else if (M1_Tick_Count > M1_Time_T4)									// Finished Time
//	{
//		M1_Timestamp = FINISHED_TIMESTAMP;
//	}
//
//	else if (M1_Tick_Count > M1_Time_T3)									// Deceleration Time
//	{
//		M1_Timestamp = DECELERATION_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//	}
//	else if (M1_Tick_Count > M1_Time_T2)									// Normal Time
//	{
//		M1_Timestamp = NORMAL_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//	}
//	else if (M1_Tick_Count > M1_Time_T1)									// Boots Time
//	{
//		M1_Timestamp = BOOST_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//	}
//	else if (M1_Tick_Count > 0)												// Startup Time
//	{
//		M1_Timestamp = STARTUP_TIMESTAMP;
//	}
//
//	if (NumOfGate == 2)														// when using 2 Motors
//	{
//		if((Tick_Count >= CFG_parameter.Motor_No2_Open_Delay_Time) && (Leaf_No2_State != GATE_STATE_OPENED))
//		{
//			Motor2_Open_Enable();											// Enable Motor #2 Open
//			Leaf_No2_State = GATE_STATE_OPENING;
//		}
//
//		if ((M2_Tick_Count > M2_Time_T5) || (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))
//		{
//			M2_Timestamp = UNDEFINE_TIMESTAMP;
//			Motor2_Disable();
//			Leaf_No2_State = GATE_STATE_OPENED;
//		}
//		else if (M2_Tick_Count > M2_Time_T4)								// Finished Time
//		{
//			M2_Timestamp = FINISHED_TIMESTAMP;
//		}
//		else if (M2_Tick_Count > M2_Time_T3)								// Deceleration Time
//		{
//			M2_Timestamp = DECELERATION_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//		}
//		else if (M2_Tick_Count > M2_Time_T2)								// Normal Time
//		{
//			M2_Timestamp = NORMAL_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//		}
//		else if (M2_Tick_Count > M2_Time_T1)								// Boots Time
//		{
//			M2_Timestamp = BOOST_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//		}
//		else if (M2_Tick_Count > 0)											// Startup Time
//		{
//			M2_Timestamp = STARTUP_TIMESTAMP;
//		}
//	}
//
//	if (NumOfGate == 2)
//	{
//		if ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED))
//			return NORMAL_STOP;
//		else
//			return 0x00;
//	}
//	else
//	{
//		if (Leaf_No1_State == GATE_STATE_OPENED)
//			return NORMAL_STOP;
//		else
//			return 0x00;
//	}
//
//}

//uint8_t Close_Gate (uint8_t NumOfGate)
//{
//	if (NumOfGate == 0x02)								// when using 2 Motors
//	{
//		if((Tick_Count >= CFG_parameter.Motor_No1_Close_Delay_Time) && (Leaf_No1_State != GATE_STATE_CLOSED))
//		{
//			Motor1_Close_Enable();											// Enable Motor #2 Open
//			Leaf_No1_State = GATE_STATE_CLOSING;
//		}
//	}
//
//	if ((M1_Tick_Count > M1_Time_T5) || (FCC1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))
//	{
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		Leaf_No1_State = GATE_STATE_CLOSED;
//		Motor1_Disable();
//	}
//	else if (M1_Tick_Count > M1_Time_T4)									// Finished Time
//	{
//		M1_Timestamp = FINISHED_TIMESTAMP;
//	}
//	else if (M1_Tick_Count > M1_Time_T3)									// Deceleration Time
//	{
//		M1_Timestamp = DECELERATION_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//	}
//	else if (M1_Tick_Count > M1_Time_T2)									// Normal Time
//	{
//		M1_Timestamp = NORMAL_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//	}
//	else if (M1_Tick_Count > M1_Time_T1)									// Boots Time
//	{
//		M1_Timestamp = BOOST_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//	}
//	else if (M1_Tick_Count > 0)												// Startup Time
//	{
//		M1_Timestamp = STARTUP_TIMESTAMP;
//	}
//
//	if (NumOfGate == 0x02)								// when using 2 Motors
//	{
//		if ((M2_Tick_Count > M2_Time_T5) || (FCC2_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))
//		{
//			M2_Timestamp = UNDEFINE_TIMESTAMP;
//			Leaf_No2_State = GATE_STATE_CLOSED;
//			Motor2_Disable();
//		}
//		else if (M2_Tick_Count > M2_Time_T4)								// Finished Time
//		{
//			M2_Timestamp = FINISHED_TIMESTAMP;
//		}
//		else if (M2_Tick_Count > M2_Time_T3)								// Deceleration Time
//		{
//			M2_Timestamp = DECELERATION_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//		}
//		else if (M2_Tick_Count > M2_Time_T2)								// Normal Time
//		{
//			M2_Timestamp = NORMAL_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//		}
//		else if (M2_Tick_Count > M2_Time_T1)								// Boots Time
//		{
//			M2_Timestamp = BOOST_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//		}
//		else if (M2_Tick_Count > 0)											// Startup Time
//		{
//			M2_Timestamp = STARTUP_TIMESTAMP;
//		}
//	}
//
//	if (NumOfGate == 2)
//	{
//		if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED))
//			return NORMAL_STOP;
//		else
//			return 0x00;
//	}
//	else
//	{
//		if (Leaf_No1_State == GATE_STATE_CLOSED)
//			return NORMAL_STOP;
//		else
//			return 0x00;
//	}
//}

//uint8_t Do_Open_Gate (uint8_t Step_Mode_Ctrl, uint8_t Open_Cmd_Enable, uint8_t Photocell_Enable, uint8_t Sensing_Enable)
//{
//	uint8_t Open_Moving_Done;
//
//	Prepare_To_Moving(OPEN_MOVING_DIRECTION);		// Prepare to moving
//
//	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
//	if (Leaf_No1_State != GATE_STATE_OPENED)
//		{
//		Motor1_Open_Enable();										// Enable Motor #1 to Open
//		Leaf_No1_State = GATE_STATE_OPENING;
//		}
//
//	Tick_Count = 0;								// Reset Tick count
//	M1_Tick_Count = 0;
//	M2_Tick_Count = 0;
//	Open_Moving_Done = 0;
//
//	R_TAU0_Channel1_Start();					// Start Timer channel 1
//
//	while (Open_Moving_Done == 0x00)
//	{
//		if (CFG_parameter.Number_Of_Motors == 0x02)
//			Open_Moving_Done = Open_Gate(2);
//		else
//			Open_Moving_Done = Open_Gate(1);
//
//		if (E_STOP)
//			{
//			Open_Moving_Done = SAFETY_EMERGENCY_STOP;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((FT1 && (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Open_Moving_Done = SAFETY_PHOTOCELL_FT1;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((FT2 && (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Open_Moving_Done = SAFETY_PHOTOCELL_FT2;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((COS1 && (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)) && (Sensing_Enable == 0x01))
//			{
//			Open_Moving_Done = SAFETY_SENSING_EDGE_COS1;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((COS2 && (CFG_parameter.Sensing_Edge_COS2_Config > 0x02)) && (Sensing_Enable == 0x01))
//			{
//			Open_Moving_Done = SAFETY_SENSING_EDGE_COS2;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			}
//
//		if (!PP_IN)
//		{
//			while(!PP_IN);
//
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//
//			switch (Step_Mode_Ctrl)
//			{
//				case 0x00:
//				{
//					Open_Moving_Done = STEP_COMMAND_STOP;
//				} break;
//
//				case 0x03:
//				case 0x04:
//				{
//					Open_Moving_Done = STEP_COMMAND_REVERSE;
//				} break;
//
//				default:
//				{
//					// Do nothing
//				} break;
//			}
//		}
//
//		if ((AP_IN) && (Open_Cmd_Enable))
//		{
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			Open_Moving_Done = OPEN_COMMAND_STOP;
//		}
//	}
//
//	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
//	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Disable();
//	Motor2_Disable();
//
//	if (Leaf_No1_State != GATE_STATE_OPENED)
//	{
//		if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
//			M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//		M1_Time_Position = 	M1_Last_Time_Position +	M1_Tick_Count;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//	else
//	{
//		M1_Time_Position = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//
//	if (Leaf_No2_State != GATE_STATE_OPENED)
//	{
//		if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
//			M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//		M2_Time_Position = 	M2_Last_Time_Position +	M2_Tick_Count;
//		M2_Last_Time_Position = M2_Time_Position;
//	}
//	else
//	{
//		M2_Time_Position = CFG_parameter.M2_Maneuver_Time + CFG_parameter.M2_Deceleration_Time;
//		M2_Last_Time_Position = M2_Time_Position;
//	}
//
//	return (Open_Moving_Done);
//}

//uint8_t Do_Close_Gate (uint8_t Step_Mode_Ctrl, uint8_t Close_Cmd_Enable, uint8_t Photocell_Enable, uint8_t Sensing_Enable)
//{
//	uint8_t Close_Moving_Done;
//
//	Prepare_To_Moving(CLOSE_MOVING_DIRECTION);		// Prepare to moving
//
//	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
//	if (CFG_parameter.Number_Of_Motors == 0x02)
//	{
//		if (Leaf_No2_State != GATE_STATE_CLOSED)
//		{
//			Motor2_Close_Enable();										// Enable Motor #1 to Open
//			Leaf_No2_State = GATE_STATE_CLOSING;
//		}
//	}
//	else
//	{
//		if (Leaf_No1_State != GATE_STATE_CLOSED)
//		{
//			Motor1_Close_Enable();										// Enable Motor #1 to Open
//			Leaf_No1_State = GATE_STATE_CLOSING;
//		}
//	}
//
//	Tick_Count = 0;								// Reset Tick count
//	M1_Tick_Count = 0;
//	M2_Tick_Count = 0;
//
//	Close_Moving_Done = 0x00;
//
//	R_TAU0_Channel1_Start();					// Start Timer channel 1
//
//	while (Close_Moving_Done == 0)
//	{
//		if (CFG_parameter.Number_Of_Motors == 0x02)
//			Close_Moving_Done = Close_Gate(2);
//		else
//			Close_Moving_Done = Close_Gate(1);
//
//		if (E_STOP)
//			{
//			Close_Moving_Done = SAFETY_EMERGENCY_STOP;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Close_Moving_Done = SAFETY_PHOTOCELL_FT1;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((FT2 && (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Close_Moving_Done = SAFETY_PHOTOCELL_FT2;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((COS1 && (CFG_parameter.Sensing_Edge_COS1_Config > 0x02)) && (Sensing_Enable == 0x01))
//			{
//			Close_Moving_Done = SAFETY_SENSING_EDGE_COS1;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((COS2 && (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)) && (Sensing_Enable == 0x01))
//			{
//			Close_Moving_Done = SAFETY_SENSING_EDGE_COS2;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			}
//
//		if (!PP_IN)
//		{
//			while(!PP_IN);
//
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//
//			switch (Step_Mode_Ctrl)
//			{
//				case 0x00:
//				case 0x01:
//				case 0x02:
//				case 0x04:
//				{
//					Close_Moving_Done = STEP_COMMAND_STOP;
//				} break;
//
//				case 0x03:
//				{
//					Close_Moving_Done = STEP_COMMAND_REVERSE;
//				} break;
//
//				default:
//				{
//					// Do nothing
//				} break;
//			}
//
//			if ((CH_IN) && (Close_Cmd_Enable))
//			{
//				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//				Close_Moving_Done = CLOSE_COMMAND_STOP;
//			}
//		}
//	}
//	R_TAU0_Channel1_Stop();						// Stop Timer channel 1
//	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Disable();
//	Motor2_Disable();
//
////		while(!PP_IN);
//
//	if (Leaf_No1_State != GATE_STATE_CLOSED)
//	{
//		if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//		M1_Time_Position = 	M1_Last_Time_Position -	M1_Tick_Count;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//	else
//	{
//		M1_Time_Position = 0;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//
//	if (Leaf_No2_State != GATE_STATE_CLOSED)
//	{
//		if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//		M2_Time_Position = 	M2_Last_Time_Position -	M2_Tick_Count;
//		M2_Last_Time_Position = M2_Time_Position;
//	}
//	else
//	{
//		M2_Time_Position = 0;
//		M2_Last_Time_Position = M2_Time_Position;
//	}
//
//	return (Close_Moving_Done);
//}

//void Open_Gate_After_Safety_Event (void)
//{
//	uint8_t Open_Finish_Condition;
//
//	Open_Finish_Condition = Do_Open_Gate(0x05,0x00, 0x00, 0x00);
//
//	if (Open_Finish_Condition == NORMAL_STOP)
//	{
//		Flash_Enable = 0;
//		FLASH = 0;
//
//		Courtesy_Light_After_Maneuver_Is_Complete();
//	}
//
//}

//void Close_Gate_After_Safety_Event (void)
//{
//	uint8_t Close_Finish_Condition;
//
//	Close_Finish_Condition = Do_Close_Gate(0x05, 0x00, 0x00, 0x00);
//
//	if (Close_Finish_Condition == NORMAL_STOP)
//	{
//		Flash_Enable = 0;
//		FLASH = 0;
//		Courtesy_Light_After_Maneuver_Is_Complete();
//	}
//}

//void Safety_Event_Process_During_Gate_Opening (uint8_t Event_Type)
//{
//	switch(Event_Type)
//	{
//	case SAFETY_PHOTOCELL_FT1:
//	{
//		if (CFG_parameter.Photocell_FT1_Mode_Opening > 0x02)
//		{
//			while(FT1);
//			if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x03)	// Temporary Stop
//			{
//				Open_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Close_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)	// Immediate Reverse
//		{
//			Close_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_PHOTOCELL_FT2:
//	{
//		if (CFG_parameter.Photocell_FT2_Mode_Opening > 0x02)
//		{
//			while(FT2);
//			if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x03)	// Temporary Stop
//			{
//				Open_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Close_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)	// Immediate Reverse
//		{
//			Close_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_SENSING_EDGE_COS1:
//	case SAFETY_SENSING_EDGE_COS2:
//	case SAFETY_EMERGENCY_STOP:
//	default: break;
//	}
//}

//void Safety_Event_Process_During_Gate_Closing (uint8_t Event_Type)
//{
//	switch(Event_Type)
//	{
//	case SAFETY_PHOTOCELL_FT1:
//	{
//		if (CFG_parameter.Photocell_FT1_Mode_Closing > 0x02)
//		{
//			while(FT1);
//			if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x03)	// Temporary Stop
//			{
//				Close_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Open_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
//		{
//			Open_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_PHOTOCELL_FT2:
//	{
//		if (CFG_parameter.Photocell_FT2_Mode_Closing > 0x02)
//		{
//			while(FT2);
//			if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x03)	// Temporary Stop
//			{
//				Close_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Open_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
//		{
//			Open_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_SENSING_EDGE_COS1:
//	case SAFETY_SENSING_EDGE_COS2:
//	case SAFETY_EMERGENCY_STOP:
//	default: break;
//	}
//}

//void Do_After_Open_Close_Gate (uint8_t Move_Stop_Condition, uint8_t Move_Direction)
//{
//	switch (Move_Stop_Condition)
//	{
//		case NORMAL_STOP:	// Normal finish Open
//		case OPEN_COMMAND_STOP:
//		{
//			Flash_Enable = 0;
//			FLASH = 0;
//			Courtesy_Light_After_Maneuver_Is_Complete();
//		} break;
//
//		case SAFETY_EMERGENCY_STOP:
//		{
//			// Do nothing
//		} break;
//
//		case SAFETY_PHOTOCELL_FT1:
//		case SAFETY_PHOTOCELL_FT2:
//		case SAFETY_SENSING_EDGE_COS1:
//		case SAFETY_SENSING_EDGE_COS2:
//		{
//			if (Move_Direction == 0x00)
//			{
//				Safety_Event_Process_During_Gate_Opening(Move_Stop_Condition);
//			}
//			else
//			{
//				Safety_Event_Process_During_Gate_Closing(Move_Stop_Condition);
//			}
//		} break;
//
//		case STEP_COMMAND_REVERSE:
//		case STEP_COMMAND_STOP:
//		default:
//		{
//			// Do nothing
//		} break;
//	}
//
//}

//void Do_After_Open_Gate (uint8_t Open_Stop_Condition)
//{
//	switch (Open_Stop_Condition)
//	{
//		case NORMAL_STOP:	// Normal finish Open
//		case OPEN_COMMAND_STOP:
//		{
//			Flash_Enable = 0;
//			FLASH = 0;
//			Courtesy_Light_After_Maneuver_Is_Complete();
//		} break;
//
//		case SAFETY_EMERGENCY_STOP:
//		{
//			// Do nothing
//		} break;
//
//		case SAFETY_PHOTOCELL_FT1:
//		case SAFETY_PHOTOCELL_FT2:
//		case SAFETY_SENSING_EDGE_COS1:
//		case SAFETY_SENSING_EDGE_COS2:
//		{
//			Safety_Event_Process_During_Gate_Opening(Open_Stop_Condition);
//		} break;
//
//		case STEP_COMMAND_STOP:
//		case STEP_COMMAND_REVERSE:
//		default:
//		{
//			// Do nothing
//		} break;
//	}
//}

//void Do_After_Close_Gate (uint8_t Close_Stop_Condition)
//{
//
//	switch (Close_Stop_Condition)
//	{
//		case NORMAL_STOP:	// Normal finish Open
//		case CLOSE_COMMAND_STOP:
//		{
//			Flash_Enable = 0;
//			FLASH = 0;
//			Courtesy_Light_After_Maneuver_Is_Complete();
//		} break;
//
//		case SAFETY_EMERGENCY_STOP:
//		{
//			// Do nothing
//		} break;
//
//		case SAFETY_PHOTOCELL_FT1:
//		case SAFETY_PHOTOCELL_FT2:
//		case SAFETY_SENSING_EDGE_COS1:
//		case SAFETY_SENSING_EDGE_COS2:
//		{
//			Safety_Event_Process_During_Gate_Closing(Close_Stop_Condition);
//		} break;
//
//		case STEP_COMMAND_STOP:
//		case STEP_COMMAND_REVERSE:
//		default:
//		{
//			// Do nothing
//		} break;
//	}
//}

void Prepare_To_Partial_Moving(uint8_t Moving_Direction)
{
	uint16_t M1_Running_Time, Temp_Time;

	Calculate_Torque_Parameter();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (CFG_parameter.Number_Of_Motors == 0x02)
			M1_Running_Time = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time - M1_Time_Position;
		else
		{
			Temp_Time = ((CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time) /100)* CFG_parameter.Patial_Open_Adjust;
			if (Temp_Time > M1_Time_Position)
				M1_Running_Time = Temp_Time - M1_Time_Position;
			else
				M1_Running_Time = 0;
		}
	}
	else
	{
		M1_Running_Time = M1_Time_Position;
	}

	/* Calculate Time point for time stamp*/
	M1_Time_T1 = CFG_parameter.Motor_Start_Time;
	M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time;
	M1_Time_T3 = M1_Time_T2 + M1_Running_Time /100 * (100 - CFG_parameter.M1_Deceleration_Space);
	M1_Time_T4 = M1_Time_T3 + M1_Running_Time /100 * CFG_parameter.M1_Deceleration_Space;
	M1_Time_T5 = M1_Time_T4 + CFG_parameter.Motor_Finish_Time;

	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
}

//uint8_t Partial_Moving_Gate(uint8_t Moving_Direction)
//{
//	if ((M1_Tick_Count > M1_Time_T5) || (FCC1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))
//	{
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		Motor1_Disable();
//		if (Moving_Direction == OPEN_MOVING_DIRECTION)
//			Leaf_No1_State = GATE_STATE_PARTIAL;
//		else
//			Leaf_No1_State = GATE_STATE_CLOSED;
//
//		return 0x01;
//	}
//	else if (M1_Tick_Count > M1_Time_T4)									// Finished Time
//	{
//		M1_Timestamp = FINISHED_TIMESTAMP;
//	}
//	else if (M1_Tick_Count > M1_Time_T3)									// Deceleration Time
//	{
//		M1_Timestamp = DECELERATION_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//	}
//	else if (M1_Tick_Count > M1_Time_T2)									// Normal Time
//	{
//		M1_Timestamp = NORMAL_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//	}
//	else if (M1_Tick_Count > M1_Time_T1)									// Boots Time
//	{
//		M1_Timestamp = BOOST_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//	}
//	else if (M1_Tick_Count > 0)												// Startup Time
//	{
//		M1_Timestamp = STARTUP_TIMESTAMP;
//	}
//	return 0x00;
//}

//uint8_t Do_Partial_Open_Gate (uint8_t Photocell_Enable, uint8_t Sensing_Enable)
//{
//	uint8_t Partial_Moving_Done;
//
//	Prepare_To_Partial_Moving(OPEN_MOVING_DIRECTION);
//
//	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Open_Enable();										// Enable Motor #1 to Open
//	Leaf_No1_State = GATE_STATE_OPENING;
//
//	Tick_Count = 0;								// Reset Tick count
//	M1_Tick_Count = 0;
//	Partial_Moving_Done = 0;
//
//	R_TAU0_Channel1_Start();					// Start Timer channel 1
//
//	while (Partial_Moving_Done == 0x00)
//	{
//		Partial_Moving_Done = Partial_Moving_Gate (OPEN_MOVING_DIRECTION);
//		if (E_STOP)
//			{
//			Partial_Moving_Done = SAFETY_EMERGENCY_STOP;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((FT1 && (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_PHOTOCELL_FT1;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((FT2 && (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_PHOTOCELL_FT2;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((COS1 && (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)) && (Sensing_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_SENSING_EDGE_COS1;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//		else if ((COS2 && (CFG_parameter.Sensing_Edge_COS2_Config > 0x02)) && (Sensing_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_SENSING_EDGE_COS2;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//
//		if (!PED_IN)
//		{
//			while(!PED_IN);
//			if (CFG_parameter.Patial_Function == 0x00)
//			{
//				Partial_Moving_Done = PARTIAL_COMMAND_STOP;
//				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			}
//		}
//	}
//
//	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
//	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Disable();
//
//	if (Leaf_No1_State != GATE_STATE_PARTIAL)
//	{
//		if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
//			M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//		M1_Time_Position = 	M1_Last_Time_Position +	M1_Tick_Count;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//	else
//	{
//		if (CFG_parameter.Number_Of_Motors == 0x02)
//			M1_Time_Position = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time;
//		else
//			M1_Time_Position = (CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time) / 100 * CFG_parameter.Patial_Open_Adjust;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//
//	return (Partial_Moving_Done);
//}

//uint8_t Do_Partial_Close_Gate (uint8_t Photocell_Enable, uint8_t Sensing_Enable)
//{
//	uint8_t Partial_Moving_Done;
//
//	Prepare_To_Partial_Moving(CLOSE_MOVING_DIRECTION);
//
//	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Close_Enable();										// Enable Motor #1 to Open
//	Leaf_No1_State = GATE_STATE_CLOSING;
//
//	Tick_Count = 0;								// Reset Tick count
//	M1_Tick_Count = 0;
//
//	Partial_Moving_Done = 0;
//
//	R_TAU0_Channel1_Start();					// Start Timer channel 1
//
//	while (Partial_Moving_Done == 0x00)
//	{
//		Partial_Moving_Done = Partial_Moving_Gate (CLOSE_MOVING_DIRECTION);
//		if (E_STOP)
//			{
//			Partial_Moving_Done = SAFETY_EMERGENCY_STOP;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_PHOTOCELL_FT1;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((FT2 && (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)) && (Photocell_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_PHOTOCELL_FT2;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((COS1 && (CFG_parameter.Sensing_Edge_COS1_Config > 0x02)) && (Sensing_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_SENSING_EDGE_COS1;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			}
//		else if ((COS2 && (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)) && (Sensing_Enable == 0x01))
//			{
//			Partial_Moving_Done = SAFETY_SENSING_EDGE_COS2;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			}
//
//		if (!PED_IN)
//		{
//			while(!PED_IN);
//			Partial_Moving_Done = PARTIAL_COMMAND_STOP;
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//		}
//	}
//
//	R_TAU0_Channel1_Stop();						// Stop Timer channel 1
//	R_INTC0_Stop();								// Enable Sync to control, Motor 1 start opening
//
//	Motor1_Disable();
//
//	if (Leaf_No1_State != GATE_STATE_CLOSED)
//	{
//		if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//		M1_Time_Position = 	M1_Last_Time_Position -	M1_Tick_Count;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//	else
//	{
//		M1_Time_Position = 0;
//		M1_Last_Time_Position = M1_Time_Position;
//	}
//
//	return (Partial_Moving_Done);
//}

//void Partial_Open_Gate_After_Safety_Event()
//{
//	uint8_t Open_Finish_Condition;
//
//	Open_Finish_Condition = Do_Partial_Open_Gate(0x00, 0x00);
//
//	if ((Open_Finish_Condition == NORMAL_STOP) || (Open_Finish_Condition == PARTIAL_COMMAND_STOP))
//	{
//		Flash_Enable = 0;
//		FLASH = 0;
//
//		Courtesy_Light_After_Maneuver_Is_Complete();
//	}
//}

//void Partial_Close_Gate_After_Safety_Event()
//{
//	uint8_t Close_Finish_Condition;
//
//	Close_Finish_Condition = Do_Partial_Close_Gate(0x00, 0x00);
//
//	if ((Close_Finish_Condition == NORMAL_STOP) || (Close_Finish_Condition == PARTIAL_COMMAND_STOP))
//	{
//		Flash_Enable = 0;
//		FLASH = 0;
//
//		Courtesy_Light_After_Maneuver_Is_Complete();
//	}
//
//}

//void Safety_Event_Process_During_Gate_Partial_Opening (uint8_t Event_Type)
//{
//	switch(Event_Type)
//	{
//	case SAFETY_PHOTOCELL_FT1:
//	{
//		if (CFG_parameter.Photocell_FT1_Mode_Opening > 0x02)
//		{
//			while(FT1);
//			if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x03)	// Temporary Stop
//			{
//				Partial_Open_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Partial_Close_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)	// Immediate Reverse
//		{
//			Partial_Close_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_PHOTOCELL_FT2:
//	{
//		if (CFG_parameter.Photocell_FT2_Mode_Opening > 0x02)
//		{
//			while(FT2);
//			if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x03)	// Temporary Stop
//			{
//				Partial_Open_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Partial_Close_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)	// Immediate Reverse
//		{
//			Partial_Close_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_SENSING_EDGE_COS1:
//	case SAFETY_SENSING_EDGE_COS2:
//	case SAFETY_EMERGENCY_STOP:
//	default: break;
//	}
//
//}

//void Safety_Event_Process_During_Gate_Partial_Closing (uint8_t Event_Type)
//{
//	switch(Event_Type)
//	{
//	case SAFETY_PHOTOCELL_FT1:
//	{
//		if (CFG_parameter.Photocell_FT1_Mode_Closing > 0x02)
//		{
//			while(FT1);
//			if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x03)	// Temporary Stop
//			{
//				Partial_Close_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Partial_Open_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
//		{
//			Partial_Open_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_PHOTOCELL_FT2:
//	{
//		if (CFG_parameter.Photocell_FT2_Mode_Closing > 0x02)
//		{
//			while(FT2);
//			if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x03)	// Temporary Stop
//			{
//				Partial_Close_Gate_After_Safety_Event();
//			}
//			else	//Delayed Reverse
//			{
//				Partial_Open_Gate_After_Safety_Event();
//			}
//		}
//		else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
//		{
//			Partial_Open_Gate_After_Safety_Event();
//		}
//	} break;
//
//	case SAFETY_SENSING_EDGE_COS1:
//	case SAFETY_SENSING_EDGE_COS2:
//	case SAFETY_EMERGENCY_STOP:
//	default: break;
//	}
//}

//void Do_After_Partial_Open_Close_Gate (uint8_t Partial_State, uint8_t Move_Direction)
//{
//	switch (Partial_State)
//	{
//		case NORMAL_STOP:	// Normal finish Open
//		case PARTIAL_COMMAND_STOP:
//		{
//			Flash_Enable = 0;
//			FLASH = 0;
//			Courtesy_Light_After_Maneuver_Is_Complete();
//		} break;
//
//		case SAFETY_EMERGENCY_STOP:
//		{
//			// Do nothing
//		} break;
//
//		case SAFETY_PHOTOCELL_FT1:
//		case SAFETY_PHOTOCELL_FT2:
//		case SAFETY_SENSING_EDGE_COS1:
//		case SAFETY_SENSING_EDGE_COS2:
//		{
//			if (Move_Direction == OPEN_MOVING_DIRECTION)
//			{
//				Safety_Event_Process_During_Gate_Partial_Opening(Partial_State);
//			}
//			else
//			{
//				Safety_Event_Process_During_Gate_Partial_Closing(Partial_State);
//			}
//		} break;
//
//		default:
//		{
//			// Do nothing
//		} break;
//	}
//
//}

void Power_Failure_Recovery_Process (void)
{
	uint8_t Recovery_Done = 0;

    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
	memcpy(&Parameter_Value[0], "Co", sizeof("--"));

	Display_Mode = PARA_MODE;

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	Flash_Enable = 1;
	FLASH = 1;

	Pre_Flash_Type = FLASH_ENABLE_TIMER;
	Flash_Timer_Counter = 0;

	//code [A3]
	SC_EN = 0;
	//code [A3] end
	while (Flash_Timer_Counter < 10); // Flashing warning signal 5 second prior to recovery
	Pre_Flash_Type = FLASH_DISABLE_TIMER;

	FLASH = 0;
	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;

	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
	{
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

		M1_Timestamp = UNDEFINE_TIMESTAMP;
		M2_Timestamp = UNDEFINE_TIMESTAMP;

//		//code [A3]
//		R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//		Motor1_Open_Enable();											// Enable Motor #1 Open
//		Leaf_No1_State = GATE_STATE_OPENING;
//
//
//		Tick_Count = 0;													// Reset Tick count
//		M1_Tick_Count = 0;
//		M2_Tick_Count = 0;												// Start counter to M2
//		Recovery_Done = 0;
//
//		R_TAU0_Channel1_Start();										// Start Timer channel 1
//
//		if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//		{
//			while (Tick_Count < CFG_parameter.Motor_No2_Open_Delay_Time);
//			M2_Tick_Count = 0;												// Start counter to M2
//			if (Leaf_No2_State != GATE_STATE_OPENED)
//			{
//				Motor2_Open_Enable();											// Enable Motor #2 Open
//				Leaf_No2_State = GATE_STATE_OPENING;
//			}
//		}
//
//		while (Recovery_Done == 0)				// Start Opening
//		{
//			if ((M1_Tick_Count >= (CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time - CFG_parameter.M1_Time_Increase -150)) 	// Over time
//				|| (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))								// Open Limit switch
//			{
//				Motor1_Disable();
//				Leaf_No1_State = GATE_STATE_OPENED;
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//
//				if ((M2_Tick_Count >= (CFG_parameter.M2_Maneuver_Time + CFG_parameter.M2_Deceleration_Time - CFG_parameter.M2_Time_Increase - 150)) 	// Over time
//					|| (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))								// Open Limit switch
//				{
//					Motor2_Disable();
//					Leaf_No2_State = GATE_STATE_OPENED;
//				}
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//				if ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED))
//				{
//					Recovery_Done = 1;
//				}
//			}
//			else
//			{
//				if (Leaf_No1_State == GATE_STATE_OPENED)
//				{
//					Recovery_Done = 1;
//				}
//			}
//		}
//
//		Tick_Count = 0;													// Reset Tick count
//		while (Tick_Count < 100);										// wait for 1 second
//		R_TAU0_Channel1_Stop();											// Stop Timer channel 1
//
//		//code [A3] end

		if (CFG_parameter.Number_Of_Motors == 0x02)							// when using 2 Motors
		{
			Motor2_Close_Enable();											// Enable Motor #2 Open
			Leaf_No2_State = GATE_STATE_CLOSING;
		}
		else
		{
			Motor1_Close_Enable();											// Enable Motor #1 Open
			Leaf_No1_State = GATE_STATE_CLOSING;
		}

		Tick_Count = 0;													// Reset Tick count
		M1_Tick_Count = 0;
		M2_Tick_Count = 0;												// Start counter to M2
		Recovery_Done = 0;

		R_TAU0_Channel1_Start();										// Start Timer channel 1

		if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
		{
			while (Tick_Count < CFG_parameter.Motor_No1_Close_Delay_Time);
			M1_Tick_Count = 0;
			if (Leaf_No1_State != GATE_STATE_CLOSED)
			{
				Motor1_Close_Enable();											// Enable Motor #2 Open
				Leaf_No1_State = GATE_STATE_CLOSING;
			}
		}
		while (Recovery_Done == 0)				// Start Closing
		{
			if ((M1_Tick_Count >= (CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time - CFG_parameter.M1_Time_Increase -50)) 	// Over time
				|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
			{
				Motor1_Disable();
				Leaf_No1_State = GATE_STATE_CLOSED;
			}

			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
			{

				if ((M2_Tick_Count >= (CFG_parameter.M2_Maneuver_Time + CFG_parameter.M2_Deceleration_Time - CFG_parameter.M2_Time_Increase -50)) 	// Over time
					|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
				{
					Motor2_Disable();
					Leaf_No2_State = GATE_STATE_CLOSED;
				}
			}

			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
			{
				if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED))
				{
					Recovery_Done = 1;
				}
			}
			else
			{
				if (Leaf_No1_State == GATE_STATE_CLOSED)
				{
					Recovery_Done = 1;
				}
			}
		}

		R_TAU0_Channel1_Stop();											// Stop Timer channel 1
		R_INTC0_Stop();													// Disable Sync to control

		M1_Time_Position = 0;
		M1_Last_Time_Position = 0;
		M2_Time_Position = 0;
		M2_Last_Time_Position = 0;

		Power_failure = POWER_GOOD_STATE;
	}
	else												// With Encoder is installed
	{

	}

	Flash_Enable = 0;
	FLASH = 0;

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;
}

void Timing_Motor_Machine_State (void)
{

	if ((M1_Tick_Count > M1_Time_T5))	// Opening
	{
		M1_Timestamp = UNDEFINE_TIMESTAMP;
		Motor1_Disable();
		switch (Machine_State)
		{
			case MACHINE_STATE_NORMAL_OPENING:
			{
				Leaf_No1_State = GATE_STATE_OPENED;
			} break;
			case MACHINE_STATE_NORMAL_CLOSING:
			{
				Leaf_No1_State = GATE_STATE_CLOSED;
			} break;
			case MACHINE_STATE_PARTIAL_OPENING:
			{
				Leaf_No1_State = GATE_STATE_PARTIAL;
			} break;
			case MACHINE_STATE_PARTIAL_CLOSING:
			{
				Leaf_No1_State = GATE_STATE_CLOSED;
			} break;
			default:
			{
				// Do Nothing
			} break;
		}
	}

	else if (M1_Tick_Count > M1_Time_T4)									// Finished Time
	{
		M1_Timestamp = FINISHED_TIMESTAMP;
	}

	else if (M1_Tick_Count > M1_Time_T3)									// Deceleration Time
	{
		M1_Timestamp = DECELERATION_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
	}
	else if (M1_Tick_Count > M1_Time_T2)									// Normal Time
	{
		M1_Timestamp = NORMAL_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
	}
	else if (M1_Tick_Count > M1_Time_T1)									// Boots Time
	{
		M1_Timestamp = BOOST_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
	}
	else if (M1_Tick_Count > 0)												// Startup Time
	{
		M1_Timestamp = STARTUP_TIMESTAMP;
	}

	if (CFG_parameter.Number_Of_Motors == 0x02)	// when using 2 Motors Delay
	{
		if((Tick_Count >= CFG_parameter.Motor_No2_Open_Delay_Time) && (Leaf_No2_State != GATE_STATE_OPENED) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
		{
			Motor2_Open_Enable();											// Enable Motor #2 Open
			Leaf_No2_State = GATE_STATE_OPENING;
		}

		if((Tick_Count >= CFG_parameter.Motor_No1_Close_Delay_Time) && (Leaf_No1_State != GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
		{
			Motor1_Close_Enable();											// Enable Motor #2 Open
			Leaf_No1_State = GATE_STATE_CLOSING;
		}
	}

	if (CFG_parameter.Number_Of_Motors == 0x02)														// when using 2 Motors
	{

		if ((M2_Tick_Count > M2_Time_T5))
		{
			M2_Timestamp = UNDEFINE_TIMESTAMP;
			Motor2_Disable();
			switch (Machine_State)
			{
				case MACHINE_STATE_NORMAL_OPENING:
				{
					Leaf_No2_State = GATE_STATE_OPENED;
				} break;
				case MACHINE_STATE_NORMAL_CLOSING:
				{
					Leaf_No2_State = GATE_STATE_CLOSED;
				} break;
				default:
				{
					// Do Nothing
				} break;
			}
		}
		else if (M2_Tick_Count > M2_Time_T4)								// Finished Time
		{
			M2_Timestamp = FINISHED_TIMESTAMP;
		}
		else if (M2_Tick_Count > M2_Time_T3)								// Deceleration Time
		{
			M2_Timestamp = DECELERATION_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
		}
		else if (M2_Tick_Count > M2_Time_T2)								// Normal Time
		{
			M2_Timestamp = NORMAL_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		}
		else if (M2_Tick_Count > M2_Time_T1)								// Boots Time
		{
			M2_Timestamp = BOOST_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
		}
		else if (M2_Tick_Count > 0)											// Startup Time
		{
			M2_Timestamp = STARTUP_TIMESTAMP;
		}
	}
}

void __Do_Open_Gate (void)
{
	Prepare_To_Moving(OPEN_MOVING_DIRECTION);		// Prepare to moving

	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	if (Leaf_No1_State != GATE_STATE_OPENED)
		{
		Motor1_Open_Enable();										// Enable Motor #1 to Open
		Leaf_No1_State = GATE_STATE_OPENING;
		}

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;
	M2_Tick_Count = 0;

	Machine_State = MACHINE_STATE_NORMAL_OPENING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Close_Gate (void)
{
	Prepare_To_Moving(CLOSE_MOVING_DIRECTION);		// Prepare to moving

	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	if (CFG_parameter.Number_Of_Motors == 0x02)
	{
		if (Leaf_No2_State != GATE_STATE_CLOSED)
		{
			Motor2_Close_Enable();										// Enable Motor #1 to Open
			Leaf_No2_State = GATE_STATE_CLOSING;
		}
	}
	else
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			Motor1_Close_Enable();										// Enable Motor #1 to Open
			Leaf_No1_State = GATE_STATE_CLOSING;
		}
	}

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;
	M2_Tick_Count = 0;

	Machine_State = MACHINE_STATE_NORMAL_CLOSING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Stop_Gate (uint8_t Moving_Direction)
{
	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening

	Motor1_Disable();
	Motor2_Disable();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
	{
		if (Leaf_No1_State != GATE_STATE_OPENED)
		{
			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
				M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M1_Time_Position = 	M1_Last_Time_Position +	M1_Tick_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time;
			M1_Last_Time_Position = M1_Time_Position;
		}

		if (Leaf_No2_State != GATE_STATE_OPENED)
		{
			if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
				M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M2_Time_Position = 	M2_Last_Time_Position +	M2_Tick_Count;
			M2_Last_Time_Position = M2_Time_Position;
		}
		else
		{
			M2_Time_Position = CFG_parameter.M2_Maneuver_Time + CFG_parameter.M2_Deceleration_Time;
			M2_Last_Time_Position = M2_Time_Position;
		}
	}
	else														// Calculate Position when stop Normal Closing
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M1_Time_Position = 	M1_Last_Time_Position -	M1_Tick_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = 0;
			M1_Last_Time_Position = M1_Time_Position;
		}

		if (Leaf_No2_State != GATE_STATE_CLOSED)
		{
			if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M2_Time_Position = 	M2_Last_Time_Position -	M2_Tick_Count;
			M2_Last_Time_Position = M2_Time_Position;
		}
		else
		{
			M2_Time_Position = 0;
			M2_Last_Time_Position = M2_Time_Position;
		}
	}

	Machine_State = MACHINE_STATE_STOP;

	if (((CFG_parameter.Number_Of_Motors == 0x02) && (Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED))
		|| ((CFG_parameter.Number_Of_Motors == 0x02) && (Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED))
		|| ((CFG_parameter.Number_Of_Motors == 0x01) && (Leaf_No1_State == GATE_STATE_CLOSED))
		|| ((CFG_parameter.Number_Of_Motors == 0x01) && (Leaf_No1_State == GATE_STATE_OPENED)))
	{
		Flash_Enable = 0;
		FLASH = 0;
		Courtesy_Light_After_Maneuver_Is_Complete();
	}
}

void __Do_Brake_Motor_Type_1 (uint8_t Current_Direction)
{
	Motor1_Disable();

	Disable_Tick_Count = 0;
	Disable_Time_Enable = 0x01;
	while (Disable_Time_Enable == 0x01);

	M1_Timestamp = BRAKING_TIMESTAMP;
	M1_Torque_Setting = CFG_parameter.Braking_Force;
	if (Current_Direction == OPEN_MOVING_DIRECTION)
	{
		Motor1_Close_Enable();
	}
	else
	{
		Motor1_Open_Enable();
	}

	Brake_Tick_Count = 0;
	Braking_Time_Enable = 0x01;
	while(Braking_Time_Enable == 0x01);
}

void __Do_Brake_Motor_Type_2 (uint8_t Current_Direction)
{
	Motor2_Disable();
	Motor1_Disable();

	Disable_Tick_Count = 0;
	Disable_Time_Enable = 0x01;
	while (Disable_Time_Enable == 0x01);

	M1_Timestamp = BRAKING_TIMESTAMP;
	M2_Timestamp = BRAKING_TIMESTAMP;
	M1_Torque_Setting = CFG_parameter.Braking_Force;
	M2_Torque_Setting = CFG_parameter.Braking_Force;

	if (Current_Direction == OPEN_MOVING_DIRECTION)
	{
		Motor1_Close_Enable();
		Motor2_Close_Enable();
	}
	else
	{
		Motor1_Open_Enable();
		Motor2_Open_Enable();
	}

	Brake_Tick_Count = 0;
	Braking_Time_Enable = 0x01;
	while(Braking_Time_Enable == 0x01);
}

void __Do_Brake_Motor_Type_3 (uint8_t Current_Direction)
{
	Motor2_Disable();

	Disable_Tick_Count = 0;
	Disable_Time_Enable = 0x01;
	while (Disable_Time_Enable == 0x01);

	M2_Timestamp = BRAKING_TIMESTAMP;
	M2_Torque_Setting = CFG_parameter.Braking_Force;

	if (Current_Direction == OPEN_MOVING_DIRECTION)
	{
		Motor2_Close_Enable();
	}
	else
	{
		Motor2_Open_Enable();
	}

	Brake_Tick_Count = 0;
	Braking_Time_Enable = 0x01;
	while(Braking_Time_Enable == 0x01);
}

void __Do_Partial_Open_Gate (void)
{
	Prepare_To_Partial_Moving(OPEN_MOVING_DIRECTION);

	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	Motor1_Open_Enable();										// Enable Motor #1 to Open
	Leaf_No1_State = GATE_STATE_OPENING;

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;

	Machine_State = MACHINE_STATE_PARTIAL_OPENING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Partial_Close_Gate (void)
{
	Prepare_To_Partial_Moving(CLOSE_MOVING_DIRECTION);

	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	Motor1_Close_Enable();										// Enable Motor #1 to Open
	Leaf_No1_State = GATE_STATE_CLOSING;

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;

	Machine_State = MACHINE_STATE_PARTIAL_CLOSING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Partial_Stop_Gate (uint8_t Moving_Direction)
{
	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening

	Motor1_Disable();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (Leaf_No1_State != GATE_STATE_PARTIAL)
		{
			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
				M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
			M1_Time_Position = 	M1_Last_Time_Position +	M1_Tick_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				M1_Time_Position = CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time;
			else
				M1_Time_Position = (CFG_parameter.M1_Maneuver_Time + CFG_parameter.M1_Deceleration_Time) / 100 * CFG_parameter.Patial_Open_Adjust;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}
	else
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
			M1_Time_Position = 	M1_Last_Time_Position -	M1_Tick_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = 0;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}

	Machine_State = MACHINE_STATE_STOP;

	if ((Leaf_No1_State == GATE_STATE_CLOSED)
		|| (Leaf_No1_State == GATE_STATE_OPENED)
		|| (Leaf_No1_State == GATE_STATE_PARTIAL))
	{
		Flash_Enable = 0;
		FLASH = 0;
		Courtesy_Light_After_Maneuver_Is_Complete();
	}
}
