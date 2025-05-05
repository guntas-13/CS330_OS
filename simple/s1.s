	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 14, 4
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3, 0x0                          ; -- Begin function GetTime
lCPI0_0:
	.quad	0x412e848000000000              ; double 1.0E+6
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_GetTime
	.p2align	2
_GetTime:                               ; @GetTime
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	add	x0, sp, #16
	mov	x1, #0
	bl	_gettimeofday
	str	w0, [sp, #12]
	ldr	w8, [sp, #12]
	subs	w8, w8, #0
	cset	w9, ne
                                        ; implicit-def: $x8
	mov	x8, x9
	ands	x8, x8, #0x1
	cset	w8, eq
	tbnz	w8, #0, LBB0_2
	b	LBB0_1
LBB0_1:
	adrp	x0, l___func__.GetTime@PAGE
	add	x0, x0, l___func__.GetTime@PAGEOFF
	adrp	x1, l_.str@PAGE
	add	x1, x1, l_.str@PAGEOFF
	mov	w2, #11
	adrp	x3, l_.str.1@PAGE
	add	x3, x3, l_.str.1@PAGEOFF
	bl	___assert_rtn
LBB0_2:
	b	LBB0_3
LBB0_3:
	ldr	d0, [sp, #16]
	scvtf	d0, d0
	ldr	s2, [sp, #24]
                                        ; implicit-def: $d1
	fmov	s1, s2
	sshll.2d	v1, v1, #0
                                        ; kill: def $d1 killed $d1 killed $q1
	scvtf	d1, d1
	adrp	x8, lCPI0_0@PAGE
	ldr	d2, [x8, lCPI0_0@PAGEOFF]
	fdiv	d1, d1, d2
	fadd	d0, d0, d1
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_Spin                           ; -- Begin function Spin
	.p2align	2
_Spin:                                  ; @Spin
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	w0, [x29, #-4]
	bl	_GetTime
	str	d0, [sp]
	b	LBB1_1
LBB1_1:                                 ; =>This Inner Loop Header: Depth=1
	bl	_GetTime
	ldr	d1, [sp]
	fsub	d0, d0, d1
	ldur	s2, [x29, #-4]
                                        ; implicit-def: $d1
	fmov	s1, s2
	sshll.2d	v1, v1, #0
                                        ; kill: def $d1 killed $d1 killed $q1
	scvtf	d1, d1
	fcmp	d0, d1
	cset	w8, pl
	tbnz	w8, #0, LBB1_3
	b	LBB1_2
LBB1_2:                                 ;   in Loop: Header=BB1_1 Depth=1
	b	LBB1_1
LBB1_3:
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	wzr, [x29, #-4]
	stur	w0, [x29, #-8]
	str	x1, [sp, #16]
	ldur	w8, [x29, #-8]
	subs	w8, w8, #2
	cset	w8, eq
	tbnz	w8, #0, LBB2_2
	b	LBB2_1
LBB2_1:
	adrp	x8, ___stderrp@GOTPAGE
	ldr	x8, [x8, ___stderrp@GOTPAGEOFF]
	ldr	x0, [x8]
	adrp	x1, l_.str.2@PAGE
	add	x1, x1, l_.str.2@PAGEOFF
	bl	_fprintf
	mov	w0, #1
	bl	_exit
LBB2_2:
	ldr	x8, [sp, #16]
	ldr	x8, [x8, #8]
	str	x8, [sp, #8]
	b	LBB2_3
LBB2_3:                                 ; =>This Inner Loop Header: Depth=1
	mov	w0, #1
	bl	_Spin
	ldr	x8, [sp, #8]
	mov	x9, sp
	str	x8, [x9]
	adrp	x0, l_.str.3@PAGE
	add	x0, x0, l_.str.3@PAGEOFF
	bl	_printf
	b	LBB2_3
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l___func__.GetTime:                     ; @__func__.GetTime
	.asciz	"GetTime"

l_.str:                                 ; @.str
	.asciz	"common.h"

l_.str.1:                               ; @.str.1
	.asciz	"rc == 0"

l_.str.2:                               ; @.str.2
	.asciz	"usage: cpu <string>\n"

l_.str.3:                               ; @.str.3
	.asciz	"%s\n"

.subsections_via_symbols
