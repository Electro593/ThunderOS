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
   Serial_Write(Global.SerialPort, "Exception hit: Divide by zero\r\n");
}

external void
Exception_Debug(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Debug\r\n");
}

external void
Exception_NonMaskableInterrupt(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Non-maskable interrupt\r\n");
}

external void
Exception_Breakpoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Breakpoint\r\n");
}

external void
Exception_Overflow(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Overflow\r\n");
}

external void
Exception_BoundsCheck(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Bounds check\r\n");
}

external void
Exception_InvalidOpcode(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Invalid opcode\r\n");
}

external void
Exception_DeviceNotAvailable(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Device not available\r\n");
}

external void
Exception_DoubleFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Double fault\r\n");
}

external void
Exception_InvalidTSS(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Invalid TSS\r\n");
}

external void
Exception_SegmentNotPresent(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Segment not present\r\n");
}

external void
Exception_StackSegmentFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Stack segment fault\r\n");
}

external void
Exception_GeneralProtection(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: General protection fault\r\n");
   __asm__("hlt");
}

external void
Exception_PageFault(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Page fault\r\n");
   __asm__("hlt");
}

external void
Exception_FloatingPoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Floating point\r\n");
}

external void
Exception_AlignmentCheck(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Alignment check\r\n");
}

external void
Exception_MachineCheck(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Machine check\r\n");
}

external void
Exception_SIMDFloatingPoint(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: SIMD floating point\r\n");
}

external void
Exception_Virtualization(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Virtualization\r\n");
}

external void
Exception_ControlProtection(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Control protection\r\n");
}

external void
Exception_HypervisorInjection(void)
{
   Serial_Write(Global.SerialPort, "Exception hit: Hypervisor injection\r\n");
}

external void
Exception_VMMCommunication(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: VMM communication\r\n");
}

external void
Exception_Security(u32 ErrorCode)
{
   Serial_Write(Global.SerialPort, "Exception hit: Security\r\n");
}

#endif