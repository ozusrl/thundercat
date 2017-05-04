# mark_description "Intel(R) C Intel(R) 64 Compiler for applications running on Intel(R) 64, Version 17.0.2.163 Build 20170213";
# mark_description "-std=c99 -O3 -S -o icc/duffsdevice_const.s";
	.file "duffsdevice_const.c"
	.section	__TEXT, __text
L_TXTST0:
# -- Begin  _spmv
	.section	__TEXT, __text
# mark_begin;
       .align    4
	.globl _spmv
# --- spmv(double *, int *, int *, int, double *, double *)
_spmv:
# parameter 1: %rdi
# parameter 2: %rsi
# parameter 3: %rdx
# parameter 4: %ecx
# parameter 5: %r8
# parameter 6: %r9
L_B1.1:                         # Preds L_B1.0
                                # Execution count [1.00e+00]
..LCFI1:
L____tag_value__spmv.1:
L_L2:
                                                          #1.76
        xorl      %eax, %eax                                    #3.14
        testl     %ecx, %ecx                                    #3.23
        jle       L_B1.9        # Prob 10%                      #3.23
                                # LOE rax rdx rbx rbp rsi rdi r8 r9 r12 r13 r14 r15 ecx
L_B1.2:                         # Preds L_B1.1
                                # Execution count [9.00e-01]
        movslq    %ecx, %rcx                                    #3.3
        movq      %rbp, -8(%rsp)                                #3.3[spill]
..LCFI2:
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r12 r13 r14 r15
L_B1.3:                         # Preds L_B1.7 L_B1.2
                                # Execution count [1.34e+00]
        movl      (%rdx,%rax,4), %r10d                          #5.13
        movslq    %r10d, %r10                                   #5.13
        xorps     %xmm0, %xmm0                                  #4.16
        movl      4(%rdx,%rax,4), %r11d                         #6.24
        subl      %r10d, %r11d                                  #6.24
        movl      %r11d, %ebp                                   #7.23
        sarl      $1, %ebp                                      #7.23
        shrl      $30, %ebp                                     #7.23
        addl      %r11d, %ebp                                   #7.23
        sarl      $2, %ebp                                      #7.23
        andl      $-2147483645, %r11d                           #8.38
        jge       L_B1.22       # Prob 50%                      #8.38
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp r11d xmm0
L_B1.23:                        # Preds L_B1.3
                                # Execution count [1.34e+00]
        subl      $1, %r11d                                     #8.38
        orl       $-4, %r11d                                    #8.38
        incl      %r11d                                         #8.38
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp r11d xmm0
L_B1.22:                        # Preds L_B1.3 L_B1.23
                                # Execution count [1.34e+00]
        cmpl      $3, %r11d                                     #9.13
        je        L_B1.17       # Prob 20%                      #9.13
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp r11d xmm0
L_B1.4:                         # Preds L_B1.22
                                # Execution count [1.07e+00]
        cmpl      $2, %r11d                                     #9.13
        je        L_B1.13       # Prob 25%                      #9.13
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp r11d xmm0
L_B1.5:                         # Preds L_B1.4
                                # Execution count [8.04e-01]
        cmpl      $1, %r11d                                     #9.13
        je        L_B1.14       # Prob 33%                      #9.13
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp r11d xmm0
L_B1.6:                         # Preds L_B1.5
                                # Execution count [5.36e-01]
        testl     %r11d, %r11d                                  #9.13
        je        L_B1.15       # Prob 50%                      #9.13
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
L_B1.7:                         # Preds L_B1.15 L_B1.6
                                # Execution count [5.36e-01]
        incq      %rax                                          #3.26
        addsd     (%r9), %xmm0                                  #23.5
        movsd     %xmm0, (%r9)                                  #23.5
        addq      $8, %r9                                       #3.26
        cmpq      %rcx, %rax                                    #3.23
        jl        L_B1.3        # Prob 82%                      #3.23
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r12 r13 r14 r15
L_B1.8:                         # Preds L_B1.7
                                # Execution count [9.64e-02]
        movq      -8(%rsp), %rbp                                #[spill]
..LCFI3:
                                # LOE rbx rbp r12 r13 r14 r15
L_B1.9:                         # Preds L_B1.8 L_B1.1
                                # Execution count [1.96e-01]
        ret                                                     #25.1
..LCFI4:
                                # LOE
L_B1.13:                        # Preds L_B1.4 L_B1.17
                                # Execution count [1.49e+00]
        movslq    (%rsi,%r10,4), %r11                           #15.28
        movsd     (%rdi,%r10,8), %xmm1                          #15.16
        incq      %r10                                          #15.38
        mulsd     (%r8,%r11,8), %xmm1                           #15.26
        addsd     %xmm1, %xmm0                                  #15.9
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
L_B1.14:                        # Preds L_B1.5 L_B1.13
                                # Execution count [1.49e+00]
        movslq    (%rsi,%r10,4), %r11                           #17.28
        movsd     (%rdi,%r10,8), %xmm1                          #17.16
        incq      %r10                                          #17.38
        mulsd     (%r8,%r11,8), %xmm1                           #17.26
        addsd     %xmm1, %xmm0                                  #17.9
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
L_B1.15:                        # Preds L_B1.6 L_B1.14
                                # Execution count [1.49e+00]
        decl      %ebp                                          #21.16
        js        L_B1.7        # Prob 18%                      #21.21
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
L_B1.16:                        # Preds L_B1.15
                                # Execution count [1.22e+00]
        movslq    (%rsi,%r10,4), %r11                           #11.28
        movsd     (%rdi,%r10,8), %xmm1                          #11.16
        incq      %r10                                          #11.38
        mulsd     (%r8,%r11,8), %xmm1                           #11.26
        addsd     %xmm1, %xmm0                                  #11.9
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
L_B1.17:                        # Preds L_B1.22 L_B1.16
                                # Execution count [1.49e+00]
        movslq    (%rsi,%r10,4), %r11                           #13.28
        movsd     (%rdi,%r10,8), %xmm1                          #13.16
        incq      %r10                                          #13.38
        mulsd     (%r8,%r11,8), %xmm1                           #13.26
        addsd     %xmm1, %xmm0                                  #13.9
        jmp       L_B1.13       # Prob 100%                     #13.9
        .align    4
                                # LOE rax rdx rcx rbx rsi rdi r8 r9 r10 r12 r13 r14 r15 ebp xmm0
..LCFI5:
# mark_end;
	.section	__DATA, __data
# -- End  _spmv
	.section	__DATA, __data
	.globl _spmv.eh
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
_spmv.eh:
	.long 0x0000002c
	.long 0x0000001c
	.quad ..LCFI1-_spmv.eh-0x8
	.set L_Qlab1,..LCFI5-..LCFI1
	.quad L_Qlab1
	.short 0x0400
	.set L_lab1,..LCFI2-..LCFI1
	.long L_lab1
	.short 0x0286
	.byte 0x04
	.set L_lab2,..LCFI3-..LCFI2
	.long L_lab2
	.short 0x04c6
	.set L_lab3,..LCFI4-..LCFI3
	.long L_lab3
	.long 0x00000286
	.byte 0x00
# End
	.subsections_via_symbols
