/*
 * Common.h
 *
 *  Created on: Nov 21, 2023
 *      Author: Mr. Han
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifndef TRUE
#define TRUE   		1
#endif

#ifndef FALSE
#define FALSE  		0
#endif

#define OCTET (0x08)

#define SET_BIT(reg, mask)       		(reg  |= mask)
#define CLEAR_BIT(reg, mask)     		(reg  &= (~mask))
#define TOGGLE_BIT(reg, mask)    		(reg  ^= mask)
#define BITISSET(reg, mask)      		(reg  &  mask)
#define BITISCLEAR(reg, mask)    		((reg &  mask) == 0)


#define MAKEWORD(msb, lsb)      		(((uint16_t)msb <<8) | (uint16_t)lsb)
#define MAKEUINT32(msb,midb,mida,lsb)	(((uint32_t)msb << 24) | ((uint32_t)midb << 16) | ((uint32_t)mida << 8) | (uint32_t)lsb)

#define BIT(n)    	(1 << n)

#define OK    		0

#define OFFSET_ASCII_NUMBER    (0x30)
#define OFFSET_ASCII_HAX_LOWER (0x37)
#define OFFSET_ASCII_HAX_UPPER (0x57)

#define GET_LOW_1BIT(d)       (d & 0x01)
#define GET_LOW_4BIT(d)       (d & 0x0F)
#define GET_HIGH_4BIT(d)      (d >> 4)
#define GET_LOW_8BIT(d)       (d & 0x00FF)
#define GET_HIGH_8BIT(d)      (d >> OCTET)
#define JOIN_BYTE_DATA(h, l)  ((h << 4) | l)


#endif /* COMMON_H_ */
