/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct rsdp {
    u08 Signature[8];
    u08 Checksum;
    u08 OEMID[6];
    u08 Revision;
    u32 RSDTAddress;
    u32 Length;
    u64 XSDTAddress;
    u08 ExtendedChecksum;
    u08 _Reserved[3];
} rsdp;