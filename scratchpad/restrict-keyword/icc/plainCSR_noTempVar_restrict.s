# mark_description "Intel(R) C Intel(R) 64 Compiler for applications running on Intel(R) 64, Version 17.0.2.163 Build 20170213";
# mark_description "-std=c99 -O3 -S -o icc/plainCSR_noTempVar_restrict.s";
	.file "plainCSR_noTempVar_restrict.c"
	.section	__TEXT, __text
L_TXTST0:
# -- Begin  _spMV_CSR1
	.section	__TEXT, __text
# mark_begin;
       .align    4
	.globl _spMV_CSR1
# --- spMV_CSR1(double *__restrict__, int *__restrict__, int *__restrict__, int, double *__restrict__, double *__restrict__)
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
                                                          #2.67
        movq      %rsi, %r10                                    #2.67
        movslq    %ecx, %rcx                                    #2.67
        xorl      %esi, %esi                                    #3.3
        movl      $1, %eax                                      #3.3
        testq     %rcx, %rcx                                    #3.23
        jle       L_B1.22       # Prob 9%                       #3.23
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 r13 r14 r15
L_B1.2:                         # Preds L_B1.1
                                # Execution count [9.00e-01]
        movq      %r12, -48(%rsp)                               #5.7[spill]
        movq      %r13, -56(%rsp)                               #5.7[spill]
        xorps     %xmm0, %xmm0                                  #5.7
        movq      %r14, -40(%rsp)                               #5.7[spill]
        movq      %r15, -32(%rsp)                               #5.7[spill]
        movq      %rbx, -24(%rsp)                               #5.7[spill]
        movq      %rbp, -16(%rsp)                               #5.7[spill]
..LCFI2:
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 xmm0
L_B1.3:                         # Preds L_B1.20 L_B1.2
                                # Execution count [5.00e+00]
        movl      (%rdx,%rsi,4), %ebx                           #4.18
        movl      (%rdx,%rax,4), %r13d                          #4.31
        cmpl      %r13d, %ebx                                   #4.31
        movslq    %ebx, %r14                                    #4.18
        jge       L_B1.20       # Prob 50%                      #4.31
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r14 ebx r13d xmm0
L_B1.4:                         # Preds L_B1.3
                                # Execution count [4.50e+00]
        subl      %ebx, %r13d                                   #4.5
        cmpl      $8, %r13d                                     #4.5
        jl        L_B1.23       # Prob 10%                      #4.5
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r14 r13d xmm0
L_B1.5:                         # Preds L_B1.4
                                # Execution count [4.50e+00]
        lea       (%rdi,%r14,8), %r11                           #5.15
        movq      %r11, %rbx                                    #4.5
        andq      $15, %rbx                                     #4.5
        movl      %ebx, %ebx                                    #4.5
        testl     %ebx, %ebx                                    #4.5
        je        L_B1.8        # Prob 50%                      #4.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r14 r13d xmm0
L_B1.6:                         # Preds L_B1.5
                                # Execution count [4.50e+00]
        testb     $7, %bl                                       #4.5
        jne       L_B1.23       # Prob 10%                      #4.5
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r11 r14 r13d xmm0
L_B1.7:                         # Preds L_B1.6
                                # Execution count [2.25e+00]
        movl      $1, %ebx                                      #4.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r14 r13d xmm0
L_B1.8:                         # Preds L_B1.7 L_B1.5
                                # Execution count [4.50e+00]
        lea       8(%rbx), %ebp                                 #4.5
        cmpl      %ebp, %r13d                                   #4.5
        jl        L_B1.23       # Prob 10%                      #4.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r11 r14 r13d xmm0
L_B1.9:                         # Preds L_B1.8
                                # Execution count [5.00e+00]
        movl      %r13d, %r12d                                  #4.5
        xorl      %r15d, %r15d                                  #4.5
        subl      %ebx, %r12d                                   #4.5
        lea       (%r10,%r14,4), %rbp                           #5.27
        andl      $7, %r12d                                     #4.5
        negl      %r12d                                         #4.5
        addl      %r13d, %r12d                                  #4.5
        testl     %ebx, %ebx                                    #4.5
        jbe       L_B1.13       # Prob 10%                      #4.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r14 r15 r12d r13d xmm0
L_B1.10:                        # Preds L_B1.9
                                # Execution count [4.50e+00]
        movsd     (%r9,%rsi,8), %xmm1                           #5.7
        movq      %r9, -8(%rsp)                                 #5.7[spill]
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r14 r15 r12d r13d xmm0 xmm1
L_B1.11:                        # Preds L_B1.11 L_B1.10
                                # Execution count [2.50e+01]
        movslq    (%rbp,%r15,4), %r9                            #5.27
        movsd     (%r11,%r15,8), %xmm2                          #5.15
        incq      %r15                                          #4.5
        cmpq      %rbx, %r15                                    #4.5
        mulsd     (%r8,%r9,8), %xmm2                            #5.25
        addsd     %xmm2, %xmm1                                  #5.7
        jb        L_B1.11       # Prob 82%                      #4.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r14 r15 r12d r13d xmm0 xmm1
