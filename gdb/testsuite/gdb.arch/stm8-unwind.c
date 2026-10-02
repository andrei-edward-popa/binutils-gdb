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

/* Exercise large frames, VLA anchors and near callee cleanup.  */

volatile unsigned marker;

__attribute__((noipa)) unsigned
leaf (unsigned a, unsigned char b, unsigned c)
{
  volatile unsigned char frame[600];
  frame[599] = a + b + c;
  marker = frame[599];
  return marker + 1;
}

__attribute__((noipa)) unsigned long
dynamic (unsigned a, unsigned b)
{
  volatile unsigned char frame[a];
  frame[0] = a;
  marker = frame[0] + b;
  return (unsigned long) marker + 0x12340000UL;
}

__attribute__((noipa)) float
dynamic_float (float a, unsigned b)
{
  volatile unsigned char frame[b];
  frame[0] = b;
  marker = frame[0];
  return a;
}

__attribute__((noipa)) unsigned
caller (unsigned a)
{
  unsigned n = leaf (a, 5, 0x100);
  n += (unsigned) dynamic (300, a);
  n += (unsigned) dynamic_float (1.25f, 300);
  return n;
}

int
main (void)
{
  marker = caller (7);
  return marker != 65;
}
