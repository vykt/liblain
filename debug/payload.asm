BITS 64;

add_sveta:
	push rbp
	mov rbp, rsp	; stack setup

	mov rax, rdi
	add rax, rsi
	add rax, 3
	pop rbp
	ret
