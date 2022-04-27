/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum ps2_controller_command {
    PS2_Controller_DisablePort2 = 0xA7,
    PS2_Controller_DisablePort1 = 0xAD,
} ps2_controller_command;

typedef enum ps2_device_command {
    PS2_Device_IdentifyKeyboard = 0xF2,
    PS2_Device_EnableScanning   = 0xF4,
    PS2_Device_DisableScanning  = 0xF5,
} ps2_device_command;

typedef enum ps2_response_type {
    PS2_Response_Error0      = 0x00,
    PS2_Response_Passed      = 0xAA,
    PS2_Response_Echo        = 0xEE,
    PS2_Response_Acknowledge = 0xFA,
    PS2_Response_Failed0     = 0xFC,
    PS2_Response_Failed1     = 0xFD,
    PS2_Response_Resend      = 0xFF,
    PS2_Response_Error1      = 0xFE,
} ps2_response_type;

typedef enum ps2_status_flags {
    PS2_Status_OutputFull          = (1<<0),
    PS2_Status_InputFull           = (1<<1),
    PS2_Status_SystemFlag          = (1<<2),
    PS2_Status_DataIsForController = (1<<3),
    PS2_Status_TimeoutError        = (1<<6),
    PS2_Status_ParityError         = (1<<7),
} ps2_status_flags;

// typedef struct ps2_command_queue {
//     u32 ReadCursor;
//     u32 WriteCursor;
//     u32 MaxIndex;
//     u08 *Commands;
// } ps2_command_queue;

internal b08
PS2_SendCommand(u08 Command)
{
    u08 Status = PortIn08(Port_PS2_Status);
    if(Status & PS2_Status_InputFull)
        return FALSE;
    
    PortOut08(Port_PS2_Command, Command);
    return TRUE;
}

internal b08
PS2_SendData(u08 Data)
{
    u08 Status = PortIn08(Port_PS2_Status);
    if(Status & PS2_Status_InputFull)
        return FALSE;
    
    PortOut08(Port_PS2_Data, Data);
    return TRUE;
}

internal b08
PS2_ReceiveData(u08 *Data)
{
    u08 Status = PortIn08(Port_PS2_Status);
    if(!(Status & PS2_Status_OutputFull))
        return FALSE;
    
    *Data = PortIn08(Port_PS2_Data);
    return TRUE;
}

// internal b08
// InitPS2Controller(acpi ACPI)
// {
//     if(!(ACPI.FADT->IAPCBootArch & IAPCBootArch_8042))
//         return FALSE;
    
//     Assert(PS2_SendCommand(PS2_Controller_DisablePort1));
//     Assert(PS2_SendCommand(PS2_Controller_DisablePort2));
    
//     return TRUE;
// }

// internal void
// DetectPS2Device(void)
// {
//     u08 Ack, Byte1=0xFF, Byte2=0xFF;
    
//     while(!PS2_SendData(PS2_Device_DisableScanning));
//     while(!PS2_ReceiveData(&Ack));
//     Assert(Ack == PS2_Response_Acknowledge);
    
//     while(!PS2_SendData(PS2_Device_IdentifyKeyboard));
//     while(!PS2_ReceiveData(&Ack));
//     Assert(Ack == PS2_Response_Acknowledge);
//     u32 I;
//     for(I = 0; I < 20; I++) {
//         if(PS2_ReceiveData(&Byte1)) break;
//     }
//     if(I == 20) {
//         for(I = 0; I < 20; I++) {
//             if(PS2_ReceiveData(&Byte2)) break;
//         }
//     }
    
//     while(!PS2_SendData(PS2_Device_EnableScanning));
//     while(!PS2_ReceiveData(&Ack));
//     Assert(Ack == PS2_Response_Acknowledge);
    
//     if(Byte1 == 0x00 || Byte1 == 0x03 || Byte1 == 0x04)
// }

// internal b08
// PS2_AddToQueue(ps2_command_queue *Queue,
//                u08 Command)
// {
//     u32 NextWriteCursor = (Queue->WriteCursor + 1) % Queue->MaxIndex;
    
//     // Overflow, discard the new command
//     if(NextWriteCursor == Queue->ReadCursor)
//         return FALSE;
    
//     // Empty queue, send immediately
//     if(Queue->WriteCursor == Queue->ReadCursor)
//         PortOut08(Port_PS2_Data, Command);
    
//     Queue->Commands[Queue->WriteCursor] = Command;
//     Queue->WriteCursor = NextWriteCursor;
    
//     return TRUE;
// }

// internal void
// PS2_Receive(ps2_command_queue *Queue)
// {
//     ps2_response Response = PortIn08(Port_PS2_Data);
    
//     switch(Response) {
//         case PS2_Response_Acknowledge: {
//             Queue->ReadCursor = (Queue->ReadCursor + 1) % Queue->MaxIndex;
            
//             // Queue is empty, nothing to send
//             if(Queue->ReadCursor == Queue->WriteCursor)
//                 break;
            
//             u08 NextCommand = Queue->Commands[Queue->ReadCursor];
//             PortOut08(Port_PS2_Data, NextCommand);
//         } break;
        
//         case PS2_Response_Resend: {
//             u08 Command = Queue->Commands[Queue->ReadCursor];
//             PortOut08(Port_PS2_Data, Command);
//         } break;
//     }
// }