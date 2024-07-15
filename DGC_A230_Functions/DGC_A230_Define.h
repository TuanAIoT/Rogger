/*
 * DGC_A230_Define.h
 *
 *  Created on: Nov 22, 2023
 *      Author: Mr. Han
 */

#ifndef DGC_A230_DEFINE_H_
#define DGC_A230_DEFINE_H_

/*Display Mode*/
#define NORMAL_MODE				0x00
#define TEST_MODE				0x01
#define PROG_MODE				0x02
#define PARA_MODE				0x03
#define STANDBY_MODE			0x04

/*Test Mode: Safety Device and Limit Switch Signal State CODE*/
#define NO_ALARM_AND_NO_LIMIT_IS_ACTIVATED			0x00
#define EMEGENCY_STOP_CONTACT_IS_OPENED				0x41
#define SENSING_EDGE_COS1_CONTACT_IS_OPENED			0x39
#define SENSING_EDGE_COS2_CONTACT_IS_OPENED			0x38
#define PHOTOCELL_FT1_CONTACT_IS_OPENED				0x37
#define PHOTOCELL_FT2_CONTACT_IS_OPENED				0x36
#define MORE_THAN_THREE_LIMIT_SW_IS_ACTIVATED		0xFE
#define GATE_OPEN_LIMIT_SWITCH_IS_ACTIVATED			0xFA
#define GATE_CLOSED_LIMIT_SWITCH_IS_ACTIVATED		0xFC
#define LIMIT_SWITCH_ON_GATE_LEAF1_IS_ERROR			0xF1
#define LIMIT_SWITCH_ON_GATE_LEAF2_IS_ERROR			0xF2
#define MOTOR_1_OPEN_LIMIT_SWITCH_IS_ACTIVATED		0x20
#define MOTOR_1_CLOSED_LIMIT_SWITCH_IS_ACTIVATED	0x21
#define MOTOR_2_OPEN_LIMIT_SWITCH_IS_ACTIVATED		0x22
#define MOTOR_2_CLOSED_LIMIT_SWITCH_IS_ACTIVATED	0x23

/*Test Mode: Command Input Message Code*/
#define OPEN_COMMAND		"AP"
#define STEP_COMMAND		"PP"
#define CLOSE_COMMAND		"CH"
#define PATIAL_COMMAND		"PE"
#define CLOCK_COMMAND		"Or"

#define COMMAND_RELEASE_STATE	0x01
#define COMMAND_PRESSED_STATE	0x00

#define TORQUE_LEVEL_8		0x2EDF	// 1000 us
#define TORQUE_LEVEL_7		0x464F	// 2000 us
#define TORQUE_LEVEL_6		0x5DBF	// 3000 us
#define TORQUE_LEVEL_5		0x752F	// 4000 us
#define TORQUE_LEVEL_4		0x8C9F	// 5000 us
#define TORQUE_LEVEL_3		0xA40F	// 6000 us
#define TORQUE_LEVEL_2		0xBB7F	// 7000 us
#define TORQUE_LEVEL_1		0xD2EF	// 8000 us

#define GATE_STATE_UNKNOWN				0x00
#define GATE_STATE_CLOSED				0x01
#define GATE_STATE_OPENING				0x02
#define GATE_STATE_OPENED				0x03
#define GATE_STATE_CLOSING				0x04
#define GATE_STATE_PARTIAL				0x05
#define GATE_STATE_OPENING_PAUSE		0x06
#define GATE_STATE_CLOSING_PAUSE		0x07
#define GATE_STATE_PATIAL_OPENING_PAUSE	0x08

#define MACHINE_STATE_STOP				0x00
#define MACHINE_STATE_NORMAL_OPENING	0x01
#define MACHINE_STATE_NORMAL_CLOSING	0x02
#define MACHINE_STATE_PARTIAL_OPENING	0x03
#define MACHINE_STATE_PARTIAL_CLOSING	0x04

