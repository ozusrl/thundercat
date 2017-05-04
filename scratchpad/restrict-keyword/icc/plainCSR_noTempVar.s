# mark_description "Intel(R) C Intel(R) 64 Compiler for applications running on Intel(R) 64, Version 17.0.2.163 Build 20170213";
# mark_description "-std=c99 -O3 -S -o icc/plainCSR_noTempVar.s";
	.file "plainCSR_noTempVar.c"
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
        movq      %rsi, %r10                                    #1.81
        movslq    %ecx, %rcx                                    #1.81
        xorl      %esi, %esi                                    #2.3
        movl      $1, %eax                                      #2.3
        testq     %rcx, %rcx                                    #2.23
        jle       L_B1.12       # Prob 9%                       #2.23
                                # LOE rax rdx rcx rbx rbp rsi rdi r8 r9 r10 r12 r13 r14 r15
L_B1.2:                         # Preds L_B1.1
                                # Execution count [9.00e-01]
        movq      %r12, -56(%rsp)                               #[spill]
        movq      %r13, -48(%rsp)                               #[spill]
        movq      %r14, -40(%rsp)                               #[spill]
        movq      %r15, -32(%rsp)                               #[spill]
        movq      %rbx, -24(%rsp)                               #[spill]
        movq      %rbp, -16(%rsp)                               #[spill]
..LCFI2:
                                # LOE rax rdx rcx rsi rdi r8 r9 r10
L_B1.3:                         # Preds L_B1.10 L_B1.2
                                # Execution count [5.00e+00]
        movl      (%rdx,%rsi,4), %r11d                          #3.18
        movl      (%rdx,%rax,4), %r12d                          #3.31
        cmpl      %r12d, %r11d                                  #3.31
        jge       L_B1.10       # Prob 50%                      #3.31
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 r11d r12d
L_B1.4:                         # Preds L_B1.3
                                # Execution count [5.00e+00]
        subl      %r11d, %r12d                                  #3.5
        movl      $1, %ebp                                      #3.5
        movl      %r12d, %r13d                                  #3.5
        xorl      %ebx, %ebx                                    #3.5
        shrl      $31, %r13d                                    #3.5
        addl      %r12d, %r13d                                  #3.5
        sarl      $1, %r13d                                     #3.5
        testl     %r13d, %r13d                                  #3.5
        jbe       L_B1.8        # Prob 10%                      #3.5
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 ebx ebp r11d r12d r13d
L_B1.5:                         # Preds L_B1.4
                                # Execution count [4.50e+00]
        movslq    %r11d, %r11                                   #4.27
        movsd     (%r9,%rsi,8), %xmm0                           #4.7
        movq      %rdx, -8(%rsp)                                #4.15[spill]
        lea       (%r10,%r11,4), %r14                           #4.27
        lea       (%rdi,%r11,8), %rbp                           #4.15
        .align    4
                                # LOE rax rcx rbp rsi rdi r8 r9 r10 r14 ebx r11d r12d r13d xmm0
L_B1.6:                         # Preds L_B1.6 L_B1.5
                                # Execution count [1.25e+01]
        lea       (%rbx,%rbx), %edx                             #4.15
        incl      %ebx                                          #3.5
        movslq    %edx, %rdx                                    #4.27
        cmpl      %r13d, %ebx                                   #3.5
        movslq    (%r14,%rdx,4), %r15                           #4.27
        movsd     (%rbp,%rdx,8), %xmm1                          #4.15
        mulsd     (%r8,%r15,8), %xmm1                           #4.25
        movslq    4(%r14,%rdx,4), %r15                          #4.27
        addsd     %xmm1, %xmm0                                  #4.7
        movsd     %xmm0, (%r9,%rsi,8)                           #4.7
        movsd     8(%rbp,%rdx,8), %xmm2                         #4.15
        mulsd     (%r8,%r15,8), %xmm2                           #4.25
        addsd     %xmm2, %xmm0                                  #4.7
        movsd     %xmm0, (%r9,%rsi,8)                           #4.7
        jb        L_B1.6        # Prob 64%                      #3.5
                                # LOE rax rcx rbp rsi rdi r8 r9 r10 r14 ebx r11d r12d r13d xmm0
L_B1.7:                         # Preds L_B1.6
                                # Execution count [4.50e+00]
        movq      -8(%rsp), %rdx                                #[spill]
        lea       1(%rbx,%rbx), %ebp                            #4.7
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 ebp r11d r12d
L_B1.8:                         # Preds L_B1.7 L_B1.4
                                # Execution count [5.00e+00]
        lea       -1(%rbp), %ebx                                #3.5
        cmpl      %r12d, %ebx                                   #3.5
        jae       L_B1.10       # Prob 10%                      #3.5
                                # LOE rax rdx rcx rsi rdi r8 r9 r10 ebp r11d
L_B1.9:                         # Preds L_B1.8
                                # Execution count [4.50e+00]
        movslq    %r11d, %r11                                   #4.27
        movslq    %ebp, %rbp                                    #4.15
        addq      %rbp, %r11                                    #3.5
        movslq    -4(%r10,%r11,4), %rbx                         #4.27
        movsd     -8(%rdi,%r11,8), %xmm0                        #4.15
        mulsd     (%r8,%rbx,8), %xmm0                           #4.25
        addsd     (%r9,%rsi,8), %xmm0                           #4.7
        movsd     %xmm0, (%r9,%rsi,8)                           #4.7
                                # LOE rax rdx rcx rsi rdi r8 r9 r10
L_B1.10:                        # Preds L_B1.8 L_B1.3 L_B1.9
                                # Execution count [5.00e+00]
        incq      %rsi                                          #2.3
        incq      %rax                                          #2.3
        cmpq      %rcx, %rsi                                    #2.3
        jb        L_B1.3        # Prob 82%                      #2.3
                                # LOE rax rdx rcx rsi rdi r8 r9 r10
L_B1.11:                        # Preds L_B1.10
                                # Execution count [9.00e-01]
        movq      -56(%rsp), %r12                               #[spill]
..LCFI3:
        movq      -48(%rsp), %r13                               #[spill]
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
L_B1.12:                        # Preds L_B1.11 L_B1.1
                                # Execution count [1.00e+00]
        ret                                                     #7.1
        .align    4
                                # LOE
..LCFI9:
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
	.long 0x0000004c
	.long 0x0000001c
	.quad ..LCFI1-_spMV_CSR1.eh-0x8
	.set L_Qlab1,..LCFI9-..LCFI1
	.quad L_Qlab1
	.short 0x0400
	.set L_lab1,..LCFI2-..LCFI1
	.long L_lab1
	.long 0x03860483
	.long 0x078d088c
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
	.short 0x00c6
	.byte 0x00
# End
	.subsections_via_symbols
