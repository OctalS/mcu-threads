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
#include <platform.h>
#include <arch_api.h>

/* This always points to the currently running thread */
thread_t *current;

/* List of all running threads */
thread_t *running;

/* Initial thread */
static thread_t thread0;

/**
 * Checks if this thread is last on the list
 *
 * @tr: thread to check
*/
#define thr_lonely(tr) ((tr)->next == (tr))

/**
 * Adds the very first thread
 *
 * @tr: pointer to static thread
*/
#define add_first_thread(tr)		\
	do {				\
		(tr)->next = (tr);	\
		(tr)->prev = (tr);	\
		running = (tr);		\
	} while (0)

/**
 * Inserts a new thread after another thread
 *
 * @where: pointer to a thread from where to start
 * @tr: pointer to the new thread. It will be
 *	added after @where
*/
#define add_thread_after(where, tr)	__add_thread((where), (tr))

/**
 * Inserts a new thread before another thread
 *
 * @where: pointer to a thread from where to start
 * @tr: pointer to the new thread. It will be
 *	added before @where
*/
#define add_thread_before(where, tr)	__add_thread((where)->prev, (tr))

/**
 * The same as add_thread_after()
*/
static inline void __add_thread(thread_t *where, thread_t *t)
{
	thread_t *next;

	next = where->next;
	where->next = t;
	t->prev = where;
	t->next = next;
	next->prev = t;
}

/**
 * Unlinks a thread from the list of running threads.
 *
 * @t: thread to delete
 *
 * @notice: t->next, and t->prev are left untouched 
 *	and still points to valid threads
*/
static inline void del_thread(thread_t *t)
{
	thread_t *prev, *next;

	prev = t->prev;
	next = t->next;
	prev->next = next;
	next->prev = prev;

	/* fix the list head pointer */
	if (running == t)
		running = next;
}

/**
 * Puts a thread to sleep.
 *
 * @wq: wait queue to sleep on
 * @t: thread to put to sleep
 *
 * This is for internal use.
 * Must be called with threads locked.
*/
static inline void __thread_sleep(thread_wait_queue *wq, thread_t *t)
{
	thread_t *l;

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

/**
 * Puts a thread to sleep.
 *
 * @wq: wait queue to sleep on
 * @t: thread to put to sleep
 *
 * Move the thread from the list of running threads
 * to a defined wait queue. The CPU is given to the
 * next thread after this.
 *
 * @notice: Undefined behaviour if @wq is not initialized.
*/
void thread_sleep(thread_wait_queue *wq, thread_t *t)
{
	thr_lock();
	__thread_sleep(wq, t);
}

/**
 * Wakes up all threads in the wait queue.
 *
 * @wq: wait queue holding sleeping threads.
 *
 * This is for internal use.
 * Must be called with threads locked.
*/
static inline void __thread_wakeup(thread_wait_queue *wq)
{
	thread_t *rlast, *slast;

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

/**
 * Wakes up all threads in the wait queue.
 *
 * @wq: wait queue holding sleeping threads.
 *
 * Appends all threads from the wait queue at the end of
 * the list of running threads.
 * No context switch will occur after this call.
*/
void thread_wakeup(thread_wait_queue *wq)
{
	thr_lock();
	__thread_wakeup(wq);
}

/**
 * Wakes up all threads in the wait queue.
 *
 * @wq: wait queue holding sleeping threads.
 *
 * This is for internal use.
 * Must be called with threads locked.
*/
static inline void __thread_wakeup_now(thread_wait_queue *wq)
{
	thread_t *rnext, *slast;

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

/**
 * Wakes up all threads in the wait queue.
 *
 * @wq: wait queue holding sleeping threads.
 *
 * Appends all threads from the wait queue right after
 * current and gives the CPU to the first thread immediately.
 * No context switch will occur if the wait queue is empty.
*/
void thread_wakeup_now(thread_wait_queue *wq)
{
	thr_lock();
	__thread_wakeup_now(wq);
}

/**
 * Locks a mutex.
 *
 * @mutex: the mutex to lock.
 *
 * Blocks the current process until the mutex
 * is released. Context switch will occur if the
 * mutex is still held by another thread.
 *
 * @notice: Undefined behaviour if the mutex is not initialized.
*/
void thread_mutex_lock(struct thread_mutex *mutex)
{
	while (1) {
		thr_lock();
		if (mutex->count) {
			__thread_sleep(&mutex->wq, current);
		} else {
			mutex->count++;
			break;
		}
	}

	thr_unlock();
}

/**
 * Try to lock a mutex.
 *
 * @mutex: the mutex to lock.
 *
 * This function will lock the mutex if it is not held
 * by another thread. Otherwise it will return a non-negative
 * value indicating that the mutex is already locked.
 * This is the non-blocking version of thread_mutex_lock()
 *
 * @notice: Undefined behaviour if the mutex is not initialized.
 * @return: Non-zero value if the mutex is already locked.
*/
int thread_mutex_trylock(struct thread_mutex *mutex)
{
	int ret = 0;

	thr_lock();

	if (mutex->count)
		ret = 1;
	else
		mutex->count++;

	thr_unlock();

	return ret;
}

/**
 * Unlocks a mutex
 *
 * @mutex: the mutex to lock.
 *
 * Releases the mutex and wakes up any other threads
 * that are waiting on it.
 * Context switch may occur.
 *
 * @notice: Undefined behaviour if the mutex is not initialized.
*/
void thread_mutex_unlock(struct thread_mutex *mutex)
{
	thr_lock();
	mutex->count--;

	/* Use wakeup_now() here so we can give other
	 * threads a chance to get the mutex.
	 * This can be explained in the following example:
	 * thread1:
	 *	while (1) {
	 *		thread_mutex_lock();
	 *		...
	 *		thread_mutex_unlock();
	 *	}
	 * If using the normal wakeup and the code in the critical
	 * section is short, other threads may never have a chance
	 * to get the mutex as the above code will acquire the lock
	 * almost immediately after releasing it.
	*/
	__thread_wakeup_now(&mutex->wq);
}

/**
 * Setups and adds a newly created thread.
 *
 * @t: new thread to add
 * @fn: thread function to execute
 *
 * The new thread is added at the end of the
 * list of running threads.
 * No context switch will occur after this call.
 *
 * @notice: this is called with threads locked.
 * @notice: arch should setup the new thread's stack
*/
static inline void add_thread(thread_t *t, void *fn)
{
	thr_prepare(t, fn);
	add_thread_before(running, t);
}

/**
 * Creates a new thread.
 *
 * @t: new thread to create
 * @fn: thread function to execute
*/
void thread_create(thread_t *t, void *fn)
{
	thr_lock();
	add_thread(t, fn);
	thr_unlock();
}

/**
 * Terminates a thread.
 *
 * Permanently unlinks the current thread from
 * the list of running threads.
 * CPU is given to the next thread in the list.
*/
void thread_exit(void)
{
	thr_lock();

	if (thr_lonely(current))
		while (1); // the only thing we can do is spin

	del_thread(current);
	thr_unlock();
	thr_sched();
}

/**
 * Initializes the thread library.
 *
 * @fn: pointer to thread main function.
 *
 * The caller must supply a new 'main' function
 * which will be executed in threading context.
 * This function is the thread function of the
 * static thread 0.
*/
void threads_init(void (*fn)(void))
{
	thr_disable_interrupts();

	add_first_thread(&thread0);
	current = &thread0;

	thr_init();

	thr_enable_interrupts();

	/* Enter thread 0 immediately before first timer tick! */
	(*fn)();
}
