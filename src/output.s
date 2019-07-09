.intel_syntax noprefix
.global main
add:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov [rbp-8], RDI
    mov [rbp-16], RSI
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    mov rax, rbp
    sub rax, 16
    push rax
    pop rax
    mov rax, [rax]
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    push 1
    pop rax
    mov RDI, rax
    push 2
    pop rax
    mov RSI, rax
    call add
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
