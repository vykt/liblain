BITS 64;

add_sveta:
	push rbp
	mov rbp, rsp	; le stack is ready

	mov rax, rdi
	add rax, rsi
	add rax, 3
	pop rbp
	ret
