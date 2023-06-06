BITS 64

; All of this assumes the child thread runs indefinitely and doesn't try to
; terminate by returning, since the is nowhere to return to.

handle_syscall:
	syscall
	cmp rax, 0
	jne main_thread_loop

child_thread_loop:
	mov rbp, 0xFEDCBA98 ; stack base      | SET AT INJECT TIME
	mov rsp, 0x89ABCDEF ; stack pointer   | SET AT INJECT TIME
	jmp 0x00000000      ; thread function | SET AT INJECT TIME

parent_thread_loop:
	nop
	jmp parent_thread_loop
