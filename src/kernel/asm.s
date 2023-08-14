[section .text]

[global _start]
_start:
   mov rdi, rdx
   mov rsi, rcx
   call EFI_Entry
   ret

EFI_Entry:
   sub rsp, 0x28
   mov QWORD [rsp + 0x8], rdi
   mov QWORD [rsp], rsi
   mov rax, QWORD [rsp + 0x8]
   mov rax, QWORD [rax + 0x40]
   mov rdx, QWORD [rax + 0x30]
   mov rax, QWORD [rsp + 0x8]
   mov rax, QWORD [rax + 0x40]
   sub rsp, 0x20
   mov rcx, rax
   call rdx
   add rsp, 0x20
   nop
   jmp EFI_Entry + 0x30
   nop

[section .data]

dummy: dd 0

[section .reloc alloc]

label1:
   dd dummy - label1
   dd 10
   dw 0