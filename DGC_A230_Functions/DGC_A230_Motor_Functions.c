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
extern uint16_t Torque_Inc;

uint16_t M1_Time_T1, M1_Time_T2, M1_Time_T3, M1_Time_T4;
uint16_t M2_Time_T1, M2_Time_T2, M2_Time_T3, M2_Time_T4;

extern uint8_t Leaf_No1_State, Leaf_No2_State;
extern struct Device_Config_Parameters CFG_parameter;
extern uint16_t Tick_Count, M1_Tick_Count, M2_Tick_Count, Brake_Tick_Count, Disable_Tick_Count;
extern uint16_t M1_Time_Position, M2_Time_Position;				// Gate position in time
extern uint16_t M1_Last_Time_Position, M2_Last_Time_Position;		// Last Gate Position in time
extern uint16_t M1_Position_Count, M2_Position_Count;

extern uint8_t Power_failure;
extern char Parameter_Name[2];
extern char Parameter_Value[2];
extern uint8_t	HMI_Flash_Count;
extern uint8_t Mask;
extern uint8_t HMI_Flash_Enable;

extern uint8_t Display_Mode;

uint8_t	Flash_Timer_Counter, Pre_Flash_Type, Flash_Frequency_Config, Flash_Enable = 0, Pre_flash_Time = 0;
extern uint8_t Flash_Frequency_Count;

uint8_t Machine_State = MACHINE_STATE_STOP;
uint8_t Braking_Time_Enable = 0x00, Disable_Time_Enable = 0x00;

uint8_t Last_Moving_Direction = GATE_MOVING_DIRECTION_UNKNOW;

uint8_t Stop_Timer_Enable = 0x00;
uint8_t Stop_Timer_Counter = 0x00;

////code
//
//extern uint8_t Last_Sensing_Edge_COS1_status;
//extern uint8_t Last_Sensing_Edge_COS2_status;
//extern uint8_t Last_Close_Command_State;
//extern uint8_t Last_Open_Command_State;
//extern uint8_t Auto_Close_Gate_Command;
//int check_recovery = 1;
////code

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



void Calculate_Torque_Parameter_Motor1 (void)
{
	Torque_Inc = 0x0000;

	if(CFG_parameter.Motor_Start_Time > 0)
	{
		M1_Torque_Setting = TORQUE_LEVEL_1;
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
		}
		else
		{
			M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		}
	}
}


void Calculate_Torque_Parameter_Motor2 (void)
{
//	Torque_Inc = 0x0000;

	if(CFG_parameter.Motor_Start_Time > 0)
	{
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
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
		}
		else
		{
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		}
	}
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

//	if (CFG_parameter.Motor_Finish_Time > 0)
//		Torque_Dec = (TORQUE_LEVEL_1 - CFG_parameter.Motor_Torque_Decelaration) / CFG_parameter.Motor_Finish_Time;
//	else
//		Torque_Dec = 0x0000;

}

void Setting_Courtesy_Light_and_Flash_Light(uint8_t Moving_Direction)
{
	if(CFG_parameter.Courtery_Light_Mode == 1)	// PULSE 1500ms
	{
		COURTESY = 1;
		delay_ms(1500);
		COURTESY = 0;
	}
	else if (CFG_parameter.Courtery_Light_Mode >= 2)	// ACTIVE
	{
		COURTESY = 1;
	}

	if (CFG_parameter.Flashing_Light_Frequency == 0x00)
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_EXTERNAL;
	else if (CFG_parameter.Flashing_Light_Frequency == 0x01)
		{
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_SLOWLY;
		Flash_Enable = 1;
		Flash_Frequency_Count = 0;
		}
	else
		{
		if (Moving_Direction == CLOSE_MOVING_DIRECTION)
			Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RAPIDLY;
		else
			Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_SLOWLY;
		Flash_Enable = 1;
		Flash_Frequency_Count = 0;
		}

	FLASH = 1;

	if (Pre_flash_Time == 0x00) 	//	if (CFG_parameter.Pre_Flashing == 0x00)
	{
		Pre_Flash_Type = FLASH_DISABLE_TIMER;
	}
	else if (Pre_flash_Time < 10)
	{
		FLASH = 1;
		Pre_Flash_Type = FLASH_ENABLE_TIMER;
		Flash_Timer_Counter = 0;
		while (Flash_Timer_Counter < (Pre_flash_Time * 2)); 	// Flashing warning signal for 1 to 10 seconds prior to every maneuver
		Pre_Flash_Type = FLASH_DISABLE_TIMER;
	}
	else
	{
		if (Moving_Direction == CLOSE_MOVING_DIRECTION)
		{
			FLASH = 1;
			Pre_Flash_Type = FLASH_ENABLE_TIMER;
			Flash_Timer_Counter = 0;
			while (Flash_Timer_Counter < 10);								// Flashing warning signal 5 second prior to closing maneuver
			Pre_Flash_Type = FLASH_DISABLE_TIMER;
		}
	}

	FLASH = 1;
}

void Prepare_To_Moving_Motor1(uint8_t Moving_Direction)
{
	uint16_t M1_Running_Time, Temp_Time;

	Calculate_Torque_Parameter_Motor1();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		M1_Running_Time = CFG_parameter.M1_Total_Space - M1_Time_Position;

	}
	else
	{
		M1_Running_Time = M1_Time_Position;
	}

	/* Calculate Time point for time stamp*/

	if (M1_Running_Time > CFG_parameter.M1_Deceleration_Space)
	{
		M1_Time_T1 = CFG_parameter.Motor_Start_Time;

		M1_Running_Time -= CFG_parameter.M1_Deceleration_Space;
		if (M1_Running_Time > CFG_parameter.M1_Maneuver_Space)
		{
			M1_Running_Time -= CFG_parameter.M1_Maneuver_Space;
			if (M1_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
			{
				M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
			}
			else
			{
				M1_Time_T2 = M1_Running_Time;
			}

			M1_Time_T3 = M1_Time_T2 + CFG_parameter.M1_Maneuver_Space;
		}
		else
		{
			M1_Time_T2 = 0;
			M1_Time_T3 = M1_Running_Time;
		}

		M1_Time_T4 = M1_Time_T3 + CFG_parameter.M1_Deceleration_Space;
//		M1_Time_T4 = CFG_parameter.M1_Deceleration_Space;
	}
	else
	{
		M1_Time_T1 = 0;
		M1_Time_T2 = M1_Time_T1 + 0;
		M1_Time_T3 = M1_Time_T2 + 0;
		M1_Time_T4 = M1_Time_T3 + M1_Running_Time;
//		M1_Time_T4 = M1_Running_Time;
	}



	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
		{
			M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;

		}
	}
	else
	{
		if (Leaf_No1_State == GATE_STATE_OPENING_PAUSE)
		{
			M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}
	}
	if(CFG_parameter.Number_Of_Motors == 0x01)
	{
		Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
	}
}


void Prepare_To_Moving_Motor2(uint8_t Moving_Direction)
{
	uint16_t M2_Running_Time, Temp_Time;

	Calculate_Torque_Parameter_Motor2();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		M2_Running_Time = CFG_parameter.M2_Total_Space - M2_Time_Position;
	}
	else
	{
		M2_Running_Time = M2_Time_Position;
	}

	/* Calculate Time point for time stamp*/

	if (M2_Running_Time > CFG_parameter.M2_Deceleration_Space)
	{
		M2_Time_T1 = CFG_parameter.Motor_Start_Time;
		M2_Running_Time -= CFG_parameter.M2_Deceleration_Space;
		if (M2_Running_Time > CFG_parameter.M2_Maneuver_Space)
		{
			M2_Running_Time -= CFG_parameter.M2_Maneuver_Space;
			if (M2_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
			{
				M2_Time_T2 = M2_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
			}
			else
			{
				M2_Time_T2 = M2_Running_Time;
			}

			M2_Time_T3 = M2_Time_T2 + CFG_parameter.M2_Maneuver_Space;
		}
		else
		{
			M2_Time_T2 = 0;
			M2_Time_T3 = M2_Running_Time;
		}

		M2_Time_T4 = M2_Time_T3 + CFG_parameter.M2_Deceleration_Space;
	}
	else
	{
		M2_Time_T1 = 0;
		M2_Time_T2 = M2_Time_T1 + 0;
		M2_Time_T3 = M2_Time_T2 + 0;
		M2_Time_T4 = M2_Time_T3 + M2_Running_Time;
	}

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (Leaf_No2_State == GATE_STATE_CLOSING_PAUSE)
		{
			M2_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}
	}
	else
	{
		if (Leaf_No2_State == GATE_STATE_OPENING_PAUSE)
		{
			M2_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}
	}
	if(CFG_parameter.Number_Of_Motors == 0x02)
	{
		Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
	}
}


