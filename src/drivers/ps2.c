/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef u08 ps2_command;
typedef u08 ps2_response;

typedef enum ps2_command_type {
} ps2_command_type;

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

typedef struct ps2_command_queue {
    u32 ReadCursor;
    u32 WriteCursor;
    u32 MaxIndex;
    ps2_command *Commands;
} ps2_command_queue;

internal void
PS2_Init(void)
{
    
}

internal b08
PS2_AddToQueue(ps2_command_queue *Queue,
               ps2_command Command)
{
    u32 NextWriteCursor = (Queue->WriteCursor + 1) % Queue->MaxIndex;
    
    // Overflow, discard the new command
    if(NextWriteCursor == Queue->ReadCursor)
        return FALSE;
    
    // Empty queue, send immediately
    if(Queue->WriteCursor == Queue->ReadCursor)
        PortOut08(Port_PS2_Data, Command);
    
    Queue->Commands[Queue->WriteCursor] = Command;
    Queue->WriteCursor = NextWriteCursor;
    
    return TRUE;
}

internal void
PS2_Receive(ps2_command_queue *Queue)
{
    ps2_response Response = PortIn08(Port_PS2_Data);
    
    switch(Response) {
        case PS2_Response_Acknowledge: {
            Queue->ReadCursor = (Queue->ReadCursor + 1) % Queue->MaxIndex;
            
            // Queue is empty, nothing to send
            if(Queue->ReadCursor == Queue->WriteCursor)
                break;
            
            ps2_command NextCommand = Queue->Commands[Queue->ReadCursor];
            PortOut08(Port_PS2_Data, NextCommand);
        } break;
        
        case PS2_Response_Resend: {
            ps2_command Command = Queue->Commands[Queue->ReadCursor];
            PortOut08(Port_PS2_Data, Command);
        } break;
    }
}