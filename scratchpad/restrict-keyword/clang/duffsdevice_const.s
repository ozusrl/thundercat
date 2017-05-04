	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 12
	.globl	_spmv
	.p2align	4, 0x90
_spmv:                                  ## @spmv
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
	pushq	%rbx
Lcfi3:
	.cfi_offset %rbx, -40
Lcfi4:
	.cfi_offset %r14, -32
Lcfi5:
	.cfi_offset %r15, -24
	testl	%ecx, %ecx
	jle	LBB0_10
## BB#1:
	movl	(%rdx), %eax
	movl	%ecx, %r14d
	addq	$4, %rdx
	leaq	LJTI0_0(%rip), %r10
	.p2align	4, 0x90
LBB0_2:                                 ## =>This Inner Loop Header: Depth=1
	movl	(%rdx), %r11d
	movl	%r11d, %ecx
	subl	%eax, %ecx
	movl	%ecx, %r15d
	sarl	$31, %r15d
	shrl	$30, %r15d
	addl	%ecx, %r15d
	movl	%r15d, %ebx
	andl	$-4, %ebx
	subl	%ebx, %ecx
	xorpd	%xmm0, %xmm0
	cmpl	$3, %ecx
	ja	LBB0_9
## BB#3:                                ##   in Loop: Header=BB0_2 Depth=1
	sarl	$2, %r15d
	movslq	(%r10,%rcx,4), %rcx
	addq	%r10, %rcx
	jmpq	*%rcx
	.p2align	4, 0x90
LBB0_4:                                 ##   in Loop: Header=BB0_2 Depth=1
	decl	%r15d
	cltq
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm1, %xmm0
	incl	%eax
LBB0_5:                                 ##   in Loop: Header=BB0_2 Depth=1
	cltq
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm1, %xmm0
	incl	%eax
LBB0_6:                                 ##   in Loop: Header=BB0_2 Depth=1
	cltq
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm1, %xmm0
	incl	%eax
LBB0_7:                                 ##   in Loop: Header=BB0_2 Depth=1
	cltq
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rcx
	mulsd	(%r8,%rcx,8), %xmm1
	addsd	%xmm1, %xmm0
	incl	%eax
LBB0_8:                                 ##   in Loop: Header=BB0_2 Depth=1
	testl	%r15d, %r15d
	jg	LBB0_4
LBB0_9:                                 ##   in Loop: Header=BB0_2 Depth=1
	addsd	(%r9), %xmm0
	movsd	%xmm0, (%r9)
	addq	$8, %r9
	addq	$4, %rdx
	decq	%r14
	movl	%r11d, %eax
	jne	LBB0_2
LBB0_10:
	popq	%rbx
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc
	.p2align	2, 0x90
	.data_region jt32
L0_0_set_8 = LBB0_8-LJTI0_0
L0_0_set_7 = LBB0_7-LJTI0_0
L0_0_set_6 = LBB0_6-LJTI0_0
L0_0_set_5 = LBB0_5-LJTI0_0
LJTI0_0:
	.long	L0_0_set_8
	.long	L0_0_set_7
	.long	L0_0_set_6
	.long	L0_0_set_5
	.end_data_region


.subsections_via_symbols
