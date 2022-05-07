/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_SOURCE

external void
Exception_DivideByZero(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_Debug(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_NonMaskableInterrupt(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_Breakpoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_Overflow(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_BoundsCheck(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_InvalidOpcode(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_DeviceNotAvailable(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_DoubleFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_InvalidTSS(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_SegmentNotPresent(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_StackSegmentFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_GeneralProtection(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_PageFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_FloatingPoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_AlignmentCheck(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_MachineCheck(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_SIMDFloatingPoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_Virtualization(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_ControlProtection(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_HypervisorInjection(void)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_VMMCommunication(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

external void
Exception_Security(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit!\r\n");
}

#endif