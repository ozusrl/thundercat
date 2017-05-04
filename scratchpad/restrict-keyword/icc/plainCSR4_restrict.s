# mark_description "Intel(R) C Intel(R) 64 Compiler for applications running on Intel(R) 64, Version 17.0.2.163 Build 20170213";
# mark_description "-std=c99 -O3 -S -o icc/plainCSR4_restrict.s";
	.file "plainCSR4_restrict.c"
	.section	__TEXT, __text
L_TXTST0:
# -- Begin  _spMV_CSR1
	.section	__TEXT, __text
# mark_begin;
       .align    4
	.globl _spMV_CSR1
# --- spMV_CSR1(double *, int *, int *, int, double *, double *)
_spMV_CSR1:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %ecx
# parameter 5: %r8
# parameter 6: %r9
L_B1.1:                         # Preds L_B1.0
                                # Execution count [1.00e+00]
..LCFI1:
L____tag_value__spMV_CSR1.1:
L_L2:
                                                          #1.81
        movl      $1, %eax                                      #2.3
        movslq    %ecx, %r10                                    #1.81
        xorl      %ecx, %ecx                                    #2.3
        testq     %r10, %r10                                    #2.23
        jle       L_B1.26       # Prob 9%                       #2.23
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 r13 r14 r15
L_B1.2:                         # Preds L_B1.1
                                # Execution count [9.00e-01]
        movq      %r12, -48(%rsp)                               #3.15[spill]
        movq      %r13, -56(%rsp)                               #3.15[spill]
        xorps     %xmm0, %xmm0                                  #3.15
        movq      %r14, -40(%rsp)                               #3.15[spill]
        movq      %r15, -32(%rsp)                               #3.15[spill]
        movq      %rbx, -24(%rsp)                               #3.15[spill]
        movq      %rbp, -16(%rsp)                               #3.15[spill]
..LCFI2:
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 xmm0
L_B1.3:                         # Preds L_B1.24 L_B1.2
                                # Execution count [5.00e+00]
        movl      (%rdx,%rax,4), %ebx                           #5.27
        xorl      %r13d, %r13d                                  #5.5
        movl      (%rdx,%rcx,4), %r12d                          #5.14
        xorps     %xmm1, %xmm1                                  #3.15
        lea       -3(%rbx), %ebp                                #5.41
        cmpl      %ebp, %r12d                                   #5.41
        jge       L_B1.7        # Prob 10%                      #5.41
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 ebx r12d r13d xmm0 xmm1
L_B1.4:                         # Preds L_B1.3
                                # Execution count [4.50e+00]
        movslq    %r12d, %r12                                   #6.13
        movl      %ebx, %r11d                                   #5.41
        subl      %r12d, %r11d                                  #5.41
        sarl      $1, %r11d                                     #5.41
        shrl      $30, %r11d                                    #5.41
        addl      %ebx, %r11d                                   #5.41
        lea       (%rdi,%r12,8), %r14                           #6.13
        subl      %r12d, %r11d                                  #5.41
        lea       (%rsi,%r12,4), %r15                           #6.25
        sarl      $2, %r11d                                     #5.41
        movq      %r9, -8(%rsp)                                 #5.41[spill]
        .align    4
                                # LOE rax rdx rcx rsi rdi r8 r10 r14 r15 ebx r11d r12d r13d xmm0 xmm1