void Prepare_To_Moving(uint8_t Moving_Direction)
{
	uint16_t M1_Running_Time, M2_Running_Time, Temp_Time;

	Calculate_Torque_Parameter();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		M1_Running_Time = CFG_parameter.M1_Total_Space - M1_Time_Position;
		M2_Running_Time = CFG_parameter.M2_Total_Space - M2_Time_Position;

	}
	else
	{
		M1_Running_Time = M1_Time_Position;
		M2_Running_Time = M2_Time_Position;
	}

	/* Calculate Time point for time stamp*/

	if (M1_Running_Time > CFG_parameter.M1_Deceleration_Space)
	{
		M1_Time_T1 = CFG_parameter.Motor_Start_Time;

		M1_Running_Time -= CFG_parameter.M1_Deceleration_Space;
		if (M1_Running_Time > CFG_parameter.M1_Maneuver_Space)
		{
			M1_Running_Time -= CFG_parameter.M1_Maneuver_Space;
			if (M1_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
			{
				M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
			}
			else
			{
				M1_Time_T2 = M1_Running_Time;
			}

			M1_Time_T3 = M1_Time_T2 + CFG_parameter.M1_Maneuver_Space;
		}
		else
		{
			M1_Time_T2 = 0;
			M1_Time_T3 = M1_Running_Time;
		}

		M1_Time_T4 = M1_Time_T3 + CFG_parameter.M1_Deceleration_Space;
	}
	else
	{
		M1_Time_T1 = 0;
		M1_Time_T2 = M1_Time_T1 + 0;
		M1_Time_T3 = M1_Time_T2 + 0;
		M1_Time_T4 = M1_Time_T3 + M1_Running_Time;
	}


	if (M2_Running_Time > CFG_parameter.M2_Deceleration_Space)
	{
		M2_Time_T1 = CFG_parameter.Motor_Start_Time;
		M2_Running_Time -= CFG_parameter.M2_Deceleration_Space;
		if (M2_Running_Time > CFG_parameter.M2_Maneuver_Space)
		{
			M2_Running_Time -= CFG_parameter.M2_Maneuver_Space;
			if (M2_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
			{
				M2_Time_T2 = M2_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
			}
			else
			{
				M2_Time_T2 = M2_Running_Time;
			}

			M2_Time_T3 = M2_Time_T2 + CFG_parameter.M2_Maneuver_Space;
		}
		else
		{
			M2_Time_T2 = 0;
			M2_Time_T3 = M2_Running_Time;
		}

		M2_Time_T4 = M2_Time_T3 + CFG_parameter.M2_Deceleration_Space;
	}
	else
	{
		M2_Time_T1 = 0;
		M2_Time_T2 = M2_Time_T1 + 0;
		M2_Time_T3 = M2_Time_T2 + 0;
		M2_Time_T4 = M2_Time_T3 + M2_Running_Time;
	}

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
		{
			M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;

		}

		if (Leaf_No2_State == GATE_STATE_CLOSING_PAUSE)
		{
			M2_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}
	}
	else
	{
		if (Leaf_No1_State == GATE_STATE_OPENING_PAUSE)
		{
			M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}

		if (Leaf_No2_State == GATE_STATE_OPENING_PAUSE)
		{
			M2_Time_T4 += CFG_parameter.Addition_Time_After_INV;
		}
	}

//	M2_Time_T1 = CFG_parameter.Motor_Start_Time;
//	M2_Time_T2 = M2_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time;
//	M2_Time_T3 = M2_Time_T2 + (M2_Running_Time / 100) * (100 - CFG_parameter.M2_Deceleration_Space);
//	M2_Time_T4 = M2_Time_T3 + (M2_Running_Time / 100) * CFG_parameter.M2_Deceleration_Space;

	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
}

