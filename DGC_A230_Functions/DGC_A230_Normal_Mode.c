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
extern uint16_t M1_Tick_Count, M2_Tick_Count;
extern uint8_t	M1_Timestamp, M2_Timestamp;
extern uint16_t Key_Tick_Count;
//code [21]
extern uint16_t Automatic_Close_Flag;
extern uint16_t Automatic_Close_Count;
//code [21]
//code [A7] [A6]
uint8_t flag_PP = 0;
//code [A7] [A6]
extern uint8_t	Flash_Enable;

extern uint8_t Power_failure;

uint16_t M1_Last_Time_Position = 0, M2_Last_Time_Position = 0;		// Last Gate Position in time
uint16_t M1_Time_Position = 0, M2_Time_Position = 0;				// Gate position in time

uint16_t Courtesy_Light_Time, Courtesy_Light_Count;
uint8_t  Courtesy_Light_Enable = 0;

extern uint8_t Machine_State;

uint8_t Last_Open_Command_State, Last_Close_Command_State, Last_Clock_Command_State;

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
			__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
		} break;

		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
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
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				__Do_Partial_Open_Gate();
			}
		}
		else if (Leaf_No1_State == GATE_STATE_CLOSING_PAUSE)
		{
			__Do_Partial_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Partial_Close_Gate();
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
		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
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
				__Do_Open_Gate();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			__Do_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Close_Gate();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
		Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
		Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
		__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	case MACHINE_STATE_PARTIAL_CLOSING:
	default: break;
	}
}

void Step_Command_Process_Mode_1 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				__Do_Open_Gate();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			__Do_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Close_Gate();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	case MACHINE_STATE_PARTIAL_CLOSING:
	default: break;
	}

}

void Step_Command_Process_Mode_2 (void)
{
	switch(Machine_State)
	{
	case MACHINE_STATE_STOP:
	{
		if (Leaf_No1_State == GATE_STATE_CLOSED) // Step OPen
		{
			if (!((FT1 && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x00)) || (FT2 && (CFG_parameter.Photocell_FT2_Mode_Closed == 0x00))))
			{
				__Do_Open_Gate();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			__Do_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Close_Gate();
		}
	} break;

	case MACHINE_STATE_NORMAL_OPENING:
	{
	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	case MACHINE_STATE_PARTIAL_CLOSING:
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
				__Do_Open_Gate();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			__Do_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Close_Gate();
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
		__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
		__Do_Close_Gate();

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

		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
		__Do_Open_Gate();
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	case MACHINE_STATE_PARTIAL_CLOSING:
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
				__Do_Open_Gate();
			}
		}
		else if ((Leaf_No1_State == GATE_STATE_CLOSING_PAUSE) || (Leaf_No1_State == GATE_STATE_PATIAL_OPENING_PAUSE))
		{
			__Do_Open_Gate();
		}
		else		// Step Close
		{
			__Do_Close_Gate();
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

		__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
		__Do_Close_Gate();

	} break;

	case MACHINE_STATE_NORMAL_CLOSING:
	{
		Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
		Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
		__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
	} break;

	case MACHINE_STATE_PARTIAL_OPENING:
	case MACHINE_STATE_PARTIAL_CLOSING:
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
					__Do_Open_Gate();
				}
			}
			else if (Leaf_No1_State != GATE_STATE_OPENED)
			{
				__Do_Open_Gate();
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
			__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);

			__Do_Open_Gate();
		} break;

		case MACHINE_STATE_PARTIAL_OPENING:
		case MACHINE_STATE_PARTIAL_CLOSING:
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
						__Do_Open_Gate();
					}
				}
				else if (Leaf_No1_State != GATE_STATE_OPENED)
				{
					__Do_Open_Gate();
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
				__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);

				__Do_Open_Gate();

			} break;

			case MACHINE_STATE_PARTIAL_OPENING:
			case MACHINE_STATE_PARTIAL_CLOSING:
			default: break;
			}
		}
	}
}

