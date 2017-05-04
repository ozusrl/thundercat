	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 12
	.globl	_spMV_CSR1
	.p2align	4, 0x90
_spMV_CSR1:                             ## @spMV_CSR1
	.cfi_startproc
## BB#0:
	pushq	%rbp
Lcfi0:
	.cfi_def_cfa_offset 16
Lcfi1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
Lcfi2:
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
Lcfi3:
	.cfi_offset %rbx, -56
Lcfi4:
	.cfi_offset %r12, -48
Lcfi5:
	.cfi_offset %r13, -40
Lcfi6:
	.cfi_offset %r14, -32
Lcfi7:
	.cfi_offset %r15, -24
	testl	%ecx, %ecx
	jle	LBB0_10
## BB#1:
	movl	(%rdx), %r14d
	movl	%ecx, %r10d
	xorl	%r11d, %r11d
	.p2align	4, 0x90
LBB0_4:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_9 Depth 2
	movq	%r11, %r13
	movl	%r14d, %eax
	leaq	1(%r13), %r11
	movl	4(%rdx,%r13,4), %r14d
	cmpl	%r14d, %eax
	jge	LBB0_3
## BB#5:                                ##   in Loop: Header=BB0_4 Depth=1
	movslq	%r14d, %rbx
	movsd	(%r9,%r13,8), %xmm0     ## xmm0 = mem[0],zero
	movslq	%eax, %r15
	movl	%r14d, %ecx
	subl	%eax, %ecx
	leaq	-1(%rbx), %r12
	testb	$1, %cl
	jne	LBB0_7
## BB#6:                                ##   in Loop: Header=BB0_4 Depth=1
	movq	%r15, %rax
	cmpq	%r15, %r12
	jne	LBB0_9
	jmp	LBB0_2
	.p2align	4, 0x90
LBB0_7:                                 ##   in Loop: Header=BB0_4 Depth=1
	movsd	(%rdi,%r15,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%r15,4), %rax
	mulsd	(%r8,%rax,8), %xmm1
	addsd	%xmm1, %xmm0
	leaq	1(%r15), %rax
	cmpq	%r15, %r12
	je	LBB0_2
	.p2align	4, 0x90
LBB0_9:                                 ##   Parent Loop BB0_4 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm0, %xmm1
	movsd	8(%rdi,%rax,8), %xmm0   ## xmm0 = mem[0],zero
	movslq	4(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm0
	addsd	%xmm1, %xmm0
	addq	$2, %rax
	cmpq	%rbx, %rax
	jl	LBB0_9
LBB0_2:                                 ##   in Loop: Header=BB0_4 Depth=1
	movsd	%xmm0, (%r9,%r13,8)
LBB0_3:                                 ##   in Loop: Header=BB0_4 Depth=1
	cmpq	%r10, %r11
	jne	LBB0_4
LBB0_10:
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
