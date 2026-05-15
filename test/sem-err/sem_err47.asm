section .bss

section .text
global _start
_start:
    call main
    mov rdi, rax
    mov rax, 60
    syscall

main:
    push rbp
    mov rbp, rsp
    push 0
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    xor rax, rax
    mov rsp, rbp
    pop rbp
    ret


; ---------------- Fonctions predefinies TPC ----------------
putchar:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov byte [rbp-1], dil
    mov rax, 1
    mov rdi, 1
    lea rsi, [rbp-1]
    mov rdx, 1
    syscall
    xor rax, rax
    mov rsp, rbp
    pop rbp
    ret

getchar:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rbp-1]
    mov rdx, 1
    syscall
    cmp rax, 1
    jne io_error
    movzx rax, byte [rbp-1]
    mov rsp, rbp
    pop rbp
    ret

putint:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov rax, rdi
    lea rsi, [rbp-1]
    mov byte [rsi], 10
    mov rcx, 1
    cmp rax, 0
    jge .putint_abs
    neg rax
    mov r8, 1
    jmp .putint_loop
.putint_abs:
    xor r8, r8
.putint_loop:
    xor rdx, rdx
    mov rbx, 10
    div rbx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    inc rcx
    test rax, rax
    jne .putint_loop
    cmp r8, 0
    je .putint_write
    dec rsi
    mov byte [rsi], '-'
    inc rcx
.putint_write:
    mov rax, 1
    mov rdi, 1
    mov rdx, rcx
    syscall
    xor rax, rax
    mov rsp, rbp
    pop rbp
    ret

getint:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    xor r9, r9
    mov r10, 1
.getint_read_first:
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rbp-1]
    mov rdx, 1
    syscall
    cmp rax, 1
    jne io_error
    movzx rbx, byte [rbp-1]
    cmp bl, '-'
    jne .getint_plus
    mov r10, -1
    jmp .getint_read_digit
.getint_plus:
    cmp bl, '+'
    jne .getint_digit_got
.getint_read_digit:
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rbp-1]
    mov rdx, 1
    syscall
    cmp rax, 1
    jne io_error
    movzx rbx, byte [rbp-1]
.getint_digit_got:
    cmp bl, '0'
    jl io_error
    cmp bl, '9'
    jg io_error
.getint_digits:
    imul r9, r9, 10
    sub bl, '0'
    movzx rbx, bl
    add r9, rbx
    xor rax, rax
    xor rdi, rdi
    lea rsi, [rbp-1]
    mov rdx, 1
    syscall
    cmp rax, 1
    jne io_error
    movzx rbx, byte [rbp-1]
    cmp bl, 10
    je .getint_done
    cmp bl, '0'
    jl io_error
    cmp bl, '9'
    jle .getint_digits
    jmp io_error
.getint_done:
    mov rax, r9
    cmp r10, 1
    je .getint_ret
    neg rax
.getint_ret:
    mov rsp, rbp
    pop rbp
    ret
io_error:
    mov rax, 60
    mov rdi, 5
    syscall
