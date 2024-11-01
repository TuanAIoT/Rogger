/*
 * 4Digit_7Segment.c
 *
 *  Created on: Nov 21, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/

char Parameters_Display_Data[4] = {'A', '1', '0', '0'};
char Parameter_Name[2] = "A0";
char Parameter_Value[2] = "r-";

uint8_t Mask = 0xff;
//unsigned char Status_Display_Data[4] = {0b00000001, 0b00000000, 0b00000000, 0b00000000};
uint8_t Command_Status_No1 = 0x01, Command_Status_No2, Safety_Status_No1, Safety_Status_No2;
uint8_t Digit_Count;

uint8_t convert_ascii_to_seven_segment(uint8_t ascii_character)
{
	uint8_t ascii_number_table[] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
									0b10110110, 0b10111110,	0b11100000, 0b11111110, 0b11110110,};
    uint8_t Ret_Value;
    switch (ascii_character)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    	{
		Ret_Value = ascii_number_table[ascii_character-'0'];
    	} break;
    case 'A':
    	{
    	Ret_Value = 0b11101110;
    	} break;
    case 'C':
    	{
    	Ret_Value = 0b10011100;
    	} break;
    case 'E':
    	{
    	Ret_Value = 0b10011110;
    	} break;
    case 'F':
    	{
    	Ret_Value = 0b10001110;
    	} break;
    case 'H':
    	{
    	Ret_Value = 0b01101110;
    	} break;
    case 'J':
    	{
    	Ret_Value = 0b01110000;
    	} break;
    case 'O':
    	{
    	Ret_Value = 0b11111100;
    	} break;
    case 'P':
    	{
    	Ret_Value = 0b11001110;
    	} break;
    case 'S':
    	{
    	Ret_Value = 0b10110110;
    	} break;
    case 'U':
    	{
    	Ret_Value = 0b01111100;
    	} break;
    case 'd':
    	{
    	Ret_Value = 0b01111010;
    	} break;
    case 'h':
    	{
    	Ret_Value = 0b00101110;
    	} break;
    case 'i':
    	{
    	Ret_Value = 0b00011000;
    	} break;
    case 'n':
    	{
    	Ret_Value = 0b00101010;
    	} break;
    case 'o':
    	{
    	Ret_Value = 0b00111010;
    	} break;
    case 'r':
    	{
    	Ret_Value = 0b00001010;
    	} break;
    case 't':
    	{
    	Ret_Value = 0b00011110;
    	} break;
    case '-':
    	{
    	Ret_Value = 0b00000010;
    	} break;
    case ' ':
    	{
    	Ret_Value = 0b00000000;
    	} break;
    default: Ret_Value = 0b01101100; break;
    }
    return Ret_Value;
}

uint8_t convert_number_to_seven_segment(uint8_t _number)
{
	uint8_t ascii_number_table[] = {0b11111100, 0b01100000, 0b11011010, 0b11110010,
									0b01100110,	0b10110110, 0b10111110,	0b11100000,
									0b11111110, 0b11110110, 0b11101110, 0b00111110,
									0b10011100, 0b01111010, 0b10011110, 0b10001110};
    uint8_t Ret_Value;
	if (_number < 16)
    {
		Ret_Value = ascii_number_table[_number];
    }
    else
    {
		Ret_Value = 0b01101100;
    }
	return Ret_Value;
}

void Output_Anode_Seven_Segment (uint8_t Digit_Val)
{
	SEG_A =	 (Digit_Val & (1<<7))? 1: 0;
	SEG_B =  (Digit_Val & (1<<6))? 1: 0;
	SEG_C =  (Digit_Val & (1<<5))? 1: 0;
	SEG_D =  (Digit_Val & (1<<4))? 1: 0;
	SEG_E =  (Digit_Val & (1<<3))? 1: 0;
	SEG_F =  (Digit_Val & (1<<2))? 1: 0;
	SEG_G =  (Digit_Val & (1<<1))? 1: 0;
	SEG_DP = (Digit_Val & (1<<0))? 1: 0;
}

void Display_Parameter_Callback()
{
	switch(Digit_Count)
	{
		case 0:
		{
			DIG_K3 = 0;
			Output_Anode_Seven_Segment(convert_ascii_to_seven_segment(Parameter_Value[1]) & Mask);
			DIG_K0 = 1;
		}
		break;
		case 1:
		{
			DIG_K0 = 0;
			Output_Anode_Seven_Segment(convert_ascii_to_seven_segment(Parameter_Value[0]) & Mask);
			DIG_K1 = 1;
		}
		break;
		case 2:
		{
			DIG_K1 = 0;
			Output_Anode_Seven_Segment(convert_ascii_to_seven_segment(Parameter_Name[1]) & Mask);
			DIG_K2 = 1;
		}
		break;
		case 3:
		{
			DIG_K2 = 0;
			Output_Anode_Seven_Segment(convert_ascii_to_seven_segment(Parameter_Name[0]) & Mask);
			DIG_K3 = 1;
		}
		break;
		default:
		{
			// Do nothing;
		}
		break;
	}
}

void Display_Status_Callback()
{
	switch(Digit_Count)
	{
		case 0:
		{
			DIG_K3 = 0;
			Output_Anode_Seven_Segment(Safety_Status_No2);
			DIG_K0 = 1;
		}
		break;
		case 1:
		{
			DIG_K0 = 0;
			Output_Anode_Seven_Segment(Safety_Status_No1);
			DIG_K1 = 1;
		}
		break;
		case 2:
		{
			DIG_K1 = 0;
			Output_Anode_Seven_Segment(Command_Status_No2);
			DIG_K2 = 1;
		}
		break;
		case 3:
		{
			DIG_K2 = 0;
			Output_Anode_Seven_Segment(Command_Status_No1);
			DIG_K3 = 1;
		}
		break;
		default:
		{
			// Do nothing;
		}
		break;
	}
}