void Prepare_To_Partial_Moving(uint8_t Moving_Direction)
{
	uint16_t M1_Running_Time, Temp_Time;

	Calculate_Torque_Parameter_Motor1();

	if (CFG_parameter.Number_Of_Motors == 0x02)
	{
		/* Calculate Time point for time stamp*/
		if (Moving_Direction == OPEN_MOVING_DIRECTION)
		{
			M1_Running_Time = CFG_parameter.M1_Partial_Space - M1_Time_Position;
		}
		else
		{
			M1_Running_Time = M1_Time_Position;
		}

		if (M1_Running_Time > CFG_parameter.M1_Deceleration_Space)
		{
			M1_Time_T1 = CFG_parameter.Motor_Start_Time;
			M1_Running_Time -= CFG_parameter.M1_Deceleration_Space;
			if (M1_Running_Time > CFG_parameter.M1_Maneuver_Space)
			{
				M1_Running_Time -= CFG_parameter.M1_Maneuver_Space;
				if (M1_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
				{
					M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
				}
				else
				{
					M1_Time_T2 = M1_Running_Time;
				}

				M1_Time_T3 = M1_Time_T2 + CFG_parameter.M1_Maneuver_Space;
			}
			else
			{
				M1_Time_T2 = 0;
				M1_Time_T3 = M1_Running_Time;
			}

			M1_Time_T4 = M1_Time_T3 + CFG_parameter.M1_Deceleration_Space;
		}
		else
		{
			M1_Time_T1 = 0;
			M1_Time_T2 = M1_Time_T1 + 0;
			M1_Time_T3 = M1_Time_T2 + 0;
			M1_Time_T4 = M1_Time_T3 + M1_Running_Time;
		}

		if (Moving_Direction == OPEN_MOVING_DIRECTION)
		{
			if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
			{
				M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;

			}
		}
		else
		{
			if (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE)
			{
				M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;
			}
		}
	}
	else
	{
		if (Moving_Direction == OPEN_MOVING_DIRECTION)
		{
			if (CFG_parameter.M1_Partial_Space > M1_Time_Position)
			{
				M1_Running_Time = CFG_parameter.M1_Partial_Space - M1_Time_Position;
			}
			else
			{
				M1_Running_Time = 0;
			}
			M1_Time_T1 = 0;
			M1_Time_T2 = M1_Time_T1 + 0;
			M1_Time_T3 = M1_Time_T2 + M1_Running_Time;
			M1_Time_T4 = M1_Time_T3 + 0;
		}
		else
		{
			M1_Running_Time = M1_Time_Position;
			if (M1_Running_Time > CFG_parameter.M1_Deceleration_Space)
			{
				M1_Time_T1 = CFG_parameter.Motor_Start_Time;
				M1_Running_Time -= CFG_parameter.M1_Deceleration_Space;
				if (M1_Running_Time > CFG_parameter.M1_Maneuver_Space)
				{
					M1_Running_Time -= CFG_parameter.M1_Maneuver_Space;
					if (M1_Running_Time > CFG_parameter.Motor_Maximum_Boost_Time*3)
					{
						M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time*3;
					}
					else
					{
						M1_Time_T2 = M1_Running_Time;
					}

					M1_Time_T3 = M1_Time_T2 + CFG_parameter.M1_Maneuver_Space;
				}
				else
				{
					M1_Time_T2 = 0;
					M1_Time_T3 = M1_Running_Time;
				}

				M1_Time_T4 = M1_Time_T3 + CFG_parameter.M1_Deceleration_Space;
			}
			else
			{
				M1_Time_T1 = 0;
				M1_Time_T2 = M1_Time_T1 + 0;
				M1_Time_T3 = M1_Time_T2 + 0;
				M1_Time_T4 = M1_Time_T3 + M1_Running_Time;
			}

			if (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE)
			{
				M1_Time_T4 += CFG_parameter.Addition_Time_After_INV;
			}
		}
	}
	//	M1_Time_T1 = CFG_parameter.Motor_Start_Time;
//	M1_Time_T2 = M1_Time_T1 + CFG_parameter.Motor_Maximum_Boost_Time;
//	M1_Time_T3 = M1_Time_T2 + M1_Running_Time /100 * (100 - CFG_parameter.M1_Deceleration_Space);
//	M1_Time_T4 = M1_Time_T3 + M1_Running_Time /100 * CFG_parameter.M1_Deceleration_Space;

	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
}
//
//void Auto_Power_Failure_Recovery_Process(void)
//{
//	uint8_t Recovery_Done = 0;
//
//    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
//	memcpy(&Parameter_Value[0], "Co", sizeof("--"));
//
//	Display_Mode = PARA_MODE;
//
//	HMI_Flash_Count = 0;
//	HMI_Flash_Enable = 1;
//
//	Flash_Enable = 1;
//	FLASH = 1;
//
//	Pre_Flash_Type = FLASH_ENABLE_TIMER;
//	Flash_Timer_Counter = 0;
//	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
//	Flash_Frequency_Count = 0;
//	while (Flash_Timer_Counter < 10);									// Flashing warning signal 5 second prior to recovery
//	Pre_Flash_Type = FLASH_DISABLE_TIMER;
//
//	FLASH = 1;
////	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
////	Flash_Frequency_Count = 0;
//
//	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
//	{
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//		M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		M2_Timestamp = UNDEFINE_TIMESTAMP;
//
//		TRIG_EN = 1;			// Enable Pulse out to control Motor
//		R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
//		if (CFG_parameter.Number_Of_Motors == 0x02)							// when using 2 Motors
//		{
//			Motor2_Close_Enable();											// Enable Motor #2 Open
//			Leaf_No2_State = GATE_STATE_CLOSING;
//		}
//		else
//		{
//			Motor1_Close_Enable();											// Enable Motor #1 Open
//			Leaf_No1_State = GATE_STATE_CLOSING;
//		}
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
//			while (Tick_Count < CFG_parameter.Motor_No1_Close_Delay_Time);
//			M1_Tick_Count = 0;
////			if (Leaf_No1_State != GATE_STATE_CLOSED)
////			{
//				Motor1_Close_Enable();											// Enable Motor #2 Open
//				Leaf_No1_State = GATE_STATE_CLOSING;
////			}
//		}
//		while (Recovery_Done == 0)				// Start Closing
//		{
//			if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
//				|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//			{
//				Motor1_Disable();
//				Leaf_No1_State = GATE_STATE_CLOSED;
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//
//				if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
//					|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//				{
//					Motor2_Disable();
//					Leaf_No2_State = GATE_STATE_CLOSED;
//				}
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//				if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED))
//				{
//					Recovery_Done = 1;
//				}
//			}
//			else
//			{
//				if (Leaf_No1_State == GATE_STATE_CLOSED)
//				{
//					Recovery_Done = 1;
//				}
//			}
//		}
//
//		R_TAU0_Channel1_Stop();											// Stop Timer channel 1
//		R_INTC0_Stop();													// Disable Sync to control
//		TRIG_EN = 0;			// Enable Pulse out to control Motor
//		M1_PWM = 1;					// Clear Trigger Pulse Output
//		M2_PWM = 1;					// Clear M2 PWM Pulse Output
//
//		M1_Time_Position = 0;
//		M1_Last_Time_Position = 0;
//		M2_Time_Position = 0;
//		M2_Last_Time_Position = 0;
//
//		Power_failure = POWER_GOOD_STATE;
//	}
//	else												// With Encoder is installed
//	{
//
//	}
//
//	Flash_Enable = 0;
//	FLASH = 0;
//
//	HMI_Flash_Enable = 0;
//	Mask = 0xff;
//
//	Display_Mode = NORMAL_MODE;
//}

//int check()
//{
//
//	//code
//
//	if (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
//	{
//		Open_Limit_Switch_No1_Process();
//		return 0;
//	}
//
//	if (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
//	{
//		Open_Limit_Switch_No2_Process();
//		return 0;
//	}
//
//	if (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
//	{
//		Close_Limit_Switch_No1_Process();
//		return 0;
//	}
//
//	if (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
//	{
//		Close_Limit_Switch_No2_Process();
//		return 0;
//	}
//	if (Auto_Close_Gate_Command == AUTO_CLOSE_GATE_COMMAND_ON)
//	{
//		Auto_Close_Gate_Command_Process();
//		return 0;
//	}
//	//  Emergency Stop
//	if (E_STOP)
//	{
//		Emergency_Stop_Process();
//		return 0;
//	}
//
//	if (FT1)
//	{
//		Photocell_FT1_Process();
//		return 0;
//	}
//
//	if (FT2)
//	{
//		Photocell_FT2_Process();
//		return 0;
//	}
//
//	if ((COS1) && (Last_Sensing_Edge_COS1_status == SENSING_EDGE_STATUS_INACTIVE))
//	{
//		Last_Sensing_Edge_COS1_status = SENSING_EDGE_STATUS_ACTIVE;
//		Sensing_Edge_COS1_Process();
//		return 0;
//	}
//
//	if ((COS2) && (Last_Sensing_Edge_COS2_status == SENSING_EDGE_STATUS_INACTIVE))
//	{
//		Last_Sensing_Edge_COS2_status = SENSING_EDGE_STATUS_ACTIVE;
//		Sensing_Edge_COS2_Process();
//		return 0;
//	}
//
//	if ((!COS1) && (Last_Sensing_Edge_COS1_status == SENSING_EDGE_STATUS_ACTIVE))
//	{
//		Last_Sensing_Edge_COS1_status = SENSING_EDGE_STATUS_INACTIVE;
//		return 0;
//	}
//
//	if ((!COS2) && (Last_Sensing_Edge_COS2_status == SENSING_EDGE_STATUS_ACTIVE))
//	{
//		Last_Sensing_Edge_COS2_status = SENSING_EDGE_STATUS_INACTIVE;
//		return 0;
//	}
//
//	if ((!PED_IN)						// Partial Command is activated
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//		if((CFG_parameter.Manual_Function == 0x00) && (Power_failure == POWER_GOOD_STATE))
//		{
//			Partial_Command_Process();		// Partial Command Procedure
//		}
//		return 0;
//		while(!PED_IN);
//	}
//
//	if ((!PP_IN)							// Step Command is activated
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//
//		if (Power_failure == POWER_FAILURE_STATE)
//		{
////				Manual_Power_Failure_Recovery_Process();
////				Manual_Power_Failure_Recovery_Process_Test();
//			Step_Command_Process_Mode_5();
//		}
//		else
//		{
//			if(CFG_parameter.Manual_Function == 0x00)
//			{
//				Step_Command_Process();			// Step Command Procedure
//			}
//		}
//		return 0;
//		while(!PP_IN);
//	}
//
//	if ((!CH_IN)							// Close Command is activated
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//		Close_Command_Process();		// Close Command Procedure
//		return 0;
//	}
//
//	if (((CH_IN)
//		&& (Last_Close_Command_State == COMMAND_PRESSED_STATE)
//		&& (CFG_parameter.Manual_Function == 0x01))
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//		if (Machine_State == MACHINE_STATE_NORMAL_CLOSING)
//		{
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
//			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
//		}
//		else if (Machine_State == MACHINE_STATE_PARTIAL_CLOSING)
//		{
//			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
//		}
//		Last_Close_Command_State = COMMAND_RELEASE_STATE;
//		return 0;
//	}
//
//	if 	((!AP_IN)
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//		Open_Command_Process();		// Open Command Procedure
//		return 0;
//	}
//
//	if (((AP_IN)
//		&& (Last_Open_Command_State == COMMAND_PRESSED_STATE)
//		&& (CFG_parameter.Manual_Function == 0x01))
//		&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
//	{
//		if (Machine_State == MACHINE_STATE_NORMAL_OPENING)
//			{
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//			__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
//			__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
//			}
//		Last_Open_Command_State = COMMAND_RELEASE_STATE;
//		return 0;
//	}
//	return 1;
//	//code
//}

uint8_t Recovery_Done = 0;
uint8_t a1 = 0;
uint8_t a2;
uint8_t a3;
void Auto_test_start()
{
	if(a1 == 0)
	{
		a1 = 1;

	    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
		memcpy(&Parameter_Value[0], "Co", sizeof("--"));

		Display_Mode = PARA_MODE;

		HMI_Flash_Count = 0;
		HMI_Flash_Enable = 1;

		Flash_Enable = 1;
		FLASH = 1;

		Pre_Flash_Type = FLASH_ENABLE_TIMER;
		Flash_Timer_Counter = 0;
		Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
		Flash_Frequency_Count = 0;
	}
	if(Flash_Timer_Counter >= 10 && a1 == 1)
	{
		a1 = 2;
	}
	if(a1 == 2)
	{
		a1 = 3;
		Pre_Flash_Type = FLASH_DISABLE_TIMER;

		FLASH = 1;
		Recovery_Done = 0;
		if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
		{

			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

			M2_Timestamp = UNDEFINE_TIMESTAMP;

			TRIG_EN = 1;			// Enable Pulse out to control Motor
			R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

			Motor2_Close_Enable();											// Enable Motor #2 Open
			Leaf_No2_State = GATE_STATE_CLOSING;


			Tick_Count = 0;													// Reset Tick count
			M2_Tick_Count = 0;												// Start counter to M2

			R_TAU0_Channel1_Start();										// Start Timer channel 1
		}

		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

		M1_Timestamp = UNDEFINE_TIMESTAMP;


		if (CFG_parameter.Number_Of_Motors == 0x01)				// when using 1 Motors
		{
			TRIG_EN = 1;			// Enable Pulse out to control Motor
			R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

			Tick_Count = 0;													// Reset Tick count
			R_TAU0_Channel1_Start();										// Start Timer channel 1

		}
		else
		{

//						while (Tick_Count < CFG_parameter.Motor_No1_Close_Delay_Time);

		}
	}

	if((Tick_Count > CFG_parameter.Motor_No1_Close_Delay_Time) && a1 == 3)
	{
		a1 = 4;
		M1_Tick_Count = 0;
	//			if (Leaf_No1_State != GATE_STATE_CLOSED)
	//			{
			Motor1_Close_Enable();											// Enable Motor #2 Open
			Leaf_No1_State = GATE_STATE_CLOSING;
	//			}
	}
//		while (Flash_Timer_Counter < 10);									// Flashing warning signal 5 second prior to recovery

}
void Auto_test_end()
{
	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
	{
		if (Recovery_Done == 0)				// Start Closing
		{

//			if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
//				|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//			{
//				Motor1_Disable();
//				Leaf_No1_State = GATE_STATE_CLOSED;
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//
//				if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
//					|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//				{
//					Motor2_Disable();
//					Leaf_No2_State = GATE_STATE_CLOSED;
//				}
//			}
//			Auto_Wait_Recorery_Motor2();
//			Auto_Wait_Recorery_Motor1();
			if(a1 == 4)
			{
				a1 = 5;

			}
			if(a1 == 5)
			{
				a1 = 6;

			}

			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
			{

				if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
					|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
				{
					Motor2_Disable();
					Leaf_No2_State = GATE_STATE_CLOSED;
				}
			}
			if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
				|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
			{
				Motor1_Disable();
				Leaf_No1_State = GATE_STATE_CLOSED;
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
		else {a1 = 7;}
		if(a1 == 7)
		{
			a1 = 8;
			R_TAU0_Channel1_Stop();											// Stop Timer channel 1
			R_INTC0_Stop();													// Disable Sync to control
			TRIG_EN = 0;			// Enable Pulse out to control Motor
			M1_PWM = 1;					// Clear Trigger Pulse Output
			M2_PWM = 1;					// Clear M2 PWM Pulse Output

			M1_Time_Position = 0;
			M1_Last_Time_Position = 0;
			M2_Time_Position = 0;
			M2_Last_Time_Position = 0;

			Power_failure = POWER_GOOD_STATE;
		}

	}
	else												// With Encoder is installed
	{

	}
	if(a1 == 8)
	{
		a1 = 9;

		Flash_Enable = 0;
		FLASH = 0;

		HMI_Flash_Enable = 0;
		Mask = 0xff;

		Display_Mode = NORMAL_MODE;

	}
}
void Manual_test_start()
{

    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
	memcpy(&Parameter_Value[0], "Co", sizeof("--"));

	Display_Mode = PARA_MODE;

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	FLASH = 1;
	Flash_Enable = 1;

	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;

}
void Manual_test_end()
{
	Tick_Count = 0;													// Reset Tick count
	while (Tick_Count < 100);										// wait for 1 second
	R_TAU0_Channel1_Stop();											// Stop Timer channel 1

	R_INTC0_Stop();													// Disable Sync to control
	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M1_PWM = 1;					// Clear Trigger Pulse Output
	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	M1_Time_Position = CFG_parameter.M1_Total_Space;
	M1_Last_Time_Position = M1_Time_Position;
	M2_Time_Position = CFG_parameter.M1_Total_Space;
	M2_Last_Time_Position = M2_Time_Position;

	Power_failure = POWER_GOOD_STATE;


	Flash_Enable = 0;
	FLASH = 0;

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;
}
void Auto_Start_Recovery_Motor_1()
{
	M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

	M1_Timestamp = UNDEFINE_TIMESTAMP;


	if (CFG_parameter.Number_Of_Motors == 0x01)				// when using 1 Motors
	{
		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

		Tick_Count = 0;													// Reset Tick count
		R_TAU0_Channel1_Start();										// Start Timer channel 1

	}
	else
	{

		while (Tick_Count < CFG_parameter.Motor_No1_Close_Delay_Time);

	}

	M1_Tick_Count = 0;
//			if (Leaf_No1_State != GATE_STATE_CLOSED)
//			{
		Motor1_Close_Enable();											// Enable Motor #2 Open
		Leaf_No1_State = GATE_STATE_CLOSING;
//			}

}
void Auto_Start_Recovery_Motor_2()
{

	M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

	M2_Timestamp = UNDEFINE_TIMESTAMP;

	TRIG_EN = 1;			// Enable Pulse out to control Motor
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	Motor2_Close_Enable();											// Enable Motor #2 Open
	Leaf_No2_State = GATE_STATE_CLOSING;


	Tick_Count = 0;													// Reset Tick count
	M2_Tick_Count = 0;												// Start counter to M2

	R_TAU0_Channel1_Start();										// Start Timer channel 1
}
void Auto_Wait_Recorery_Motor1(void)
{
	if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
		|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
	{
		Motor1_Disable();
		Leaf_No1_State = GATE_STATE_CLOSED;
	}
}
void Auto_Wait_Recorery_Motor2(void)
{
	if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
	{

		if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
			|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
		{
			Motor2_Disable();
			Leaf_No2_State = GATE_STATE_CLOSED;
		}
	}
}
void Auto_Power_Failure_Recovery_Process_Test(void)
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
	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;
	while (Flash_Timer_Counter < 10);									// Flashing warning signal 5 second prior to recovery
	Pre_Flash_Type = FLASH_DISABLE_TIMER;

	FLASH = 1;
	Recovery_Done = 0;
//	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
//	Flash_Frequency_Count = 0;
	if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
	{
		Auto_Start_Recovery_Motor_2();
	}
	Auto_Start_Recovery_Motor_1();
	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
	{
		while (Recovery_Done == 0)				// Start Closing
		{

//			if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
//				|| (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//			{
//				Motor1_Disable();
//				Leaf_No1_State = GATE_STATE_CLOSED;
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//
//				if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
//					|| (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01)))								// Open Limit switch
//				{
//					Motor2_Disable();
//					Leaf_No2_State = GATE_STATE_CLOSED;
//				}
//			}
			Auto_Wait_Recorery_Motor2();
			Auto_Wait_Recorery_Motor1();
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
		TRIG_EN = 0;			// Enable Pulse out to control Motor
		M1_PWM = 1;					// Clear Trigger Pulse Output
		M2_PWM = 1;					// Clear M2 PWM Pulse Output

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

//void Manual_Power_Failure_Recovery_Process (void)
//{
//	uint8_t Recovery_Done = 0;
//
//    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
//	memcpy(&Parameter_Value[0], "Co", sizeof("--"));
//
//	Display_Mode = PARA_MODE;
//
//	HMI_Flash_Count = 0;
//	HMI_Flash_Enable = 1;
//
//	FLASH = 1;
//	Flash_Enable = 1;
//
//	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
//	Flash_Frequency_Count = 0;
//
//	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
//	{
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//		M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		M2_Timestamp = UNDEFINE_TIMESTAMP;
//
//		TRIG_EN = 1;			// Enable Pulse out to control Motor
//		R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
//
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
////			if (Leaf_No2_State != GATE_STATE_OPENED)
////			{
//				Motor2_Open_Enable();											// Enable Motor #2 Open
//				Leaf_No2_State = GATE_STATE_OPENING;
////			}
//		}
//
//		while (Recovery_Done == 0)				// Start Opening
//		{
//			if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
//				|| (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))								// Open Limit switch
//			{
//				Motor1_Disable();
//				Leaf_No1_State = GATE_STATE_OPENED;
//			}
//
//			if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
//			{
//
//				if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
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
//		R_INTC0_Stop();													// Disable Sync to control
//		TRIG_EN = 0;			// Enable Pulse out to control Motor
//		M1_PWM = 1;					// Clear Trigger Pulse Output
//		M2_PWM = 1;					// Clear M2 PWM Pulse Output
//
//		M1_Time_Position = CFG_parameter.M1_Total_Space;
//		M1_Last_Time_Position = M1_Time_Position;
//		M2_Time_Position = CFG_parameter.M1_Total_Space;
//		M2_Last_Time_Position = M2_Time_Position;
//
//		Power_failure = POWER_GOOD_STATE;
//	}
//	else												// With Encoder is installed
//	{
//
//	}
//
//	Flash_Enable = 0;
//	FLASH = 0;
//
//	HMI_Flash_Enable = 0;
//	Mask = 0xff;
//
//	Display_Mode = NORMAL_MODE;
//}


void Manual_Start_Recovery_Motor_1(void)
{

	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
	{
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

		M1_Timestamp = UNDEFINE_TIMESTAMP;

		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

		Motor1_Open_Enable();											// Enable Motor #1 Open
		Leaf_No1_State = GATE_STATE_OPENING;


		Tick_Count = 0;													// Reset Tick count
		M1_Tick_Count = 0;											// Start counter to M2

		R_TAU0_Channel1_Start();										// Start Timer channel 1

	}
}
void Manual_Start_Recovery_Motor_2(void)
{
	if (CFG_parameter.Encoder_Config == 0x00)			// Without Encoder is installed
	{
		M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

		M2_Timestamp = UNDEFINE_TIMESTAMP;
		if (CFG_parameter.Number_Of_Motors == 0x01)				// when using 2 Motors
		{
			TRIG_EN = 1;			// Enable Pulse out to control Motor
			R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

			Tick_Count = 0;													// Reset Tick count
			R_TAU0_Channel1_Start();										// Start Timer channel 1
		}
		M2_Tick_Count = 0;												// Start counter to M2


		if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
		{
			while (Tick_Count < CFG_parameter.Motor_No2_Open_Delay_Time);
			M2_Tick_Count = 0;												// Start counter to M2
//			if (Leaf_No2_State != GATE_STATE_OPENED)
//			{
				Motor2_Open_Enable();											// Enable Motor #2 Open
				Leaf_No2_State = GATE_STATE_OPENING;
//			}
		}

	}
	else												// With Encoder is installed
	{

	}
}

void Manual_Wait_Recorery_Motor1(void)
{

	if ((M1_Tick_Count >= (CFG_parameter.M1_Travel_Time)) 	// Over time
		|| (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))								// Open Limit switch
	{
		Motor1_Disable();
		Leaf_No1_State = GATE_STATE_OPENED;
	}
}
void Manual_Wait_Recorery_Motor2(void)
{
	if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
	{

		if ((M2_Tick_Count >= (CFG_parameter.M2_Travel_Time)) 	// Over time
			|| (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00)))								// Open Limit switch
		{
			Motor2_Disable();
			Leaf_No2_State = GATE_STATE_OPENED;
		}
	}
}

void Manual_Power_Failure_Recovery_Process_Test (void)
{
	uint8_t Recovery_Done = 0;

    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
	memcpy(&Parameter_Value[0], "Co", sizeof("--"));

	Display_Mode = PARA_MODE;

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	FLASH = 1;
	Flash_Enable = 1;

	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;

	Recovery_Done = 0;
	Manual_Start_Recovery_Motor_1();
	if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
	{
		while (Tick_Count < CFG_parameter.Motor_No2_Open_Delay_Time);
		Manual_Start_Recovery_Motor_2();
	}
	while (Recovery_Done == 0)				// Start Opening
	{
		Manual_Wait_Recorery_Motor1();
		Manual_Wait_Recorery_Motor2();
		if (CFG_parameter.Number_Of_Motors == 0x02)				// when using 2 Motors
		{
			if ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED))
			{
				Recovery_Done = 1;
			}
		}
		else
		{
			if (Leaf_No1_State == GATE_STATE_OPENED)
			{
				Recovery_Done = 1;
			}
		}
	}

	Tick_Count = 0;													// Reset Tick count
	while (Tick_Count < 100);										// wait for 1 second
	R_TAU0_Channel1_Stop();											// Stop Timer channel 1

	R_INTC0_Stop();													// Disable Sync to control
	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M1_PWM = 1;					// Clear Trigger Pulse Output
	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	M1_Time_Position = CFG_parameter.M1_Total_Space;
	M1_Last_Time_Position = M1_Time_Position;
	M2_Time_Position = CFG_parameter.M1_Total_Space;
	M2_Last_Time_Position = M2_Time_Position;

	Power_failure = POWER_GOOD_STATE;


	Flash_Enable = 0;
	FLASH = 0;

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;
}

void Start_auto_recovery()
{
	//display
    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
	memcpy(&Parameter_Value[0], "Co", sizeof("--"));

	Display_Mode = PARA_MODE;

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	Flash_Enable = 1;
	FLASH = 1;
	//end display

	Pre_Flash_Type = FLASH_ENABLE_TIMER;
	Flash_Timer_Counter = 0;
	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;
	while (Flash_Timer_Counter < 10);									// Flashing warning signal 5 second prior to recovery
	Pre_Flash_Type = FLASH_DISABLE_TIMER;

	FLASH = 1;

//    Power_failure_gate_1 = POWER_FAILURE_STATE;
//	Power_failure_gate_2 = POWER_FAILURE_STATE;
	Tick_Count = 0;													// Reset Tick count
	R_TAU0_Channel1_Start();
	R_INTC0_Start();
	TRIG_EN = 1;
//	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
//	R_INTC0_Stop();													// Disable Sync to control
//	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M1_PWM = 1;					// Clear Trigger Pulse Output
	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	M1_Time_Position = CFG_parameter.M1_Total_Space;
	M1_Last_Time_Position = M1_Time_Position;
	M2_Time_Position = CFG_parameter.M1_Total_Space;
	M2_Last_Time_Position = M2_Time_Position;

	Power_failure = POWER_GOOD_STATE;


//	Flash_Enable = 0;
//	FLASH = 0;
//
//	HMI_Flash_Enable = 0;
//	Mask = 0xff;

}
void End_auto_recovery()
{

	Flash_Enable = 0;
	FLASH = 0;

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;

}

void Start_manual_recovery()
{
	//display

    memcpy(&Parameter_Name[0], "rE", sizeof("--"));
	memcpy(&Parameter_Value[0], "Co", sizeof("--"));

	Display_Mode = PARA_MODE;

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	FLASH = 1;
	Flash_Enable = 1;

	Flash_Frequency_Config = FLASH_LIGHT_FREQUENCY_RECOVERY;
	Flash_Frequency_Count = 0;
	//end display



	M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
	M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

	M1_Timestamp = UNDEFINE_TIMESTAMP;
	M2_Timestamp = UNDEFINE_TIMESTAMP;


	Tick_Count = 0;													// Reset Tick count
	R_TAU0_Channel1_Start();
	R_INTC0_Start();
	TRIG_EN = 1;

	M1_PWM = 1;					// Clear Trigger Pulse Output
	M2_PWM = 1;					// Clear M2 PWM Pulse Output


	Tick_Count = 0;													// Reset Tick count
	M1_Tick_Count = 0;
	M2_Tick_Count = 0;												// Start counter to M2
	Leaf_No1_State = GATE_STATE_CLOSED;
	Leaf_No2_State = GATE_STATE_CLOSED;
	Power_failure = POWER_GOOD_STATE;


//	Flash_Enable = 0;
//	FLASH = 0;
//
//	HMI_Flash_Enable = 0;
//	Mask = 0xff;

}
void End_manual_recovery()
{

	Flash_Enable = 0;
	FLASH = 0;

	HMI_Flash_Enable = 0;
	Mask = 0xff;

	Display_Mode = NORMAL_MODE;

}
void Timing_Motor1_Machine_State (void)
{

	if ((M1_Position_Count > M1_Time_T4))	// Opening
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

	else if (M1_Position_Count > M1_Time_T3)									// Deceleration Time
	{
		M1_Timestamp = DECELERATION_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
	}
	else if (M1_Position_Count > M1_Time_T2)									// Normal Time
	{
		M1_Timestamp = NORMAL_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
	}
	else if (M1_Position_Count > M1_Time_T1)									// Boots Time
	{
		M1_Timestamp = BOOST_TIMESTAMP;
		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
	}
	else if (M1_Position_Count > 0)												// Startup Time
	{
		M1_Timestamp = STARTUP_TIMESTAMP;
	}


	if((Tick_Count >= CFG_parameter.Motor_No1_Close_Delay_Time) && (Leaf_No1_State != GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
	{
		Motor1_Close_Enable();											// Enable Motor #2 Open
		Leaf_No1_State = GATE_STATE_CLOSING;
	}
}
void Timing_Motor2_Machine_State (void)
{

	if (CFG_parameter.Number_Of_Motors == 0x02)	// when using 2 Motors Delay
	{
		if((Tick_Count >= CFG_parameter.Motor_No2_Open_Delay_Time) && (Leaf_No2_State != GATE_STATE_OPENED) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
		{
			Motor2_Open_Enable();											// Enable Motor #2 Open
			Leaf_No2_State = GATE_STATE_OPENING;
		}
	}

	if (CFG_parameter.Number_Of_Motors == 0x02)														// when using 2 Motors
	{

		if ((M2_Position_Count > M2_Time_T4))
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
		else if (M2_Position_Count > M2_Time_T3)								// Deceleration Time
		{
			M2_Timestamp = DECELERATION_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
		}
		else if (M2_Position_Count > M2_Time_T2)								// Normal Time
		{
			M2_Timestamp = NORMAL_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
		}
		else if (M2_Position_Count > M2_Time_T1)								// Boots Time
		{
			M2_Timestamp = BOOST_TIMESTAMP;
			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
		}
		else if (M2_Position_Count > 0)											// Startup Time
		{
			M2_Timestamp = STARTUP_TIMESTAMP;
		}
	}
}
//void Timing_Motor_Machine_State (void)
//{
//
//	if ((M1_Position_Count > M1_Time_T4))	// Opening
//	{
//		M1_Timestamp = UNDEFINE_TIMESTAMP;
//		Motor1_Disable();
//		switch (Machine_State)
//		{
//			case MACHINE_STATE_NORMAL_OPENING:
//			{
//				Leaf_No1_State = GATE_STATE_OPENED;
//			} break;
//			case MACHINE_STATE_NORMAL_CLOSING:
//			{
//				Leaf_No1_State = GATE_STATE_CLOSED;
//			} break;
//			case MACHINE_STATE_PARTIAL_OPENING:
//			{
//				Leaf_No1_State = GATE_STATE_PARTIAL;
//			} break;
//			case MACHINE_STATE_PARTIAL_CLOSING:
//			{
//				Leaf_No1_State = GATE_STATE_CLOSED;
//			} break;
//			default:
//			{
//				// Do Nothing
//			} break;
//		}
//	}
//
//	else if (M1_Position_Count > M1_Time_T3)									// Deceleration Time
//	{
//		M1_Timestamp = DECELERATION_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//	}
//	else if (M1_Position_Count > M1_Time_T2)									// Normal Time
//	{
//		M1_Timestamp = NORMAL_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//	}
//	else if (M1_Position_Count > M1_Time_T1)									// Boots Time
//	{
//		M1_Timestamp = BOOST_TIMESTAMP;
//		M1_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//	}
//	else if (M1_Position_Count > 0)												// Startup Time
//	{
//		M1_Timestamp = STARTUP_TIMESTAMP;
//	}
//
//	if (CFG_parameter.Number_Of_Motors == 0x02)	// when using 2 Motors Delay
//	{
//		if((Tick_Count >= CFG_parameter.Motor_No2_Open_Delay_Time) && (Leaf_No2_State != GATE_STATE_OPENED) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
//		{
//			Motor2_Open_Enable();											// Enable Motor #2 Open
//			Leaf_No2_State = GATE_STATE_OPENING;
//		}
//
//		if((Tick_Count >= CFG_parameter.Motor_No1_Close_Delay_Time) && (Leaf_No1_State != GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
//		{
//			Motor1_Close_Enable();											// Enable Motor #2 Open
//			Leaf_No1_State = GATE_STATE_CLOSING;
//		}
//	}
//
//	if (CFG_parameter.Number_Of_Motors == 0x02)														// when using 2 Motors
//	{
//
//		if ((M2_Position_Count > M2_Time_T4))
//		{
//			M2_Timestamp = UNDEFINE_TIMESTAMP;
//			Motor2_Disable();
//			switch (Machine_State)
//			{
//				case MACHINE_STATE_NORMAL_OPENING:
//				{
//					Leaf_No2_State = GATE_STATE_OPENED;
//				} break;
//				case MACHINE_STATE_NORMAL_CLOSING:
//				{
//					Leaf_No2_State = GATE_STATE_CLOSED;
//				} break;
//				default:
//				{
//					// Do Nothing
//				} break;
//			}
//		}
//		else if (M2_Position_Count > M2_Time_T3)								// Deceleration Time
//		{
//			M2_Timestamp = DECELERATION_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Decelaration;
//		}
//		else if (M2_Position_Count > M2_Time_T2)								// Normal Time
//		{
//			M2_Timestamp = NORMAL_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
//		}
//		else if (M2_Position_Count > M2_Time_T1)								// Boots Time
//		{
//			M2_Timestamp = BOOST_TIMESTAMP;
//			M2_Torque_Setting = CFG_parameter.Motor_Torque_Boots;
//		}
//		else if (M2_Position_Count > 0)											// Startup Time
//		{
//			M2_Timestamp = STARTUP_TIMESTAMP;
//		}
//	}
//}


void __Do_Moving_Gate_After_COS_Action_Motor1_Start(uint8_t Moving_Direction, uint8_t COS_Type)
{
	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);

	M1_Torque_Setting = CFG_parameter.Motor_Torque_After_Edge_Sense;

	M1_Timestamp = UNDEFINE_TIMESTAMP;

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		Motor1_Open_Enable();										// Enable Motor #1 to Open
		Leaf_No1_State = GATE_STATE_OPENING;
//		Machine_State = MACHINE_STATE_COS_OPENING;
	}
	else
	{
		Motor1_Close_Enable();										// Enable Motor #1 Close
		Leaf_No1_State = GATE_STATE_CLOSING;

//		Machine_State = MACHINE_STATE_COS_CLOSING;
	}
	Tick_Count = 0;

	TRIG_EN = 1;													// Enable Pulse out to control Motor
	R_INTC0_Start();												// Enable Sync to control
	R_TAU0_Channel1_Start();										// Start Timer channel 1
}
void __Do_Moving_Gate_After_COS_Action_Motor1_End(uint8_t Moving_Direction, uint8_t COS_Type)
{
	while(Tick_Count < CFG_parameter.Reverse_Time_After_Edge_Sense);

	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
	R_INTC0_Stop();													// Disable Sync to control
	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M1_PWM = 1;					// Clear Trigger Pulse Output

	Motor1_Disable();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
	{
		M1_Time_Position = 	M1_Last_Time_Position +	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		if (M1_Time_Position > CFG_parameter.M1_Total_Space)
		{
			Leaf_No1_State = GATE_STATE_OPENED;
			M1_Time_Position = CFG_parameter.M1_Total_Space;
		}
		M1_Last_Time_Position = M1_Time_Position;

	}
	else														// Calculate Position when stop Normal Closing
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		if (M1_Last_Time_Position > CFG_parameter.Reverse_Time_After_Edge_Sense*3)
		{
			M1_Time_Position = 	M1_Last_Time_Position -	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
		}
		else
		{
			Leaf_No1_State = GATE_STATE_CLOSED;
			M1_Time_Position = 0;
		}
		M1_Last_Time_Position = M1_Time_Position;

	}

	Flash_Enable = 0;
	FLASH = 0;

	if (CFG_parameter.Number_Of_Motors != 0x02)
	Courtesy_Light_After_Maneuver_Is_Complete();

}
void __Do_Moving_Gate_After_COS_Action_Motor2_Start(uint8_t Moving_Direction, uint8_t COS_Type)
{

	if (CFG_parameter.Number_Of_Motors != 0x02)
	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);

	M2_Torque_Setting = CFG_parameter.Motor_Torque_After_Edge_Sense;

	M2_Timestamp = UNDEFINE_TIMESTAMP;

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (COS_Type == 0x02)
		{
			Motor2_Open_Enable();									// Enable Motor #2 to Open
			Leaf_No2_State = GATE_STATE_OPENING;
		}
//		Machine_State = MACHINE_STATE_COS_OPENING;
	}
	else
	{
		if (COS_Type == 0x02)
		{
			Motor2_Close_Enable();									// Enable Motor #2 Close
			Leaf_No2_State = GATE_STATE_CLOSING;
		}

//		Machine_State = MACHINE_STATE_COS_CLOSING;
	}
	if(TRIG_EN != 1)
	{
		Tick_Count = 0;

		TRIG_EN = 1;													// Enable Pulse out to control Motor
		R_INTC0_Start();												// Enable Sync to control
		R_TAU0_Channel1_Start();										// Start Timer channel 1
	}
}
void __Do_Moving_Gate_After_COS_Action_Motor2_End(uint8_t Moving_Direction, uint8_t COS_Type)
{
	while(Tick_Count < CFG_parameter.Reverse_Time_After_Edge_Sense);

	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
	R_INTC0_Stop();													// Disable Sync to control
	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	Motor2_Disable();

	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
	{

		if (COS_Type == 0x02)
		{
			M2_Time_Position = 	M2_Last_Time_Position +	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			if (M2_Time_Position > CFG_parameter.M2_Total_Space)
			{
				Leaf_No1_State = GATE_STATE_OPENED;
				M2_Time_Position = CFG_parameter.M2_Total_Space;
			}
			M2_Last_Time_Position = M2_Time_Position;
		}
	}
	else														// Calculate Position when stop Normal Closing
	{

		if (COS_Type == 0x02)
		{
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			if (M2_Last_Time_Position > CFG_parameter.Reverse_Time_After_Edge_Sense*3)
			{
				M2_Time_Position = 	M2_Last_Time_Position -	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
			}
			else
			{
				Leaf_No2_State = GATE_STATE_CLOSED;
				M2_Time_Position = 0;
			}
			M2_Last_Time_Position = M2_Time_Position;
		}
	}

	Flash_Enable = 0;
	FLASH = 0;

	Courtesy_Light_After_Maneuver_Is_Complete();

}
//void __Do_Moving_Gate_After_COS_Action(uint8_t Moving_Direction, uint8_t COS_Type)
//{
//	Setting_Courtesy_Light_and_Flash_Light(Moving_Direction);
//
//	M1_Torque_Setting = CFG_parameter.Motor_Torque_After_Edge_Sense;
//	M2_Torque_Setting = CFG_parameter.Motor_Torque_After_Edge_Sense;
//
//	M1_Timestamp = UNDEFINE_TIMESTAMP;
//	M2_Timestamp = UNDEFINE_TIMESTAMP;
//
//	if (Moving_Direction == OPEN_MOVING_DIRECTION)
//	{
//		Motor1_Open_Enable();										// Enable Motor #1 to Open
//		Leaf_No1_State = GATE_STATE_OPENING;
//		if (COS_Type == 0x02)
//		{
//			Motor2_Open_Enable();									// Enable Motor #2 to Open
//			Leaf_No2_State = GATE_STATE_OPENING;
//		}
////		Machine_State = MACHINE_STATE_COS_OPENING;
//	}
//	else
//	{
//		Motor1_Close_Enable();										// Enable Motor #1 Close
//		Leaf_No1_State = GATE_STATE_CLOSING;
//		if (COS_Type == 0x02)
//		{
//			Motor2_Close_Enable();									// Enable Motor #2 Close
//			Leaf_No2_State = GATE_STATE_CLOSING;
//		}
//
////		Machine_State = MACHINE_STATE_COS_CLOSING;
//	}
//	Tick_Count = 0;
//
//	TRIG_EN = 1;													// Enable Pulse out to control Motor
//	R_INTC0_Start();												// Enable Sync to control
//	R_TAU0_Channel1_Start();										// Start Timer channel 1
//
//	while(Tick_Count < CFG_parameter.Reverse_Time_After_Edge_Sense);
//
//	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
//	R_INTC0_Stop();													// Disable Sync to control
//	TRIG_EN = 0;			// Enable Pulse out to control Motor
//	M1_PWM = 1;					// Clear Trigger Pulse Output
//	M2_PWM = 1;					// Clear M2 PWM Pulse Output
//
//	Motor1_Disable();
//	Motor2_Disable();
//
//	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
//	{
//		M1_Time_Position = 	M1_Last_Time_Position +	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
//		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//		if (M1_Time_Position > CFG_parameter.M1_Total_Space)
//		{
//			Leaf_No1_State = GATE_STATE_OPENED;
//			M1_Time_Position = CFG_parameter.M1_Total_Space;
//		}
//		M1_Last_Time_Position = M1_Time_Position;
//
//		if (COS_Type == 0x02)
//		{
//			M2_Time_Position = 	M2_Last_Time_Position +	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
//			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//			if (M2_Time_Position > CFG_parameter.M2_Total_Space)
//			{
//				Leaf_No1_State = GATE_STATE_OPENED;
//				M2_Time_Position = CFG_parameter.M2_Total_Space;
//			}
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//	}
//	else														// Calculate Position when stop Normal Closing
//	{
//		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//		if (M1_Last_Time_Position > CFG_parameter.Reverse_Time_After_Edge_Sense*3)
//		{
//			M1_Time_Position = 	M1_Last_Time_Position -	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
//		}
//		else
//		{
//			Leaf_No1_State = GATE_STATE_CLOSED;
//			M1_Time_Position = 0;
//		}
//		M1_Last_Time_Position = M1_Time_Position;
//
//		if (COS_Type == 0x02)
//		{
//			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//			if (M2_Last_Time_Position > CFG_parameter.Reverse_Time_After_Edge_Sense*3)
//			{
//				M2_Time_Position = 	M2_Last_Time_Position -	CFG_parameter.Reverse_Time_After_Edge_Sense*3;
//			}
//			else
//			{
//				Leaf_No2_State = GATE_STATE_CLOSED;
//				M2_Time_Position = 0;
//			}
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//	}
//
//	Flash_Enable = 0;
//	FLASH = 0;
//
//	Courtesy_Light_After_Maneuver_Is_Complete();
//
//}

