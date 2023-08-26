[section .text]

extern uefi_entry
extern _text

;
; UEFI Entry Point
;
[global _start]
_start:
   mov rdi, rcx
   mov rsi, rdx
   lea rdx, [rel _text]
   jmp uefi_entry

;
; System-V x86_64 ABI -> Microsoft x64 ABI
;
; [rsp+N*8] -> [rsp+N*8] (arg 4+N)
; [rsp]     -> [rsp]     (return address)
; r9        -> r9        (arg 4)
; r8        -> r8        (arg 3)
; rcx       -> rdx       (arg 2)
; rdx       -> rcx       (arg 1)
; rsi       -> <ignored>
; rdi       -> rip       (call)
; rax       -> rax       (return value)
;
[global _system_v_x86_64_abi_to_microsoft_x64_abi]
_system_v_x86_64_abi_to_microsoft_x64_abi:
   mov rax, rdx
   mov rdx, rcx
   mov rcx, rax
   jmp rdi

;
; Fake Relocation Record
;
[section .data]

dummy: dd 0
[section .reloc alloc]

label1:
   dd dummy - label1
   dd 10
   dw 0