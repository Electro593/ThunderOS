/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <drivers/interrupts.h>
#include <drivers/serial.h>
#include <kernel/kernel.h>

void
Exception_DivideByZero(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Divide by zero\r\n");
}

void
Exception_Debug(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Debug\r\n");
}

void
Exception_NonMaskableInterrupt(void)
{
	Serial_Write(
		Global.SerialPort,
		"Exception hit: Non-maskable interrupt\r\n"
	);
}

void
Exception_Breakpoint(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Breakpoint\r\n");
}

void
Exception_Overflow(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Overflow\r\n");
}

void
Exception_BoundsCheck(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Bounds check\r\n");
}

void
Exception_InvalidOpcode(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Invalid opcode\r\n");
}

void
Exception_DeviceNotAvailable(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Device not available\r\n");
}

void
Exception_DoubleFault(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Double fault\r\n");
}

void
Exception_InvalidTSS(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Invalid TSS\r\n");
}

void
Exception_SegmentNotPresent(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Segment not present\r\n");
}

void
Exception_StackSegmentFault(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Stack segment fault\r\n");
}

void
Exception_GeneralProtection(u32 ErrorCode)
{
	Serial_Write(
		Global.SerialPort,
		"Exception hit: General protection fault\r\n"
	);
	__asm__("hlt");
}

void
Exception_PageFault(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Page fault\r\n");
	__asm__("hlt");
}

void
Exception_FloatingPoint(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Floating point\r\n");
}

void
Exception_AlignmentCheck(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Alignment check\r\n");
}

void
Exception_MachineCheck(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Machine check\r\n");
}

void
Exception_SIMDFloatingPoint(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: SIMD floating point\r\n");
}

void
Exception_Virtualization(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Virtualization\r\n");
}

void
Exception_ControlProtection(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Control protection\r\n");
}

void
Exception_HypervisorInjection(void)
{
	Serial_Write(Global.SerialPort, "Exception hit: Hypervisor injection\r\n");
}

void
Exception_VMMCommunication(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: VMM communication\r\n");
}

void
Exception_Security(u32 ErrorCode)
{
	Serial_Write(Global.SerialPort, "Exception hit: Security\r\n");
}
