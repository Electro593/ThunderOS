/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <drivers/ps2.h>
#include <kernel/kernel.h>

static b08
PS2_SendCommand(u08 Command)
{
	u08 Status = PortIn08(PS2_PORT_STATUS);
	if (Status & PS2_Status_InputFull) return FALSE;

	PortOut08(PS2_PORT_CONTROL, Command);
	return TRUE;
}

static b08
PS2_SendData(u08 Data)
{
	u08 Status = PortIn08(PS2_PORT_STATUS);
	if (Status & PS2_Status_InputFull) return FALSE;

	PortOut08(PS2_PORT_DATA, Data);
	return TRUE;
}

static b08
PS2_ReceiveData(u08 *Data)
{
	u08 Status = PortIn08(PS2_PORT_STATUS);
	if (!(Status & PS2_Status_OutputFull)) return FALSE;

	*Data = PortIn08(PS2_PORT_DATA);
	return TRUE;
}

// static b08
// InitPS2Controller(acpi ACPI)
// {
//     if(!(ACPI.FADT->IAPCBootArch & IAPCBootArch_8042))
//         return FALSE;

// Assert(PS2_SendCommand(PS2_Controller_DisablePort1));
// Assert(PS2_SendCommand(PS2_Controller_DisablePort2));

// return TRUE;
// }

// static void
// DetectPS2Device(void)
// {
//     u08 Ack, Byte1=0xFF, Byte2=0xFF;

// while(!PS2_SendData(PS2_Device_DisableScanning));
// while(!PS2_ReceiveData(&Ack));
// Assert(Ack == PS2_Response_Acknowledge);

// while(!PS2_SendData(PS2_Device_IdentifyKeyboard));
// while(!PS2_ReceiveData(&Ack));
// Assert(Ack == PS2_Response_Acknowledge);
// u32 I;
// for(I = 0; I < 20; I++) {
//     if(PS2_ReceiveData(&Byte1)) break;
// }
// if(I == 20) {
//     for(I = 0; I < 20; I++) {
//         if(PS2_ReceiveData(&Byte2)) break;
//     }
// }

// while(!PS2_SendData(PS2_Device_EnableScanning));
// while(!PS2_ReceiveData(&Ack));
// Assert(Ack == PS2_Response_Acknowledge);

// if(Byte1 == 0x00 || Byte1 == 0x03 || Byte1 == 0x04)
// }

// static b08
// PS2_AddToQueue(ps2_command_queue *Queue,
//                u08 Command)
// {
//     u32 NextWriteCursor = (Queue->WriteCursor + 1) % Queue->MaxIndex;

// // Overflow, discard the new command
// if(NextWriteCursor == Queue->ReadCursor)
//     return FALSE;

// // Empty queue, send immediately
// if(Queue->WriteCursor == Queue->ReadCursor)
//     PortOut08(Port_PS2_Data, Command);

// Queue->Commands[Queue->WriteCursor] = Command;
// Queue->WriteCursor = NextWriteCursor;

// return TRUE;
// }

// static void
// PS2_Receive(ps2_command_queue *Queue)
// {
//     ps2_response Response = PortIn08(Port_PS2_Data);

// switch(Response) {
//     case PS2_Response_Acknowledge: {
//         Queue->ReadCursor = (Queue->ReadCursor + 1) % Queue->MaxIndex;

// // Queue is empty, nothing to send
// if(Queue->ReadCursor == Queue->WriteCursor)
//     break;

// u08 NextCommand = Queue->Commands[Queue->ReadCursor];
// PortOut08(Port_PS2_Data, NextCommand);
// } break;

// case PS2_Response_Resend: {
//     u08 Command = Queue->Commands[Queue->ReadCursor];
//     PortOut08(Port_PS2_Data, Command);
// } break;
// }
// }
