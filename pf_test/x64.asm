extern ori_pf : qword
extern hk_pf_handler : proc

.code

macro_store macro
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push rdi
	push rsi 
	push rbx
	push rbp
endm

macro_load macro
	pop rbp
	pop rbx
	pop rsi
	pop rdi
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
endm	

hk_pf proc
	macro_store
	sub rsp,20h
	call hk_pf_handler
	add rsp,20h
	macro_load
	jmp [ori_pf]
hk_pf endp

get_current_prcb proc
	;lea rax,gs:[0]
	mov rax,gs:[18h]
	ret
get_current_prcb endp

asm_swapgs proc
	swapgs
	ret
asm_swapgs endp


end