void Close_Command_Process ()
{
	if (CFG_parameter.Manual_Function == 0x00)
	{
		while(!CH_IN);
		switch(Machine_State)
		{
			case MACHINE_STATE_STOP:
			{
				__Do_Close_Gate();
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
				__Do_Stop_Gate(OPEN_MOVING_DIRECTION);

				__Do_Close_Gate();
			} break;

			case MACHINE_STATE_NORMAL_CLOSING:
			case MACHINE_STATE_PARTIAL_OPENING:
			case MACHINE_STATE_PARTIAL_CLOSING:
			default: break;
		}
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
					__Do_Close_Gate();
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
					__Do_Stop_Gate(OPEN_MOVING_DIRECTION);

					__Do_Close_Gate();
				} break;

				case MACHINE_STATE_NORMAL_CLOSING:
				case MACHINE_STATE_PARTIAL_OPENING:
				case MACHINE_STATE_PARTIAL_CLOSING:
				default: break;
			}

		}
	}
}

void Clock_Command_Process ()
{

}

void Photocell_FT1_Process (void)
{
	switch(Machine_State)
	{
		case MACHINE_STATE_STOP:
		{
			if ((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Photocell_FT1_Mode_Closed == 0x02))
			{
				__Do_Open_Gate();
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
				__Do_Stop_Gate(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Opening > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x03)		// Temporary Stop
					{
						__Do_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						__Do_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)		// Immediate Reverse
				{
					__Do_Close_Gate();
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
				__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT1_Mode_Closing > 0x02)
				{
					while(FT1);
					if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x03)	// Temporary Stop
					{
						__Do_Close_Gate();
					}
					else	//Delayed Reverse
					{
						__Do_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
				{
					__Do_Open_Gate();
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
						__Do_Partial_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						__Do_Partial_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Opening == 0x02)		// Immediate Reverse
				{
					__Do_Partial_Close_Gate();
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
						__Do_Partial_Close_Gate();
					}
					else	//Delayed Reverse
					{
						__Do_Partial_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT1_Mode_Closing == 0x02)	// Immediate Reverse
				{
					__Do_Partial_Open_Gate();
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
				__Do_Open_Gate();
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
				__Do_Stop_Gate(OPEN_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Opening > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x03)		// Temporary Stop
					{
						__Do_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						__Do_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)		// Immediate Reverse
				{
					__Do_Close_Gate();
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
				__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);

				if (CFG_parameter.Photocell_FT2_Mode_Closing > 0x02)
				{
					while(FT2);
					if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x03)	// Temporary Stop
					{
						__Do_Close_Gate();
					}
					else	//Delayed Reverse
					{
						__Do_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
				{
					__Do_Open_Gate();
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
						__Do_Partial_Open_Gate();
					}
					else														//	Delayed Reverse
					{
						__Do_Partial_Close_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Opening == 0x02)		// Immediate Reverse
				{
					__Do_Partial_Close_Gate();
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
						__Do_Partial_Close_Gate();
					}
					else	//Delayed Reverse
					{
						__Do_Partial_Open_Gate();
					}
				}
				else if (CFG_parameter.Photocell_FT2_Mode_Closing == 0x02)	// Immediate Reverse
				{
					__Do_Partial_Open_Gate();
				}
			}
		} break;

		default: break;
	}
}

void Sensing_Edge_COS1_Process (void)
{

}

void Sensing_Edge_COS2_Process (void)
{

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
			__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
		} break;
		case MACHINE_STATE_NORMAL_CLOSING:
		{
			Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
			Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
			__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
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
void Normal_Mode(void)
{

	while (1U)
	{

		//code test
		//Led SC sang khi cong mo
		if (Leaf_No1_State != GATE_STATE_CLOSED)
		{
			SC_EN = 1;
		}
		else
		{
			SC_EN = 0;
		}
		//code test


		//code [A2]
		//neu 2 cua da mo hoan toan, bat dau kiem tra tham so tu dong dong cua
		if(((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02))
			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01) ))
		{
			if(CFG_parameter.Auto_Close_After_OPen != 0x00)
			{
				if(Automatic_Close_Flag != CFG_parameter.Auto_Close_After_OPen)
				{
					Automatic_Close_Flag = CFG_parameter.Auto_Close_After_OPen;
					Automatic_Close_Count = 0;
				}
			}
			else if(((CFG_parameter.Step_Mode == 0x01) || (CFG_parameter.Step_Mode == 0x02)))
			{
				if(Automatic_Close_Flag != 0x01)
				{
					Automatic_Close_Flag = 0x01;
					Automatic_Close_Count = 0;
				}
			}

		}
		//code [21]
		//kiem tra so lan tu dong dong cua va thoi gian troi qua
		if((Automatic_Close_Flag > 0) && (Automatic_Close_Count > CFG_parameter.Automatic_Closing_Time) && (Machine_State == MACHINE_STATE_STOP) && (Leaf_No1_State != GATE_STATE_CLOSED))
		{
			if(Automatic_Close_Flag <= 15)
			{
				Automatic_Close_Flag--;
			}
			else if(Automatic_Close_Flag == 99)
			{
				Automatic_Close_Flag = 16;
			}
			Close_Command_Process();
		}
		//code [21] end
		//code [A2] end

		//  Normally Open Stop
		if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_OPENING)))
		{
			__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
		}

		//  Normally Close Stop
		if (((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
			|| ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING)))
		{
			__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
		}

		//  Normally Partial Open Stop
		if ((Leaf_No1_State == GATE_STATE_PARTIAL) && (Machine_State == MACHINE_STATE_PARTIAL_OPENING))
		{
			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
		}

		//  Normally Partial Close Stop
		if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_PARTIAL_CLOSING))
		{
			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
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

		if (COS1)
		{
			Sensing_Edge_COS1_Process();
		}

		if (COS2)
		{
			Sensing_Edge_COS2_Process();
		}

		//code [A7] [A6]
		if(Leaf_No1_State == GATE_STATE_CLOSED && Leaf_No2_State == GATE_STATE_CLOSED && flag_PP == 1)
		{
			//neu ca hai cong da dong thi co PP se tat, PE hoat dong binh thuong
			flag_PP = 0;
		}
		if (!PED_IN)						// Partial Command is activated
		{
			while(!PED_IN);
			//code [A4]
			if(CFG_parameter.Step_Mode == 0x01)
			{
				Automatic_Close_Count = 0;
			}
			//code [A4]
			if((CFG_parameter.Manual_Function == 0x00) && (flag_PP == 0))
			{
				if (Power_failure == POWER_FAILURE_STATE)
					Power_Failure_Recovery_Process();
				else
					Partial_Command_Process();		// Partial Command Procedure
			}
		}

		//code [A7] [A6]

//		//admin
//		if (!PED_IN)						// Partial Command is activated
//		{
//			while(!PED_IN);
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Partial_Command_Process();		// Partial Command Procedure
//		}
//		//admin

		//code [A7] [A6]
		if ((!PP_IN) && (CFG_parameter.Manual_Function == 0x00))							// Step Command is activated
		{
			while(!PP_IN);
			//code
			if (flag_PP == 0 && Leaf_No1_State == GATE_STATE_CLOSED && Leaf_No2_State == GATE_STATE_CLOSED)
			{
				//Neu da nhan PP, thi co PP duoc bat, PE se bi vo hieu hoa
				flag_PP = 1;
			}
			//code


			//code [A4]
			if(CFG_parameter.Step_Mode == 0x01)
			{
				Automatic_Close_Count = 0;
			}
			//code [A4]

			if (Power_failure == POWER_FAILURE_STATE)
				Power_Failure_Recovery_Process();
			else
				Step_Command_Process();			// Step Command Procedure
		}
		//code [A7] [A6]

//		//admin
//		if (!PP_IN)							// Step Command is activated
//		{
//			while(!PP_IN);
//			//code
//			if (flag_PP == 0 && Leaf_No1_State == GATE_STATE_CLOSED && Leaf_No2_State == GATE_STATE_CLOSED)
//			{
//				flag_PP = 1;
//			}
//			//code
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Step_Command_Process();			// Step Command Procedure
//		}
//		//admin

		if (!CH_IN)							// Close Command is activated
		{
			Close_Command_Process();		// Close Command Procedure
		}

		if (
			(CH_IN)
			&& (Last_Close_Command_State == COMMAND_PRESSED_STATE)
			&& (CFG_parameter.Manual_Function == 0x01)
			)
		{
			if (Machine_State == MACHINE_STATE_NORMAL_CLOSING)
			{
				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
				__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
			}

			Last_Close_Command_State = COMMAND_RELEASE_STATE;
		}

		if 	(!AP_IN)
		{
			Open_Command_Process();		// Open Command Procedure
		}

		if (
			(AP_IN)
			&& (Last_Open_Command_State == COMMAND_PRESSED_STATE)
			&& (CFG_parameter.Manual_Function == 0x01)
			)
		{
			if (Machine_State == MACHINE_STATE_NORMAL_OPENING)
				{
				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
				__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
				}
			Last_Open_Command_State = COMMAND_RELEASE_STATE;
		}

		if (!ORO_IN)						// Clock function is activated
		{
			Clock_Command_Process();	// Clock Command Procedure
		}

		if (!TEST_KEY)						// Test_Key is pressed
		{
			while(!TEST_KEY);				// Do nothing while Test_Key is still pressed
			TestPB_Pressed_Process();
		}

		if (!LEFT_KEY)						// Left_Key is pressed
		{
			while(!LEFT_KEY);				// Do nothing while Left_Key is still pressed
			Parameters_Edit_Process();		// Edit Parameters Procedure
		}

		if (!RIGHT_KEY)						// Right_Key is pressed
		{
			while(!RIGHT_KEY);				// Do nothing while Right_Key is still pressed
			Parameters_Edit_Process();		// Edit Parameters Procedure
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

//
//extern uint8_t Display_Mode;
//extern char Parameter_Name[2];
//extern char Parameter_Value[2];
//void Test_Normal_Mode(void)
//{
//	int a = 0;
//	int b = 0;
////	Test_Command_Process();
//
//	SC_EN = 1;
//	uint8_t working_done = 0;
//	uint8_t Alarm_And_Limit_Error_Code = 0x00;
//	Display_Mode = TEST_MODE;
//
//	while(1)
//	{
//		if (!PP_IN)
//		{
//			while(!PP_IN);
//
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Step_Command_Process();			// Step Command Procedure
//			memcpy(&Parameter_Name, "00", 2);
//			memcpy(&Parameter_Value, "00", 2);
//		}
//		if (!PED_IN)
//		{
//			while(!PED_IN);
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Partial_Command_Process();		// Partial Command Procedure
//			memcpy(&Parameter_Name, "11", 2);
//			memcpy(&Parameter_Value, "11", 2);
//		}
//	}
//	memcpy(&Parameter_Name, "00", 2);
//	memcpy(&Parameter_Value, "00", 2);
//
////	convert_binary_to_ascii(Alarm_And_Limit_Error_Code, &Parameter_Value[0], &Parameter_Value[1]);
////	convert_binary_to_ascii(Alarm_And_Limit_Error_Code, &Parameter_Name[0], &Parameter_Name[1]);
//
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "11", 2);
//	memcpy(&Parameter_Value, "22", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "22", 2);
//	memcpy(&Parameter_Value, "33", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "33", 2);
//	memcpy(&Parameter_Value, "44", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "44", 2);
//	memcpy(&Parameter_Value, "55", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "55", 2);
//	memcpy(&Parameter_Value, "66", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "66", 2);
//	memcpy(&Parameter_Value, "77", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "77", 2);
//	memcpy(&Parameter_Value, "88", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "88", 2);
//	memcpy(&Parameter_Value, "99", 2);
//	delay_ms(5000);
//	memcpy(&Parameter_Name, "nn", 2);
//	memcpy(&Parameter_Value, "nn", 2);
////	Key_Tick_Count = 0;				// Reset Tick count
////
////	while ((working_done == 0) && (Key_Tick_Count < 2000) && a < 40)
////	{
////		Alarm_And_Limit_Error_Code = Check_Safety_and_Limit_Switch();
////		convert_binary_to_ascii(Alarm_And_Limit_Error_Code, &Parameter_Value[0], &Parameter_Value[1]);
////
////		delay_ms(500);
////		a++;
////	}
//	while(a<40)
//	{
//		a++;
//		delay_ms(500);
//	}
//	a = 0;
//	b = 0;
//
//	while(a<40)
//	{
//		a++;
//		delay_ms(500);
//	}
//	a = 0;
//	b = 0;
//	while(a<40)
//	{
//		a++;
//		delay_ms(500);
//	}
//	a = 0;
//	b = 0;
//	while(a<40)
//	{
//		a++;
//		delay_ms(500);
//	}
//	a = 0;
//	b = 0;
//	__Do_Open_Gate();
//	while(a<40)
//	{
//		a++;
//		a = a+1;
//		if(a>10)
//			{
//				b++;
//			}
////		if(b>20) break;
//		delay_ms(1000);
//	}
//	a = 0;
//	b = 0;
//	__Do_Close_Gate();
////	__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
//
//	while(a<50)
//	{
//		a++;
//		a = a+1;
//		if(a==26)
//			{
//			__Do_Close_Gate();
//			}
////		if(b>20) break;
//		delay_ms(1000);
//	}
//	a = 0;
//	b = 0;
////	__Do_Close_Gate();
////	__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
//	__Do_Partial_Open_Gate();
//	while(a<40)
//	{
//		a++;
//		a = a+1;
//		delay_ms(1000);
//	}
//	a = 0;
//	b = 0;
//	__Do_Partial_Close_Gate();
//	while(1U)
//	{
//		;
//	}
//	while (1U)
//	{
//		//  Normally Open Stop
//		if (((Leaf_No1_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_OPENING))
//			|| ((Leaf_No1_State == GATE_STATE_OPENED) && (Leaf_No2_State == GATE_STATE_OPENED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_OPENING)))
//		{
//			__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
//		}
//
//		//  Normally Close Stop
//		if (((Leaf_No1_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x01) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING))
//			|| ((Leaf_No1_State == GATE_STATE_CLOSED) && (Leaf_No2_State == GATE_STATE_CLOSED) && (CFG_parameter.Number_Of_Motors == 0x02) && (Machine_State == MACHINE_STATE_NORMAL_CLOSING)))
//		{
//			__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
//		}
//
//		//  Normally Partial Open Stop
//		if ((Leaf_No1_State == GATE_STATE_PARTIAL) && (Machine_State == MACHINE_STATE_PARTIAL_OPENING))
//		{
//			__Do_Partial_Stop_Gate(OPEN_MOVING_DIRECTION);
//		}
//
//		//  Normally Partial Close Stop
//		if ((Leaf_No1_State == GATE_STATE_CLOSED) && (Machine_State == MACHINE_STATE_PARTIAL_CLOSING))
//		{
//			__Do_Partial_Stop_Gate(CLOSE_MOVING_DIRECTION);
//		}
//
//		if (FCA1_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
//		{
//			Open_Limit_Switch_No1_Process();
//		}
//
//		if (FCA2_IN && (CFG_parameter.Limit_Switch_Config != 0x00))
//		{
//			Open_Limit_Switch_No2_Process();
//		}
//
//		if (FCC1_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
//		{
//			Close_Limit_Switch_No1_Process();
//		}
//
//		if (FCC2_IN && (CFG_parameter.Limit_Switch_Config == 0x01))
//		{
//			Close_Limit_Switch_No2_Process();
//		}
//
//		//  Emergency Stop
//		if (E_STOP)
//		{
//			Emergency_Stop_Process();
//		}
//
//		if (FT1)
//		{
//			Photocell_FT1_Process();
//		}
//
//		if (FT2)
//		{
//			Photocell_FT2_Process();
//		}
//
//		if (COS1)
//		{
//			Sensing_Edge_COS1_Process();
//		}
//
//		if (COS2)
//		{
//			Sensing_Edge_COS2_Process();
//		}
//
//		if (!PED_IN)						// Partial Command is activated
//		{
//			while(!PED_IN);
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Partial_Command_Process();		// Partial Command Procedure
//		}
//
//		if (!PP_IN)							// Step Command is activated
//		{
//			while(!PP_IN);
//
//			if (Power_failure == POWER_FAILURE_STATE)
//				Power_Failure_Recovery_Process();
//			else
//				Step_Command_Process();			// Step Command Procedure
//		}
//
//		if (!CH_IN)							// Close Command is activated
//		{
//			Close_Command_Process();		// Close Command Procedure
//		}
//
//		if (
//			(CH_IN)
//			&& (Last_Close_Command_State == COMMAND_PRESSED_STATE)
//			&& (CFG_parameter.Manual_Function == 0x01)
//			)
//		{
//			if (Machine_State == MACHINE_STATE_NORMAL_CLOSING)
//			{
//				Leaf_No1_State = GATE_STATE_CLOSING_PAUSE;
//				Leaf_No2_State = GATE_STATE_CLOSING_PAUSE;
//				__Do_Stop_Gate(CLOSE_MOVING_DIRECTION);
//			}
//
//			Last_Close_Command_State = COMMAND_RELEASE_STATE;
//		}
//
//		if 	(!AP_IN)
//		{
//			Open_Command_Process();		// Open Command Procedure
//		}
//
//		if (
//			(AP_IN)
//			&& (Last_Open_Command_State == COMMAND_PRESSED_STATE)
//			&& (CFG_parameter.Manual_Function == 0x01)
//			)
//		{
//			if (Machine_State == MACHINE_STATE_NORMAL_OPENING)
//				{
//				Leaf_No1_State = GATE_STATE_OPENING_PAUSE;
//				Leaf_No2_State = GATE_STATE_OPENING_PAUSE;
//				__Do_Stop_Gate(OPEN_MOVING_DIRECTION);
//				}
//			Last_Open_Command_State = COMMAND_RELEASE_STATE;
//		}
//
//		if (!ORO_IN)						// Clock function is activated
//		{
//			Clock_Command_Process();	// Clock Command Procedure
//		}
//
//		if (!TEST_KEY)						// Test_Key is pressed
//		{
//			while(!TEST_KEY);				// Do nothing while Test_Key is still pressed
//			TestPB_Pressed_Process();
//		}
//
//		if (!LEFT_KEY)						// Left_Key is pressed
//		{
//			while(!LEFT_KEY);				// Do nothing while Left_Key is still pressed
//			Parameters_Edit_Process();		// Edit Parameters Procedure
//		}
//
//		if (!RIGHT_KEY)						// Right_Key is pressed
//		{
//			while(!RIGHT_KEY);				// Do nothing while Right_Key is still pressed
//			Parameters_Edit_Process();		// Edit Parameters Procedure
//		}
//
//		if ((!PROG_KEY) && (Machine_State == MACHINE_STATE_STOP))	// Prog_Key is pressed
//		{
//			Key_Tick_Count = 0;										// Reset Tick count
////			R_TAU0_Channel1_Start();								// Start Timer channel 1
//			while((!PROG_KEY) && (Key_Tick_Count < 400)); 			// Check time that Prog_Key pressed
////			R_TAU0_Channel1_Stop();									// Stop Timer channel 1
//
//			if(Key_Tick_Count >= 400)
//			{
//				Program_Command_Process();							// Travel Acquisition Procedure
//			}
//		}
//	}
//}
