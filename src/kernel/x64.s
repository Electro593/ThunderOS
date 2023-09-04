default rel

[section .text]

extern handle_interrupt

[global get_cr0]
get_cr0:
   mov rax, cr0
   ret

[global get_cr3]
get_cr3:
   mov rax, cr3
   ret

[global get_cr4]
get_cr4:
   mov rax, cr4
   ret

[global set_cr0]
set_cr0:
   mov cr0, rdi
   ret

[global set_cr3]
set_cr3:
   mov cr3, rdi
   ret

[global set_segments]
set_segments:
   mov rax, rsp
   push rsi          ; will be loaded into SS
   push rax          ; will be loaded into RSP
   pushfq            ; will be loaded into RFLAGS
   push rdi          ; will be loaded into CS
   lea rax, [rel .R]
   push rax          ; will be loaded into RIP
   iretq

.R: ltr dx          ; the iretq returns here
   ret

[global interrupt_switch]
interrupt_switch:
   ; Generate the jump table
   %assign INTERRUPT_SWITCH_DELTA 12
   %assign i 0
   %rep 256
      %define label . %+ i
      label %+ :
      
      ; If there's no error code, push a 0 to have the rsp be the same
      %if i != 8 && i != 10 && i != 11 && i != 12 && i != 13 && i != 14 && i != 17 && i != 21 && i != 29 && i != 30
         push 0
      %endif
      
      push qword i ; Push the interrupt index
      
      jmp .call
      
      times label+INTERRUPT_SWITCH_DELTA-$ nop
      
      %assign i i+1
   %endrep
.call:
   push r15
   push r14
   push r13
   push r12
   push r11
   push r10
   push r9
   push r8
   push rdi
   push rsi
   push rbp
   push rbx
   push rdx
   push rcx
   push rax
   
   mov dil, [rsp + 0x78] ; interrupt_index: u8
   mov esi, [rsp + 0x80] ; error_code: u32
   call handle_interrupt
   
   pop rax
   pop rcx
   pop rdx
   pop rbx
   pop rbp
   pop rsi
   pop rdi
   pop r8
   pop r9
   pop r10
   pop r11
   pop r12
   pop r13
   pop r14
   pop r15
   
   add rsp, 0x10
   
   iretq

[section .data]

[global interrupt_switch_delta]
interrupt_switch_delta dq INTERRUPT_SWITCH_DELTA