L_B1.5:                         # Preds L_B1.5 L_B1.4
                                # Execution count [2.50e+01]
        lea       (,%r13,4), %ebp                               #6.13
        incl      %r13d                                         #5.5
        movslq    %ebp, %rbp                                    #9.27
        cmpl      %r11d, %r13d                                  #5.5
        movslq    (%r15,%rbp,4), %r9                            #6.25
        movsd     (%r14,%rbp,8), %xmm5                          #6.13
        movsd     8(%r14,%rbp,8), %xmm2                         #7.13
        mulsd     (%r8,%r9,8), %xmm5                            #6.23
        movslq    4(%r15,%rbp,4), %r9                           #7.27
        movsd     16(%r14,%rbp,8), %xmm3                        #8.13
        movsd     24(%r14,%rbp,8), %xmm4                        #9.13
        mulsd     (%r8,%r9,8), %xmm2                            #7.25
        movslq    8(%r15,%rbp,4), %r9                           #8.27
        addsd     %xmm2, %xmm5                                  #7.7
        mulsd     (%r8,%r9,8), %xmm3                            #8.25
        movslq    12(%r15,%rbp,4), %rbp                         #9.27
        addsd     %xmm3, %xmm5                                  #8.7
        mulsd     (%r8,%rbp,8), %xmm4                           #9.25
        addsd     %xmm4, %xmm5                                  #9.7
        addsd     %xmm5, %xmm1                                  #6.7
        jb        L_B1.5        # Prob 82%                      #5.5
                                # LOE rax rdx rcx rsi rdi r8 r10 r14 r15 ebx r11d r12d r13d xmm0 xmm1
L_B1.6:                         # Preds L_B1.5
                                # Execution count [4.50e+00]
        movq      -8(%rsp), %r9                                 #[spill]
        lea       (%r12,%r13,4), %r12d                          #12.7
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 ebx r12d xmm0 xmm1
L_B1.7:                         # Preds L_B1.6 L_B1.3
                                # Execution count [5.00e+00]
        cmpl      %ebx, %r12d                                   #11.16
        movslq    %r12d, %r12                                   #11.5
        jge       L_B1.24       # Prob 50%                      #11.16
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r12 ebx xmm0 xmm1
L_B1.8:                         # Preds L_B1.7
                                # Execution count [4.50e+00]
        movslq    %ebx, %rbx                                    #11.5
        subq      %r12, %rbx                                    #11.5
        cmpq      $8, %rbx                                      #11.5
        jl        L_B1.27       # Prob 10%                      #11.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 xmm0 xmm1
L_B1.9:                         # Preds L_B1.8
                                # Execution count [4.50e+00]
        lea       (%rdi,%r12,8), %rbp                           #12.13
        movq      %rbp, %r13                                    #11.5
        andq      $15, %r13                                     #11.5
        testl     %r13d, %r13d                                  #11.5
        je        L_B1.12       # Prob 50%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 r13d xmm0 xmm1
L_B1.10:                        # Preds L_B1.9
                                # Execution count [4.50e+00]
        testl     $7, %r13d                                     #11.5
        jne       L_B1.27       # Prob 10%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 xmm0 xmm1
L_B1.11:                        # Preds L_B1.10
                                # Execution count [2.25e+00]
        movl      $1, %r13d                                     #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 r13d xmm0 xmm1
L_B1.12:                        # Preds L_B1.11 L_B1.9
                                # Execution count [4.50e+00]
        movl      %r13d, %r11d                                  #11.5
        lea       8(%r11), %r14                                 #11.5
        cmpq      %r14, %rbx                                    #11.5
        jl        L_B1.27       # Prob 10%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13d xmm0 xmm1
L_B1.13:                        # Preds L_B1.12
                                # Execution count [5.00e+00]
        movl      %ebx, %r15d                                   #11.5
        movl      %r15d, %r14d                                  #11.5
        subl      %r13d, %r14d                                  #11.5
        andl      $7, %r14d                                     #11.5
        subl      %r14d, %r15d                                  #11.5
        xorl      %r14d, %r14d                                  #11.5
        movslq    %r15d, %r15                                   #11.5
        testl     %r13d, %r13d                                  #11.5
        lea       (%rsi,%r12,4), %r13                           #12.25
        jbe       L_B1.17       # Prob 10%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15 xmm0 xmm1
L_B1.14:                        # Preds L_B1.13
                                # Execution count [4.50e+00]
        movq      %r9, -8(%rsp)                                 #[spill]
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r12 r13 r14 r15 xmm0 xmm1
L_B1.15:                        # Preds L_B1.14 L_B1.15
                                # Execution count [2.50e+01]
        movslq    (%r13,%r14,4), %r9                            #12.25
        movsd     (%rbp,%r14,8), %xmm2                          #12.13
        incq      %r14                                          #11.5
        cmpq      %r11, %r14                                    #11.5
        mulsd     (%r8,%r9,8), %xmm2                            #12.23
        addsd     %xmm2, %xmm1                                  #12.7
        jb        L_B1.15       # Prob 82%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r12 r13 r14 r15 xmm0 xmm1
