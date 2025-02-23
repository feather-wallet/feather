    .text
    .global m_symlink
    .global m_exit
    .global m_mkdir
    .global m_execve
    .global _start

_start:
    pop %rdi
    mov %rsp, %rsi
    lea 8(%rsi, %rdi, 8), %rdx
    call start_c
    mov $60, %rax
    xor %rdi, %rdi
    syscall

m_symlink:
    mov $88, %rax
    syscall
    ret

m_mkdir:
    mov $83, %rax
    syscall
    ret

m_execve:
    mov $59, %rax
    syscall
    ret

m_exit:
    mov $60, %rax
    syscall
