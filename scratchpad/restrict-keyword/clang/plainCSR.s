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
	jle	LBB0_7
## BB#1:
	movl	(%rdx), %r14d
	movl	%ecx, %r10d
	xorl	%r11d, %r11d
	xorpd	%xmm0, %xmm0
	.p2align	4, 0x90
LBB0_2:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_8 Depth 2
	movq	%r11, %r13
	movl	%r14d, %eax
	leaq	1(%r13), %r11
	movl	4(%rdx,%r13,4), %r14d
	cmpl	%r14d, %eax
	xorpd	%xmm1, %xmm1
	jge	LBB0_6
## BB#3:                                ##   in Loop: Header=BB0_2 Depth=1
	movslq	%r14d, %rbx
	movslq	%eax, %r15
	movl	%r14d, %ecx
	subl	%eax, %ecx
	leaq	-1(%rbx), %r12
	xorpd	%xmm1, %xmm1
	testb	$1, %cl
	movq	%r15, %rax
	je	LBB0_5
## BB#4:                                ##   in Loop: Header=BB0_2 Depth=1
	movsd	(%rdi,%r15,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%r15,4), %rax
	mulsd	(%r8,%rax,8), %xmm1
	addsd	%xmm0, %xmm1
	leaq	1(%r15), %rax
LBB0_5:                                 ##   in Loop: Header=BB0_2 Depth=1
	cmpq	%r15, %r12
	je	LBB0_6
	.p2align	4, 0x90
LBB0_8:                                 ##   Parent Loop BB0_2 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movsd	(%rdi,%rax,8), %xmm2    ## xmm2 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm2
	addsd	%xmm1, %xmm2
	movsd	8(%rdi,%rax,8), %xmm1   ## xmm1 = mem[0],zero
	movslq	4(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm2, %xmm1
	addq	$2, %rax
	cmpq	%rbx, %rax
	jl	LBB0_8
LBB0_6:                                 ##   in Loop: Header=BB0_2 Depth=1
	addsd	(%r9,%r13,8), %xmm1
	movsd	%xmm1, (%r9,%r13,8)
	cmpq	%r10, %r11
	jne	LBB0_2
LBB0_7:
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
