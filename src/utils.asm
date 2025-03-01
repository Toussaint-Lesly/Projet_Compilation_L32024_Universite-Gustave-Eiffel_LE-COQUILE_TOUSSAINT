; utils.asm
section .data
    format_registers db "rax:%ld", 10, 0
    format_stack db "sommet (rsp): %ld, base du bloc (rbp): %ld", 10, 0

section .text
global show_registers
global show_stack
extern printf
show_registers:
    push rbp
    mov rbp, rsp
    
    mov r9,  0
    mov r8,  0
    mov rcx, 0
    mov rdx, 0
    mov rsi, rax
    mov rdi, format_registers
    mov rax, 0
    call printf 
    
    mov rsp, rbp
    pop rbp
    ret

show_stack:
    push rbp
    mov rbp, rsp

    mov rdx, [rsp]
    mov rax, rsp
    add rax, 16
    mov rsi, rax
    mov rdi, format_stack
    mov rax, 0
    call printf WRT ..plt
    
    pop rbp
    ret
