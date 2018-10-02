.globl syscall_entry, kernel_stack
.extern syscall_handler
.intel_syntax noprefix

kernel_stack: .quad 0
user_stack: .quad 0

syscall_entry:
  mov [rip + user_stack], rsp
  mov rsp, [rip + kernel_stack]
  /* save non-callee saved registers */
  push rdi
  push rsi
  push rdx
  push rcx
  push r8
  push r9
  push r10
  push r11

  /* the forth argument */
  mov rcx, r10
  call syscall_handler

  pop r11
  pop r10
  pop r9
  pop r8
  pop rcx
  pop rdx
  pop rsi
  pop rdi

  mov rsp, [rip + user_stack]
  .byte 0x48
  sysret
