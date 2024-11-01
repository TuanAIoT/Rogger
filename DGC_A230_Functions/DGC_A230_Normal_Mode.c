/*
 * DGC_A230_Normal_Mode.c
 *
 *  Created on: Nov 22, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

#include <DGC_A230_Define.h>
#include <Common.h>


/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern uint8_t Command_Status_No1, Command_Status_No2, Safety_Status_No1, Safety_Status_No2;
uint8_t Leaf_No1_State = GATE_STATE_UNKNOWN, Leaf_No2_State = GATE_STATE_UNKNOWN;
extern struct Device_Config_Parameters CFG_parameter;
extern uint8_t	M1_Timestamp, M2_Timestamp;
extern uint16_t Key_Tick_Count;
extern uint8_t	Pre_flash_Time;

extern uint8_t Power_failure;
//code
extern uint8_t Power_failure_gate_1;
extern uint8_t Power_failure_gate_2;
//code

uint16_t M1_Last_Time_Position = 0, M2_Last_Time_Position = 0;		// Last Gate Position in time
uint16_t M1_Time_Position = 0, M2_Time_Position = 0;				// Gate position in time

uint16_t Courtesy_Light_Time, Courtesy_Light_Count;
uint8_t  Courtesy_Light_Enable = 0;

extern uint8_t Machine_State;

uint8_t Last_Open_Command_State, Last_Close_Command_State, Last_Clock_Command_State;

uint8_t Gate_Open_Indicator_Mode = 0;
uint8_t Gate_Open_Indicator_Flash_Count = 0;

uint8_t Auto_Close_Gate_Command = AUTO_CLOSE_GATE_COMMAND_OFF;
uint8_t Auto_Close_Gate_Timer_Enable = 0;
uint16_t Auto_Close_Gate_Timer_Counter = 0;
uint8_t Auto_Close_Gate_Retry_Number = 0;
uint8_t Auto_Close_Retry_Number;

uint8_t Last_Sensing_Edge_COS1_status = SENSING_EDGE_STATUS_INACTIVE, Last_Sensing_Edge_COS2_status = SENSING_EDGE_STATUS_INACTIVE;

void Check_Safety_Device_Status(void)
{
	// E_STOP Emergency Stop Status
	if (E_STOP) SET_BIT(Command_Status_No2, BIT(0));
	else CLEAR_BIT(Command_Status_No2, BIT(0));

	// Photo cell FT1 Status
	if (FT1) SET_BIT(Safety_Status_No1, BIT(7));
	else CLEAR_BIT(Safety_Status_No1, BIT(7));

	// COS1 sensing edge Status
	if (COS1) SET_BIT(Safety_Status_No1, BIT(1));
	else CLEAR_BIT(Safety_Status_No1, BIT(1));

	// FCA1 Gate Open limit switches LEAF#1 Status
	if (FCA1_IN) SET_BIT(Safety_Status_No1, BIT(3));
	else CLEAR_BIT(Safety_Status_No1, BIT(3));

	// FCC1 Gate Close limit switches LEAF#1 Status
	if (FCC1_IN) SET_BIT(Safety_Status_No1, BIT(5));
	else CLEAR_BIT(Safety_Status_No1, BIT(5));

	// ENC1 Encoder Motor#1 Status
	if (ENC1) SET_BIT(Safety_Status_No1, BIT(0));
	else CLEAR_BIT(Safety_Status_No1, BIT(0));

	// Photo cell FT2 Status
	if (FT2) SET_BIT(Safety_Status_No2, BIT(7));
	else CLEAR_BIT(Safety_Status_No2, BIT(7));

	// COS2 sensing edge Status
	if (COS2) SET_BIT(Safety_Status_No2, BIT(1));
	else CLEAR_BIT(Safety_Status_No2, BIT(1));

	// FCA2 Gate Open limit switches LEAF#1 Status
	if (FCA2_IN) SET_BIT(Safety_Status_No2, BIT(3));
	else CLEAR_BIT(Safety_Status_No2, BIT(3));

	// FCC2 Gate Close limit switches LEAF#1 Status
	if (FCC2_IN) SET_BIT(Safety_Status_No2, BIT(5));
	else CLEAR_BIT(Safety_Status_No2, BIT(5));

	// ENC2 Encoder Motor#2 Status
	if (ENC2) SET_BIT(Safety_Status_No2, BIT(0));
	else CLEAR_BIT(Safety_Status_No2, BIT(0));

	// AP Input Command
	if (!AP_IN) SET_BIT(Command_Status_No1, BIT(7));
	else CLEAR_BIT(Command_Status_No1, BIT(7));

	// CH Input Command
	if (!CH_IN) SET_BIT(Command_Status_No1, BIT(4));
	else CLEAR_BIT(Command_Status_No1, BIT(4));

	// PED Input Command
	if (!PED_IN) SET_BIT(Command_Status_No1, BIT(1));
	else CLEAR_BIT(Command_Status_No1, BIT(1));

	// PP Input Command
	if (!PP_IN) SET_BIT(Command_Status_No2, BIT(7));
	else CLEAR_BIT(Command_Status_No2, BIT(7));

	// ORO Input Command
	if (!ORO_IN) SET_BIT(Command_Status_No2, BIT(4));
	else CLEAR_BIT(Command_Status_No2, BIT(4));
}

void Courtesy_Light_After_Maneuver_Is_Complete (void)
{
	Courtesy_Light_Enable = 0;
	if (CFG_parameter.Courtery_Light_Mode >= 3)
	{
		Courtesy_Light_Enable = 1;
		if (CFG_parameter.Courtery_Light_Mode <= 91)
		{
			Courtesy_Light_Time = CFG_parameter.Courtery_Light_Mode * 2;
		}
		else if (CFG_parameter.Courtery_Light_Mode >= 92)
		{
			Courtesy_Light_Time = (CFG_parameter.Courtery_Light_Mode - 90) * 60 *2;
		}
		COURTESY = 1;
		Courtesy_Light_Count = 0;
	}
	else
	{
		COURTESY = 0;
	}
}

void TestPB_Pressed_Process(void)
{
	switch (Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			Test_Command_Process();			// Test Command and Safety Signal Procedure
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
			__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		} break;

		default: break;
	}
}

void Partial_Command_Process ()
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if ((Leaf_No2_State == GATE_STATE_CLOSED) || (CFG_parameter.Number_Of_Motors == 0x01))
		{
			if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
			{
				if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Partial_Open_Gate();
				}
			}
			else if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
			{
				if (CFG_parameter.M1_Partial_Space >= M1_Time_Position)
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Partial_Open_Gate();
				}
			}
			else if (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE)
			{
				if (CFG_parameter.Patial_Function == 0x01)
				{
					Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
					Auto_Close_Gate_Timer_Counter = 0;
					Auto_Close_Gate_Timer_Enable = 0;
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Partial_Open_Gate();
				}
				else
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Partial_Close_Gate();
				}
			}
			else 	// Step Close
			{
				if (CFG_parameter.Patial_Function == 0x00)
				{
					if (Leaf_No1_State == GATE_STATE_PARTIAL)
					{
						Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
						Auto_Close_Gate_Timer_Counter = 0;
						Auto_Close_Gate_Timer_Enable = 0;
					}
					if (CFG_parameter.M1_Partial_Space >= M1_Time_Position)
					{
						Pre_flash_Time = CFG_parameter.Pre_Flashing;
						__Do_Partial_Close_Gate();
					}
				}
				else
				{
					if (Leaf_No1_State == GATE_STATE_PARTIAL)
					{
						Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
						Auto_Close_Gate_Timer_Counter = 0;
					}
				}
			}
		}
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		if (CFG_parameter.Patial_Function == 0x00)
		{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		}
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
		Auto_Close_Gate_Timer_Counter = 0;
		Auto_Close_Gate_Timer_Enable = 0;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		if (CFG_parameter.Patial_Function == 0x01)
		{
			Pre_flash_Time = 0x00;
			__Do_Partial_Open_Gate();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	case MACHINE_STATE_NORMAL_CLOSING:
	default: break;
	}
}

void Step_Command_Process_Mode_0 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
		else if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else if ((Leaf_No1_State == GATE_STATE_PARTIAL) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Partial_Close_Gate();
		}
		else		// Step Close
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
		__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	default: break;
	}
}

void Step_Command_Process_Mode_1 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01))
			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02)))
		{
			Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 1;
		}
		else if (Leaf_No1_State == GATE_STATE_PARTIAL)
		{
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 0;
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else
		{
			if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
			{
				if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Open_Gate_Motor1();
					__Do_Open_Gate_Motor2();
				}
			}
			else
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;

		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
			else
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
		}
		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

		Auto_Close_Gate_Timer_Counter = 0;
		Auto_Close_Gate_Timer_Enable = 0;
		Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		Auto_Close_Gate_Timer_Counter = 0;
		Auto_Close_Gate_Timer_Enable = 0;
		Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	default: break;
	}

}

void Step_Command_Process_Mode_2 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01))
			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02)))
		{
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 0;
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
		else if (Leaf_No1_State == GATE_STATE_PARTIAL)
		{
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 0;
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else
		{
			if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
			{
				if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Open_Gate_Motor1();
					__Do_Open_Gate_Motor2();
				}
			}
			else
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
			else
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
		}
		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
		Auto_Close_Gate_Timer_Counter = 0;
		Auto_Close_Gate_Timer_Enable = 0;
		Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		Auto_Close_Gate_Timer_Counter = 0;
		Auto_Close_Gate_Timer_Enable = 0;
		Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	default: break;
	}
}

void Step_Command_Process_Mode_3 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else if ((Leaf_No1_State == GATE_STATE_PARTIAL) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Partial_Close_Gate();
		}
		else		// Step Close
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		Leaf_No2_State = GATE_STATE_OPENING_PAUSE;

		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
			else
				__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
		}
		__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Close_Gate_Motor1();
		__Do_Close_Gate_Motor2();

	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
			else
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
		}

		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
		}
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Partial_Close_Gate();
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
		}
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Open_Gate_Motor1();
		__Do_Open_Gate_Motor2();
	} break;

	default: break;
	}

}

void Step_Command_Process_Mode_4 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else if ((Leaf_No1_State == GATE_STATE_PARTIAL) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Partial_Close_Gate();
		}
		else		// Step Close
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		Leaf_No2_State = GATE_STATE_OPENING_PAUSE;

		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			if (CFG_parameter.Number_Of_Motors == 0x02)
				__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
			else
				__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
		}

		__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Close_Gate_Motor1();
		__Do_Close_Gate_Motor2();

	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
		{
			__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
		}
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		Pre_flash_Time = 0x00;
		__Do_Partial_Close_Gate();
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	default: break;
	}
}

void Step_Command_Process_Mode_5 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		}
		else if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		}
		else if ((Leaf_No1_State == GATE_STATE_PARTIAL) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Partial_Close_Gate();
		}
		else		// Step Close
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
		__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
		__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
		__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	default: break;
	}
}


void Step_Command_Process ()
{
	switch(CFG_parameter.Step_Mode)
	{
	case 0x00:	Step_Command_Process_Mode_0(); break;
	case 0x01: 	Step_Command_Process_Mode_1(); break;
	case 0x02: 	Step_Command_Process_Mode_2(); break;
	case 0x03: 	Step_Command_Process_Mode_3(); break;
	case 0x04: 	Step_Command_Process_Mode_4(); break;
	default: 	Step_Command_Process_Mode_0(); break;
	}
}

void Open_Command_Process ()
{
	if (CFG_parameter.Manual_Function == 0x00)
	{
		while(!AP_IN);
		switch(Machine_State)
		{
		case MACHINE_STATE_STOP:
		{
			if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
			{
				if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Open_Gate_Motor1();
					__Do_Open_Gate_Motor2();
				}
			}
			else if (Leaf_No1_State != GATE_STATE_OPENED)
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			// Do nothing
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
				else
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			}
			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
			if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
			}
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			}
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		} break;

		default: break;
		}
	}
	else
	{
		if (Last_Open_Command_State == COMMAND_RELEASE_STATE)
		{
			Last_Open_Command_State = COMMAND_PRESSED_STATE;
			switch(Machine_State)
			{
			case MACHINE_STATE_STOP:
			{
				if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
				{
					if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
					{
						Pre_flash_Time = CFG_parameter.Pre_Flashing;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
				}
				else if (Leaf_No1_State != GATE_STATE_OPENED)
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Open_Gate_Motor1();
					__Do_Open_Gate_Motor2();
				}
			} break;

			case MACHINE_STATE_NORMAL_OPENING:
			{
				// Do nothing
			} break;

			case MACHINE_STATE_NORMAL_CLOSING:
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);

				}
				__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();

			} break;

			case MACHINE_STATE_PARTIAL_OPENING:
			{
				Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
				if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			} break;

			case MACHINE_STATE_PARTIAL_CLOSING:
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			} break;

			default: break;
			}
		}
	}
}

void Close_Command_Process ()
{
	if (CFG_parameter.Manual_Function == 0x00)
	{
		switch(Machine_State)
		{
			case MACHINE_STATE_STOP:
			{
				if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01))
					|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02))
					|| (Leaf_No1_State == GATE_STATE_PARTIAL))
				{
					Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
					Auto_Close_Gate_Timer_Counter = 0;
					Auto_Close_Gate_Timer_Enable = 0;
				}
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				if ((Leaf_No1_State == GATE_STATE_PARTIAL)
						|| (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE)
						|| (Leaf_No2_State == GATE_STATE_CLOSED))
				{
					__Do_Partial_Close_Gate();
				}
				else
				{
					__Do_Close_Gate_Motor1();
					__Do_Close_Gate_Motor2();
				}
			} break;

			case MACHINE_STATE_NORMAL_OPENING:
			{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				__Do_Close_Gate_Motor1();
				__Do_Close_Gate_Motor2();
			} break;

			case MACHINE_STATE_PARTIAL_OPENING:
			{
				Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
				if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
				{
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				__Do_Partial_Close_Gate();
			} break;

			case MACHINE_STATE_NORMAL_CLOSING:
			case MACHINE_STATE_PARTIAL_CLOSING:
			default: break;
		}
		while(!CH_IN);
	}
	else
	{
		if (Last_Close_Command_State == COMMAND_RELEASE_STATE)
		{
			Last_Close_Command_State = COMMAND_PRESSED_STATE;

			switch(Machine_State)
			{
				case MACHINE_STATE_STOP:
				{
					if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01))
						|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02))
						|| (Leaf_No1_State == GATE_STATE_PARTIAL))
					{
						Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
						Auto_Close_Gate_Timer_Counter = 0;
						Auto_Close_Gate_Timer_Enable = 0;
					}
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					if ((Leaf_No1_State == GATE_STATE_PARTIAL)
							|| (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE)
							|| (Leaf_No2_State == GATE_STATE_CLOSED))
					{
						__Do_Partial_Close_Gate();
					}
					else
					{
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
				} break;

				case MACHINE_STATE_NORMAL_OPENING:
				{
					Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
					Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
					if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
					{
						if (CFG_parameter.Number_Of_Motors == 0x02)
							__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
						else
							__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);

					}
					__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
					__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;
					__Do_Close_Gate_Motor1();
					__Do_Close_Gate_Motor2();
				} break;

				case MACHINE_STATE_PARTIAL_OPENING:
				{
					Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
					if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
					{
						__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
					}
					__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;
					__Do_Partial_Close_Gate();
				} break;

				case MACHINE_STATE_NORMAL_CLOSING:
				case MACHINE_STATE_PARTIAL_CLOSING:
				default: break;
			}

		}
	}
}

void Auto_Close_Gate_Command_Process(void)
{
	Auto_Close_Gate_Command = AUTO_CLOSE_GATE_COMMAND_OFF;
	Auto_Close_Gate_Timer_Enable = 0;
	Auto_Close_Gate_Timer_Counter = 0;

	if(ORO_IN)
	{
		if ((Auto_Close_Gate_Retry_Number > 0) && (Auto_Close_Gate_Retry_Number != 99))
		{
			Auto_Close_Gate_Retry_Number -= 1;
		}

		if (Leaf_No1_State == GATE_STATE_PARTIAL)
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Partial_Close_Gate();
		}
		if ((Leaf_No1_State != GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_STOP))
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		}
	}
}

void Ext_Clock_Open_Command_Process ()
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
			{
				if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
				{
					Pre_flash_Time = CFG_parameter.Pre_Flashing;
					__Do_Open_Gate_Motor1();
					__Do_Open_Gate_Motor2();
				}
			}
			else if (Leaf_No1_State != GATE_STATE_OPENED)
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			// Do nothing
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
				else
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);

			}
			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();

		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Open_Gate_Motor1();
			__Do_Open_Gate_Motor2();
		} break;

		default: break;
	}
}

void Ext_Clock_Close_Command_Process ()
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			Pre_flash_Time = CFG_parameter.Pre_Flashing;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
			if (CFG_parameter.Reverse_Command_Enable_Braking == 0x01)
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
				else
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);

			}
			__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			// Do nothing
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

			Pre_flash_Time = 0x00;
			__Do_Close_Gate_Motor1();
			__Do_Close_Gate_Motor2();
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
		} break;

		default:
		{
			// Do nothing
		} break;
	}
}

void Photocell_FT1_Process (void)
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			if ((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x02))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
			else if ((Leaf_No1_State != GATE_STATE_CLOSED) && (CFG_parameter.Close_Command_Enable_Photocell == 0x01))
			{
				while(FT1);
				// When the photocell gate FT1 is crossed, a close command is sent 6 seconds later.
			}
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			if (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)
			{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Opening > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x03)		// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
					else														//	Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)		// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)
					{
						__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
						__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
				}
			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			if (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
				}
				__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Closing > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x03)	// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
					else	//Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)
					{
						__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
						__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			if (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)
			{
				Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Opening > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x03)		// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)		// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)
					{
						__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			if (CFG_parameter.Photocell_FT1_Mode_Closing != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Closing > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x03)	// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
					else	//Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT1_Mode_Opening != 0x00)
					{
						__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
				}
			}
		} break;

		default: break;
	}

}

void Photocell_FT2_Process (void)
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			if ((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x02))
			{
				Pre_flash_Time = CFG_parameter.Pre_Flashing;
				__Do_Open_Gate_Motor1();
				__Do_Open_Gate_Motor2();
			}
			else if ((Leaf_No1_State != GATE_STATE_CLOSED) && (CFG_parameter.Close_Command_Enable_Photocell == 0x02))
			{
				while(FT2);
				// When the photocell gate FT1 is crossed, a close command is sent 6 seconds later.
			}
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			if (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)
			{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Opening > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x03)		// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
					else														//	Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)		// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)
					{
						__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
						__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
				}
			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			if (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					if (CFG_parameter.Number_Of_Motors == 0x02)
						__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
					else
						__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
				}
				__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Closing > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x03)	// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Close_Gate_Motor1();
						__Do_Close_Gate_Motor2();
					}
					else	//Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)
					{
						__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
						__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Open_Gate_Motor1();
						__Do_Open_Gate_Motor2();
					}
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			if (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)
			{
				Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Opening > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x03)		// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)		// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)
					{
						__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			if (CFG_parameter.Photocell_FT2_Mode_Closing != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				if (CFG_parameter.Photocell_Enable_Braking == 0x01)
				{
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
				}
				__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Closing > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x03)	// Temporary Stop
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Close_Gate();
					}
					else	//Delayed Reverse
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
				{
					if (CFG_parameter.Photocell_FT2_Mode_Opening != 0x00)
					{
						__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
					}
					else
					{
						Pre_flash_Time = 0x00;
						__Do_Partial_Open_Gate();
					}
				}
			}
		} break;

		default: break;
	}
}

void Sensing_Edge_COS1_Process (void)
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			// Do nothing
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			if (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)
			{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				if(CFG_parameter.Number_Of_Motors == 0x02)
				{
					__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x02);
				}
				else
				{
					__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x01);
				}

			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			if (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)
			{
				if ((CFG_parameter.Sensing_Edge_COS1_Config == 0x03) || (CFG_parameter.Sensing_Edge_COS1_Config == 0x04))
				{
					Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
					Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
					__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
					__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;
					if(CFG_parameter.Number_Of_Motors == 0x02)
					{
						__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x02);
					}
					else
					{
						__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x01);
					}
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			if (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)
			{
				Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
				__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				{
					__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x01);
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			if (CFG_parameter.Sensing_Edge_COS1_Config != 0x00)
			{
				if ((CFG_parameter.Sensing_Edge_COS1_Config == 0x03) || (CFG_parameter.Sensing_Edge_COS1_Config == 0x04))
				{
					Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
					__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;

					__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x01);
				}
			}
		} break;

		default:
		{
			// Do nothing
		} break;
	}
}

void Sensing_Edge_COS2_Process (void)
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			// Do nothing
		} break;

		case MACHINE_STATE_NORMAL_OPENING:
		{
			if (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)
			{
				if (CFG_parameter.Sensing_Edge_COS2_Config > 0x02)
				{
					Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
					Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
					__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
					__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;
					if(CFG_parameter.Number_Of_Motors == 0x02)
					{
						__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x02);
						__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x02);
					}
					else
					{
						__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x01);
						__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x01);
					}

				}
			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			if (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;
				if(CFG_parameter.Number_Of_Motors == 0x02)
				{
					__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x02);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x02);
				}
				else
				{
					__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x01);
				}

			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			if (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)
			{
				if (CFG_parameter.Sensing_Edge_COS2_Config > 0x02)
				{
					Leaf_No1_State = GATE_STATE_PATIAL_OPENING_PAUSE;
					__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

					Pre_flash_Time = 0x00;

					__Do_Moving_Gate_After_COS_Action_Motor1_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_Start(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor1_End(CLOSE_MOVING_DIRECTION, 0x01);
					__Do_Moving_Gate_After_COS_Action_Motor2_End(CLOSE_MOVING_DIRECTION, 0x01);
				}
			}
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			if (CFG_parameter.Sensing_Edge_COS2_Config != 0x00)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

				Pre_flash_Time = 0x00;

				__Do_Moving_Gate_After_COS_Action_Motor1_Start(OPEN_MOVING_DIRECTION, 0x01);
				__Do_Moving_Gate_After_COS_Action_Motor2_Start(OPEN_MOVING_DIRECTION, 0x01);
				__Do_Moving_Gate_After_COS_Action_Motor1_End(OPEN_MOVING_DIRECTION, 0x01);
				__Do_Moving_Gate_After_COS_Action_Motor2_End(OPEN_MOVING_DIRECTION, 0x01);
			}
		} break;

		default:
		{
			// Do nothing
		} break;
	}
}

void Emergency_Stop_Process (void)
{
	if (CFG_parameter.Stop_Command_Enable_Braking == 0x01)
	{
		switch (Machine_State)
		{
			case MACHINE_STATE_NORMAL_OPENING:
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_2(OPEN_MOVING_DIRECTION);
				else
					__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
			} break;
			case MACHINE_STATE_NORMAL_CLOSING:
			{
				if (CFG_parameter.Number_Of_Motors == 0x02)
					__Do_Brake_Motor_Type_2(CLOSE_MOVING_DIRECTION);
				else
					__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			} break;
			case MACHINE_STATE_PARTIAL_OPENING:
			{
				__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
			} break;
			case MACHINE_STATE_PARTIAL_CLOSING:
			{
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			} break;
			default: break;
		}
	}

	switch (Machine_State)
	{
		case MACHINE_STATE_NORMAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
			__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
		} break;
		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
		} break;
		case MACHINE_STATE_PARTIAL_OPENING:
		{
			Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		} break;
		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
		} break;
		default: break;
	}
}

void Open_Limit_Switch_No1_Process (void)
{
	switch (Machine_State)
	{
		case MACHINE_STATE_NORMAL_OPENING:
		{
			M1_Timestamp = UNDEFINE_TIMESTAMP;
			Motor1_Disable();
			Leaf_No1_State = GATE_STATE_OPENED;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			// Do Nothing
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		{
			M1_Timestamp = UNDEFINE_TIMESTAMP;
			Motor1_Disable();
			Leaf_No1_State = GATE_STATE_PARTIAL;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_1(OPEN_MOVING_DIRECTION);
			}
		} break;

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			// Do Nothing
		} break;
		default: break;
	}
}

void Open_Limit_Switch_No2_Process (void)
{
	switch (Machine_State)
	{
		case MACHINE_STATE_NORMAL_OPENING:
		{
			M2_Timestamp = UNDEFINE_TIMESTAMP;
			Motor2_Disable();
			Leaf_No2_State = GATE_STATE_OPENED;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_3(OPEN_MOVING_DIRECTION);
			}
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		case MACHINE_STATE_PARTIAL_OPENING:
		case MACHINE_STATE_PARTIAL_CLOSING:
		default: break;
	}

}

void Close_Limit_Switch_No1_Process (void)
{
	switch (Machine_State)
	{
		case MACHINE_STATE_NORMAL_OPENING:
		{
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			M1_Timestamp = UNDEFINE_TIMESTAMP;
			Motor1_Disable();
			Leaf_No1_State = GATE_STATE_CLOSED;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:

		case MACHINE_STATE_PARTIAL_CLOSING:
		{
			M1_Timestamp = UNDEFINE_TIMESTAMP;
			Motor1_Disable();
			Leaf_No1_State = GATE_STATE_CLOSED;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_1(CLOSE_MOVING_DIRECTION);
			}
		} break;

		default: break;
	}
}

void Close_Limit_Switch_No2_Process (void)
{
	switch (Machine_State)
	{
		case MACHINE_STATE_NORMAL_OPENING:
		{

		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			M2_Timestamp = UNDEFINE_TIMESTAMP;
			Motor2_Disable();
			Leaf_No2_State = GATE_STATE_CLOSED;

			if (CFG_parameter.Limit_Switch_Enable_Braking == 0x01)
			{
				__Do_Brake_Motor_Type_3(CLOSE_MOVING_DIRECTION);
			}
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		case MACHINE_STATE_PARTIAL_CLOSING:
		default: break;
	}
}

void Gate_Open_Indicator_Process(void)
{
	switch(CFG_parameter.Gate_Open_Indicator)
	{
		case 0x01:
		{
			switch(Machine_State)
			{
				case MACHINE_STATE_STOP:
				{
					if (Leaf_No1_State == GATE_STATE_CLOSED)			// Gate completely Closed
					{
						SC_EN = 0;
						Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_0;
					}
					else if (
							(Leaf_No2_State == GATE_STATE_OPENED)		// Gate completely opened
						    || (Leaf_No1_State == GATE_STATE_PARTIAL)
							|| ((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01))
							)
					{
						SC_EN = 1;
						Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_0;
					}
					else												// the gate is stopped in an intermediate position
					{
						if (Gate_Open_Indicator_Mode != GATE_OPEN_INDICATOR_MODE_3)
						{
							Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_3;
							Gate_Open_Indicator_Flash_Count = 0;
							SC_EN = 1;
						}
					}
				} break;

				case MACHINE_STATE_NORMAL_OPENING:
				case MACHINE_STATE_PARTIAL_OPENING:						// the gate during opening maneuver
				{
					if (Gate_Open_Indicator_Mode != GATE_OPEN_INDICATOR_MODE_1)
					{
						Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_1;
						Gate_Open_Indicator_Flash_Count = 0;
						SC_EN = 1;
					}
				} break;

				case MACHINE_STATE_NORMAL_CLOSING:
				case MACHINE_STATE_PARTIAL_CLOSING:						// the gate during closing maneuver
				{
					if (Gate_Open_Indicator_Mode != GATE_OPEN_INDICATOR_MODE_2)
					{
						Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_2;
						Gate_Open_Indicator_Flash_Count = 0;
						SC_EN = 1;
					}
				} break;

				default:
				{
					// Do nothing
				} break;
			}
		} break;

		case 0x02:
		{
			SC_EN = 1;
			Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_0;
		} break;

		case 0x00:
		default:
		{
			Gate_Open_Indicator_Mode = GATE_OPEN_INDICATOR_MODE_0;
			if (Leaf_No1_State == GATE_STATE_CLOSED)
			{
				SC_EN = 0;
			}
			else
			{
				SC_EN = 1;
			}
		} break;
	}
}

void Normal_Mode(void)
{
	while (1U)
	{

		//code
		if ((CFG_parameter.Auto_Close_After_Power == 0x01) && (Power_failure_gate_2 == POWER_FAILURE_STATE))
		{
			Power_failure_gate_1 = POWER_RUN_STATE;
			Power_failure_gate_2 = POWER_RUN_STATE;
			Start_auto_recovery();
			Step_Command_Process();			// Step Command Procedure
		}

		//code
		Gate_Open_Indicator_Process();
		//  Normally Open Stop
		if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_OPENING)))
		{
			__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
			if (Auto_Close_Gate_Retry_Number > 0)
			{
				Auto_Close_Gate_Timer_Counter = 0;
				Auto_Close_Gate_Timer_Enable = 1;
			}
			//code
			if(Power_failure_gate_2 == POWER_RUN_STATE)
			{
				Power_failure_gate_1 = POWER_GOOD_STATE;
				Power_failure_gate_2 = POWER_GOOD_STATE;
				End_manual_recovery();
			}
			//code
		}

		//  Normally Close Stop
		if (((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
			|| ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING)))
		{
			__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
			__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
			Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 0;
			//code
			if(Power_failure_gate_2 == POWER_RUN_STATE)
			{
				Power_failure_gate_1 = POWER_GOOD_STATE;
				Power_failure_gate_2 = POWER_GOOD_STATE;
				End_auto_recovery();
			}
			//code
		}

		//  Normally Partial Open Stop
		if ((Leaf_No1_State == GATE_STATE_PARTIAL) && (Machine_State == MACHINE_STATE_PARTIAL_OPENING))
		{
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);

			if (Auto_Close_Gate_Retry_Number > 0)
			{
				Auto_Close_Gate_Timer_Counter = 0;
				Auto_Close_Gate_Timer_Enable = 1;
			}
		}

		//  Normally Partial Close Stop
		if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_PARTIAL_CLOSING))
		{
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);

			Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
			Auto_Close_Gate_Timer_Counter = 0;
			Auto_Close_Gate_Timer_Enable = 0;
		}

		if (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
		{
			Open_Limit_Switch_No1_Process();
		}

		if (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
		{
			Open_Limit_Switch_No2_Process();
		}

		if (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
		{
			Close_Limit_Switch_No1_Process();
		}

		if (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
		{
			Close_Limit_Switch_No2_Process();
		}
		if (Auto_Close_Gate_Command == AUTO_CLOSE_GATE_COMMAND_ON)
		{
			Auto_Close_Gate_Command_Process();
		}
		//  Emergency Stop
		if (E_STOP)
		{
			Emergency_Stop_Process();
		}

		if (FT1)
		{
			Photocell_FT1_Process();
		}

		if (FT2)
		{
			Photocell_FT2_Process();
		}

		if ((COS1) && (Last_Sensing_Edge_COS1_status == SENSING_EDGE_STATUS_INACTIVE))
		{
			Last_Sensing_Edge_COS1_status = SENSING_EDGE_STATUS_ACTIVE;
			Sensing_Edge_COS1_Process();
		}

		if ((COS2) && (Last_Sensing_Edge_COS2_status == SENSING_EDGE_STATUS_INACTIVE))
		{
			Last_Sensing_Edge_COS2_status = SENSING_EDGE_STATUS_ACTIVE;
			Sensing_Edge_COS2_Process();
		}

		if ((!COS1) && (Last_Sensing_Edge_COS1_status == SENSING_EDGE_STATUS_ACTIVE))
		{
			Last_Sensing_Edge_COS1_status = SENSING_EDGE_STATUS_INACTIVE;
		}

		if ((!COS2) && (Last_Sensing_Edge_COS2_status == SENSING_EDGE_STATUS_ACTIVE))
		{
			Last_Sensing_Edge_COS2_status = SENSING_EDGE_STATUS_INACTIVE;
		}

		if ((!PED_IN)						// Partial Command is activated
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{
			if((CFG_parameter.Manual_Function == 0x00) && (Power_failure == POWER_GOOD_STATE))
			{
				Partial_Command_Process();		// Partial Command Procedure
			}
			while(!PED_IN);
		}

		if ((!PP_IN)							// Step Command is activated
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{

			if (Power_failure_gate_2 == POWER_FAILURE_STATE)
			{
//				Manual_Power_Failure_Recovery_Process();
//				Manual_Power_Failure_Recovery_Process_Test();

				//code
				Power_failure_gate_1 = POWER_RUN_STATE;
				Power_failure_gate_2 = POWER_RUN_STATE;
				Start_manual_recovery();
				Step_Command_Process();			// Step Command Procedure
				//code


			}
			else
			{
				if(CFG_parameter.Manual_Function == 0x00)
				{
					Step_Command_Process();			// Step Command Procedure
				}
			}
			while(!PP_IN);
		}

		if ((!CH_IN)							// Close Command is activated
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{
			Close_Command_Process();		// Close Command Procedure
		}

		if (((CH_IN)
			&& (Last_Close_Command_State == COMMAND_PRESSED_STATE)
			&& (CFG_parameter.Manual_Function == 0x01))
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{
			if (Machine_State == MACHINE_STATE_NORMAL_CLOSING)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				__Do_Stop_Gate_Motor1(CLOSE_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(CLOSE_MOVING_DIRECTION);
			}
			else if (Machine_State == MACHINE_STATE_PARTIAL_CLOSING)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
			}
			Last_Close_Command_State = COMMAND_RELEASE_STATE;
		}

		if 	((!AP_IN)
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{
			Open_Command_Process();		// Open Command Procedure
		}

		if (((AP_IN)
			&& (Last_Open_Command_State == COMMAND_PRESSED_STATE)
			&& (CFG_parameter.Manual_Function == 0x01))
			&& (!((!ORO_IN) && (CFG_parameter.Clock_Contact_Mode == 0x00))))
		{
			if (Machine_State == MACHINE_STATE_NORMAL_OPENING)
				{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				__Do_Stop_Gate_Motor1(OPEN_MOVING_DIRECTION);
				__Do_Stop_Gate_Motor2(OPEN_MOVING_DIRECTION);
				}
			Last_Open_Command_State = COMMAND_RELEASE_STATE;
		}

		if ((!ORO_IN)						// Clock function is activated
			&& (Last_Clock_Command_State == COMMAND_RELEASE_STATE))
		{
			Last_Clock_Command_State = COMMAND_PRESSED_STATE;
			Ext_Clock_Open_Command_Process();	// External Clock Open Command Procedure
		}

		if ((ORO_IN)						// Clock function is activated
			&& (Last_Clock_Command_State == COMMAND_PRESSED_STATE))
		{
			Last_Clock_Command_State = COMMAND_RELEASE_STATE;
			Ext_Clock_Close_Command_Process();	// External Clock Open Command Procedure
		}

		if (!TEST_KEY)						// Test_Key is pressed
		{
			TestPB_Pressed_Process();
			while(!TEST_KEY);				// Do nothing while Test_Key is still pressed
		}

		if (!LEFT_KEY)						// Left_Key is pressed
		{
			Parameters_Edit_Process();		// Edit Parameters Procedure
			while(!LEFT_KEY);				// Do nothing while Left_Key is still pressed
		}

		if (!RIGHT_KEY)						// Right_Key is pressed
		{
			Parameters_Edit_Process();		// Edit Parameters Procedure
			while(!RIGHT_KEY);				// Do nothing while Right_Key is still pressed
		}

		if ((!PROG_KEY) && (Machine_State == MACHINE_STATE_STOP))	// Prog_Key is pressed
		{
			Key_Tick_Count = 0;										// Reset Tick count
//			R_TAU0_Channel1_Start();								// Start Timer channel 1
			while((!PROG_KEY) && (Key_Tick_Count < 400)); 			// Check time that Prog_Key pressed
//			R_TAU0_Channel1_Stop();									// Stop Timer channel 1

			if(Key_Tick_Count >= 400)
			{
				Program_Command_Process();							// Travel Acquisition Procedure
			}
		}
	}
}
