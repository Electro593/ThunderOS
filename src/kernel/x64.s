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

[global PortIn32]
PortIn32:
    mov rdx, rdi
    xor rax, rax
    in  eax, dx
    ret

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

[global PortOut32]
PortOut32:
    mov rdx, rdi
    mov rax, rsi
    out dx,  eax
    ret

[global SetSegments]
SetSegments:
    mov rax, rsp
    push rsi
    push rax
    pushfq
    push rdi
    lea rax, [rel .L]
    push rax
    iretq
.L: ret

[global SetSS]
SetSS:
    mov ss, di
    ret

[global SetGDTR]
gdtr dw 0
     dq 0
SetGDTR:
    mov  [gdtr+2], rdi
    mov  [gdtr], si
    lgdt [gdtr]
    
    ; mov rax, rsp
    ; push qword 0x10
    ; push rax
    ; pushfq
    ; push qword 0x08
    ; lea rax, [rel .L]
    ; push rax
    ; iretq
.L: ret

[global SetTR]
SetTR:
    ltr di
    ret

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

[global GetCR0]
GetCR0:
    mov rax, cr0
    ret

[global GetCR3]
GetCR3:
    mov rax, cr3
    ret

[global GetCR4]
GetCR4:
    mov rax, cr4
    ret

[global SetCR0]
SetCR0:
    mov cr0, rdi
    ret

[global SetCR3]
SetCR3:
    mov cr3, rdi
    ret

[global DisableInterrupts]
DisableInterrupts:
    cli
    ret

[global EnableInterrupts]
EnableInterrupts:
    sti
    ret

[global InvalidatePage]
InvalidatePage:
    invlpg [rdi]
    ret

%assign i 0
%rep 256
    %define label InterruptSwitch %+ i
    [global label]
    label %+ :
    %if !(i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 21)
        push qword 0
    %endif
    push word i
    jmp InterruptSwitchEnd
    %assign i i+1
%endrep
InterruptSwitchEnd:
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
    
    xor rdx, rdx
    mov dl, [rsp + 128]
    mov ecx, edx
    and ecx, 0x1F
    
    mov rdx, [InterruptHandlers + rdx*8]
    call rdx
    
    mov eax, [APICBase + 0x100]
    mov ebx, 1
    shl ebx, cl
    and eax, ebx
    cmp eax, 0
    je .no_eoi
        lea rax, [APICBase + 0x0B0]
        mov [rax], dword 1
    .no_eoi:
    
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    popfq
    
    add rsp, 2+8
    
    iretq