L_B1.12:                        # Preds L_B1.11
                                # Execution count [4.50e+00]
        movq      -8(%rsp), %r9                                 #[spill]
        movsd     %xmm1, (%r9,%rsi,8)                           #5.7
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r14 r12d r13d xmm0
L_B1.13:                        # Preds L_B1.9 L_B1.12
                                # Execution count [4.50e+00]
        movsd     (%r9,%rsi,8), %xmm4                           #5.7
        movaps    %xmm0, %xmm3                                  #5.7
        movslq    %r12d, %r15                                   #4.5
        movaps    %xmm0, %xmm2                                  #5.7
        movq      %r9, -8(%rsp)                                 #4.5[spill]
        movaps    %xmm0, %xmm1                                  #5.7
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r14 r15 r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4
L_B1.14:                        # Preds L_B1.14 L_B1.13
                                # Execution count [2.50e+01]
        movslq    (%rbp,%rbx,4), %r9                            #5.25
        movsd     (%r8,%r9,8), %xmm5                            #5.25
        movslq    4(%rbp,%rbx,4), %r9                           #5.25
        movhpd    (%r8,%r9,8), %xmm5                            #5.25
        movslq    8(%rbp,%rbx,4), %r9                           #5.25
        mulpd     (%r11,%rbx,8), %xmm5                          #5.25
        movsd     (%r8,%r9,8), %xmm6                            #5.25
        movslq    12(%rbp,%rbx,4), %r9                          #5.25
        addpd     %xmm5, %xmm4                                  #5.7
        movhpd    (%r8,%r9,8), %xmm6                            #5.25
        movslq    16(%rbp,%rbx,4), %r9                          #5.25
        mulpd     16(%r11,%rbx,8), %xmm6                        #5.25
        movsd     (%r8,%r9,8), %xmm7                            #5.25
        movslq    20(%rbp,%rbx,4), %r9                          #5.25
        addpd     %xmm6, %xmm3                                  #5.7
        movhpd    (%r8,%r9,8), %xmm7                            #5.25
        movslq    24(%rbp,%rbx,4), %r9                          #5.25
        mulpd     32(%r11,%rbx,8), %xmm7                        #5.25
        movsd     (%r8,%r9,8), %xmm8                            #5.25
        movslq    28(%rbp,%rbx,4), %r9                          #5.25
        addpd     %xmm7, %xmm2                                  #5.7
        movhpd    (%r8,%r9,8), %xmm8                            #5.25
        mulpd     48(%r11,%rbx,8), %xmm8                        #5.25
        addq      $8, %rbx                                      #4.5
        addpd     %xmm8, %xmm1                                  #5.7
        cmpq      %r15, %rbx                                    #4.5
        jb        L_B1.14       # Prob 82%                      #4.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r10 r11 r14 r15 r12d r13d xmm0 xmm1 xmm2 xmm3 xmm4
L_B1.15:                        # Preds L_B1.14
                                # Execution count [4.50e+00]
        addpd     %xmm3, %xmm4                                  #5.7
        addpd     %xmm1, %xmm2                                  #5.7
        addpd     %xmm2, %xmm4                                  #5.7
        movaps    %xmm4, %xmm1                                  #5.7
        unpckhpd  %xmm4, %xmm1                                  #5.7
        movq      -8(%rsp), %r9                                 #[spill]
        addsd     %xmm1, %xmm4                                  #5.7
        movsd     %xmm4, (%r9,%rsi,8)                           #5.7
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r14 r12d r13d xmm0
L_B1.16:                        # Preds L_B1.15 L_B1.23
                                # Execution count [5.00e+00]
        xorl      %ebx, %ebx                                    #4.5
        lea       1(%r12), %ebp                                 #4.5
        cmpl      %r13d, %ebp                                   #4.5
        ja        L_B1.20       # Prob 10%                      #4.5
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r14 r12d r13d xmm0
L_B1.17:                        # Preds L_B1.16
                                # Execution count [4.50e+00]
        movslq    %r12d, %r12                                   #5.15
        addq      %r12, %r14                                    #5.15
        movslq    %r13d, %r13                                   #4.5
        movsd     (%r9,%rsi,8), %xmm1                           #5.7
        subq      %r12, %r13                                    #4.5
        lea       (%rdi,%r14,8), %r11                           #5.15
        lea       (%r10,%r14,4), %rbp                           #5.27
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r13 xmm0 xmm1
L_B1.18:                        # Preds L_B1.18 L_B1.17
                                # Execution count [2.50e+01]
        movslq    (%rbp,%rbx,4), %r12                           #5.27
        movsd     (%r11,%rbx,8), %xmm2                          #5.15
        incq      %rbx                                          #4.5
        cmpq      %r13, %rbx                                    #4.5
        mulsd     (%r8,%r12,8), %xmm2                           #5.25
        addsd     %xmm2, %xmm1                                  #5.7
        jb        L_B1.18       # Prob 82%                      #4.5
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r11 r13 xmm0 xmm1
L_B1.19:                        # Preds L_B1.18
                                # Execution count [4.50e+00]
        movsd     %xmm1, (%r9,%rsi,8)                           #5.7
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 xmm0
L_B1.20:                        # Preds L_B1.3 L_B1.16 L_B1.19
                                # Execution count [5.00e+00]
        incq      %rsi                                          #3.3
        incq      %rax                                          #3.3
        cmpq      %rcx, %rsi                                    #3.3
        jb        L_B1.3        # Prob 82%                      #3.3
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 xmm0
L_B1.21:                        # Preds L_B1.20
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
L_B1.22:                        # Preds L_B1.21 L_B1.1
                                # Execution count [1.00e+00]
        ret                                                     #8.1
..LCFI9:
                                # LOE
L_B1.23:                        # Preds L_B1.4 L_B1.6 L_B1.8
                                # Execution count [4.50e-01]: Infreq
        xorl      %r12d, %r12d                                  #4.5
        jmp       L_B1.16       # Prob 100%                     #4.5
        .align    4
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r14 r12d r13d xmm0
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
