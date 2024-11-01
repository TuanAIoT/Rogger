/*
 * DGC_A230_Program_Mode.c
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
extern uint8_t  HMI_Flash_Enable;
extern uint16_t Tick_Count, M1_Tick_Count, M2_Tick_Count;
extern uint8_t 	Mask;

extern uint8_t Leaf_No1_State, Leaf_No2_State;

extern struct Device_Config_Parameters CFG_parameter;
extern uint8_t    	CFG_Parameters_Value[128];

extern uint16_t M1_Torque_Setting, M2_Torque_Setting;
uint8_t	M1_Timestamp = UNDEFINE_TIMESTAMP, M2_Timestamp = UNDEFINE_TIMESTAMP;
uint16_t Torque_Inc = 0x0032, Torque_Dec = 0x0032;

extern uint8_t Auto_Close_Retry_Number;
extern uint8_t Auto_Close_Gate_Retry_Number;
extern uint8_t Auto_Close_Gate_Timer_Enable;
extern uint16_t Auto_Close_Gate_Timer_Counter;

/*
 *Acquisition procedure WITH the encoder enabled
 */
void Travel_Acquisition_Procedure_With_Encoder(void)
{
	memcpy(&Parameter_Name, "AU", 2);
	memcpy(&Parameter_Value, "to", 2);

	while (PROG_KEY);
	while (!PROG_KEY);
}

/*
 * Acquisition procedure WITHOUT encoder
 */
void Travel_Acquisition_Procedure_Without_Eencoder(void)
{
	uint16_t M1_Time_Counter, M2_Time_Counter, Parameter_17, Parameter_19;
	uint16_t Temp_Torque;

	Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
	Auto_Close_Gate_Timer_Counter = 0;
	Auto_Close_Gate_Timer_Enable = 0;
	/*
	 * Start  opening travel acquisition
	 */
	memcpy(&Parameter_Name, "AP", 2);								// Display "AP 1"
	memcpy(&Parameter_Value, "1 ", 2);

	M1_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;
	M2_Torque_Setting = CFG_parameter.Motor_Torque_Maneuver;

	M1_Timestamp = UNDEFINE_TIMESTAMP;
	M2_Timestamp = UNDEFINE_TIMESTAMP;

	TRIG_EN = 1;
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
	Motor1_Open_Enable();											// Enable Motor #1 Open

	Tick_Count = 0;													// Reset Tick count
	M1_Tick_Count = 0;
	Leaf_No1_State = GATE_STATE_OPENING;

	R_TAU0_Channel1_Start();										// Start Timer channel 1

	while(Tick_Count < CFG_parameter.Motor_No2_Open_Delay_Time);	// Delay time to enable Motor 2 Opening
	Motor2_Open_Enable();											// Enable Motor #2 Open
	M2_Tick_Count = 0;												// Start counter to M2
	Leaf_No2_State = GATE_STATE_OPENING;

	memcpy(&Parameter_Name, "AP", 2);								// Display "AP 2"
	memcpy(&Parameter_Value, "2 ", 2);

	Tick_Count = 0;													// Reset Tick count
	while (Tick_Count < 200);										// Display 2 seconds "AP 2"

	memcpy(&Parameter_Name, "AP", 2);								// Display "AP 1"
	memcpy(&Parameter_Value, "1 ", 2);

	if (CFG_parameter.Limit_Switch_Config == 0x00)					// wait for Gate 1 reach to Opened position
	{
		while (PROG_KEY);
		while (!PROG_KEY);
	}
	else
	{
		while (!FCA1_IN);
	}

	Leaf_No1_State = GATE_STATE_OPENED;

	Motor1_Disable();												// Stop Motor 1
	M1_Time_Counter = M1_Tick_Count;								// Motor 1 counter value

	memcpy(&Parameter_Name, "AP", 2);								// Display "AP 2"
	memcpy(&Parameter_Value, "2 ", 2);

	if (CFG_parameter.Limit_Switch_Config == 0x00)					// wait for Gate 2 reach to Opened position
	{
		while (PROG_KEY);
		while (!PROG_KEY);
	}
	else
	{
		while (!FCA1_IN);
	}

	Leaf_No2_State = GATE_STATE_OPENED;
	Motor2_Disable();												// Stop Motor 1
	M2_Time_Counter = M2_Tick_Count;								// Motor 2 counter value

	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
	R_INTC0_Stop();													// Disable Sync
	M1_PWM = M2_PWM = 1;
	TRIG_EN = 0;

	memcpy(&Parameter_Name, "PA", 2);								// Display "PA  "
	memcpy(&Parameter_Value, "  ", 2);

	HMI_Flash_Enable = 1;
	HMI_Flash_Count = 0;
	while (HMI_Flash_Count <6);										// 3 second
	HMI_Flash_Enable = 0;
	Mask = 0xff;

	/*
	 * Start  Closing travel acquisition
	 */
	memcpy(&Parameter_Name, "CH", 2);								// Display "CH 2"
	memcpy(&Parameter_Value, "2 ", 2);

	TRIG_EN = 1;
	R_INTC0_Start();												// Enable Sync to control, Motor 1 start opening
	Motor2_Close_Enable();											// Enable Motor #2 to close

	Tick_Count = 0;													// Reset Tick count
	M2_Tick_Count = 0;

	Leaf_No2_State = GATE_STATE_CLOSING;

	R_TAU0_Channel1_Start();										// Start Timer channel 1

	while(Tick_Count < CFG_parameter.Motor_No1_Close_Delay_Time);	// Delay time to enable Motor 1 Closing
	Motor1_Close_Enable();											// Enable Motor #1 to close
	M1_Tick_Count = 0;												// Start counter to M1
	Leaf_No1_State = GATE_STATE_CLOSING;

	memcpy(&Parameter_Name, "CH", 2);								// Display "CH 1"
	memcpy(&Parameter_Value, "1 ", 2);

	Tick_Count = 0;													// Reset Tick count
	while (Tick_Count < 200);										// Display 2 seconds "CH 1"

	memcpy(&Parameter_Name, "CH", 2);								// Display "CH 2"
	memcpy(&Parameter_Value, "2 ", 2);

	if (CFG_parameter.Limit_Switch_Config == 0x01)					// wait for Gate 2 reach to Closed position
	{
		while (!FCC2_IN);
	}
	else
	{
		while (PROG_KEY);
		while (!PROG_KEY);
	}

	Leaf_No2_State = GATE_STATE_CLOSED;
	Motor2_Disable();												// Stop Motor #2
	M2_Time_Counter += M2_Tick_Count;								// Motor 2 counter value

	memcpy(&Parameter_Name, "CH", 2);								// Display "CH 1"
	memcpy(&Parameter_Value, "1 ", 2);

	if (CFG_parameter.Limit_Switch_Config == 0x01)					// wait for Gate 1 reach to Closed position
	{
		while (!FCC1_IN);
	}
	else
	{
		while (PROG_KEY);
		while (!PROG_KEY);
	}

	Leaf_No1_State = GATE_STATE_CLOSED;
	Motor1_Disable();												// Stop Motor 1
	M1_Time_Counter += M1_Tick_Count;								// Motor 1 counter value

	R_TAU0_Channel1_Stop();											// Stop Timer channel 1
	R_INTC0_Stop();													// Disable Sync
	TRIG_EN = 0;

	Parameter_17 = M1_Time_Counter/200;							// Calculate Parameter 17
	Parameter_19 = M2_Time_Counter/200;							// Calculate Parameter 19

	if (((Parameter_17 > 99) || (Parameter_19 > 99)) && (CFG_parameter.Double_Time_Enable == 0x00))
	{
		CFG_parameter.Double_Time_Enable = 0x01;
		CFG_Parameters_Value[24] = 0x01;
	}

	if (CFG_parameter.Double_Time_Enable == 0x01)
	{
		Parameter_17 >>= 1;
		Parameter_19 >>= 1;
	}


	Parameter_17 += 1;
	Parameter_19 += 1;

	CFG_Parameters_Value[17] = convert_binary_to_BCD(Parameter_17);
	CFG_Parameters_Value[19] = convert_binary_to_BCD(Parameter_19);

	memcpy(&Parameter_Name, "17", 2);								// Display "CH  "
	convert_binary_to_ascii(CFG_Parameters_Value[17], &Parameter_Value[0], &Parameter_Value[1]);

	HMI_Flash_Enable = 1;
	HMI_Flash_Count = 0;
	while (HMI_Flash_Count <4);										// 3 second
	HMI_Flash_Enable = 0;
	Mask = 0xff;

	memcpy(&Parameter_Name, "19", 2);								// Display "CH  "
	convert_binary_to_ascii(CFG_Parameters_Value[19], &Parameter_Value[0], &Parameter_Value[1]);

	HMI_Flash_Enable = 1;
	HMI_Flash_Count = 0;
	while (HMI_Flash_Count <4);										// 3 second
	HMI_Flash_Enable = 0;
	Mask = 0xff;

	memcpy(&Parameter_Name, "--", 2);								// Display "----"  Travel acquisition is OK
	memcpy(&Parameter_Value, "--", 2);

	Write_Parameters_To_Flash_Memory();				// Store parameters
    Calculate_CFG_Parameters();						// Re Calculate Working parameters
}