L_B1.16:                        # Preds L_B1.15
                                # Execution count [4.50e+00]
        movq      -8(%rsp), %r9                                 #[spill]
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r15 xmm0 xmm1
L_B1.17:                        # Preds L_B1.13 L_B1.16
                                # Execution count [4.50e+00]
        movaps    %xmm0, %xmm4                                  #3.15
        movaps    %xmm0, %xmm3                                  #3.15
        movsd     %xmm1, %xmm4                                  #3.15
        movaps    %xmm0, %xmm2                                  #3.15
        movaps    %xmm0, %xmm1                                  #3.15
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r15 xmm0 xmm1 xmm2 xmm3 xmm4
L_B1.18:                        # Preds L_B1.18 L_B1.17
                                # Execution count [2.50e+01]
        movslq    (%r13,%r11,4), %r14                           #12.23
        movsd     (%r8,%r14,8), %xmm5                           #12.23
        movslq    4(%r13,%r11,4), %r14                          #12.23
        movhpd    (%r8,%r14,8), %xmm5                           #12.23
        movslq    8(%r13,%r11,4), %r14                          #12.23
        mulpd     (%rbp,%r11,8), %xmm5                          #12.23
        movsd     (%r8,%r14,8), %xmm6                           #12.23
        movslq    12(%r13,%r11,4), %r14                         #12.23
        addpd     %xmm5, %xmm4                                  #12.7
        movhpd    (%r8,%r14,8), %xmm6                           #12.23
        movslq    16(%r13,%r11,4), %r14                         #12.23
        mulpd     16(%rbp,%r11,8), %xmm6                        #12.23
        movsd     (%r8,%r14,8), %xmm7                           #12.23
        movslq    20(%r13,%r11,4), %r14                         #12.23
        addpd     %xmm6, %xmm3                                  #12.7
        movhpd    (%r8,%r14,8), %xmm7                           #12.23
        movslq    24(%r13,%r11,4), %r14                         #12.23
        mulpd     32(%rbp,%r11,8), %xmm7                        #12.23
        movsd     (%r8,%r14,8), %xmm8                           #12.23
        movslq    28(%r13,%r11,4), %r14                         #12.23
        addpd     %xmm7, %xmm2                                  #12.7
        movhpd    (%r8,%r14,8), %xmm8                           #12.23
        mulpd     48(%rbp,%r11,8), %xmm8                        #12.23
        addq      $8, %r11                                      #11.5
        addpd     %xmm8, %xmm1                                  #12.7
        cmpq      %r15, %r11                                    #11.5
        jb        L_B1.18       # Prob 82%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r12 r13 r15 xmm0 xmm1 xmm2 xmm3 xmm4
L_B1.19:                        # Preds L_B1.18
                                # Execution count [4.50e+00]
        addpd     %xmm3, %xmm4                                  #3.15
        addpd     %xmm1, %xmm2                                  #3.15
        addpd     %xmm2, %xmm4                                  #3.15
        movaps    %xmm4, %xmm1                                  #3.15
        unpckhpd  %xmm4, %xmm1                                  #3.15
        addsd     %xmm1, %xmm4                                  #3.15
        movaps    %xmm4, %xmm1                                  #3.15
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r15 xmm0 xmm1
L_B1.20:                        # Preds L_B1.19 L_B1.27
                                # Execution count [5.00e+00]
        cmpq      %rbx, %r15                                    #11.5
        jae       L_B1.24       # Prob 10%                      #11.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r15 xmm0 xmm1
