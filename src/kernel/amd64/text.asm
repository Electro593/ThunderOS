;-----------------------------------------------------------------------|
;                                                                       |
;  Author: Aria Seiler                                                  |
;                                                                       |
;  This program is in the public domain. There is no implied warranty,  |
;  so use it at your own risk.                                          |
;                                                                       |
;-----------------------------------------------------------------------|


; 


; RCX: Start, RDX: End
enable_cursor:
    out 0x3D4, 0x0A
    in  rax, 0x3D5
    and rax, 0xC0
    or  rax, rcx
    out 0x3D5, rax
    
    out 0x3D4, 0x0B
    in  rax, 0x3D5
    and rax, 0xE0
    or  rax, rdx
    out 0x3D5, rax