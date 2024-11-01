/*
 * Common.c
 *
 *  Created on: Nov 21, 2023
 *      Author: Mr. Han
 */

#include "r_cg_macrodriver.h"
#include "common.h"

void delay_ms(uint16_t m_seccond)
{
	uint16_t tick_count;
	while(m_seccond>0)
		{
		for(tick_count = 0; tick_count < 2904; tick_count ++) NOP();
		m_seccond -= 1;
		}
}

void delay_us(uint16_t u_seccond)
{
	while(u_seccond>0)
		{
		NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); //NOP();
//		NOP(); NOP(); NOP(); NOP();
//		NOP(); NOP(); NOP(); NOP();
		u_seccond -= 1;
		}
}

void convert_binary_to_ascii(uint8_t binary, uint8_t *buffer_high, uint8_t *buffer_low)
{
    uint8_t table_ascii[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    *buffer_high = table_ascii[GET_HIGH_4BIT(binary)];
    *buffer_low = table_ascii[GET_LOW_4BIT(binary)];
}
/***********************************************************************************************************************
* Function Name: convert_ascii_to_binary
* Description  : .
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
uint8_t convert_ascii_to_binary(uint8_t ascii_high, uint8_t ascii_low)
{
    if ((ascii_high >= '0') && ('9' >= ascii_high))
    {
        ascii_high = (ascii_high - OFFSET_ASCII_NUMBER);
    }
    else if((ascii_high >= 'A') && ('F' >= ascii_high))
    {
        ascii_high = (ascii_high - OFFSET_ASCII_HAX_LOWER);
    }
    else if((ascii_high >= 'a') && ('f' >= ascii_high))
    {
        ascii_high = (ascii_high - OFFSET_ASCII_HAX_UPPER);
    }
    else
    {
        /* Do nothing */
    }

    if ((ascii_low >= '0') && ('9' >= ascii_low))
    {
        ascii_low = (ascii_low - OFFSET_ASCII_NUMBER);
    }
    else if((ascii_low >= 'A') && ('F' >= ascii_low))
    {
        ascii_low = (ascii_low - OFFSET_ASCII_HAX_LOWER);
    }
    else if((ascii_low >= 'a') && ('f' >= ascii_low))
    {
        ascii_low = (ascii_low - OFFSET_ASCII_HAX_UPPER);
    }
    else
    {
        /* Do nothing */
    }

    return JOIN_BYTE_DATA(ascii_high, ascii_low);
}

uint8_t convert_BCD_to_binary (uint8_t BCD_number)
{
	return (GET_HIGH_4BIT(BCD_number)*10 + GET_LOW_4BIT(BCD_number));
}

uint8_t convert_binary_to_BCD (uint8_t binary)
{
	uint8_t tmp1, tmp2;

	if (binary > 99) binary = 99;
	tmp1 = binary/10;
	tmp2 = binary - tmp1*10;

	return(JOIN_BYTE_DATA(tmp1,tmp2));
}

