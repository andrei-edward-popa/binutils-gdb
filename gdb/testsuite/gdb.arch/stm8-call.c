/* Copyright (C) 2026 Free Software Foundation, Inc.
   Copyright (c) 2026 Andrei-Edward Popa

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>

struct small
{
  unsigned char a, b;
};

struct big
{
  unsigned long a;
  unsigned char b;
};

struct small sample = { 3, 5 };
volatile unsigned marker;

/* An interrupt handler requires a hardware interrupt frame, not CALL's
   return address.  Inferior calls must reject it before changing SP.  */
extern void interrupt_fn (void);
__asm__ (".text\n"
	 ".global _interrupt_fn\n"
	 ".type _interrupt_fn,@function\n"
	 "_interrupt_fn:\n"
	 "\tiret\n"
	 ".size _interrupt_fn,.-_interrupt_fn\n");

unsigned char
byte_fn (unsigned char a, unsigned b, unsigned char c)
{
  return a + b + c;
}

unsigned
word_fn (unsigned a, unsigned char b, unsigned c)
{
  return a + b + c;
}

unsigned long
long_fn (unsigned long a, unsigned b)
{
  return a + b;
}

unsigned long long
wide_fn (unsigned a, unsigned long long b)
{
  return b + a;
}

float
float_fn (float a, float b)
{
  return a + b;
}

struct small
small_fn (unsigned a, unsigned char b)
{
  struct small s = { a, b };
  return s;
}

struct big
big_fn (unsigned a, unsigned long b)
{
  struct big s = { b, a };
  return s;
}

unsigned
struct_arg (struct small s, unsigned b, unsigned c)
{
  return s.a + s.b + b + c;
}

unsigned
variadic_fn (unsigned n, ...)
{
  va_list ap;
  unsigned r = 0;
  va_start (ap, n);
  while (n--)
    r += va_arg (ap, unsigned);
  va_end (ap);
  return r;
}

unsigned
dynamic_fn (unsigned a)
{
  volatile unsigned char buf[a];
  buf[0] = a;
  marker = buf[0];
  return marker + 1;
}

/* Retain callable functions when the target links with --gc-sections.  */
typedef void (*function_ptr) (void);
function_ptr volatile keep_functions[] = {
  (function_ptr) byte_fn, (function_ptr) word_fn,
  (function_ptr) long_fn, (function_ptr) wide_fn,
  (function_ptr) float_fn, (function_ptr) small_fn,
  (function_ptr) big_fn, (function_ptr) struct_arg,
  (function_ptr) variadic_fn, (function_ptr) dynamic_fn, interrupt_fn
};

int
main (void)
{
  marker = sample.a + sample.b + (keep_functions[0] != 0);
  return 0;
}
