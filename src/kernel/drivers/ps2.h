/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _DRIVERS_PS2_H
#define _DRIVERS_PS2_H

#define PS2_PORT_DATA    0x60
#define PS2_PORT_STATUS  0x64
#define PS2_PORT_CONTROL 0x64

typedef enum ps2_controller_command {
	PS2_Controller_DisablePort2 = 0xA7,
	PS2_Controller_DisablePort1 = 0xAD,
} ps2_controller_command;

typedef enum ps2_device_command {
	PS2_Device_IdentifyKeyboard = 0xF2,
	PS2_Device_EnableScanning	= 0xF4,
	PS2_Device_DisableScanning	= 0xF5,
} ps2_device_command;

typedef enum ps2_response_type {
	PS2_Response_Error0		 = 0x00,
	PS2_Response_Passed		 = 0xAA,
	PS2_Response_Echo		 = 0xEE,
	PS2_Response_Acknowledge = 0xFA,
	PS2_Response_Failed0	 = 0xFC,
	PS2_Response_Failed1	 = 0xFD,
	PS2_Response_Resend		 = 0xFF,
	PS2_Response_Error1		 = 0xFE,
} ps2_response_type;

typedef enum ps2_status_flags {
	PS2_Status_OutputFull		   = (1 << 0),
	PS2_Status_InputFull		   = (1 << 1),
	PS2_Status_SystemFlag		   = (1 << 2),
	PS2_Status_DataIsForController = (1 << 3),
	PS2_Status_TimeoutError		   = (1 << 6),
	PS2_Status_ParityError		   = (1 << 7),
} ps2_status_flags;

// typedef struct ps2_command_queue {
// 	u32	 ReadCursor;
// 	u32	 WriteCursor;
// 	u32	 MaxIndex;
// 	u08 *Commands;
// } ps2_command_queue;

#endif
