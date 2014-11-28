#include <msp430g2553.h>
#include <isr_compat.h>
#include "threads.h"

#define TIMER_TICK	1000

#define PC_OFF		((THREAD_STACK_SIZE) - 1)
#define SR_OFF		((THREAD_STACK_SIZE) - 2)
#define SP_OFF		((THREAD_SIZE) - 4)

static thread_t *current;
static unsigned int save_r15;

static thread_t thread0;

static void add_thread(thread_t *t, void *fn)
{
	unsigned int ptr;

	t->stack[PC_OFF] = (unsigned int)fn;
	t->stack[SR_OFF] = GIE;
	t->regs[0] = (unsigned int)t + SP_OFF;

	/* insert new thread after thread 0 */
	ptr = thread0.next;
	t->next = ptr;
	thread0.next = (unsigned int)t;
}

void thread_create(thread_t *t, void *fn)
{
	thr_lock();
	add_thread(t, fn);
	thr_unlock();
}

void threads_init(void (*fn)(void))
{
	__disable_interrupt();

	thread0.next = (unsigned int)&thread0;
	current = &thread0;

	/* setup Timer A */
	TACTL = TACLR;
	TACCTL0 = CCIE;
	TACCR0 = TIMER_TICK;
	TACTL = TASSEL_2 | ID_3 | MC_1;

	__enable_interrupt();

	/* Enter thread 0 immediately before first timer tick! */
	(*fn)();
}

ISR(TIMER0_A0, timerA_isr)
{
	__asm__ __volatile__ (

"	mov r15, %1\n"
"	mov %0, r15\n"		/* get current */

"	mov r1, 0(r15)\n"	/* save current stack */
"	mov r4, 2(r15)\n"	/* save rest of cpu regs */
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
"	mov %1, 24(r15)\n"	/* save original r15 */

"	mov 26(r15), r15\n"	/* get next thread */
"	mov r15, %0\n"		/* current = next */

"	mov 0(r15), r1\n"	/* get next stack */
"	mov 2(r15), r4\n"	/* restore next context */
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

	: "+m" (current), "+m" (save_r15)

	);
}