#define UNDEFINE_TIMESTAMP			0x00
#define STARTUP_TIMESTAMP			0x01
#define BOOST_TIMESTAMP				0x02
#define NORMAL_TIMESTAMP			0x03
#define DECELERATION_TIMESTAMP		0x04
#define FINISHED_TIMESTAMP			0x05
#define BRAKING_TIMESTAMP			0x06
#define DISABLE_TIMESTAMP			0x07

#define SAFETY_NONE_EVENT			0x00
#define NORMAL_STOP					0x01
#define SAFETY_EMERGENCY_STOP		0x02
#define SAFETY_PHOTOCELL_FT1		0x03
#define SAFETY_PHOTOCELL_FT2		0x04
#define SAFETY_SENSING_EDGE_COS1	0x05
#define SAFETY_SENSING_EDGE_COS2	0x06
#define STEP_COMMAND_STOP			0x07
#define STEP_COMMAND_REVERSE		0x08
#define OPEN_COMMAND_STOP			0x09
#define CLOSE_COMMAND_STOP			0x0A
#define PARTIAL_COMMAND_STOP		0X0B

#define OPEN_MOVING_DIRECTION		0x00
#define CLOSE_MOVING_DIRECTION		0x01

#define POWER_FAILURE_STATE			0x01
#define POWER_GOOD_STATE			0x00

#define FLASH_DISABLE_TIMER			0x00
#define FLASH_ENABLE_TIMER			0x01

#define FLASH_LIGHT_FREQUENCY_EXTERNAL	0x00
#define FLASH_LIGHT_FREQUENCY_SLOWLY	0x01
#define FLASH_LIGHT_FREQUENCY_RAPIDLY	0x02
#define FLASH_LIGHT_FREQUENCY_RECOVERY	0x03

