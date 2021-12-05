;-----------------------------------------------------------------------|
;                                                                       |
;  Author: Aria Seiler                                                  |
;                                                                       |
;  This program is in the public domain. There is no implied warranty,  |
;  so use it at your own risk.                                          |
;                                                                       |
;-----------------------------------------------------------------------|


section .text
global  _start
        
_start: sub  rsp, 8
        push rcx
        push rdx
        
; 0:      lea  rdi, ImageBase(rip)
;         lea  rsi, _DYNAMIC(rip)
        
        pop  rcx
        pop  rdx
        push rcx
        push rdx
        call _relocate
        
        pop  rdi
        pop  rsi
        
        call EFI_Entry
        add  rsp, 8
        
.exit:  ret
        
section .data
dummy:  dd   0

%define IMAGE_REL_ABSOLUTE 0
section .reloc
label1: dd   dummy - label1
        dd   12
        dw   (IMAGE_REL_ABSOLUTE << 12) + 0
        dw   (IMAGE_REL_ABSOLUTE << 12) + 0