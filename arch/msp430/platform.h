/*
 * msp430 interface for mcu-threads library.
 *
 * Copyright (C) 2014 Vladislav Levenetz - octal.s@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <msp430g2553.h>
#include <isr_compat.h>
#include <common.h>

#define TIMER_TICK	1000

static unsigned int save_r14;

ISR(TIMER0_A0, timerA_isr)
{
	__asm__ __volatile__ (

"	cmp #0, %2\n"		/* if timer is disabled do nothing */
"	jeq 1f\n"

"	mov r14, %1\n"
"	mov %0, r14\n"		/* get current */

"	mov r1, 0(r14)\n"	/* save current stack */
"	mov r4, 2(r14)\n"	/* save rest of cpu regs */
"	mov r5, 4(r14)\n"
"	mov r6, 6(r14)\n"
"	mov r7, 8(r14)\n"
"	mov r8, 10(r14)\n"
"	mov r9, 12(r14)\n"
"	mov r10, 14(r14)\n"
"	mov r11, 16(r14)\n"
"	mov r12, 18(r14)\n"
"	mov r13, 20(r14)\n"
"	mov %1, 22(r14)\n"	/* save original r14 */
"	mov r15, 24(r14)\n"

"	mov 26(r14), r15\n"	/* get next thread */
"	mov r15, %0\n"		/* current = next */

"	mov 0(r15), r1\n"	/* get next stack */
"	mov 2(r15), r4\n"	/* restore next's context */
"	mov 4(r15), r5\n"
"	mov 6(r15), r6\n"
"	mov 8(r15), r7\n"
"	mov 10(r15), r8\n"
"	mov 12(r15), r9\n"
"	mov 14(r15), r10\n"
"	mov 16(r15), r11\n"
"	mov 18(r15), r12\n"
"	mov 20(r15), r13\n"
"	mov 22(r15), r14\n"
"	mov 24(r15), r15\n"
"1:\n"

	: "+m" (current), "+m" (save_r14)
	: "m" (TACCTL0)

	);
}

/* This yelds the cpu to another thread.
 * As there is no sw int instruction,
 * we simulate one */
#define arch_schedule()		\
	asm (			\
"	dint\n"			\
"	nop\n"			\
"	push.w #1f\n"		\
"	push.w r2\n"		\
"	bis #8, @r1\n"		\
"	jmp timerA_isr\n"	\
"1:	nop\n"			\
	)

/* Disables timer A
 *
 * The nop is for safety if an interrupt
 * rises while still disalbeing the timer */
#define arch_threads_lock()	\
	do {			\
		TACCTL0 = 0;	\
		nop();		\
	} while (0)

/* Reenables timer A */
#define arch_threads_unlock()	\
	do {			\
		TACCTL0 = CCIE;	\
	} while (0)

#define arch_disable_interrupts() __disable_interrupt();
#define arch_enable_interrupts() __enable_interrupt();

#define PC_OFF		((THREAD_SIZE / 2) - 1)
#define SR_OFF		((THREAD_SIZE / 2) - 2)
#define SP_OFF		(THREAD_SIZE - 4)

static inline void arch_prepare_thread(thread_t *t, void *fn)
{
	unsigned int *p = (unsigned int *)&t->stack;

	p[PC_OFF] = (unsigned int)fn;
	p[SR_OFF] = GIE;
	t->regs[0] = (unsigned int)t + SP_OFF;
}

static inline void arch_init(void)
{
	/* setup Timer A */
	TACTL = TACLR;
	TACCTL0 = CCIE;
	TACCR0 = TIMER_TICK;
	TACTL = TASSEL_2 | ID_0 | MC_1;
}
