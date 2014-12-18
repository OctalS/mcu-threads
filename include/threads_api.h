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

#ifndef THREADS_API
#define THREADS_API

#define thr_lock()	arch_threads_lock()
#define thr_unlock()	arch_threads_unlock()
#define thr_sched()	arch_schedule()
#define disable_interrupts() arch_disable_interrupts();
#define enable_interrupts() arch_enable_interrupts();

static void arch_init(void);
static void arch_prepare_thread(thread_t *t, void *fn);

#endif /* THREADS_API */