L_B1.21:                        # Preds L_B1.20
                                # Execution count [4.50e+00]
        lea       (%rdi,%r12,8), %r11                           #12.13
        lea       (%rsi,%r12,4), %rbp                           #12.25
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r15 xmm0 xmm1
L_B1.22:                        # Preds L_B1.22 L_B1.21
                                # Execution count [2.50e+01]
        movslq    (%rbp,%r15,4), %r12                           #12.25
        movsd     (%r11,%r15,8), %xmm2                          #12.13
        incq      %r15                                          #11.5
        cmpq      %rbx, %r15                                    #11.5
        mulsd     (%r8,%r12,8), %xmm2                           #12.23
        addsd     %xmm2, %xmm1                                  #12.7
        jb        L_B1.22       # Prob 82%                      #11.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r15 xmm0 xmm1
L_B1.24:                        # Preds L_B1.22 L_B1.20 L_B1.7
                                # Execution count [5.00e+00]
        incq      %rcx                                          #2.3
        incq      %rax                                          #2.3
        addsd     (%r9), %xmm1                                  #14.5
        movsd     %xmm1, (%r9)                                  #14.5
        addq      $8, %r9                                       #2.3
        cmpq      %r10, %rcx                                    #2.3
        jb        L_B1.3        # Prob 82%                      #2.3
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 xmm0
L_B1.25:                        # Preds L_B1.24
                                # Execution count [9.00e-01]
        movq      -48(%rsp), %r12                               #[spill]
..LCFI3:
        movq      -56(%rsp), %r13                               #[spill]
..LCFI4:
        movq      -40(%rsp), %r14                               #[spill]
..LCFI5:
        movq      -32(%rsp), %r15                               #[spill]
..LCFI6:
        movq      -24(%rsp), %rbx                               #[spill]
..LCFI7:
        movq      -16(%rsp), %rbp                               #[spill]
..LCFI8:
                                # LOE rbx rbp r12 r13 r14 r15
L_B1.26:                        # Preds L_B1.25 L_B1.1
                                # Execution count [1.00e+00]
        ret                                                     #16.1
..LCFI9:
                                # LOE
L_B1.27:                        # Preds L_B1.8 L_B1.10 L_B1.12
                                # Execution count [4.50e-01]: Infreq
        xorl      %r15d, %r15d                                  #11.5
        jmp       L_B1.20       # Prob 100%                     #11.5
        .align    4
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r15 xmm0 xmm1
..LCFI10:
# mark_end;
	.section	__DATA, __data
# -- End  _spMV_CSR1
	.section	__DATA, __data
	.globl _spMV_CSR1.eh
// -- Begin SEGMENT __eh_frame
	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
__eh_frame_seg:
L.__eh_frame_seg:
EH_frame0:
L_fde_cie_0:
	.long 0x00000014
	.long 0x00000000
	.long 0x00527a01
	.long 0x01107801
	.long 0x08070c10
	.long 0x01900190
_spMV_CSR1.eh:
	.long 0x0000005c
	.long 0x0000001c
	.quad ..LCFI1-_spMV_CSR1.eh-0x8
	.set L_Qlab1,..LCFI10-..LCFI1
	.quad L_Qlab1
	.short 0x0400
	.set L_lab1,..LCFI2-..LCFI1
	.long L_lab1
	.long 0x03860483
	.long 0x088d078c
	.long 0x058f068e
	.byte 0x04
	.set L_lab2,..LCFI3-..LCFI2
	.long L_lab2
	.short 0x04cc
	.set L_lab3,..LCFI4-..LCFI3
	.long L_lab3
	.short 0x04cd
	.set L_lab4,..LCFI5-..LCFI4
	.long L_lab4
	.short 0x04ce
	.set L_lab5,..LCFI6-..LCFI5
	.long L_lab5
	.short 0x04cf
	.set L_lab6,..LCFI7-..LCFI6
	.long L_lab6
	.short 0x04c3
	.set L_lab7,..LCFI8-..LCFI7
	.long L_lab7
	.short 0x04c6
	.set L_lab8,..LCFI9-..LCFI8
	.long L_lab8
	.long 0x03860483
	.long 0x088d078c
	.long 0x058f068e
	.byte 0x00
# End
	.subsections_via_symbols
