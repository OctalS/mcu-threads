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
 *
 *
 * API interface between threads library and specific arch.
 * If you want to add a new arch you must implement all the
 * arch_() calls referenced here. For more information please refer
 * to doc/arch_api.txt
*/

#ifndef ARCH_API
#define ARCH_API

static inline void thr_disable_interrupts(void)
{
	arch_disable_interrupts();
}

static inline void thr_enable_interrupts(void)
{
	arch_enable_interrupts();
}

static inline void thr_lock(void)
{
	arch_threads_lock();
}

static inline void thr_unlock(void)
{
	arch_threads_unlock();
}

static inline void thr_sched(void)
{
	arch_schedule();
}

static inline void thr_prepare(thread_t *t, void *fn)
{
	arch_prepare_thread(t, fn);
}

static inline void thr_init(void)
{
	arch_init();
}

#endif /* ARCH_API */
