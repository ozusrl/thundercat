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
	jle	LBB0_13
## BB#1:
	movl	(%rdx), %r11d
	movl	%ecx, %r10d
	xorl	%r14d, %r14d
	movq	%rdx, -48(%rbp)         ## 8-byte Spill
	.p2align	4, 0x90
LBB0_2:                                 ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_4 Depth 2
                                        ##     Child Loop BB0_11 Depth 2
	movq	%r14, %r15
	movl	%r11d, %r12d
	movl	4(%rdx,%r15,4), %r11d
	movslq	%r11d, %r13
	leaq	-3(%r13), %rcx
	xorpd	%xmm0, %xmm0
	cmpl	%ecx, %r12d
	jge	LBB0_6
## BB#3:                                ##   in Loop: Header=BB0_2 Depth=1
	movq	%r9, %rdx
	movq	%r10, %r9
	movslq	%r12d, %rax
	leal	4(%r12), %r14d
	cmpl	%r14d, %ecx
	cmovgel	%ecx, %r14d
	decl	%r14d
	subl	%r12d, %r14d
	addl	$4, %r14d
	andl	$-4, %r14d
	xorpd	%xmm0, %xmm0
	.p2align	4, 0x90
LBB0_4:                                 ##   Parent Loop BB0_2 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movsd	(%rdi,%rax,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rax,4), %rbx
	mulsd	(%r8,%rbx,8), %xmm1
	addsd	%xmm0, %xmm1
	movslq	4(%rsi,%rax,4), %rbx
	movupd	8(%rdi,%rax,8), %xmm0
	movslq	8(%rsi,%rax,4), %r10
	movsd	(%r8,%rbx,8), %xmm2     ## xmm2 = mem[0],zero
	movhpd	(%r8,%r10,8), %xmm2     ## xmm2 = xmm2[0],mem[0]
	mulpd	%xmm0, %xmm2
	addsd	%xmm2, %xmm1
	movhlps	%xmm2, %xmm2            ## xmm2 = xmm2[1,1]
	addsd	%xmm1, %xmm2
	movsd	24(%rdi,%rax,8), %xmm0  ## xmm0 = mem[0],zero
	movslq	12(%rsi,%rax,4), %rbx
	mulsd	(%r8,%rbx,8), %xmm0
	addsd	%xmm2, %xmm0
	addq	$4, %rax
	cmpq	%rcx, %rax
	jl	LBB0_4
## BB#5:                                ##   in Loop: Header=BB0_2 Depth=1
	addl	%r12d, %r14d
	movl	%r14d, %r12d
	movq	%r9, %r10
	movq	%rdx, %r9
	movq	-48(%rbp), %rdx         ## 8-byte Reload
LBB0_6:                                 ##   in Loop: Header=BB0_2 Depth=1
	leaq	1(%r15), %r14
	cmpl	%r11d, %r12d
	jge	LBB0_12
## BB#7:                                ##   in Loop: Header=BB0_2 Depth=1
	movslq	%r12d, %rcx
	movl	%r11d, %eax
	subl	%r12d, %eax
	leaq	-1(%r13), %rbx
	testb	$1, %al
	jne	LBB0_9
## BB#8:                                ##   in Loop: Header=BB0_2 Depth=1
	movq	%rcx, %rax
	cmpq	%rcx, %rbx
	jne	LBB0_11
	jmp	LBB0_12
	.p2align	4, 0x90
LBB0_9:                                 ##   in Loop: Header=BB0_2 Depth=1
	movsd	(%rdi,%rcx,8), %xmm1    ## xmm1 = mem[0],zero
	movslq	(%rsi,%rcx,4), %rax
	mulsd	(%r8,%rax,8), %xmm1
	addsd	%xmm1, %xmm0
	leaq	1(%rcx), %rax
	cmpq	%rcx, %rbx
	je	LBB0_12
	.p2align	4, 0x90
LBB0_11:                                ##   Parent Loop BB0_2 Depth=1
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
	cmpq	%r13, %rax
	jl	LBB0_11
LBB0_12:                                ##   in Loop: Header=BB0_2 Depth=1
	addsd	(%r9,%r15,8), %xmm0
	movsd	%xmm0, (%r9,%r15,8)
	cmpq	%r10, %r14
	jne	LBB0_2
LBB0_13:
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc


.subsections_via_symbols