void __Do_Open_Gate_Motor1 (void)
{

//	Prepare_To_Moving(OPEN_MOVING_DIRECTION);		// Prepare to moving

	Prepare_To_Moving_Motor1(OPEN_MOVING_DIRECTION);		// Prepare to moving

	if(TRIG_EN != 1)
	{
		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();
	}

	if (Leaf_No1_State != GATE_STATE_OPENED)
	{
		Motor1_Open_Enable();										// Enable Motor #1 to Open
		Leaf_No1_State = GATE_STATE_OPENING;
	}

	M1_Tick_Count = 0;

	M1_Position_Count = 0;

	if(Machine_State != MACHINE_STATE_NORMAL_OPENING)
	{
		Tick_Count = 0;								// Reset Tick count

		Machine_State = MACHINE_STATE_NORMAL_OPENING;

		R_TAU0_Channel1_Start();					// Start Timer channel 1
	}

	if (CFG_parameter.Number_Of_Motors == 0x01)
	Setting_Courtesy_Light_and_Flash_Light(OPEN_MOVING_DIRECTION);
}
void __Do_Open_Gate_Motor2 (void)
{
//		Prepare_To_Moving(OPEN_MOVING_DIRECTION);		// Prepare to moving
	Prepare_To_Moving_Motor2(OPEN_MOVING_DIRECTION);		// Prepare to moving

	if(TRIG_EN != 1)
	{
		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();
	}

	M2_Tick_Count = 0;

	M2_Position_Count = 0;

	if(Machine_State != MACHINE_STATE_NORMAL_OPENING)
	{
		Tick_Count = 0;								// Reset Tick count

		Machine_State = MACHINE_STATE_NORMAL_OPENING;

		R_TAU0_Channel1_Start();					// Start Timer channel 1
	}

	Setting_Courtesy_Light_and_Flash_Light(OPEN_MOVING_DIRECTION);

}
//void __Do_Open_Gate (void)
//{
//	Prepare_To_Moving(OPEN_MOVING_DIRECTION);		// Prepare to moving
//
//	TRIG_EN = 1;			// Enable Pulse out to control Motor
//	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

