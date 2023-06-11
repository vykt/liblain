BITS 64

; All of this assumes the child thread runs indefinitely and doesn't try to
; terminate by returning, since the is nowhere to return to.

handle_syscall:
	syscall
	cmp rax, 0
	jne parent_thread_loop

child_thread_loop:
	mov rbp, 0xFEDCBA9876543210 ; stack base      | SET AT INJECT TIME
	mov rsp, 0x0123456789ABCDEF ; stack pointer   | SET AT INJECT TIME
	mov rax, 0x0000000000000000 ; jump address    | SET AT INJECT TIME
	jmp rax                     ; jump to thread function

parent_thread_loop:
	nop
	jmp parent_thread_loop
