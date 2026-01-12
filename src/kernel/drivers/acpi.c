/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <util/mem.h>
#include <kernel/kernel.h>
#include <drivers/acpi.h>

vptr APICBase;

static b08
ValidateRSDP(rsdp *RSDP)
{
	if (RSDP->Revision != 2) return FALSE;

	u08 Sum = 0;
	for (u32 ByteIndex = 0; ByteIndex < sizeof(rsdp); ++ByteIndex)
		Sum += ((u08 *) RSDP)[ByteIndex];
	if (Sum != 0) return FALSE;

	return TRUE;
}

static b08
ValidateSDT(vptr SDT)
{
	u32 Size = ((acpi_sdt *) SDT)->Length;

	u08 Sum = 0;
	for (u32 ByteIndex = 0; ByteIndex < Size; ++ByteIndex)
		Sum += ((u08 *) SDT)[ByteIndex];
	if (Sum != 0) return FALSE;

	return TRUE;
}

acpi
InitACPI(rsdp *RSDP)
{
	acpi ACPI = { 0 };

	ACPI.RSDP = RSDP;
	Assert(ValidateRSDP(ACPI.RSDP));

	ACPI.XSDT = RSDP->XSDT;
	Assert(ValidateSDT(ACPI.XSDT));

	u32 SDTCount = (ACPI.XSDT->Header.Length - sizeof(acpi_sdt)) / sizeof(vptr);
	for (u32 SDTIndex = 0; SDTIndex < SDTCount; ++SDTIndex) {
		acpi_sdt *SDT = ACPI.XSDT->Entries[SDTIndex];

		if (Mem_Cmp(SDT, "FACP", SIZEOF(acpi_sdt, Signature)) == EQUAL) {
			ACPI.FADT = (acpi_fadt *) SDT;
			Assert(ValidateSDT(ACPI.FADT));
		} else if (Mem_Cmp(SDT, "APIC", SIZEOF(acpi_sdt, Signature)) == EQUAL) {
			ACPI.MADT = (acpi_madt *) SDT;
			Assert(ValidateSDT(ACPI.MADT));
		}
	}

	return ACPI;
}

status
InitAPIC(acpi ACPI)
{
	// Disable PIC
	PortOut08(0x20, 0x11);
	PortOut08(0xA0, 0x11);

	PortOut08(0x21, 0xE0);
	PortOut08(0xA1, 0xE8);

	PortOut08(0x21, 0x04);
	PortOut08(0xA1, 0x02);

	PortOut08(0x21, 0x01);
	PortOut08(0xA1, 0x01);

	PortOut08(0x21, 0xFF);
	PortOut08(0xA1, 0xFF);

	acpi_madt *MADT		 = ACPI.MADT;
	u08		  *LocalAPIC = (u08 *) (u64) MADT->LocalAPICAddress;
	// u08 *IOAPIC = NULL;

	u32 Offset = 0;
	while (Offset < MADT->Header.Length - sizeof(acpi_sdt)) {
		acpi_madt_entry *Entry =
			(acpi_madt_entry *) ((u08 *) MADT->Entries + Offset);

		switch (Entry->Type) {
			// case 1: {
			//     if(Entry->Type1.InterruptBase == 0) {
			//         IOAPIC = (u08*)Entry->Type1.IOAPICAddress;
			//     }
			// } break;
			case 5: {
				LocalAPIC = (u08 *) Entry->Type5.LocalAPICAddress;
			} break;
		}

		Offset += Entry->Length;
	}

	APICBase = LocalAPIC;
	// SetMSR(0x1B, GetMSR(0x1B)|0x800);

	u32 *SpuriousInterruptVector  = (u32 *) (LocalAPIC + 0x0F0);
	*SpuriousInterruptVector	 |= 0x00000100;

	return ST_Success;
}
