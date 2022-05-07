;-----------------------------------------------------------------------|
;                                                                       |
;  Author: Aria Seiler                                                  |
;                                                                       |
;  This program is in the public domain. There is no implied warranty,  |
;  so use it at your own risk.                                          |
;                                                                       |
;-----------------------------------------------------------------------|

default rel

extern InterruptHandlers
extern APICBase

[section .text]

[global PortIn08]
PortIn08:
    mov rdx, rdi
    xor rax, rax
    in  al,  dx
    ret

; [global PortIn16]
; PortIn16:
;     mov rdx, rdi
;     xor rax, rax
;     in  ax,  dx
;     ret

; [global PortIn32]
; PortIn32:
;     mov rdx, rdi
;     xor rax, rax
;     in  eax, dx
;     ret

[global PortOut08]
PortOut08:
    mov rdx, rdi
    mov rax, rsi
    out dx,  al
    ret

; [global PortOut16]
; PortOut16:
;     mov rdx, rdi
;     mov rax, rsi
;     out dx,  ax
;     ret

; [global PortOut32]
; PortOut32:
;     mov rdx, rdi
;     mov rax, rsi
;     out dx,  eax
;     ret

[global SetGDTR]
gdtr dw 0
     dq 0
SetGDTR:
    mov  [gdtr+2], rdi
    mov  [gdtr], si
    lgdt [gdtr]
    
    mov ax, 0x18
    ltr ax
    
    mov rax, rsp
    push qword 0x10
    push rax
    pushfq
    push qword 0x08
    lea rax, [rel .L]
    push rax
    iretq
.L: ret

[global SetIDTR]
idtr dw 0
     dq 0
SetIDTR:
    mov  [idtr+2], rdi
    mov  [idtr], si
    lidt [idtr]
    ret

[global GetMSR]
GetMSR:
    mov ecx, edi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

[global SetMSR]
SetMSR:
    mov ecx, edi
    mov eax, esi
    shr rsi, 32
    mov edx, esi
    wrmsr
    ret

[global GetCR3]
GetCR3:
    mov rax, cr3
    ret

[global DisableInterrupts]
DisableInterrupts:
    cli
    ret

[global EnableInterrupts]
EnableInterrupts:
    sti
    ret

; TODO: Make this just a number
[global GetInterruptStep]
GetInterruptStep:
    jmp .B
.A: mov rax, 30
    jmp .B
.B: mov rax, .B
    sub rax, .A
    ret

[global InterruptSwitch]
InterruptSwitch:
    push byte 0
    jmp .End
    push byte 1
    jmp .End
    push byte 2
    jmp .End
    push byte 3
    jmp .End
    push byte 4
    jmp .End
    push byte 5
    jmp .End
    push byte 6
    jmp .End
    push byte 7
    jmp .End
    push byte 8
    jmp .End
    push byte 9
    jmp .End
    push byte 10
    jmp .End
    push byte 11
    jmp .End
    push byte 12
    jmp .End
    push byte 13
    jmp .End
    push byte 14
    jmp .End
    push byte 15
    jmp .End
    push byte 16
    jmp .End
    push byte 17
    jmp .End
    push byte 18
    jmp .End
    push byte 19
    jmp .End
    push byte 20
    jmp .End
    push byte 21
    jmp .End
    push byte 22
    jmp .End
    push byte 23
    jmp .End
    push byte 24
    jmp .End
    push byte 25
    jmp .End
    push byte 26
    jmp .End
    push byte 27
    jmp .End
    push byte 28
    jmp .End
    push byte 29
    jmp .End
    push byte 30
.End:
    pushfq
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    
    
    mov rdi, [rsp + 128]
    lea rdx, [InterruptHandlers + rax*8]
    call rdx
    
    mov bx, 32
    mov ax, [rsp+1]
    div bx
    mov cl, ah
    mov ch, al
    
    mov rbx, APICBase
    ; mov rbx, [ACPIBase + ch*0x10 + 0x100]
    mov rax, [rbx]
    mov edx, 1
    shl edx, cl
    and ebx, edx
    cmp ecx, 0
    je .no_eoi
        mov rcx, 0
        lea rdx, [APICBase + 0x0B0];
        mov [rdx], rcx
        ; mov [APICBase + 0x0B0], 0
    .no_eoi:
    
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    popfq
    add rsp, 1
    iretq