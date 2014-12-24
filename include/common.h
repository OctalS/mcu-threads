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

#ifndef COMMON_H
#define COMMON_H

/** Minimum stack size in bytes */
#define STACK_MIN	16

/** Minimum thread size in bytes */
#define THREAD_SIZE_MIN	(NR_REGS * sizeof(REG_TYPE) + 2 * sizeof(void *) + STACK_MIN)

#if ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6))
_Static_assert(!(THREAD_SIZE < THREAD_SIZE_MIN), "THREAD_SIZE is too small. Wont compile.");
#endif

typedef REG_TYPE reg_t;

typedef union thread {
	struct {
		reg_t regs[NR_REGS];
		union thread *next;
		union thread *prev;
	};
	unsigned char stack[THREAD_SIZE];
} thread_t;

typedef thread_t *thread_wait_queue;

extern thread_t *current;
extern thread_t *running;

#endif /* COMMON_H */