void Program_Command_Process (void)
{
	uint8_t working_done = 0;
	uint8_t Alarm_And_Limit_Error_Code = 0x00;

	Display_Mode = PROG_MODE;

	memcpy(&Parameter_Name, "AP", 2);
	memcpy(&Parameter_Value, "P-", 2);

	while (!PROG_KEY);

	Tick_Count = 0;									// Reset Tick count
	R_TAU0_Channel1_Start();						// Start Timer channel 1

	while ((Tick_Count <1000) && (working_done == 0))
	{
		if (!PROG_KEY)
		{
			while (!PROG_KEY);
			working_done = 1;
		}
	}
	R_TAU0_Channel1_Stop();							// Stop Timer channel 1

	if (Tick_Count <1000)
	{
		Alarm_And_Limit_Error_Code = Check_Safety_and_Limit_Switch();
		if ((Alarm_And_Limit_Error_Code == NO_ALARM_AND_NO_LIMIT_IS_ACTIVATED)
				|| (Alarm_And_Limit_Error_Code == GATE_CLOSED_LIMIT_SWITCH_IS_ACTIVATED))
		{
			if (CFG_parameter.Encoder_Config != 0x00)		// With Encoder is Enable
			{
				Travel_Acquisition_Procedure_With_Encoder();
			}
			else											// Without Encoder
			{
				Travel_Acquisition_Procedure_Without_Eencoder();
			}
		}
		else
		{
			memcpy(&Parameter_Name, "AP", 2);
			memcpy(&Parameter_Value, "PE", 2);

			HMI_Flash_Count = 0;
			R_TAU0_Channel4_Start();
			while (HMI_Flash_Count < 4);
			R_TAU0_Channel4_Stop();
			Mask = 0xff;
		}
	}

	Display_Mode = NORMAL_MODE;
}
