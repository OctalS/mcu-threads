#include <msp430g2553.h>
#include <isr_compat.h>
#include "thread.h"

extern thread_t threads[THREADS_MAX];
extern thread_t *current;

static unsigned int save_r15;

ISR(TIMER0_A0, timer_isr)
{

/*
	static int x = 0;
	if (x++ % 2)
		P1OUT |= BIT6;
	else
		P1OUT &= ~BIT6;
*/

	__asm__ __volatile__ (
"	mov r15, %1\n"
"	mov %0, r15\n"		// get current

"	mov r1, 0(r15)\n"		// save current stack
"	mov r4, 2(r15)\n"
"	mov r5, 4(r15)\n"
"	mov r6, 6(r15)\n"
"	mov r7, 8(r15)\n"
"	mov r8, 10(r15)\n"
"	mov r9, 12(r15)\n"
"	mov r10, 14(r15)\n"
"	mov r11, 16(r15)\n"
"	mov r12, 18(r15)\n"
"	mov r13, 20(r15)\n"
"	mov r14, 22(r15)\n"
"	mov &save_r15, 24(r15)\n"

"	cmp %2, r15\n"		// check is current the last thread?
"	jeq 1f\n"
"	add %3, r15\n"		// next thread
"	jmp 2f\n"
"1:	mov %4, r15\n"		// thread0
"2:	mov r15, %0\n"		// current = next

"	mov 0(r15), r1\n"	// get stack
"	mov 2(r15), r4\n"
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

	: "+m" (current), "=m" (save_r15)
	: "i" (&threads[THREADS_MAX-1]), "i" (sizeof(thread_t)), "i" (&threads[0])
	:
	);
}
