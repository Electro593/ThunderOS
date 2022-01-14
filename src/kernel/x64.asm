;-----------------------------------------------------------------------|
;                                                                       |
;  Author: Aria Seiler                                                  |
;                                                                       |
;  This program is in the public domain. There is no implied warranty,  |
;  so use it at your own risk.                                          |
;                                                                       |
;-----------------------------------------------------------------------|

[section .text]

[global PortIn08]
[global PortIn16]
[global PortIn32]
[global PortOut08]
[global PortOut16]
[global PortOut32]

PortIn08:
    mov rdx, rdi
    xor rax, rax
    in  al,  dx
    ret

PortIn16:
    mov rdx, rdi
    xor rax, rax
    in  ax,  dx
    ret

PortIn32:
    mov rdx, rdi
    xor rax, rax
    in  eax, dx
    ret

PortOut08:
    mov rdx, rdi
    mov rax, rsi
    out dx,  al
    ret

PortOut16:
    mov rdx, rdi
    mov rax, rsi
    out dx,  ax
    ret

PortOut32:
    mov rdx, rdi
    mov rax, rsi
    out dx,  eax
    ret