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
	jle	LBB0_9
## BB#1:
	movl	(%rdx), %r14d
	movl	%ecx, %r13d
	xorl	%r11d, %r11d
	.p2align	4, 0x90
LBB0_3:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_8 Depth 2
	movq	%r11, %rcx
	movl	%r14d, %eax
	leaq	1(%rcx), %r11
	movl	4(%rdx,%rcx,4), %r14d
	cmpl	%r14d, %eax
	jge	LBB0_2
## BB#4:                                ##   in Loop: Header=BB0_3 Depth=1
	movq	%r13, %rbx
	movslq	%r14d, %r10
	movslq	%eax, %r15
	movsd	(%r9,%rcx,8), %xmm0     ## xmm0 = mem[0],zero
	movl	%r14d, %r13d
	subl	%eax, %r13d
	leaq	-1(%r10), %r12
	testb	$1, %r13b
	jne	LBB0_6
## BB#5:                                ##   in Loop: Header=BB0_3 Depth=1
	movq	%r15, %rax
	jmp	LBB0_7
	.p2align	4, 0x90
LBB0_6:                                 ##   in Loop: Header=BB0_3 Depth=1
	movsd	(%rdi,%r15,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%r15,4), %rax
	mulsd	(%r8,%rax,8), %xmm1
	addsd	%xmm1, %xmm0
	movsd	%xmm0, (%r9,%rcx,8)
	leaq	1(%r15), %rax
LBB0_7:                                 ##   in Loop: Header=BB0_3 Depth=1
	movq	%rbx, %r13
	cmpq	%r15, %r12
	je	LBB0_2
	.p2align	4, 0x90
LBB0_8:                                 ##   Parent Loop BB0_3 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rbx
	mulsd	(%r8,%rbx,8), %xmm1
	addsd	%xmm0, %xmm1
	movsd	%xmm1, (%r9,%rcx,8)
	movsd	8(%rdi,%rax,8), %xmm0   ## xmm0 = mem[0],zero
	movslq	4(%rsi,%rax,4), %rbx
	mulsd	(%r8,%rbx,8), %xmm0
	addsd	%xmm1, %xmm0
	movsd	%xmm0, (%r9,%rcx,8)
	addq	$2, %rax
	cmpq	%r10, %rax
	jl	LBB0_8
LBB0_2:                                 ##   in Loop: Header=BB0_3 Depth=1
	cmpq	%r13, %r11
	jne	LBB0_3
LBB0_9:
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