struct Device_Config_Parameters
	{
	uint8_t 	Motor_type;							// A1: Motor type selection
	uint8_t 	Number_Of_Motors;					// 70: Select number of motors installed

	uint8_t 	Double_Time_Enable;					// 24: Enable double maneuver time

	uint16_t	Motor_Start_Time;					// Motor Start time 				= [34]*100
	uint16_t 	Motor_Maximum_Boost_Time;			// Motor Boost with max torque time = [36]*100

	uint16_t	M1_Time_Increase;					// MOTOR 1 operating time increase	= [22] * 100
	uint16_t	M2_Time_Increase;					// MOTOR 1 operating time increase	= [23] * 100

	uint16_t 	M1_Maneuver_Time;					// Motor 1 Main maneuver time 		= ([17]+[22])*(100-[11])
	uint16_t 	M2_Maneuver_Time;					// Motor 2 Main maneuver time 		= ([19]+[23])*(100-[12])

	uint16_t 	M1_Deceleration_Time;				// Motor 1 Deceleration time 		= ([17]+[22])*[11]
	uint16_t 	M2_Deceleration_Time;				// Motor 2 Deceleration time 		= ([19]+[23])*[12]

	uint8_t		M1_Deceleration_Space;				// Motor 1 Deceleration				= [11]
	uint8_t		M2_Deceleration_Space;				// Motor 2 Deceleration				= [12]

	uint16_t 	Motor_Finish_Time;					// Motor finish time				= [41]*100;

	uint16_t 	Motor_No2_Open_Delay_Time; 			// Opening delay time of MOTOR 2 	= [25]*100
	uint16_t 	Motor_No1_Close_Delay_Time;			// Closing delay time of MOTOR 1 	= [26]*100

	uint16_t 	Addition_Time_After_INV;			// 16: additional time after direction inversion, with no encoder	= 300/600

	uint8_t 	Auto_Close_After_OPen;				// A2: Automatic close after pause time (from gate completely open)
	uint16_t 	Automatic_Closing_Time;				// 21: Setting automatic closing time = [21]*100 or [21]*60*100


	uint8_t 	Auto_Close_After_Power;				// A3: Automatic gate closing after mains power outage (black-out)

	uint8_t 	Step_Mode;							// A4: Selecting step mode control function (PP)

	uint8_t 	Pre_Flashing;						// A5: Pre-Flashing
	uint8_t 	Flashing_Light_Frequency;			// 78: Configuring flashing light frequency

	uint8_t 	Courtery_Light_Mode;				// 79: Selecting courtesy light mode

	uint8_t 	Gate_Open_Indicator;				// A8: Gate open indicator / photo cell test function

	uint8_t 	Patial_Function;					// A6: Common function for partial open command (PED)
	uint8_t 	Patial_Open_Adjust;					// 15: Partial opening adjustment (%)

	uint8_t 	Manual_Function;					// A7: Enabling operator present function

	uint8_t 	Sensing_Edge_COS1_Config;			// 73: Configuring sensing edge COS1
	uint8_t 	Sensing_Edge_COS2_Config;			// 74: Configuring sensing edge COS2
	uint8_t 	Reverse_Time_After_Edge_Sense;		// Reverse time after activation of sensing edge = [27]*100
	uint8_t 	Auto_Close_Number;					// 49: Number of automatic closure attempts after activation of sensing edge or obstacle detection (crush protection)

	uint8_t 	Lock_Activation_Lead_Time;			// Electric lock activation lead time 		= [28]*100
	uint8_t 	Lock_Activation_Time;				// Electric lock activation time 			= [29]*100
	uint8_t 	Lock_Release_Reverse_Impulse;		// Electric lock release reverse impulse	= [38]*100

	uint16_t 	Motor_Torque_Maneuver;				// 31: Set motor torque during maneuver
	uint16_t 	Motor_Torque_Decelaration;			// 32: Set motor torque during deceleration
	uint16_t 	Motor_Torque_Boots;					// 33: Set motor torque boost at start of maneuver
	uint16_t 	Motor_Torque_After_Edge_Sense;		// 35: Set torque after activation of sensing edge or obstacle detection system

	uint8_t 	Photocell_FT1_Mode_Opening;			// 50: Setting photocell mode during gate opening (FT1)
	uint8_t 	Photocell_FT1_Mode_Closing;			// 51: Setting photocell mode during gate closing (FT1)
	uint8_t 	Photocell_FT1_Mode_Closed;			// 52: Photocell (FT1) mode with gate closed
	uint8_t 	Photocell_FT2_Mode_Opening;			// 53: Setting photocell mode during gate opening (FT2)
	uint8_t 	Photocell_FT2_Mode_Closing;			// 54: Setting photocell mode during gate closing (FT2)
	uint8_t 	Photocell_FT2_Mode_Closed;			// 55: Photocell (FT2) mode with gate closed
	uint8_t 	Close_Command_Enable_Photocell;		// 56: Enable close command 6s after activation of photo-cell (FT1-FT2)

	uint8_t 	Limit_Switch_Enable_Braking;		// 60: Enable braking at open and closed mechanical stop/limit switch
	uint8_t 	Photocell_Enable_Braking;			// 61: Enable braking after activation of photo-cells
	uint8_t 	Stop_Command_Enable_Braking;		// 62: Enable braking after STOP command
	uint8_t 	Reverse_Command_Enable_Braking;		// 63: Enable braking after open > close / close > open inversion
	uint8_t 	Braking_Time;						// 64: Set braking time = [64]*10
	uint8_t 	Braking_Force;						// 65: Set braking force

	uint8_t 	Limit_Switch_Config;				// 72: Configuring limit switches

	uint8_t 	Encoder_Config;						// 75: Configuring encoder

	uint8_t 	Radio_Config_Key_1;				// 76: Configuring Communication
	uint8_t 	Radio_Config_Key_2;						// 77: Configuring radio channel

	uint8_t 	Clock_Contact_Mode;					// 80: Configuring Clock contact

	uint8_t 	Maneuver_Counter_No1[2];			// o0: Maneuver Counter
	uint8_t 	Maneuver_Counter_No2[2];			// o1: Maneuver Counter
	uint8_t 	Maneuver_Hour_Counter_No1[2];		// h0: Maneuver hour counter
	uint8_t 	Maneuver_Hour_Counter_No2[2];		// h1: Maneuver hour counter
	uint8_t 	Control_Unit_Day_Counter_No1[2];	// d0: Days with unit switched on
	uint8_t 	Control_Unit_Day_Counter_No2[2];	// d1: Days with unit switched on
	};

#endif /* DGC_A230_DEFINE_H_ */