//	__Do_Open_Gate_Motor1();
//	__Do_Open_Gate_Motor2();

//	Tick_Count = 0;								// Reset Tick count
//
//	Machine_State = MACHINE_STATE_NORMAL_OPENING;
//
//	R_TAU0_Channel1_Start();					// Start Timer channel 1
//}


void __Do_Close_Gate_Motor1 (void)
{

	Prepare_To_Moving_Motor1(CLOSE_MOVING_DIRECTION);		// Prepare to moving

	if(TRIG_EN != 1)
	{
		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();
	}

	if (CFG_parameter.Number_Of_Motors == 0x01)
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			Motor1_Close_Enable();										// Enable Motor #1 to Open
			Leaf_No1_State = GATE_STATE_CLOSING;
		}
	}

	M1_Tick_Count = 0;
	M1_Position_Count = 0;

	if(Machine_State != MACHINE_STATE_NORMAL_CLOSING)
	{
		Tick_Count = 0;								// Reset Tick count
		Machine_State = MACHINE_STATE_NORMAL_CLOSING;

		R_TAU0_Channel1_Start();					// Start Timer channel 1
	}
}
void __Do_Close_Gate_Motor2 (void)
{

	Prepare_To_Moving_Motor2(CLOSE_MOVING_DIRECTION);		// Prepare to moving
	if(TRIG_EN != 1)
	{
		TRIG_EN = 1;			// Enable Pulse out to control Motor
		R_INTC0_Start();
	}

	if (CFG_parameter.Number_Of_Motors == 0x02)
	{
		if (Leaf_No2_State != GATE_STATE_CLOSED)
		{
			Motor2_Close_Enable();										// Enable Motor #2 to Open
			Leaf_No2_State = GATE_STATE_CLOSING;
		}
	}

	M2_Tick_Count = 0;
	M2_Position_Count = 0;

	if(Machine_State != MACHINE_STATE_NORMAL_CLOSING)
	{
		Tick_Count = 0;								// Reset Tick count
		Machine_State = MACHINE_STATE_NORMAL_CLOSING;

		R_TAU0_Channel1_Start();					// Start Timer channel 1
	}
}
void __Do_Close_Gate (void)
{
	Prepare_To_Moving(CLOSE_MOVING_DIRECTION);		// Prepare to moving

	TRIG_EN = 1;			// Enable Pulse out to control Motor
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	if (CFG_parameter.Number_Of_Motors == 0x02)
	{
		if (Leaf_No2_State != GATE_STATE_CLOSED)
		{
			Motor2_Close_Enable();										// Enable Motor #2 to Close
			Leaf_No2_State = GATE_STATE_CLOSING;
		}
	}
	else
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			Motor1_Close_Enable();										// Enable Motor #1 to Close
			Leaf_No1_State = GATE_STATE_CLOSING;
		}
	}

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;
	M2_Tick_Count = 0;
	M1_Position_Count = 0;
	M2_Position_Count = 0;

	Machine_State = MACHINE_STATE_NORMAL_CLOSING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Stop_Gate_Motor1 (uint8_t Moving_Direction)
{
	if(TRIG_EN != 0)
	{
		R_TAU0_Channel1_Stop();										// Stop Timer channel 1
		R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
		TRIG_EN = 0;			// Enable Pulse out to control Motor
	}
	M1_PWM = 1;					// Clear Trigger Pulse Output

	Motor1_Disable();

	Stop_Timer_Enable = 1;
	Stop_Timer_Counter = 0;
	while (Stop_Timer_Enable == 1);

	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
	{
		if (Leaf_No1_State != GATE_STATE_OPENED)
		{
//			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
//				M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M1_Time_Position = 	M1_Last_Time_Position +	M1_Position_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = CFG_parameter.M1_Total_Space;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}
	else														// Calculate Position when stop Normal Closing
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
//			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;

			M1_Time_Position = 	M1_Last_Time_Position -	M1_Position_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = 0;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}

	if(Machine_State != MACHINE_STATE_STOP)
	{
		Machine_State = MACHINE_STATE_STOP;

		Flash_Enable = 0;
		FLASH = 0;
	}

//	Courtesy_Light_After_Maneuver_Is_Complete();
}
void __Do_Stop_Gate_Motor2 (uint8_t Moving_Direction)
{
	if(TRIG_EN != 0)
	{
		R_TAU0_Channel1_Stop();										// Stop Timer channel 1
		R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
		TRIG_EN = 0;			// Enable Pulse out to control Motor
	}

	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	Motor2_Disable();

	Stop_Timer_Enable = 1;
	Stop_Timer_Counter = 0;
	while (Stop_Timer_Enable == 1);

	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
	{
		if (Leaf_No2_State != GATE_STATE_OPENED)
		{
			M2_Time_Position = 	M2_Last_Time_Position +	M2_Position_Count;
			M2_Last_Time_Position = M2_Time_Position;
		}
		else
		{
			M2_Time_Position = CFG_parameter.M2_Total_Space;
			M2_Last_Time_Position = M2_Time_Position;
		}
	}
	else														// Calculate Position when stop Normal Closing
	{
		if (Leaf_No2_State != GATE_STATE_CLOSED)
		{
			M2_Time_Position = 	M2_Last_Time_Position -	M2_Position_Count;
			M2_Last_Time_Position = M2_Time_Position;
		}
		else
		{
			M2_Time_Position = 0;
			M2_Last_Time_Position = M2_Time_Position;
		}
	}

	if(Machine_State != MACHINE_STATE_STOP)
	{
		Machine_State = MACHINE_STATE_STOP;

		Flash_Enable = 0;
		FLASH = 0;
	}

	Courtesy_Light_After_Maneuver_Is_Complete();

}
//void __Do_Stop_Gate (uint8_t Moving_Direction)
//{
//	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
//	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
//	TRIG_EN = 0;			// Enable Pulse out to control Motor
//	M1_PWM = 1;					// Clear Trigger Pulse Output
//	M2_PWM = 1;					// Clear M2 PWM Pulse Output
//
//	Motor1_Disable();
//	Motor2_Disable();
//
//	Stop_Timer_Enable = 1;
//	Stop_Timer_Counter = 0;
//	while (Stop_Timer_Enable == 1);
//
//	if (Moving_Direction == OPEN_MOVING_DIRECTION)				// Calculate Position when stop Normal Opening
//	{
//		if (Leaf_No1_State != GATE_STATE_OPENED)
//		{
////			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
////				M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//			M1_Time_Position = 	M1_Last_Time_Position +	M1_Position_Count;
//			M1_Last_Time_Position = M1_Time_Position;
//		}
//		else
//		{
//			M1_Time_Position = CFG_parameter.M1_Total_Space;
//			M1_Last_Time_Position = M1_Time_Position;
//		}
//
//		if (Leaf_No2_State != GATE_STATE_OPENED)
//		{
////			if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
////				M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//			M2_Time_Position = 	M2_Last_Time_Position +	M2_Position_Count;
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//		else
//		{
//			M2_Time_Position = CFG_parameter.M2_Total_Space;
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//	}
//	else														// Calculate Position when stop Normal Closing
//	{
//		if (Leaf_No1_State != GATE_STATE_CLOSED)
//		{
////			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//			M1_Time_Position = 	M1_Last_Time_Position -	M1_Position_Count;
//			M1_Last_Time_Position = M1_Time_Position;
//		}
//		else
//		{
//			M1_Time_Position = 0;
//			M1_Last_Time_Position = M1_Time_Position;
//		}
//
//		if (Leaf_No2_State != GATE_STATE_CLOSED)
//		{
////			if (M2_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M2_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
//
//			M2_Time_Position = 	M2_Last_Time_Position -	M2_Position_Count;
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//		else
//		{
//			M2_Time_Position = 0;
//			M2_Last_Time_Position = M2_Time_Position;
//		}
//	}
//
//
//	Machine_State = MACHINE_STATE_STOP;
//
//
//	Flash_Enable = 0;
//	FLASH = 0;
//
//	Courtesy_Light_After_Maneuver_Is_Complete();
//
////	if (((CFG_parameter.Number_Of_Motors == 0x02) && (Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED))
////		|| ((CFG_parameter.Number_Of_Motors == 0x02) && (Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED))
////		|| ((CFG_parameter.Number_Of_Motors == 0x01) && (Leaf_No1_State == GATE_STATE_CLOSED))
////		|| ((CFG_parameter.Number_Of_Motors == 0x01) && (Leaf_No1_State == GATE_STATE_OPENED)))
////	{
////		Courtesy_Light_After_Maneuver_Is_Complete();
////	}
//}

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

	TRIG_EN = 1;			// Enable Pulse out to control Motor
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	Motor1_Open_Enable();										// Enable Motor #1 to Open
	Leaf_No1_State = GATE_STATE_OPENING;

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;
	M1_Position_Count = 0;

	Machine_State = MACHINE_STATE_PARTIAL_OPENING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Partial_Close_Gate (void)
{
	Prepare_To_Partial_Moving(CLOSE_MOVING_DIRECTION);

	TRIG_EN = 1;			// Enable Pulse out to control Motor
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening

	Motor1_Close_Enable();										// Enable Motor #1 to Open
	Leaf_No1_State = GATE_STATE_CLOSING;

	Tick_Count = 0;								// Reset Tick count
	M1_Tick_Count = 0;
	M1_Position_Count = 0;

	Machine_State = MACHINE_STATE_PARTIAL_CLOSING;

	R_TAU0_Channel1_Start();					// Start Timer channel 1
}

void __Do_Partial_Stop_Gate (uint8_t Moving_Direction)
{
	R_TAU0_Channel1_Stop();										// Stop Timer channel 1
	R_INTC0_Stop();												// Enable Sync to control, Motor 1 start opening
	TRIG_EN = 0;			// Enable Pulse out to control Motor
	M1_PWM = 1;					// Clear Trigger Pulse Output
	M2_PWM = 1;					// Clear M2 PWM Pulse Output

	Motor1_Disable();

	Stop_Timer_Enable = 1;
	Stop_Timer_Counter = 0;
	while (Stop_Timer_Enable == 1);

	if (Moving_Direction == OPEN_MOVING_DIRECTION)
	{
		if (Leaf_No1_State != GATE_STATE_PARTIAL)
		{
//			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time)
//				M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
			M1_Time_Position = 	M1_Last_Time_Position +	M1_Position_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = CFG_parameter.M1_Partial_Space;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}
	else
	{
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
//			if (M1_Tick_Count > CFG_parameter.Motor_Maximum_Boost_Time) M1_Tick_Count -= CFG_parameter.Motor_Maximum_Boost_Time;
			M1_Time_Position = 	M1_Last_Time_Position -	M1_Position_Count;
			M1_Last_Time_Position = M1_Time_Position;
		}
		else
		{
			M1_Time_Position = 0;
			M1_Last_Time_Position = M1_Time_Position;
		}
	}

	Machine_State = MACHINE_STATE_STOP;

	Flash_Enable = 0;
	FLASH = 0;
	Courtesy_Light_After_Maneuver_Is_Complete();

//	if ((Leaf_No1_State == GATE_STATE_CLOSED)
//		|| (Leaf_No1_State == GATE_STATE_OPENED)
//		|| (Leaf_No1_State == GATE_STATE_PARTIAL))
//	{
//		Flash_Enable = 0;
//		FLASH = 0;
//		Courtesy_Light_After_Maneuver_Is_Complete();
//	}
}
