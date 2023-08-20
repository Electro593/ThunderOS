[section .text]

extern uefi_entry

[global _start]
_start:
   ; mov rdi, rdx
   ; mov rsi, rcx
   mov rdi, rcx
   mov rsi, rdx
   call uefi_entry
   ret

[section .data]

dummy: dd 0

[section .reloc alloc]

label1:
   dd dummy - label1
   dd 10
   dw 0