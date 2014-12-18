/*
 * mcu-threads: threading library for embedded systems.
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
#include <common.h>
#include <threads_api.h>
#include <platform.h>

/* This always points to the currently running thread */
thread_t *current;

/* List of all running threads */
thread_t *running;

static thread_t thread0;

#define thr_lonely(t) ((t)->next == (t))
#define thr_spin() { while(1); }

/* Adds thread 't' after thread 'where' */
static inline void __add_thread(thread_t *where, thread_t *t)
{
	thread_t *next;

	next = where->next;

	where->next = t;
	t->prev = where;
	t->next = next;
	next->prev = t;
}

/* Unlinks thread 't' from the list
 * of running threads.
 * NOTE: t->next, and t->prev are left untouched */
static inline void del_thread(thread_t *t)
{
	thread_t *prev, *next;

	prev = t->prev;
	next = t->next;
	prev->next = next;
	next->prev = prev;

	if (running == t)
		running = next;
}

#define add_first_thread(tr)		\
	do {				\
		(tr)->next = (tr);	\
		(tr)->prev = (tr);	\
		running = (tr);		\
	} while (0)

#define add_thread_after(where, tr)	__add_thread((where), (tr))
#define add_thread_before(where, tr)	__add_thread((where)->prev, (tr))

/* Unlink thread 't' from the list
 * of running threads and put it
 * in the list of sleeping ones */
void thread_sleep(thread_t **wq, thread_t *t)
{
	thread_t *l;

	thr_lock();
	del_thread(t);

	if (!*wq) {
		t->prev = t;
		*wq = t;
		goto sched;
	}

	l = (*wq)->prev;
	(*wq)->prev = t;
	l->next = t;
	t->prev = l;
sched:
	thr_unlock();
	thr_sched();
}

/* Wakes up all threads in the queue 'wq' immediatly.
 * Threads are appended right after current and 
 * and thread switch is performed. */
void thread_wakeup_now(thread_t **wq)
{
	thread_t *rnext, *slast;

	thr_lock();

	if (!*wq)
		goto exit;

	rnext = current->next;
	slast = (*wq)->prev;
	current->next = *wq;
	(*wq)->prev = current;
	slast->next = rnext;
	rnext->prev = slast;

	*wq = (void *)0;

	thr_unlock();
	thr_sched();
	return;
exit:
	thr_unlock();
}

/* Wakes up all threads in the queue 'wq'.
 * Threads are appended at the end of
 * the list of running threads */
void thread_wakeup(thread_t **wq)
{
	thread_t *rlast, *slast;

	thr_lock();

	if (!*wq)
		goto exit;

	rlast = running->prev;
	slast = (*wq)->prev;
	rlast->next = (*wq);
	(*wq)->prev = rlast;
	slast->next = running;
	running->prev = slast;

	*wq = (void *)0;
exit:
	thr_unlock();
}

static inline void add_thread(thread_t *t, void *fn)
{
	arch_prepare_thread(t, fn);
	add_thread_before(running, t);
}

void thread_create(thread_t *t, void *fn)
{
	thr_lock();
	add_thread(t, fn);
	thr_unlock();
}

/* Permanently unlink current thread from
 * the list of running threads. */
void thread_exit(void)
{
	thr_lock();

	if (thr_lonely(current))
		thr_spin();

	del_thread(current);
	thr_unlock();
	thr_sched();
}

void threads_init(void (*fn)(void))
{
	disable_interrupts();

	add_first_thread(&thread0);
	current = &thread0;

	arch_init();

	enable_interrupts();

	/* Enter thread 0 immediately before first timer tick! */
	(*fn)();
}
