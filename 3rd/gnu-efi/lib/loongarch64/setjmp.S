/*
 * Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.
 * Copyright (c) 2021 Loongson Technology Corporation Limited.All rights
 * reserved.
 * Author: zhoumingtao <zhoumingtao@loongson.cn>
 *
 * This program and the accompanying materials are licensed and made
 * available
 * under the terms and conditions of the BSD License which accompanies
 * this
 * distribution.  The full text of the license may be found at
 * http://opensource.org/licenses/bsd-license.php.
 *
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
 * BASIS,
 * WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED.
 */

	.text
	.p2align 3

/*
   int setjmp(jmp_buf env);
*/
	.globl	setjmp
	.type	setjmp, @function
setjmp:
	st.d $ra, $a0, 0x0
	st.d $sp, $a0, 0x8
	st.d $fp, $a0, 0x10
	st.d $s0, $a0, 0x18
	st.d $s1, $a0, 0x20
	st.d $s2, $a0, 0x28
	st.d $s3, $a0, 0x30
	st.d $s4, $a0, 0x38
	st.d $s5, $a0, 0x40
	st.d $s6, $a0, 0x48
	st.d $s7, $a0, 0x50
	st.d $s8, $a0, 0x58

	move $a0, $zero
	jr   $ra

/*
   void longjmp(jmp_buf env, int val);
*/
	.globl	longjmp
	.type	longjmp, @function
longjmp:
	ld.d $ra, $a0, 0x0
	ld.d $sp, $a0, 0x8
	ld.d $fp, $a0, 0x10
	ld.d $s0, $a0, 0x18
	ld.d $s1, $a0, 0x20
	ld.d $s2, $a0, 0x28
	ld.d $s3, $a0, 0x30
	ld.d $s4, $a0, 0x38
	ld.d $s5, $a0, 0x40
	ld.d $s6, $a0, 0x48
	ld.d $s7, $a0, 0x50
	ld.d $s8, $a0, 0x58

	addi.d $a0, $zero, 1	# a0 = 1
	beqz $a1, .L0		# if (a1 == 0); goto L0
	move $a0, $a1		# a0 = a1
.L0:
	jr   $ra
