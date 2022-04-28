;-----------------------------------------------------------------------|
;                                                                       |
;  Author: Aria Seiler                                                  |
;                                                                       |
;  This program is in the public domain. There is no implied warranty,  |
;  so use it at your own risk.                                          |
;                                                                       |
;-----------------------------------------------------------------------|

default rel

[section .text]

[global DisableInterrupts]
[global EnableInterrupts]
[global PortIn08]
; [global PortIn16]
; [global PortIn32]
[global PortOut08]
; [global PortOut16]
; [global PortOut32]
[global WriteGDTR]
; [global WriteIDTR]

PortIn08:
    mov rdx, rdi
    xor rax, rax
    in  al,  dx
    ret

; PortIn16:
;     mov rdx, rdi
;     xor rax, rax
;     in  ax,  dx
;     ret

; PortIn32:
;     mov rdx, rdi
;     xor rax, rax
;     in  eax, dx
;     ret

PortOut08:
    mov rdx, rdi
    mov rax, rsi
    out dx,  al
    ret

; PortOut16:
;     mov rdx, rdi
;     mov rax, rsi
;     out dx,  ax
;     ret

; PortOut32:
;     mov rdx, rdi
;     mov rax, rsi
;     out dx,  eax
;     ret

gdtr dw 0
     dq 0
WriteGDTR:
    mov  rdx, rdi
    mov  [gdtr + 2], rdx
    mov  rax, rsi
    dec  ax
    mov  [gdtr], ax
    lgdt [gdtr]
    ret

; idtr dw 0
;      dq 0
; WriteIDTR:
;     mov  rdx, rdi
;     mov  [idtr + 2], rdx
;     mov  rax, rsi
;     dec  ax
;     mov  [idtr], ax
;     lidt [idtr]
;     ret

DisableInterrupts:
    cli

EnableInterrupts:
    sti
