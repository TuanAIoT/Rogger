/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_userdefine.h
* Version      : CodeGenerator for RL78/G13 V2.05.07.02 [17 Nov 2023]
* Device(s)    : R5F100LF
* Tool-Chain   : GCCRL78
* Description  : This file includes user definition.
* Creation Date: 15/06/2024
***********************************************************************************************************************/

#ifndef _USER_DEF_H
#define _USER_DEF_H

/***********************************************************************************************************************
User definitions
***********************************************************************************************************************/

/* Start user code for function. Do not edit comment generated here */

/* Output Relays Control Signals*/
#define	COURTESY	P2_bit.no0	//led COR
#define	FLASH		P2_bit.no1	//led LAM
#define	LOCK		P2_bit.no2
#define	M2_FF		P2_bit.no3	//motor2 open
#define	M2_RR		P2_bit.no4	//motor2 close
#define	M1_FF		P2_bit.no5	//motor1 open
#define	M1_RR		P2_bit.no6	//motor1 close
#define	SC_EN		P2_bit.no7	//led SC

/* Output Motor speed Control Signals*/
#define	TRIGER		P3_bit.no0
#define	M2_PWM		P0_bit.no5
#define	M1_PWM		P0_bit.no6

/* 4x7 Segment Display Control Signals*/
#define	DIG_K1		P5_bit.no2
#define	DIG_K2		P5_bit.no3
#define	DIG_K3		P5_bit.no4
#define	DIG_K0		P5_bit.no5

#define	SEG_B		P1_bit.no0
#define	SEG_C		P1_bit.no1
#define	SEG_D		P1_bit.no2
#define	SEG_E		P1_bit.no3
#define	SEG_F		P1_bit.no4
#define	SEG_G		P1_bit.no5
#define	SEG_A		P14_bit.no6
#define	SEG_DP		P1_bit.no7

/* Push Button Switch Input Signals*/
#define	UP_KEY		P12_bit.no4
#define	DOWN_KEY	P12_bit.no3
#define	LEFT_KEY	P7_bit.no7
#define	RIGHT_KEY	P7_bit.no5
#define	PROG_KEY	P1_bit.no6
#define	TEST_KEY	P14_bit.no7

/* Command Control Input Signals*/
#define	PED_IN		P6_bit.no0	//1 gate
#define	PP_IN		P6_bit.no1	//all
#define	CH_IN		P6_bit.no2	//open gate
#define	AP_IN		P6_bit.no3	//stop gate
#define	ORO_IN		P3_bit.no1	//clock

/* Safety Input Signals*/
#define	FT2			P7_bit.no0	//photocell 2
#define	FT1			P7_bit.no1	//photocell 1
#define	COS2		P7_bit.no2	//sensor 2
#define	COS1		P7_bit.no3	//sensor 1
#define	E_STOP		P7_bit.no4

/* Limitation Switch Input Signals*/
#define	FCA1_IN		P4_bit.no1	//switch open leaf 1
#define	FCC1_IN		P4_bit.no2	//switch close leaf 1
#define	FCA2_IN		P4_bit.no3	//switch open leaf 2
#define	FCC2_IN		P12_bit.no0	//switch close leaf 2

/* Encoder Input Signals*/
#define	ENC1		P5_bit.no0
#define	ENC2		P5_bit.no1

/* End user code. Do not edit comment generated here */
#endif
