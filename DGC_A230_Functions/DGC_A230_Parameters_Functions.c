/*
 * DGC_A230_Parameters_Functions.c
 *
 *  Created on: Dec 22, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "r_cg_pfdl.h"

#include "string.h"

#include <DGC_A230_Define.h>
#include <Common.h>

extern uint8_t Display_Mode;
extern char Parameter_Name[2];
extern char Parameter_Value[2];
extern uint8_t	HMI_Flash_Count;
uint16_t Key_Tick_Count;
extern uint8_t Mask;
extern uint8_t HMI_Flash_Enable;

struct Device_Config_Parameters CFG_parameter;

extern uint8_t Machine_State;

extern uint8_t Auto_Close_Retry_Number;
extern uint8_t Auto_Close_Gate_Retry_Number;
extern uint8_t Auto_Close_Gate_Timer_Enable;
extern uint16_t Auto_Close_Gate_Timer_Counter;

uint8_t		CFG_Paramenters_Index = 1;
uint8_t    	CFG_Parameters_Value[128];
const char 	CFG_Parameters_Name[128][2] =  {"--", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "--",
											"--", "11", "12", "--", "--", "15", "16", "--", "--", "--",
											"--", "21", "22", "23", "24", "25", "26", "27", "28", "29",
											"30", "31", "32", "33", "34", "35", "36", "37", "38", "--",
											"--", "41", "--", "--", "--", "--", "--", "--", "--", "49",
											"50", "51", "52", "53", "54", "55", "56", "--", "--", "--",
											"60", "61", "62", "63", "64", "65", "--", "--", "--", "--",
											"70", "--", "72", "73", "74", "75", "76", "77", "78", "79",
											"80", "--", "--", "--", "--", "--", "--", "--", "--", "--",
											"90", "--", "--", "--", "--", "--", "--", "--", "--", "--",
											"n0", "n1", "n2", "n3", "n4", "n5", "n6", "n7", "n8", "n9",
											"o0", "o1", "--", "h0", "h1", "d0", "d1", "--", "--", "--",
											"P1", "P2", "P3", "P4", "CP", "--", "--", "--"};
const char	CFG_Parameters_Min_Value[128]= {0x00, 0x00, 0x99, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x04, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00,
											0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const char	CFG_Parameters_Max_Value[128]= {0x99, 0x01, 0x15, 0x01, 0x04, 0x10, 0x01, 0x01, 0x02, 0x99,
											0x99, 0x30, 0x30, 0x40, 0x40, 0x99, 0x01, 0x99, 0x99, 0x99,
											0x99, 0x99, 0x10, 0x10, 0x01, 0x10, 0x60, 0x60, 0x02, 0x06,
											0x01, 0x08, 0x08, 0x08, 0x04, 0x08, 0x20, 0x05, 0x04, 0x99,
											0x99, 0x02, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x03,
											0x04, 0x04, 0x02, 0x04, 0x04, 0x02, 0x02, 0x99, 0x99, 0x99,
											0x01, 0x01, 0x01, 0x01, 0x20, 0x08, 0x99, 0x99, 0x99, 0x99,
											0x02, 0x99, 0x02, 0x04, 0x04, 0x00, 0x08, 0x08, 0x02, 0x99,
											0x01, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
											0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
											0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
											0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
											0x99, 0x99, 0x99, 0x99, 0x01, 0x99, 0x99, 0x99};

void Factory_Default_Setting(uint8_t The_First_Time)
{

	CFG_Parameters_Value[0] = 0xff;			// A0: Reserved #1
	if (The_First_Time == 0x01)
	{
		CFG_Parameters_Value[1] = 0x00;			// A1: Motor type selection
	}
	CFG_Parameters_Value[2] = 0x00;			// A2: Automatic close after pause time (from gate completely open)
	CFG_Parameters_Value[3] = 0x00;			// A3: Automatic gate closing after mains power outage (black-out)
	CFG_Parameters_Value[4] = 0x00;			// A4: Selecting step mode control function (PP)
	CFG_Parameters_Value[5] = 0x00;			// A5: Pre-Flashing
	CFG_Parameters_Value[6] = 0x00;			// A6: Common function for partial open command (PED)
	CFG_Parameters_Value[7] = 0x00;			// A7: Enabling operator present function
	CFG_Parameters_Value[8] = 0x00;			// A8: Gate open indicator / photo cell test function
	CFG_Parameters_Value[9] = 0xff;			// A9: Reserved #2

	CFG_Parameters_Value[10] = 0xff;		// 10: Reserved #3
	CFG_Parameters_Value[11] = 0x25;		// 11: Set MOTOR 1 deceleration space (%)
	CFG_Parameters_Value[12] = 0x25;		// 12: Set MOTOR 2 deceleration space (%)
	CFG_Parameters_Value[13] = 0x10;		// 13: Adjusting LEAF 1 position control
	CFG_Parameters_Value[14] = 0x10;		// 14: Adjusting LEAF 2 position control
	CFG_Parameters_Value[15] = 0x50;		// 15: Partial opening adjustment (%)
	CFG_Parameters_Value[16] = 0x00;		// 16: Setting additional time after direction inversion, with no encoder
	CFG_Parameters_Value[17] = 0x10;		// 17: Motor #1 maneuver time
	CFG_Parameters_Value[18] = 0xff;		// 18:
	CFG_Parameters_Value[19] = 0x10;		// 19: Motor #2 maneuver time

	CFG_Parameters_Value[20] = 0xff;		// 20: Reserved #7
	CFG_Parameters_Value[21] = 0x30;		// 21: Setting automatic closing time
	CFG_Parameters_Value[22] = 0x02;		// 22: MOTOR 1 operating time increase
	CFG_Parameters_Value[23] = 0x02;		// 23: MOTOR 2 operating time increase
	CFG_Parameters_Value[24] = 0x00;		// 24: Enable double maneuver time
	CFG_Parameters_Value[25] = 0x03; 		// 25: Adjusting opening delay of MOTOR 2
	CFG_Parameters_Value[26] = 0x05;		// 26: Adjusting closing delay of MOTOR 1
	CFG_Parameters_Value[27] = 0x02;		// 27: Setting reverse time after activation of sensing edge or obstacle detection (crush prevention)
	CFG_Parameters_Value[28] = 0x01;		// 28: Setting electric lock activation lead time
	CFG_Parameters_Value[29] = 0x03;		// 29: Setting electric lock activation time

	CFG_Parameters_Value[30] = 0x00;		// 30: Enable anti-disturbance filter for power from generator
	CFG_Parameters_Value[31] = 0x06;		// 31: Set motor torque during maneuver
	CFG_Parameters_Value[32] = 0x04;		// 32: Set motor torque during deceleration
	CFG_Parameters_Value[33] = 0x08;		// 33: Adjusting of motor torque
	CFG_Parameters_Value[34] = 0x02;		// 34: Set initial acceleration when opening/closing (soft-start)
	CFG_Parameters_Value[35] = 0x08;    	// 35: Set torque after activation of sensing edge or obstacle detection system
	CFG_Parameters_Value[36] = 0x03;		// 36: Set initial maximum torque boost time
	CFG_Parameters_Value[37] = 0x00;		// 37: Set open/closed stop approach distance
	CFG_Parameters_Value[38] = 0x00;		// 38: Enable lock release reverse impulse
	CFG_Parameters_Value[39] = 0xff;		// 39: Reserved #8

	CFG_Parameters_Value[40] = 0xff;		// 40: Reserved #9
	CFG_Parameters_Value[41] = 0x01;		// 41: Set deceleration during opening/closure
	CFG_Parameters_Value[42] = 0x20;		// 42: Set obstacle detection sensitivity during maneuvers
	CFG_Parameters_Value[43] = 0x50;		// 43: Set obstacle detection sensitivity during deceleration
	CFG_Parameters_Value[44] = 0xff;		// 44: Reserved #10
	CFG_Parameters_Value[45] = 0xff;		// 45: Reserved #11
	CFG_Parameters_Value[46] = 0xff;		// 46: Reserved #12
	CFG_Parameters_Value[47] = 0xff;		// 47: Reserved #13
	CFG_Parameters_Value[48] = 0xff;		// 48: Reserved #14
	CFG_Parameters_Value[49] = 0x00;		// 49: Number of automatic closure attempts after activation of sensing edge or obstacle detection (crush protection)

	CFG_Parameters_Value[50] = 0x00;		// 50: Setting photocell mode during gate opening (FT1)
	CFG_Parameters_Value[51] = 0x00;		// 51: Setting photocell mode during gate closing (FT1)
	CFG_Parameters_Value[52] = 0x01;		// 52: Photocell (FT1) mode with gate closed
	CFG_Parameters_Value[53] = 0x00;		// 53: Setting photocell mode during gate opening (FT2)
	CFG_Parameters_Value[54] = 0x00;		// 54: Setting photocell mode during gate closing (FT2)
	CFG_Parameters_Value[55] = 0x01;		// 55: Photocell (FT2) mode with gate closed
	CFG_Parameters_Value[56] = 0x00;		// 56: Enable close command 6s after activation of photo-cell (FT1-FT2)
	CFG_Parameters_Value[57] = 0xff;		// 57: Reserved #15
	CFG_Parameters_Value[58] = 0xff;		// 58: Reserved #16
	CFG_Parameters_Value[59] = 0xff;		// 59: Reserved #17

	CFG_Parameters_Value[60] = 0x00;		// 60: Enable braking at open and closed mechanical stop/limit switch
	CFG_Parameters_Value[61] = 0x00;		// 61: Enable braking after activation of photo-cells
	CFG_Parameters_Value[62] = 0x00;		// 62: Enable braking after STOP command
	CFG_Parameters_Value[63] = 0x00;		// 63: Enable braking after open > close / close > open inversion
	CFG_Parameters_Value[64] = 0x05;		// 64: Set braking time
	CFG_Parameters_Value[65] = 0x08;		// 65: Set braking force
	CFG_Parameters_Value[66] = 0xff;		// 66: Reserved #18
	CFG_Parameters_Value[67] = 0xff;		// 67: Reserved #19
	CFG_Parameters_Value[68] = 0xff;		// 68: Reserved #20
	CFG_Parameters_Value[69] = 0xff;		// 69: Reserved #21

	CFG_Parameters_Value[70] = 0x02;		// 70: Select number of motors installed
	CFG_Parameters_Value[71] = 0xff;		// 71: Reserved #22
	CFG_Parameters_Value[72] = 0x00;		// 72: Configuring limit switches
	CFG_Parameters_Value[73] = 0x00;		// 73: Configuring sensing edge COS1
	CFG_Parameters_Value[74] = 0x00;		// 74: Configuring sensing edge COS2
	CFG_Parameters_Value[75] = 0x00;		// 75: Configuring encoder
	CFG_Parameters_Value[76] = 0x00;		// 76: Configuring Radio Key PB#1
	CFG_Parameters_Value[77] = 0x01;		// 77: Configuring Radio Key PB#2
	CFG_Parameters_Value[78] = 0x00;		// 78: Configuring flashing light frequency
	CFG_Parameters_Value[79] = 0x60;		// 79: Selecting courtesy light mode

	CFG_Parameters_Value[80] = 0x00;		// 80: Configuring Clock contact
	CFG_Parameters_Value[81] = 0xff;		// 81: Reserved #22
	CFG_Parameters_Value[82] = 0xff;		// 82: Reserved #23
	CFG_Parameters_Value[83] = 0xff;		// 83: Reserved #24
	CFG_Parameters_Value[84] = 0xff;		// 84: Reserved #25
	CFG_Parameters_Value[85] = 0xff;		// 85: Reserved #26
	CFG_Parameters_Value[86] = 0xff;		// 86: Reserved #27
	CFG_Parameters_Value[87] = 0xff;		// 87: Reserved #28
	CFG_Parameters_Value[88] = 0xff;		// 88: Reserved #29
	CFG_Parameters_Value[89] = 0xff;		// 89: Reserved #30

	CFG_Parameters_Value[90] = 0x00;		// 90: Restoring factory default values
	CFG_Parameters_Value[91] = 0xff;		// 91: Reserved #31
	CFG_Parameters_Value[92] = 0xff;		// 92: Reserved #32
	CFG_Parameters_Value[93] = 0xff;		// 93: Reserved #33
	CFG_Parameters_Value[94] = 0xff;		// 94: Reserved #34
	CFG_Parameters_Value[95] = 0xff;		// 95: Reserved #35
	CFG_Parameters_Value[96] = 0xff;		// 96: Reserved #36
	CFG_Parameters_Value[97] = 0xff;		// 97: Reserved #37
	CFG_Parameters_Value[98] = 0xff;		// 98: Reserved #38
	CFG_Parameters_Value[99] = 0xff;		// 99: Reserved #39

	CFG_Parameters_Value[100] = 0x01;		// n0: Hardware version
	CFG_Parameters_Value[101] = 0x24;		// n1: Year of Manufacture
	CFG_Parameters_Value[102] = 0x01;		// n2: Week of Manufacture
	CFG_Parameters_Value[103] = 0x01;		// n3: Serial number # 1
	CFG_Parameters_Value[104] = 0x02;		// n4: Serial number # 2
	CFG_Parameters_Value[105] = 0x03;		// n5: Serial number # 3
	CFG_Parameters_Value[106] = 0x04;		// n6: Serial number # 4
	CFG_Parameters_Value[107] = 0x01;		// n7: Firmware version
	CFG_Parameters_Value[108] = 0x05;		// n8: AIoT Number No1
	CFG_Parameters_Value[109] = 0x23;		// n9: AIoT Number No2

	CFG_Parameters_Value[110] = 0x00;		// o0: Maneuver Counter
	CFG_Parameters_Value[111] = 0x00;		// o1: Maneuver Counter
	CFG_Parameters_Value[112] = 0xff;		// o2: Maneuver Counter
	CFG_Parameters_Value[113] = 0x00;		// h0: Maneuver hour counter
	CFG_Parameters_Value[114] = 0x00;		// h1: Maneuver hour counter
	CFG_Parameters_Value[115] = 0x00;		// d0: Days with unit switched on
	CFG_Parameters_Value[116] = 0x00;		// d1: Days with unit switched on
	CFG_Parameters_Value[117] = 0xff;		// J0: Reserved #40
	CFG_Parameters_Value[118] = 0xff;		// J1: Reserved #41
	CFG_Parameters_Value[119] = 0xff;		// J2: Reserved #42

	CFG_Parameters_Value[120] = 0x00;		// P1: Password
	CFG_Parameters_Value[121] = 0x00;		// P2: Password
	CFG_Parameters_Value[122] = 0x00;		// P3: Password
	CFG_Parameters_Value[123] = 0x00;		// P4: Password
	CFG_Parameters_Value[124] = 0x00;		// CP: Password change protection
	CFG_Parameters_Value[125] = 0xff;		// i0: Reserved #43
	CFG_Parameters_Value[126] = 0xff;		// i1: Reserved #44
	CFG_Parameters_Value[127] = 0xff;		// i2: Reserved #45
}

uint8_t Write_Config_Parameters (void)
{
	uint8_t fdl_status_return;
	uint8_t Temp_Read_Data[128], index;

//	R_FDL_Create();
	R_FDL_Open();

	fdl_status_return = R_FDL_BlankCheck(0x0000, 128);		// Blank check
	if (fdl_status_return != PFDL_OK)
	{
		fdl_status_return = R_FDL_Erase(0);
		if (fdl_status_return != PFDL_OK)
			goto done;
	}

	fdl_status_return =	R_FDL_Write(0x0000, &CFG_Parameters_Value[0], 128);
	if (fdl_status_return != PFDL_OK)
		goto done;

	fdl_status_return =	R_FDL_IVerify(0x0000, 128);
	if (fdl_status_return != PFDL_OK)
		goto done;

	fdl_status_return =	R_FDL_Read(0x0000, &Temp_Read_Data[0], 128);

	fdl_status_return = PFDL_OK;
	for (index = 0; index <128; index++)
	{
		if (Temp_Read_Data[index] != CFG_Parameters_Value[index])
			{
			fdl_status_return = 0x01;
			break;
			}
	}
	done:
	R_FDL_Close();

	return (fdl_status_return);

}

void Reset_Factory_Default_Parameters (uint8_t The_First_Time)
{
	uint8_t fdl_status_return;

	Display_Mode = PARA_MODE;

	Factory_Default_Setting(The_First_Time);
	fdl_status_return = Write_Config_Parameters();

	memcpy(&Parameter_Name[0], "rE", sizeof("rE"));
	memcpy(&Parameter_Value[0], "S-", sizeof("S-"));

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	while (HMI_Flash_Count <8);

	if (fdl_status_return == PFDL_OK)
	{
		memcpy(&Parameter_Name[0], "--", sizeof("--"));
		memcpy(&Parameter_Value[0], "--", sizeof("--"));
	}
	else
	{
		memcpy(&Parameter_Name[0], "Er", sizeof("Er"));
		memcpy(&Parameter_Value[0], "ro", sizeof("ro"));
	}

	while (HMI_Flash_Count <12);
	HMI_Flash_Enable = 0;
	Mask = 0xff;
	Display_Mode = NORMAL_MODE;
}

void Read_Parameters_From_Flash_Memory (void)
{
	uint8_t fdl_status_return;

//	R_FDL_Create();
	R_FDL_Open();
	R_FDL_Read(0x0000, &CFG_Parameters_Value[0], 128);
	R_FDL_Close();
	if (CFG_Parameters_Value[1] == 0xff)		// The first time read, memory is empty
	{
		Reset_Factory_Default_Parameters(0x01);
	}
}

void Write_Parameters_To_Flash_Memory(void)
{
	uint8_t fdl_status_return;

	fdl_status_return = Write_Config_Parameters();

	HMI_Flash_Count = 0;
	HMI_Flash_Enable = 1;

	while (HMI_Flash_Count <6);

	HMI_Flash_Enable = 0;
	Mask = 0xff;
}

void Calculate_CFG_Parameters (void)
{
	uint8_t Val1, Val2, Val3;

	CFG_parameter.Motor_type 					= CFG_Parameters_Value[1];		// A1: Motor type selection
	CFG_parameter.Number_Of_Motors 				= CFG_Parameters_Value[70];		// 70: Select number of motors installed

	CFG_parameter.Double_Time_Enable			= CFG_Parameters_Value[24];		// 24: Enable double maneuver time

	CFG_parameter.Motor_Start_Time				= CFG_Parameters_Value[34]*100;	// Motor Start time = [34]*100

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[36]);
	CFG_parameter.Motor_Maximum_Boost_Time 		= Val1*100;						// Motor Boost with max torque time = [36]*100

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[17]);
	CFG_parameter.M1_Travel_Time				= Val1*100;					// Motor 1 Main maneuver time 		= [17]*100)

	Val2 = convert_BCD_to_binary(CFG_Parameters_Value[22]);
	CFG_parameter.M1_Time_Increase = Val2*100;								// MOTOR 1 operating time increase	= [22] * 100

	Val3 = convert_BCD_to_binary(CFG_Parameters_Value[11]);
	CFG_parameter.M1_Deceleration_Time			= Val1*Val3;				// Motor 1 Deceleration time 		= ([17]+[22])*[11]

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[19]);
	CFG_parameter.M2_Travel_Time				= Val1*100;					// Motor 2 Main maneuver time 		= [19]*100
	Val2 = convert_BCD_to_binary(CFG_Parameters_Value[23]);
	CFG_parameter.M2_Time_Increase = Val2*100;									// MOTOR 2 operating time increase	= [23] * 100

	Val3 = convert_BCD_to_binary(CFG_Parameters_Value[12]);
	CFG_parameter.M2_Deceleration_Time			= Val1*Val3;				// Motor 2 Deceleration time 		= ([19]+[23])*[12]

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[15]);
	CFG_parameter.Patial_Open_Adjust			= Val1;							// 15: Partial opening adjustment (%)

	if (CFG_parameter.Double_Time_Enable == 0x01)
	{
		CFG_parameter.M1_Travel_Time <<= 1;
		CFG_parameter.M2_Travel_Time <<= 1;
		CFG_parameter.M1_Deceleration_Time <<= 1;
		CFG_parameter.M2_Deceleration_Time <<= 1;
	}

	CFG_parameter.M1_Maneuver_Time = CFG_parameter.M1_Travel_Time - CFG_parameter.M1_Deceleration_Time;
	CFG_parameter.M2_Maneuver_Time = CFG_parameter.M2_Travel_Time - CFG_parameter.M2_Deceleration_Time;

//	CFG_parameter.M1_Deceleration_Time += CFG_parameter.M1_Time_Increase;
//	CFG_parameter.M2_Deceleration_Time += CFG_parameter.M2_Time_Increase;

	CFG_parameter.M1_Maneuver_Space = CFG_parameter.M1_Maneuver_Time * 3;
	CFG_parameter.M2_Maneuver_Space = CFG_parameter.M2_Maneuver_Time * 3;

	CFG_parameter.M1_Deceleration_Space = CFG_parameter.M1_Deceleration_Time * 3 + CFG_parameter.M1_Time_Increase;
	CFG_parameter.M2_Deceleration_Space = CFG_parameter.M2_Deceleration_Time * 3 + CFG_parameter.M2_Time_Increase;

	CFG_parameter.M1_Total_Space = CFG_parameter.M1_Maneuver_Space + CFG_parameter.M1_Deceleration_Space;
	CFG_parameter.M2_Total_Space = CFG_parameter.M2_Maneuver_Space + CFG_parameter.M2_Deceleration_Space;

	if (CFG_parameter.Number_Of_Motors == 0x01)
	{
		CFG_parameter.M1_Partial_Space = CFG_parameter.M1_Total_Space /100 * CFG_parameter.Patial_Open_Adjust + 6;
	}
	else
	{
		CFG_parameter.M1_Partial_Space = CFG_parameter.M1_Total_Space;
	}

	CFG_parameter.Motor_Deceleration_Level		= CFG_Parameters_Value[41];		// Motor Deceleration level			= [41];

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[25]);

	CFG_parameter.Motor_No2_Open_Delay_Time		= Val1*100; 					// Opening delay time of MOTOR 2 	= [25]*100

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[26]);
	CFG_parameter.Motor_No1_Close_Delay_Time	= Val1*100;						// Closing delay time of MOTOR 1 	= [26]*100

	if (CFG_Parameters_Value[16] == 0x00)
		CFG_parameter.Addition_Time_After_INV 	= 300;							// 16: additional time after direction inversion, 3s/6s
	else
		CFG_parameter.Addition_Time_After_INV 	= 600;							// 16: additional time after direction inversion, 3s/6s

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[2]);
	CFG_parameter.Auto_Close_After_OPen			= Val1;							// A2: Automatic close after pause time (from gate completely open)

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[21]);
	if (Val1 <= 91)
		CFG_parameter.Automatic_Closing_Time 	= Val1*2;						// 21: Setting automatic closing time = [21]*2	0-91 second
	else
		CFG_parameter.Automatic_Closing_Time 	= (Val1-90)*120;				// 21: Setting automatic closing time = [21]*60*2	2-9  minute


	CFG_parameter.Auto_Close_After_Power		= CFG_Parameters_Value[3];		// A3: Automatic gate closing after mains power outage (black-out)

	CFG_parameter.Step_Mode						= CFG_Parameters_Value[4];		// A4: Selecting step mode control function (PP)

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[5]);
	CFG_parameter.Pre_Flashing 					= Val1;							// A5: Pre-Flashing

	CFG_parameter.Flashing_Light_Frequency		= CFG_Parameters_Value[78];		// 78: Configuring flashing light frequency

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[79]);
	CFG_parameter.Courtery_Light_Mode			= Val1;							// 79: Selecting courtesy light mode

	CFG_parameter.Gate_Open_Indicator			= CFG_Parameters_Value[8];		// A8: Gate open indicator / photo cell test function
	CFG_parameter.Patial_Function				= CFG_Parameters_Value[6];		// A6: Common function for partial open command (PED)

	CFG_parameter.Manual_Function				= CFG_Parameters_Value[7];		// A7: Enabling operator present function

	CFG_parameter.Sensing_Edge_COS1_Config		= CFG_Parameters_Value[73];		// 73: Configuring sensing edge COS1
	CFG_parameter.Sensing_Edge_COS2_Config		= CFG_Parameters_Value[74];		// 74: Configuring sensing edge COS2

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[27]);
	CFG_parameter.Reverse_Time_After_Edge_Sense = Val1*100;						// Reverse time after activation of sensing edge = [27]*100

	CFG_parameter.Auto_Close_Number				= CFG_Parameters_Value[49];		// 49: Number of automatic closure attempts after activation of sensing edge or obstacle detection (crush protection)

	CFG_parameter.Lock_Activation_Lead_Time		= CFG_Parameters_Value[28]*100;	// Electric lock activation lead time 		= [28]*100
	CFG_parameter.Lock_Activation_Time			= CFG_Parameters_Value[29]*100;	// Electric lock activation time 			= [29]*100
	CFG_parameter.Lock_Release_Reverse_Impulse	= CFG_Parameters_Value[38]*100;	// Electric lock release reverse impulse	= [38]*100

	switch(CFG_Parameters_Value[31])											// 31: Set motor torque during maneuver
	{
	case 0x01: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_1; break;
	case 0x02: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_2; break;
	case 0x03: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_3; break;
	case 0x04: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_4; break;
	case 0x05: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_5; break;
	case 0x06: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_6; break;
	case 0x07: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_7; break;
	case 0x08: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_8; break;
	default: 	CFG_parameter.Motor_Torque_Maneuver	= TORQUE_LEVEL_6; break;
	}

	switch(CFG_Parameters_Value[32])											// 32: Set motor torque during deceleration
	{
	case 0x01: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_1; break;
	case 0x02: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_2; break;
	case 0x03: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_3; break;
	case 0x04: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_4; break;
	case 0x05: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_5; break;
	case 0x06: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_6; break;
	case 0x07: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_7; break;
	case 0x08: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_8; break;
	default: 	CFG_parameter.Motor_Torque_Decelaration	= TORQUE_LEVEL_6; break;
	}

	switch(CFG_Parameters_Value[33])											// 33: Set motor torque during boots
	{
	case 0x01: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_1; break;
	case 0x02: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_2; break;
	case 0x03: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_3; break;
	case 0x04: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_4; break;
	case 0x05: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_5; break;
	case 0x06: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_6; break;
	case 0x07: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_7; break;
	case 0x08: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_8; break;
	default: 	CFG_parameter.Motor_Torque_Boots	= TORQUE_LEVEL_8; break;
	}

	switch(CFG_Parameters_Value[35])											// 35: Set torque after activation of sensing edge or obstacle detection system
	{
	case 0x00: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= CFG_parameter.Motor_Torque_Maneuver; break;
	case 0x01: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_1; break;
	case 0x02: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_2; break;
	case 0x03: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_3; break;
	case 0x04: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_4; break;
	case 0x05: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_5; break;
	case 0x06: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_6; break;
	case 0x07: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_7; break;
	case 0x08: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= TORQUE_LEVEL_8; break;
	default: 	CFG_parameter.Motor_Torque_After_Edge_Sense	= CFG_parameter.Motor_Torque_Maneuver; break;
	}

	CFG_parameter.Photocell_FT1_Mode_Opening	= CFG_Parameters_Value[50];		// 50: Setting photocell mode during gate opening (FT1)
	CFG_parameter.Photocell_FT1_Mode_Closing	= CFG_Parameters_Value[51];		// 51: Setting photocell mode during gate closing (FT1)
	CFG_parameter.Photocell_FT1_Mode_Closed		= CFG_Parameters_Value[52];		// 52: Photocell (FT1) mode with gate closed
	CFG_parameter.Photocell_FT2_Mode_Opening	= CFG_Parameters_Value[53];		// 53: Setting photocell mode during gate opening (FT2)
	CFG_parameter.Photocell_FT2_Mode_Closing	= CFG_Parameters_Value[54];		// 54: Setting photocell mode during gate closing (FT2)
	CFG_parameter.Photocell_FT2_Mode_Closed		= CFG_Parameters_Value[55];		// 55: Photocell (FT2) mode with gate closed
	CFG_parameter.Close_Command_Enable_Photocell= CFG_Parameters_Value[56];		// 56: Enable close command 6s after activation of photo-cell (FT1-FT2)

	CFG_parameter.Limit_Switch_Enable_Braking	= CFG_Parameters_Value[60];		// 60: Enable braking at open and closed mechanical stop/limit switch
	CFG_parameter.Photocell_Enable_Braking		= CFG_Parameters_Value[61];		// 61: Enable braking after activation of photo-cells
	CFG_parameter.Stop_Command_Enable_Braking	= CFG_Parameters_Value[62];		// 62: Enable braking after STOP command
	CFG_parameter.Reverse_Command_Enable_Braking= CFG_Parameters_Value[63];		// 63: Enable braking after open > close / close > open inversion

	Val1 = convert_BCD_to_binary(CFG_Parameters_Value[64]);
	CFG_parameter.Braking_Time					= Val1*10;						// 64: Set braking time = [64]*10

	switch(CFG_Parameters_Value[65])											// // 65: Set braking force
	{
	case 0x05: 	CFG_parameter.Braking_Force		= TORQUE_LEVEL_5; break;
	case 0x06: 	CFG_parameter.Braking_Force		= TORQUE_LEVEL_6; break;
	case 0x07: 	CFG_parameter.Braking_Force		= TORQUE_LEVEL_7; break;
	case 0x08: 	CFG_parameter.Braking_Force		= TORQUE_LEVEL_8; break;
	default: 	CFG_parameter.Braking_Force		= TORQUE_LEVEL_5; break;
	}

	CFG_parameter.Limit_Switch_Config			= CFG_Parameters_Value[72];		// 72: Configuring limit switches
	CFG_parameter.Encoder_Config				= CFG_Parameters_Value[75];		// 75: Configuring encoder

	CFG_parameter.Radio_Config_Key_1			= CFG_Parameters_Value[76];		// 76: Configuring Radio Key PB#1
	CFG_parameter.Radio_Config_Key_2			= CFG_Parameters_Value[77];		// 77: Configuring radio Key PB#2

	CFG_parameter.Clock_Contact_Mode			= CFG_Parameters_Value[80];		// 80: Configuring Clock contact

	convert_binary_to_ascii(CFG_Parameters_Value[110], &CFG_parameter.Maneuver_Counter_No1[0], &CFG_parameter.Maneuver_Counter_No1[1]);
	convert_binary_to_ascii(CFG_Parameters_Value[111], &CFG_parameter.Maneuver_Counter_No2[0], &CFG_parameter.Maneuver_Counter_No2[1]);

	convert_binary_to_ascii(CFG_Parameters_Value[113], &CFG_parameter.Maneuver_Hour_Counter_No1[0], &CFG_parameter.Maneuver_Hour_Counter_No1[1]);
	convert_binary_to_ascii(CFG_Parameters_Value[114], &CFG_parameter.Maneuver_Hour_Counter_No2[0], &CFG_parameter.Maneuver_Hour_Counter_No2[1]);

	convert_binary_to_ascii(CFG_Parameters_Value[115], &CFG_parameter.Control_Unit_Day_Counter_No1[0], &CFG_parameter.Control_Unit_Day_Counter_No1[1]);
	convert_binary_to_ascii(CFG_Parameters_Value[116], &CFG_parameter.Control_Unit_Day_Counter_No2[0], &CFG_parameter.Control_Unit_Day_Counter_No2[1]);

	if (((CFG_parameter.Step_Mode == 0x01) || (CFG_parameter.Step_Mode == 0x02))
		&& (CFG_parameter.Auto_Close_After_OPen == 0))
	{
		Auto_Close_Retry_Number = 1;
	}
	else
	{
		Auto_Close_Retry_Number = CFG_parameter.Auto_Close_After_OPen;
	}

	Auto_Close_Gate_Retry_Number = Auto_Close_Retry_Number;
	Auto_Close_Gate_Timer_Counter = 0;
	Auto_Close_Gate_Timer_Enable = 0;
}

void Parameters_Edit_Process (void)
{
	uint8_t working_done = 0, check_para;
	uint8_t Parameters_Are_Changed;

	Display_Mode = PARA_MODE;

	memcpy(&Parameter_Name[0], CFG_Parameters_Name[CFG_Paramenters_Index], 2);
	convert_binary_to_ascii(CFG_Parameters_Value[CFG_Paramenters_Index], &Parameter_Value[0], &Parameter_Value[1]);

	Parameters_Are_Changed = 0;
	Key_Tick_Count = 0;				// Reset Tick count
//	R_TAU0_Channel1_Start();	// Start Timer channel 1

	while ((working_done == 0) && (Key_Tick_Count < 1000))
	{
		if (!RIGHT_KEY)
		{
			Key_Tick_Count = 0;				// Reset Tick count
			check_para = 0;
			do
			{
				if (++CFG_Paramenters_Index == 128) CFG_Paramenters_Index = 0;
				memcpy(&Parameter_Name[0], CFG_Parameters_Name[CFG_Paramenters_Index], 2);
				if(Parameter_Name[0] != '-') check_para = 1;
			} while(check_para == 0);

			if ((CFG_Paramenters_Index == 2)
					&& (CFG_Parameters_Value[CFG_Paramenters_Index] == 0x00)
					&& ((CFG_Parameters_Value[4] == 0x01) || (CFG_Parameters_Value[4] == 0x02)))
			{
				convert_binary_to_ascii(0x01, &Parameter_Value[0], &Parameter_Value[1]);
			}
			else
			{
				convert_binary_to_ascii(CFG_Parameters_Value[CFG_Paramenters_Index], &Parameter_Value[0], &Parameter_Value[1]);
			}
			while(!RIGHT_KEY);
		}

		if (!LEFT_KEY)
		{
			Key_Tick_Count = 0;				// Reset Tick count
			check_para = 0;
			do
			{
				if (--CFG_Paramenters_Index == 255) CFG_Paramenters_Index = 127;

				memcpy(&Parameter_Name[0], CFG_Parameters_Name[CFG_Paramenters_Index], 2);
				if(Parameter_Name[0] != '-') check_para = 1;
			} while(check_para == 0);

			if ((CFG_Paramenters_Index == 2)
					&& (CFG_Parameters_Value[CFG_Paramenters_Index] == 0x00)
					&& ((CFG_Parameters_Value[4] == 0x01) || (CFG_Parameters_Value[4] == 0x02)))
			{
				convert_binary_to_ascii(0x01, &Parameter_Value[0], &Parameter_Value[1]);
			}
			else
			{
				convert_binary_to_ascii(CFG_Parameters_Value[CFG_Paramenters_Index], &Parameter_Value[0], &Parameter_Value[1]);
			}
			while(!LEFT_KEY);
		}

		if ((!UP_KEY) && (Machine_State == MACHINE_STATE_STOP))
		{
			Key_Tick_Count = 0;				// Reset Tick count

			if (((CFG_Paramenters_Index < 100) || (CFG_Paramenters_Index > 119)) && (CFG_Paramenters_Index != 90))
			{
				Parameters_Are_Changed = 1;

				if(CFG_Parameters_Value[CFG_Paramenters_Index] == CFG_Parameters_Max_Value[CFG_Paramenters_Index])
				{
					CFG_Parameters_Value[CFG_Paramenters_Index] = CFG_Parameters_Min_Value[CFG_Paramenters_Index];
					convert_binary_to_ascii(CFG_Parameters_Value[CFG_Paramenters_Index], &Parameter_Value[0], &Parameter_Value[1]);
				}
				else
				{
					if (++Parameter_Value[1] == 0x3A)
					{
						Parameter_Value[1] = '0';
						if (++Parameter_Value[0] == 0x3A)
						{
							Parameter_Value[0] = '0';
						}
					}

					if ((CFG_Paramenters_Index == 2)
							&& ((CFG_Parameters_Value[4] == 0x01) || (CFG_Parameters_Value[4] == 0x02)))
					{
						if ((Parameter_Value[0] == '0') && (Parameter_Value[1] == '0'))
						{
							Parameter_Value[0] = '0';
							Parameter_Value[1] = '1';
						}
					}

					CFG_Parameters_Value[CFG_Paramenters_Index] = convert_ascii_to_binary(Parameter_Value[0], Parameter_Value[1]);
				}
			}
			while(!UP_KEY);
		}

		if ((!DOWN_KEY) && (Machine_State == MACHINE_STATE_STOP))
		{
			Key_Tick_Count = 0;				// Reset Tick count

			if (((CFG_Paramenters_Index < 100) || (CFG_Paramenters_Index > 119)) && (CFG_Paramenters_Index != 90))
			{
				Parameters_Are_Changed = 1;

				if(CFG_Parameters_Value[CFG_Paramenters_Index] == CFG_Parameters_Min_Value[CFG_Paramenters_Index])
				{
					CFG_Parameters_Value[CFG_Paramenters_Index] = CFG_Parameters_Max_Value[CFG_Paramenters_Index];
					convert_binary_to_ascii(CFG_Parameters_Value[CFG_Paramenters_Index], &Parameter_Value[0], &Parameter_Value[1]);
				}
				else
				{
					if (--Parameter_Value[1] == 0x2F)
					{
						Parameter_Value[1] = '9';
						if (--Parameter_Value[0] == 0x2F)
						{
							Parameter_Value[0] = '9';
						}
					}

					if ((CFG_Paramenters_Index == 2)
							&& ((CFG_Parameters_Value[4] == 0x01) || (CFG_Parameters_Value[4] == 0x02)))
					{
						if ((Parameter_Value[0] == '0') && (Parameter_Value[1] == '0'))
						{
							Parameter_Value[0] = '9';
							Parameter_Value[1] = '9';
						}
					}

					CFG_Parameters_Value[CFG_Paramenters_Index] = convert_ascii_to_binary(Parameter_Value[0], Parameter_Value[1]);
				}
			}
			while(!DOWN_KEY);
		}

		if ((!PROG_KEY) && (Machine_State == MACHINE_STATE_STOP) && (CFG_Paramenters_Index == 90) && (CFG_Parameters_Value[124] == 0x00))
		{
			Key_Tick_Count = 0;								// Reset Tick count
			if (Key_Tick_Count > 400)
			{
		    	Reset_Factory_Default_Parameters(0x00);	// Reset Factory Default Parameters
		        Calculate_CFG_Parameters();				// ReCalculate Working parameters
		        Key_Tick_Count = 1500;
			}
			while(!PROG_KEY);
		}
	}

//	R_TAU0_Channel1_Stop();								// Stop Timer channel 1

	if (Parameters_Are_Changed == 1)
	{
		Write_Parameters_To_Flash_Memory();				// Store parameters
	    Calculate_CFG_Parameters();						// Calculate Working parameters
	}

	Display_Mode = NORMAL_MODE;
}
