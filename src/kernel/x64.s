default rel

[section .text]

[global _set_segments]
_set_segments:
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
