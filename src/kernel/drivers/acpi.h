/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _DRIVERS_ACPI_H
#define _DRIVERS_ACPI_H

#include <shared.h>

typedef enum acpi_iapc_boot_arch_flags {
	IAPCBootArch_LegacyDevices	   = (1 << 0),
	IAPCBootArch_8042			   = (1 << 1),
	IAPCBootArch_VGANotPresent	   = (1 << 2),
	IAPCBootArch_MSINotSupported   = (1 << 3),
	IAPCBootArch_PCIeASPMControls  = (1 << 4),
	IAPCBootArch_CMOSRTCNotPresent = (1 << 5),
} acpi_iapc_boot_arch_flags;

#pragma pack(push, 1)
typedef struct acpi_rptr {
	u08 AddressSpace;
	u08 BitWidth;
	u08 BitOffset;
	u08 AccessSize;
	u64 Address;
} acpi_rptr;

typedef struct acpi_madt_entry {
	u08 Type;
	u08 Length;

	union {
		struct {
			u08 ProcessorID;
			u08 APICID;
			u32 Flags;
		} Type0;

		struct {
			u08 IOAPICID;
			u08 _Reserved;
			u32 IOAPICAddress;
			u32 InterruptBase;
		} Type1;

		struct {
			u08 BusSource;
			u08 IRQSource;
			u32 GlobalSystemInterrupt;
			u16 Flags;
		} Type2;

		struct {
			u08 NMISource;
			u08 _Reserved;
			u16 Flags;
			u32 GlobalSystemInterrupt;
		} Type3;

		struct {
			u08 ProcessorID;
			u16 Flags;
			u08 LINTID;
		} Type4;

		struct {
			u16 _Reserved;
			u64 LocalAPICAddress;
		} Type5;

		struct {
			u16 _Reserved;
			u32 Localx2APICID;
			u32 Flags;
			u32 ACPIID;
		} Type9;
	};
} acpi_madt_entry;

typedef struct acpi_sdt {
	c08 Signature[4];
	u32 Length;
	u08 Revision;
	u08 Checksum;
	c08 OEMID[6];
	c08 OEMTableID[8];
	u32 OEMRevision;
	u32 CreatorID;
	u32 CreatorRevision;
} acpi_sdt;

typedef struct acpi_fadt {
	acpi_sdt  Header;
	u32		  FirmwareControl;
	u32		  DSDT;
	u08		  _Reserved0;
	u08		  PreferredPowerManagementProfile;
	u16		  SCIInterrupt;
	u32		  SCICommandPort;
	u08		  ACPIEnable;
	u08		  ACPIDisable;
	u08		  S4BIOSReq;
	u08		  PerformanceStateControl;
	u32		  PME1aBlockPort;
	u32		  PME1bBlockPort;
	u32		  PMC1aBlockPort;
	u32		  PMC1bBlockPort;
	u32		  PMC2BlockPort;
	u32		  PMTBlockPort;
	u32		  GPE0BlockPort;
	u32		  GPE1BlockPort;
	u08		  PM1EventLength;
	u08		  PM1ControlLength;
	u08		  PM2ControlLength;
	u08		  PMTimerLength;
	u08		  GPE0Length;
	u08		  GPE1Length;
	u08		  GPE1Base;
	u08		  CStateControl;
	u16		  WorstC2Latency;
	u16		  WorstC3Latency;
	u16		  FlushSize;
	u16		  FlushStride;
	u08		  DutyOffset;
	u08		  DutyWidth;
	u08		  DayAlarm;
	u08		  MonthAlarm;
	u08		  Century;
	u16		  IAPCBootArch;
	u08		  _Reserved1;
	u32		  Flag;
	acpi_rptr Reset;
	u08		  ResetValue;
	u16		  ARMBootArch;
	u08		  FADTMinorVersion;
	u64		  FirmwareControlX;
	u64		  DSDTX;
	acpi_rptr PM1aEBlockX;
	acpi_rptr PM1bEBlockX;
	acpi_rptr PM1aCBlockX;
	acpi_rptr PM1bCBlockX;
	acpi_rptr PM2CBlockX;
	acpi_rptr PMTBlockX;
	acpi_rptr GPE0BlockX;
	acpi_rptr GPE1BlockX;
	acpi_rptr SleepControl;
	acpi_rptr SleepStatus;
	u64		  HVIdentity;
} acpi_fadt;

typedef struct acpi_madt {
	acpi_sdt		Header;
	u32				LocalAPICAddress;
	u32				Flags;
	acpi_madt_entry Entries[];
} acpi_madt;

typedef struct acpi_xsdt {
	acpi_sdt  Header;
	acpi_sdt *Entries[];  // Physical address
} acpi_xsdt;

typedef struct rsdp {
	c08		   Signature[8];
	u08		   Checksum;
	c08		   OEMID[6];
	u08		   Revision;
	u32		   RSDTAddress;
	u32		   Length;
	acpi_xsdt *XSDT;
	u08		   ExtendedChecksum;
	u08		   _Reserved[3];
} rsdp;
#pragma pack(pop)

typedef struct acpi {
	rsdp *RSDP;

	acpi_fadt *FADT;
	acpi_madt *MADT;
	acpi_xsdt *XSDT;
} acpi;

typedef struct local_apic {
	u08 _Reserved0[32];
	u64 LocalAPICIDRegister[2];
	u64 LocalAPICVersionRegister[2];
} local_apic;

extern vptr APICBase;

acpi   InitACPI(rsdp *RSDP);
status InitAPIC(acpi ACPI);

#endif
