/* stm8-opc.c -- Table of opcodes for the STM8 processor.

   Copyright (C) 2007-2026 Free Software Foundation, Inc.

   Written by Åke Rehnman <ake.rehnman@gmail.com>.
   Adapted by Sophie Friedrich.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "opcode/stm8.h"

const struct stm8_opcodes_s stm8_opcodes[] =
{
  /* nop.  */
  { "nop", { ST8_END }, 0x9D },
  /* adc.  */
  { "adc", { ST8_REG_A, ST8_BYTE }, 0xA9 },
  { "adc", { ST8_REG_A, ST8_SHORTMEM }, 0xB9 },
  { "adc", { ST8_REG_A, ST8_LONGMEM }, 0xC9 },
  { "adc", { ST8_REG_A, ST8_INDX }, 0xF9 },
  { "adc", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE9 },
  { "adc", { ST8_REG_A, ST8_LONGOFF_X }, 0xD9 },
  { "adc", { ST8_REG_A, ST8_INDY }, 0x90F9 },
  { "adc", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E9 },
  { "adc", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D9 },
  { "adc", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x19 },
  { "adc", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C9 },
  { "adc", { ST8_REG_A, ST8_LONGPTRW }, 0x72C9 },
  { "adc", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D9 },
  { "adc", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D9 },
  { "adc", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D9 },
  /* add.  */
  { "add", { ST8_REG_A, ST8_BYTE }, 0xAB },
  { "add", { ST8_REG_A, ST8_SHORTMEM }, 0xBB },
  { "add", { ST8_REG_A, ST8_LONGMEM }, 0xCB },
  { "add", { ST8_REG_A, ST8_INDX }, 0xFB },
  { "add", { ST8_REG_A, ST8_SHORTOFF_X }, 0xEB },
  { "add", { ST8_REG_A, ST8_LONGOFF_X }, 0xDB },
  { "add", { ST8_REG_A, ST8_INDY }, 0x90FB },
  { "add", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90EB },
  { "add", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90DB },
  { "add", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x1B },
  { "add", { ST8_REG_A, ST8_SHORTPTRW }, 0x92CB },
  { "add", { ST8_REG_A, ST8_LONGPTRW }, 0x72CB },
  { "add", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92DB },
  { "add", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72DB },
  { "add", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91DB },
  /* addw.  */
  { "addw", { ST8_REG_X, ST8_WORD }, 0x1C },
  { "addw", { ST8_REG_X, ST8_LONGMEM }, 0x72BB },
  { "addw", { ST8_REG_X, ST8_SHORTOFF_SP }, 0x72FB },
  { "addw", { ST8_REG_Y, ST8_WORD }, 0x72A9 },
  { "addw", { ST8_REG_Y, ST8_LONGMEM }, 0x72B9 },
  { "addw", { ST8_REG_Y, ST8_SHORTOFF_SP }, 0x72F9 },
  { "addw", { ST8_REG_SP, ST8_BYTE }, 0x5B },
  /* and.  */
  { "and", { ST8_REG_A, ST8_BYTE }, 0xA4 },
  { "and", { ST8_REG_A, ST8_SHORTMEM }, 0xB4 },
  { "and", { ST8_REG_A, ST8_LONGMEM }, 0xC4 },
  { "and", { ST8_REG_A, ST8_INDX }, 0xF4 },
  { "and", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE4 },
  { "and", { ST8_REG_A, ST8_LONGOFF_X }, 0xD4 },
  { "and", { ST8_REG_A, ST8_INDY }, 0x90F4 },
  { "and", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E4 },
  { "and", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D4 },
  { "and", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x14 },
  { "and", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C4 },
  { "and", { ST8_REG_A, ST8_LONGPTRW }, 0x72C4 },
  { "and", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D4 },
  { "and", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D4 },
  { "and", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D4 },
  /* bccm.  */
  { "bccm", { ST8_LONGMEM, ST8_BIT_0 }, 0x9011 },
  { "bccm", { ST8_LONGMEM, ST8_BIT_1 }, 0x9013 },
  { "bccm", { ST8_LONGMEM, ST8_BIT_2 }, 0x9015 },
  { "bccm", { ST8_LONGMEM, ST8_BIT_3 }, 0x9017 },
  { "bccm", { ST8_LONGMEM, ST8_BIT_4 }, 0x9019 },
  { "bccm", { ST8_LONGMEM, ST8_BIT_5 }, 0x901b },
  { "bccm", { ST8_LONGMEM, ST8_BIT_6 }, 0x901d },
  { "bccm", { ST8_LONGMEM, ST8_BIT_7 }, 0x901f },
  /* bcp.  */
  { "bcp", { ST8_REG_A, ST8_BYTE }, 0xA5 },
  { "bcp", { ST8_REG_A, ST8_SHORTMEM }, 0xB5 },
  { "bcp", { ST8_REG_A, ST8_LONGMEM }, 0xC5 },
  { "bcp", { ST8_REG_A, ST8_INDX }, 0xF5 },
  { "bcp", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE5 },
  { "bcp", { ST8_REG_A, ST8_LONGOFF_X }, 0xD5 },
  { "bcp", { ST8_REG_A, ST8_INDY }, 0x90F5 },
  { "bcp", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E5 },
  { "bcp", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D5 },
  { "bcp", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x15 },
  { "bcp", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C5 },
  { "bcp", { ST8_REG_A, ST8_LONGPTRW }, 0x72C5 },
  { "bcp", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D5 },
  { "bcp", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D5 },
  { "bcp", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D5 },
  /* bcpl.  */
  { "bcpl", { ST8_LONGMEM, ST8_BIT_0 }, 0x9010 },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_1 }, 0x9012 },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_2 }, 0x9014 },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_3 }, 0x9016 },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_4 }, 0x9018 },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_5 }, 0x901a },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_6 }, 0x901c },
  { "bcpl", { ST8_LONGMEM, ST8_BIT_7 }, 0x901e },
  /* break.  */
  { "break", { ST8_END }, 0x8B },
  /* bres.  */
  { "bres", { ST8_LONGMEM, ST8_BIT_0 }, 0x7211 },
  { "bres", { ST8_LONGMEM, ST8_BIT_1 }, 0x7213 },
  { "bres", { ST8_LONGMEM, ST8_BIT_2 }, 0x7215 },
  { "bres", { ST8_LONGMEM, ST8_BIT_3 }, 0x7217 },
  { "bres", { ST8_LONGMEM, ST8_BIT_4 }, 0x7219 },
  { "bres", { ST8_LONGMEM, ST8_BIT_5 }, 0x721b },
  { "bres", { ST8_LONGMEM, ST8_BIT_6 }, 0x721d },
  { "bres", { ST8_LONGMEM, ST8_BIT_7 }, 0x721f },
  /* bset.  */
  { "bset", { ST8_LONGMEM, ST8_BIT_0 }, 0x7210 },
  { "bset", { ST8_LONGMEM, ST8_BIT_1 }, 0x7212 },
  { "bset", { ST8_LONGMEM, ST8_BIT_2 }, 0x7214 },
  { "bset", { ST8_LONGMEM, ST8_BIT_3 }, 0x7216 },
  { "bset", { ST8_LONGMEM, ST8_BIT_4 }, 0x7218 },
  { "bset", { ST8_LONGMEM, ST8_BIT_5 }, 0x721a },
  { "bset", { ST8_LONGMEM, ST8_BIT_6 }, 0x721c },
  { "bset", { ST8_LONGMEM, ST8_BIT_7 }, 0x721e },
  /* btjf.  */
  { "btjf", { ST8_LONGMEM, ST8_BIT_0, ST8_PCREL }, 0x7201 },
  { "btjf", { ST8_LONGMEM, ST8_BIT_1, ST8_PCREL }, 0x7203 },
  { "btjf", { ST8_LONGMEM, ST8_BIT_2, ST8_PCREL }, 0x7205 },
  { "btjf", { ST8_LONGMEM, ST8_BIT_3, ST8_PCREL }, 0x7207 },
  { "btjf", { ST8_LONGMEM, ST8_BIT_4, ST8_PCREL }, 0x7209 },
  { "btjf", { ST8_LONGMEM, ST8_BIT_5, ST8_PCREL }, 0x720b },
  { "btjf", { ST8_LONGMEM, ST8_BIT_6, ST8_PCREL }, 0x720d },
  { "btjf", { ST8_LONGMEM, ST8_BIT_7, ST8_PCREL }, 0x720f },
  /* btjt.  */
  { "btjt", { ST8_LONGMEM, ST8_BIT_0, ST8_PCREL }, 0x7200 },
  { "btjt", { ST8_LONGMEM, ST8_BIT_1, ST8_PCREL }, 0x7202 },
  { "btjt", { ST8_LONGMEM, ST8_BIT_2, ST8_PCREL }, 0x7204 },
  { "btjt", { ST8_LONGMEM, ST8_BIT_3, ST8_PCREL }, 0x7206 },
  { "btjt", { ST8_LONGMEM, ST8_BIT_4, ST8_PCREL }, 0x7208 },
  { "btjt", { ST8_LONGMEM, ST8_BIT_5, ST8_PCREL }, 0x720a },
  { "btjt", { ST8_LONGMEM, ST8_BIT_6, ST8_PCREL }, 0x720c },
  { "btjt", { ST8_LONGMEM, ST8_BIT_7, ST8_PCREL }, 0x720e },
  /* call.  */
  { "call", { ST8_LONGMEM }, 0xCD },
  { "call", { ST8_INDX }, 0xFD },
  { "call", { ST8_SHORTOFF_X }, 0xED },
  { "call", { ST8_LONGOFF_X }, 0xDD },
  { "call", { ST8_INDY }, 0x90FD },
  { "call", { ST8_SHORTOFF_Y }, 0x90ED },
  { "call", { ST8_LONGOFF_Y }, 0x90DD },
  { "call", { ST8_SHORTPTRW }, 0x92CD },
  { "call", { ST8_LONGPTRW }, 0x72CD },
  { "call", { ST8_SHORTPTRW_X }, 0x92DD },
  { "call", { ST8_LONGPTRW_X }, 0x72DD },
  { "call", { ST8_SHORTPTRW_Y }, 0x91DD },
  /* callf.  */
  { "callf", { ST8_EXTMEM }, 0x8D },
  { "callf", { ST8_LONGPTRE }, 0x928D },
  /* callr.  */
  { "callr", { ST8_PCREL }, 0xAD },
  /* ccf.  */
  { "ccf", { ST8_END }, 0x8C },
  /* clr.  */
  { "clr", { ST8_REG_A }, 0x4F },
  { "clr", { ST8_SHORTMEM }, 0x3F },
  { "clr", { ST8_LONGMEM }, 0x725F },
  { "clr", { ST8_INDX }, 0x7F },
  { "clr", { ST8_SHORTOFF_X }, 0x6F },
  { "clr", { ST8_LONGOFF_X }, 0x724F },
  { "clr", { ST8_INDY }, 0x907F },
  { "clr", { ST8_SHORTOFF_Y }, 0x906F },
  { "clr", { ST8_LONGOFF_Y }, 0x904F },
  { "clr", { ST8_SHORTOFF_SP }, 0x0F },
  { "clr", { ST8_SHORTPTRW }, 0x923F },
  { "clr", { ST8_LONGPTRW }, 0x723F },
  { "clr", { ST8_SHORTPTRW_X }, 0x926F },
  { "clr", { ST8_LONGPTRW_X }, 0x726F },
  { "clr", { ST8_SHORTPTRW_Y }, 0x916F },
  /* clrw.  */
  { "clrw", { ST8_REG_X }, 0x5F },
  { "clrw", { ST8_REG_Y }, 0x905F },
  /* cp.  */
  { "cp", { ST8_REG_A, ST8_BYTE }, 0xA1 },
  { "cp", { ST8_REG_A, ST8_SHORTMEM }, 0xB1 },
  { "cp", { ST8_REG_A, ST8_LONGMEM }, 0xC1 },
  { "cp", { ST8_REG_A, ST8_INDX }, 0xF1 },
  { "cp", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE1 },
  { "cp", { ST8_REG_A, ST8_LONGOFF_X }, 0xD1 },
  { "cp", { ST8_REG_A, ST8_INDY }, 0x90F1 },
  { "cp", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E1 },
  { "cp", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D1 },
  { "cp", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x11 },
  { "cp", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C1 },
  { "cp", { ST8_REG_A, ST8_LONGPTRW }, 0x72C1 },
  { "cp", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D1 },
  { "cp", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D1 },
  { "cp", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D1 },
  /* cpw.  */
  { "cpw", { ST8_REG_X, ST8_WORD }, 0xA3 },
  { "cpw", { ST8_REG_X, ST8_SHORTMEM }, 0xB3 },
  { "cpw", { ST8_REG_X, ST8_LONGMEM }, 0xC3 },
  { "cpw", { ST8_REG_X, ST8_INDY }, 0x90F3 },
  { "cpw", { ST8_REG_X, ST8_SHORTOFF_Y }, 0x90E3 },
  { "cpw", { ST8_REG_X, ST8_LONGOFF_Y }, 0x90D3 },
  { "cpw", { ST8_REG_X, ST8_SHORTOFF_SP }, 0x13 },
  { "cpw", { ST8_REG_X, ST8_SHORTPTRW }, 0x92C3 },
  { "cpw", { ST8_REG_X, ST8_LONGPTRW }, 0x72C3 },
  { "cpw", { ST8_REG_X, ST8_SHORTPTRW_Y }, 0x91D3 },
  { "cpw", { ST8_REG_Y, ST8_WORD }, 0x90A3 },
  { "cpw", { ST8_REG_Y, ST8_SHORTMEM }, 0x90B3 },
  { "cpw", { ST8_REG_Y, ST8_LONGMEM }, 0x90C3 },
  { "cpw", { ST8_REG_Y, ST8_INDX }, 0xF3 },
  { "cpw", { ST8_REG_Y, ST8_SHORTOFF_X }, 0xE3 },
  { "cpw", { ST8_REG_Y, ST8_LONGOFF_X }, 0xD3 },
  { "cpw", { ST8_REG_Y, ST8_SHORTPTRW }, 0x91C3 },
  { "cpw", { ST8_REG_Y, ST8_SHORTPTRW_X }, 0x92D3 },
  { "cpw", { ST8_REG_Y, ST8_LONGPTRW_X }, 0x72D3 },
  /* cpl.  */
  { "cpl", { ST8_REG_A }, 0x43 },
  { "cpl", { ST8_SHORTMEM }, 0x33 },
  { "cpl", { ST8_LONGMEM }, 0x7253 },
  { "cpl", { ST8_INDX }, 0x73 },
  { "cpl", { ST8_SHORTOFF_X }, 0x63 },
  { "cpl", { ST8_LONGOFF_X }, 0x7243 },
  { "cpl", { ST8_INDY }, 0x9073 },
  { "cpl", { ST8_SHORTOFF_Y }, 0x9063 },
  { "cpl", { ST8_LONGOFF_Y }, 0x9043 },
  { "cpl", { ST8_SHORTOFF_SP }, 0x03 },
  { "cpl", { ST8_SHORTPTRW }, 0x9233 },
  { "cpl", { ST8_LONGPTRW }, 0x7233 },
  { "cpl", { ST8_SHORTPTRW_X }, 0x9263 },
  { "cpl", { ST8_LONGPTRW_X }, 0x7263 },
  { "cpl", { ST8_SHORTPTRW_Y }, 0x9163 },
  /* cplw.  */
  { "cplw", { ST8_REG_X }, 0x53 },
  { "cplw", { ST8_REG_Y }, 0x9053 },
  /* dec.  */
  { "dec", { ST8_REG_A }, 0x4A },
  { "dec", { ST8_SHORTMEM }, 0x3A },
  { "dec", { ST8_LONGMEM }, 0x725A },
  { "dec", { ST8_INDX }, 0x7A },
  { "dec", { ST8_SHORTOFF_X }, 0x6A },
  { "dec", { ST8_LONGOFF_X }, 0x724A },
  { "dec", { ST8_INDY }, 0x907A },
  { "dec", { ST8_SHORTOFF_Y }, 0x906A },
  { "dec", { ST8_LONGOFF_Y }, 0x904A },
  { "dec", { ST8_SHORTOFF_SP }, 0x0A },
  { "dec", { ST8_SHORTPTRW }, 0x923A },
  { "dec", { ST8_LONGPTRW }, 0x723A },
  { "dec", { ST8_SHORTPTRW_X }, 0x926A },
  { "dec", { ST8_LONGPTRW_X }, 0x726A },
  { "dec", { ST8_SHORTPTRW_Y }, 0x916A },
  /* decw.  */
  { "decw", { ST8_REG_X }, 0x5A },
  { "decw", { ST8_REG_Y }, 0x905A },
  /* div.  */
  { "div", { ST8_REG_X, ST8_REG_A }, 0x62 },
  { "div", { ST8_REG_Y, ST8_REG_A }, 0x9062 },
  /* divw.  */
  { "divw", { ST8_REG_X, ST8_REG_Y }, 0x65 },
  /* exg.  */
  { "exg", { ST8_REG_A, ST8_REG_XL }, 0x41 },
  { "exg", { ST8_REG_A, ST8_REG_YL }, 0x61 },
  { "exg", { ST8_REG_A, ST8_LONGMEM }, 0x31 },
  /* exgw.  */
  { "exgw", { ST8_REG_X, ST8_REG_Y }, 0x51 },
  /* halt.  */
  { "halt", { ST8_END }, 0x8E },
  /* inc.  */
  { "inc", { ST8_REG_A }, 0x4C },
  { "inc", { ST8_SHORTMEM }, 0x3C },
  { "inc", { ST8_LONGMEM }, 0x725C },
  { "inc", { ST8_INDX }, 0x7C },
  { "inc", { ST8_SHORTOFF_X }, 0x6C },
  { "inc", { ST8_LONGOFF_X }, 0x724C },
  { "inc", { ST8_INDY }, 0x907C },
  { "inc", { ST8_SHORTOFF_Y }, 0x906C },
  { "inc", { ST8_LONGOFF_Y }, 0x904C },
  { "inc", { ST8_SHORTOFF_SP }, 0x0C },
  { "inc", { ST8_SHORTPTRW }, 0x923C },
  { "inc", { ST8_LONGPTRW }, 0x723C },
  { "inc", { ST8_SHORTPTRW_X }, 0x926C },
  { "inc", { ST8_LONGPTRW_X }, 0x726C },
  { "inc", { ST8_SHORTPTRW_Y }, 0x916C },
  /* incw.  */
  { "incw", { ST8_REG_X }, 0x5C },
  { "incw", { ST8_REG_Y }, 0x905C },
  /* int.  */
  { "int", { ST8_EXTMEM }, 0x82 },
  /* iret.  */
  { "iret", { ST8_END }, 0x80 },
  /* jp.  */
  { "jp", { ST8_LONGMEM }, 0xCC },
  { "jp", { ST8_INDX }, 0xFC },
  { "jp", { ST8_SHORTOFF_X }, 0xEC },
  { "jp", { ST8_LONGOFF_X }, 0xDC },
  { "jp", { ST8_INDY }, 0x90FC },
  { "jp", { ST8_SHORTOFF_Y }, 0x90EC },
  { "jp", { ST8_LONGOFF_Y }, 0x90DC },
  { "jp", { ST8_SHORTPTRW }, 0x92CC },
  { "jp", { ST8_LONGPTRW }, 0x72CC },
  { "jp", { ST8_SHORTPTRW_X }, 0x92DC },
  { "jp", { ST8_LONGPTRW_X }, 0x72DC },
  { "jp", { ST8_SHORTPTRW_Y }, 0x91DC },
  /* jpf.  */
  { "jpf", { ST8_EXTMEM }, 0xAC },
  { "jpf", { ST8_LONGPTRE }, 0x92AC },
  /* jrxx.  */
  { "jra", { ST8_PCREL }, 0x20 },
  { "jrc", { ST8_PCREL }, 0x25 },
  { "jreq", { ST8_PCREL }, 0x27 },
  { "jrf", { ST8_PCREL }, 0x21 },
  { "jrh", { ST8_PCREL }, 0x9029 },
  { "jrih", { ST8_PCREL }, 0x902F },
  { "jril", { ST8_PCREL }, 0x902E },
  { "jrm", { ST8_PCREL }, 0x902D },
  { "jrmi", { ST8_PCREL }, 0x2B },
  { "jrnc", { ST8_PCREL }, 0x24 },
  { "jrne", { ST8_PCREL }, 0x26 },
  { "jrnh", { ST8_PCREL }, 0x9028 },
  { "jrnm", { ST8_PCREL }, 0x902C },
  { "jrnv", { ST8_PCREL }, 0x28 },
  { "jrpl", { ST8_PCREL }, 0x2A },
  { "jrsge", { ST8_PCREL }, 0x2E },
  { "jrsgt", { ST8_PCREL }, 0x2C },
  { "jrsle", { ST8_PCREL }, 0x2D },
  { "jrslt", { ST8_PCREL }, 0x2F },
  { "jrt", { ST8_PCREL }, 0x20 },
  { "jruge", { ST8_PCREL }, 0x24 },
  { "jrugt", { ST8_PCREL }, 0x22 },
  { "jrule", { ST8_PCREL }, 0x23 },
  { "jrult", { ST8_PCREL }, 0x25 },
  { "jrv", { ST8_PCREL }, 0x29 },
  /* ld.  */
  { "ld", { ST8_REG_A, ST8_BYTE }, 0xA6 },
  { "ld", { ST8_REG_A, ST8_SHORTMEM }, 0xB6 },
  { "ld", { ST8_REG_A, ST8_LONGMEM }, 0xC6 },
  { "ld", { ST8_REG_A, ST8_INDX }, 0xF6 },
  { "ld", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE6 },
  { "ld", { ST8_REG_A, ST8_LONGOFF_X }, 0xD6 },
  { "ld", { ST8_REG_A, ST8_INDY }, 0x90F6 },
  { "ld", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E6 },
  { "ld", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D6 },
  { "ld", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x7B },
  { "ld", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C6 },
  { "ld", { ST8_REG_A, ST8_LONGPTRW }, 0x72C6 },
  { "ld", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D6 },
  { "ld", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D6 },
  { "ld", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D6 },
  { "ld", { ST8_SHORTMEM, ST8_REG_A }, 0xB7 },
  { "ld", { ST8_LONGMEM, ST8_REG_A }, 0xC7 },
  { "ld", { ST8_INDX, ST8_REG_A }, 0xF7 },
  { "ld", { ST8_SHORTOFF_X, ST8_REG_A }, 0xE7 },
  { "ld", { ST8_LONGOFF_X, ST8_REG_A }, 0xD7 },
  { "ld", { ST8_INDY, ST8_REG_A }, 0x90F7 },
  { "ld", { ST8_SHORTOFF_Y, ST8_REG_A }, 0x90E7 },
  { "ld", { ST8_LONGOFF_Y, ST8_REG_A }, 0x90D7 },
  { "ld", { ST8_SHORTOFF_SP, ST8_REG_A }, 0x6B },
  { "ld", { ST8_SHORTPTRW, ST8_REG_A }, 0x92C7 },
  { "ld", { ST8_LONGPTRW, ST8_REG_A }, 0x72C7 },
  { "ld", { ST8_SHORTPTRW_X, ST8_REG_A }, 0x92D7 },
  { "ld", { ST8_LONGPTRW_X, ST8_REG_A }, 0x72D7 },
  { "ld", { ST8_SHORTPTRW_Y, ST8_REG_A }, 0x91D7 },
  { "ld", { ST8_REG_XL, ST8_REG_A }, 0x97 },
  { "ld", { ST8_REG_A, ST8_REG_XL }, 0x9F },
  { "ld", { ST8_REG_YL, ST8_REG_A }, 0x9097 },
  { "ld", { ST8_REG_A, ST8_REG_YL }, 0x909F },
  { "ld", { ST8_REG_XH, ST8_REG_A }, 0x95 },
  { "ld", { ST8_REG_A, ST8_REG_XH }, 0x9E },
  { "ld", { ST8_REG_YH, ST8_REG_A }, 0x9095 },
  { "ld", { ST8_REG_A, ST8_REG_YH }, 0x909E },
  /* ldf.  */
  { "ldf", { ST8_REG_A, ST8_EXTMEM }, 0xBC },
  { "ldf", { ST8_REG_A, ST8_EXTOFF_X }, 0xAF },
  { "ldf", { ST8_REG_A, ST8_EXTOFF_Y }, 0x90AF },
  { "ldf", { ST8_REG_A, ST8_LONGPTRE_X }, 0x92AF },
  { "ldf", { ST8_REG_A, ST8_LONGPTRE_Y }, 0x91AF },
  { "ldf", { ST8_REG_A, ST8_LONGPTRE }, 0x92BC },
  { "ldf", { ST8_EXTMEM, ST8_REG_A }, 0xBD },
  { "ldf", { ST8_EXTOFF_X, ST8_REG_A }, 0xA7 },
  { "ldf", { ST8_EXTOFF_Y, ST8_REG_A }, 0x90A7 },
  { "ldf", { ST8_LONGPTRE_X, ST8_REG_A }, 0x92A7 },
  { "ldf", { ST8_LONGPTRE_Y, ST8_REG_A }, 0x91A7 },
  { "ldf", { ST8_LONGPTRE, ST8_REG_A }, 0x92BD },
  /* ldw.  */
  { "ldw", { ST8_REG_X, ST8_WORD }, 0xAE },
  { "ldw", { ST8_REG_X, ST8_SHORTMEM }, 0xBE },
  { "ldw", { ST8_REG_X, ST8_LONGMEM }, 0xCE },
  { "ldw", { ST8_REG_X, ST8_INDX }, 0xFE },
  { "ldw", { ST8_REG_X, ST8_SHORTOFF_X }, 0xEE },
  { "ldw", { ST8_REG_X, ST8_LONGOFF_X }, 0xDE },
  { "ldw", { ST8_REG_X, ST8_SHORTOFF_SP }, 0x1E },
  { "ldw", { ST8_REG_X, ST8_SHORTPTRW }, 0x92CE },
  { "ldw", { ST8_REG_X, ST8_LONGPTRW }, 0x72CE },
  { "ldw", { ST8_REG_X, ST8_SHORTPTRW_X }, 0x92DE },
  { "ldw", { ST8_REG_X, ST8_LONGPTRW_X }, 0x72DE },
  { "ldw", { ST8_SHORTMEM, ST8_REG_X }, 0xBF },
  { "ldw", { ST8_LONGMEM, ST8_REG_X }, 0xCF },
  { "ldw", { ST8_INDX, ST8_REG_Y }, 0xFF },
  { "ldw", { ST8_SHORTOFF_X, ST8_REG_Y }, 0xEF },
  { "ldw", { ST8_LONGOFF_X, ST8_REG_Y }, 0xDF },
  { "ldw", { ST8_SHORTOFF_SP, ST8_REG_X }, 0x1F },
  { "ldw", { ST8_SHORTPTRW, ST8_REG_X }, 0x92CF },
  { "ldw", { ST8_LONGPTRW, ST8_REG_X }, 0x72CF },
  { "ldw", { ST8_SHORTPTRW_X, ST8_REG_Y }, 0x92DF },
  { "ldw", { ST8_LONGPTRW_X, ST8_REG_Y }, 0x72DF },
  { "ldw", { ST8_REG_Y, ST8_WORD }, 0x90AE },
  { "ldw", { ST8_REG_Y, ST8_SHORTMEM }, 0x90BE },
  { "ldw", { ST8_REG_Y, ST8_LONGMEM }, 0x90CE },
  { "ldw", { ST8_REG_Y, ST8_INDY }, 0x90FE },
  { "ldw", { ST8_REG_Y, ST8_SHORTOFF_Y }, 0x90EE },
  { "ldw", { ST8_REG_Y, ST8_LONGOFF_Y }, 0x90DE },
  { "ldw", { ST8_REG_Y, ST8_SHORTOFF_SP }, 0x16 },
  { "ldw", { ST8_REG_Y, ST8_SHORTPTRW }, 0x91CE },
  { "ldw", { ST8_REG_Y, ST8_SHORTPTRW_Y }, 0x91DE },
  { "ldw", { ST8_SHORTMEM, ST8_REG_Y }, 0x90BF },
  { "ldw", { ST8_LONGMEM, ST8_REG_Y }, 0x90CF },
  { "ldw", { ST8_INDY, ST8_REG_X }, 0x90FF },
  { "ldw", { ST8_SHORTOFF_Y, ST8_REG_X }, 0x90EF },
  { "ldw", { ST8_LONGOFF_Y, ST8_REG_X }, 0x90DF },
  { "ldw", { ST8_SHORTOFF_SP, ST8_REG_Y }, 0x17 },
  { "ldw", { ST8_SHORTPTRW, ST8_REG_Y }, 0x91CF },
  { "ldw", { ST8_SHORTPTRW_Y, ST8_REG_X }, 0x91DF },
  { "ldw", { ST8_REG_Y, ST8_REG_X }, 0x9093 },
  { "ldw", { ST8_REG_X, ST8_REG_Y }, 0x93 },
  { "ldw", { ST8_REG_X, ST8_REG_SP }, 0x96 },
  { "ldw", { ST8_REG_SP, ST8_REG_X }, 0x94 },
  { "ldw", { ST8_REG_Y, ST8_REG_SP }, 0x9096 },
  { "ldw", { ST8_REG_SP, ST8_REG_Y }, 0x9094 },
  /* mov.  */
  { "mov", { ST8_LONGMEM, ST8_BYTE }, 0x35 },
  { "mov", { ST8_SHORTMEM, ST8_SHORTMEM }, 0x45 },
  { "mov", { ST8_LONGMEM, ST8_LONGMEM }, 0x55 },
  /* mul.  */
  { "mul", { ST8_REG_X, ST8_REG_A }, 0x42 },
  { "mul", { ST8_REG_Y, ST8_REG_A }, 0x9042 },
  /* neg.  */
  { "neg", { ST8_REG_A }, 0x40 },
  { "neg", { ST8_SHORTMEM }, 0x30 },
  { "neg", { ST8_LONGMEM }, 0x7250 },
  { "neg", { ST8_INDX }, 0x70 },
  { "neg", { ST8_SHORTOFF_X }, 0x60 },
  { "neg", { ST8_LONGOFF_X }, 0x7240 },
  { "neg", { ST8_INDY }, 0x9070 },
  { "neg", { ST8_SHORTOFF_Y }, 0x9060 },
  { "neg", { ST8_LONGOFF_Y }, 0x9040 },
  { "neg", { ST8_SHORTOFF_SP }, 0x00 },
  { "neg", { ST8_SHORTPTRW }, 0x9230 },
  { "neg", { ST8_LONGPTRW }, 0x7230 },
  { "neg", { ST8_SHORTPTRW_X }, 0x9260 },
  { "neg", { ST8_LONGPTRW_X }, 0x7260 },
  { "neg", { ST8_SHORTPTRW_Y }, 0x9160 },
  /* negw.  */
  { "negw", { ST8_REG_X }, 0x50 },
  { "negw", { ST8_REG_Y }, 0x9050 },
  /* or.  */
  { "or", { ST8_REG_A, ST8_BYTE }, 0xAA },
  { "or", { ST8_REG_A, ST8_SHORTMEM }, 0xBA },
  { "or", { ST8_REG_A, ST8_LONGMEM }, 0xCA },
  { "or", { ST8_REG_A, ST8_INDX }, 0xFA },
  { "or", { ST8_REG_A, ST8_SHORTOFF_X }, 0xEA },
  { "or", { ST8_REG_A, ST8_LONGOFF_X }, 0xDA },
  { "or", { ST8_REG_A, ST8_INDY }, 0x90FA },
  { "or", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90EA },
  { "or", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90DA },
  { "or", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x1A },
  { "or", { ST8_REG_A, ST8_SHORTPTRW }, 0x92CA },
  { "or", { ST8_REG_A, ST8_LONGPTRW }, 0x72CA },
  { "or", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92DA },
  { "or", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72DA },
  { "or", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91DA },
  /* pop.  */
  { "pop", { ST8_REG_A }, 0x84 },
  { "pop", { ST8_REG_CC }, 0x86 },
  { "pop", { ST8_LONGMEM }, 0x32 },
  /* popw.  */
  { "popw", { ST8_REG_X }, 0x85 },
  { "popw", { ST8_REG_Y }, 0x9085 },
  /* push.  */
  { "push", { ST8_REG_A }, 0x88 },
  { "push", { ST8_REG_CC }, 0x8A },
  { "push", { ST8_BYTE }, 0x4B },
  { "push", { ST8_LONGMEM }, 0x3B },
  /* pushw.  */
  { "pushw", { ST8_REG_X }, 0x89 },
  { "pushw", { ST8_REG_Y }, 0x9089 },
  /* rcf.  */
  { "rcf", { ST8_END }, 0x98 },
  /* ret.  */
  { "ret", { ST8_END }, 0x81 },
  /* retf.  */
  { "retf", { ST8_END }, 0x87 },
  /* rim.  */
  { "rim", { ST8_END }, 0x9A },
  /* rlc.  */
  { "rlc", { ST8_REG_A }, 0x49 },
  { "rlc", { ST8_SHORTMEM }, 0x39 },
  { "rlc", { ST8_LONGMEM }, 0x7259 },
  { "rlc", { ST8_INDX }, 0x79 },
  { "rlc", { ST8_SHORTOFF_X }, 0x69 },
  { "rlc", { ST8_LONGOFF_X }, 0x7249 },
  { "rlc", { ST8_INDY }, 0x9079 },
  { "rlc", { ST8_SHORTOFF_Y }, 0x9069 },
  { "rlc", { ST8_LONGOFF_Y }, 0x9049 },
  { "rlc", { ST8_SHORTOFF_SP }, 0x09 },
  { "rlc", { ST8_SHORTPTRW }, 0x9239 },
  { "rlc", { ST8_LONGPTRW }, 0x7239 },
  { "rlc", { ST8_SHORTPTRW_X }, 0x9269 },
  { "rlc", { ST8_LONGPTRW_X }, 0x7269 },
  { "rlc", { ST8_SHORTPTRW_Y }, 0x9169 },
  /* rlcw.  */
  { "rlcw", { ST8_REG_X }, 0x59 },
  { "rlcw", { ST8_REG_Y }, 0x9059 },
  /* rlwa.  */
  { "rlwa", { ST8_REG_X, ST8_REG_A }, 0x02 },
  { "rlwa", { ST8_REG_Y, ST8_REG_A }, 0x9002 },
  /* rrc.  */
  { "rrc", { ST8_REG_A }, 0x46 },
  { "rrc", { ST8_SHORTMEM }, 0x36 },
  { "rrc", { ST8_LONGMEM }, 0x7256 },
  { "rrc", { ST8_INDX }, 0x76 },
  { "rrc", { ST8_SHORTOFF_X }, 0x66 },
  { "rrc", { ST8_LONGOFF_X }, 0x7246 },
  { "rrc", { ST8_INDY }, 0x9076 },
  { "rrc", { ST8_SHORTOFF_Y }, 0x9066 },
  { "rrc", { ST8_LONGOFF_Y }, 0x9046 },
  { "rrc", { ST8_SHORTOFF_SP }, 0x06 },
  { "rrc", { ST8_SHORTPTRW }, 0x9236 },
  { "rrc", { ST8_LONGPTRW }, 0x7236 },
  { "rrc", { ST8_SHORTPTRW_X }, 0x9266 },
  { "rrc", { ST8_LONGPTRW_X }, 0x7266 },
  { "rrc", { ST8_SHORTPTRW_Y }, 0x9166 },
  /* rrcw.  */
  { "rrcw", { ST8_REG_X }, 0x56 },
  { "rrcw", { ST8_REG_Y }, 0x9056 },
  /* rrwa.  */
  { "rrwa", { ST8_REG_X, ST8_REG_A }, 0x01 },
  { "rrwa", { ST8_REG_Y, ST8_REG_A }, 0x9001 },
  /* rvf.  */
  { "rvf", { ST8_END }, 0x9C },
  /* sbc.  */
  { "sbc", { ST8_REG_A, ST8_BYTE }, 0xA2 },
  { "sbc", { ST8_REG_A, ST8_SHORTMEM }, 0xB2 },
  { "sbc", { ST8_REG_A, ST8_LONGMEM }, 0xC2 },
  { "sbc", { ST8_REG_A, ST8_INDX }, 0xF2 },
  { "sbc", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE2 },
  { "sbc", { ST8_REG_A, ST8_LONGOFF_X }, 0xD2 },
  { "sbc", { ST8_REG_A, ST8_INDY }, 0x90F2 },
  { "sbc", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E2 },
  { "sbc", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D2 },
  { "sbc", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x12 },
  { "sbc", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C2 },
  { "sbc", { ST8_REG_A, ST8_LONGPTRW }, 0x72C2 },
  { "sbc", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D2 },
  { "sbc", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D2 },
  { "sbc", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D2 },
  /* scf.  */
  { "scf", { ST8_END }, 0x99 },
  /* sim.  */
  { "sim", { ST8_END }, 0x9B },
  /* sll.  */
  { "sll", { ST8_REG_A }, 0x48 },
  { "sll", { ST8_SHORTMEM }, 0x38 },
  { "sll", { ST8_LONGMEM }, 0x7258 },
  { "sll", { ST8_INDX }, 0x78 },
  { "sll", { ST8_SHORTOFF_X }, 0x68 },
  { "sll", { ST8_LONGOFF_X }, 0x7248 },
  { "sll", { ST8_INDY }, 0x9078 },
  { "sll", { ST8_SHORTOFF_Y }, 0x9068 },
  { "sll", { ST8_LONGOFF_Y }, 0x9048 },
  { "sll", { ST8_SHORTOFF_SP }, 0x08 },
  { "sll", { ST8_SHORTPTRW }, 0x9238 },
  { "sll", { ST8_LONGPTRW }, 0x7238 },
  { "sll", { ST8_SHORTPTRW_X }, 0x9268 },
  { "sll", { ST8_LONGPTRW_X }, 0x7268 },
  { "sll", { ST8_SHORTPTRW_Y }, 0x9168 },
  /* sllw.  */
  { "sllw", { ST8_REG_X }, 0x58 },
  { "sllw", { ST8_REG_Y }, 0x9058 },
  /* sla.  */
  { "sla", { ST8_REG_A }, 0x48 },
  { "sla", { ST8_SHORTMEM }, 0x38 },
  { "sla", { ST8_LONGMEM }, 0x7258 },
  { "sla", { ST8_INDX }, 0x78 },
  { "sla", { ST8_SHORTOFF_X }, 0x68 },
  { "sla", { ST8_LONGOFF_X }, 0x7248 },
  { "sla", { ST8_INDY }, 0x9078 },
  { "sla", { ST8_SHORTOFF_Y }, 0x9068 },
  { "sla", { ST8_LONGOFF_Y }, 0x9048 },
  { "sla", { ST8_SHORTOFF_SP }, 0x08 },
  { "sla", { ST8_SHORTPTRW }, 0x9238 },
  { "sla", { ST8_LONGPTRW }, 0x7238 },
  { "sla", { ST8_SHORTPTRW_X }, 0x9268 },
  { "sla", { ST8_LONGPTRW_X }, 0x7268 },
  { "sla", { ST8_SHORTPTRW_Y }, 0x9168 },
  /* slaw.  */
  { "slaw", { ST8_REG_X }, 0x58 },
  { "slaw", { ST8_REG_Y }, 0x9058 },
  /* sra.  */
  { "sra", { ST8_REG_A }, 0x47 },
  { "sra", { ST8_SHORTMEM }, 0x37 },
  { "sra", { ST8_LONGMEM }, 0x7257 },
  { "sra", { ST8_INDX }, 0x77 },
  { "sra", { ST8_SHORTOFF_X }, 0x67 },
  { "sra", { ST8_LONGOFF_X }, 0x7247 },
  { "sra", { ST8_INDY }, 0x9077 },
  { "sra", { ST8_SHORTOFF_Y }, 0x9067 },
  { "sra", { ST8_LONGOFF_Y }, 0x9047 },
  { "sra", { ST8_SHORTOFF_SP }, 0x07 },
  { "sra", { ST8_SHORTPTRW }, 0x9237 },
  { "sra", { ST8_LONGPTRW }, 0x7237 },
  { "sra", { ST8_SHORTPTRW_X }, 0x9267 },
  { "sra", { ST8_LONGPTRW_X }, 0x7267 },
  { "sra", { ST8_SHORTPTRW_Y }, 0x9167 },
  /* sraw.  */
  { "sraw", { ST8_REG_X }, 0x57 },
  { "sraw", { ST8_REG_Y }, 0x9057 },
  /* srl.  */
  { "srl", { ST8_REG_A }, 0x44 },
  { "srl", { ST8_SHORTMEM }, 0x34 },
  { "srl", { ST8_LONGMEM }, 0x7254 },
  { "srl", { ST8_INDX }, 0x74 },
  { "srl", { ST8_SHORTOFF_X }, 0x64 },
  { "srl", { ST8_LONGOFF_X }, 0x7244 },
  { "srl", { ST8_INDY }, 0x9074 },
  { "srl", { ST8_SHORTOFF_Y }, 0x9064 },
  { "srl", { ST8_LONGOFF_Y }, 0x9044 },
  { "srl", { ST8_SHORTOFF_SP }, 0x04 },
  { "srl", { ST8_SHORTPTRW }, 0x9234 },
  { "srl", { ST8_LONGPTRW }, 0x7234 },
  { "srl", { ST8_SHORTPTRW_X }, 0x9264 },
  { "srl", { ST8_LONGPTRW_X }, 0x7264 },
  { "srl", { ST8_SHORTPTRW_Y }, 0x9164 },
  /* srlw.  */
  { "srlw", { ST8_REG_X }, 0x54 },
  { "srlw", { ST8_REG_Y }, 0x9054 },
  /* sub.  */
  { "sub", { ST8_REG_A, ST8_BYTE }, 0xA0 },
  { "sub", { ST8_REG_A, ST8_SHORTMEM }, 0xB0 },
  { "sub", { ST8_REG_A, ST8_LONGMEM }, 0xC0 },
  { "sub", { ST8_REG_A, ST8_INDX }, 0xF0 },
  { "sub", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE0 },
  { "sub", { ST8_REG_A, ST8_LONGOFF_X }, 0xD0 },
  { "sub", { ST8_REG_A, ST8_INDY }, 0x90F0 },
  { "sub", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E0 },
  { "sub", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D0 },
  { "sub", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x10 },
  { "sub", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C0 },
  { "sub", { ST8_REG_A, ST8_LONGPTRW }, 0x72C0 },
  { "sub", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D0 },
  { "sub", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D0 },
  { "sub", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D0 },
  { "sub", { ST8_REG_SP, ST8_BYTE }, 0x52 },
  /* subw.  */
  { "subw", { ST8_REG_X, ST8_WORD }, 0x1D },
  { "subw", { ST8_REG_X, ST8_LONGMEM }, 0x72B0 },
  { "subw", { ST8_REG_X, ST8_SHORTOFF_SP }, 0x72F0 },
  { "subw", { ST8_REG_Y, ST8_WORD }, 0x72A2 },
  { "subw", { ST8_REG_Y, ST8_LONGMEM }, 0x72B2 },
  { "subw", { ST8_REG_Y, ST8_SHORTOFF_SP }, 0x72F2 },
  /* swap.  */
  { "swap", { ST8_REG_A }, 0x4E },
  { "swap", { ST8_SHORTMEM }, 0x3E },
  { "swap", { ST8_LONGMEM }, 0x725E },
  { "swap", { ST8_INDX }, 0x7E },
  { "swap", { ST8_SHORTOFF_X }, 0x6E },
  { "swap", { ST8_LONGOFF_X }, 0x724E },
  { "swap", { ST8_INDY }, 0x907E },
  { "swap", { ST8_SHORTOFF_Y }, 0x906E },
  { "swap", { ST8_LONGOFF_Y }, 0x904E },
  { "swap", { ST8_SHORTOFF_SP }, 0x0E },
  { "swap", { ST8_SHORTPTRW }, 0x923E },
  { "swap", { ST8_LONGPTRW }, 0x723E },
  { "swap", { ST8_SHORTPTRW_X }, 0x926E },
  { "swap", { ST8_LONGPTRW_X }, 0x726E },
  { "swap", { ST8_SHORTPTRW_Y }, 0x916E },
  /* swapw.  */
  { "swapw", { ST8_REG_X }, 0x5E },
  { "swapw", { ST8_REG_Y }, 0x905E },
  /* tnz.  */
  { "tnz", { ST8_REG_A }, 0x4D },
  { "tnz", { ST8_SHORTMEM }, 0x3D },
  { "tnz", { ST8_LONGMEM }, 0x725D },
  { "tnz", { ST8_INDX }, 0x7D },
  { "tnz", { ST8_SHORTOFF_X }, 0x6D },
  { "tnz", { ST8_LONGOFF_X }, 0x724D },
  { "tnz", { ST8_INDY }, 0x907D },
  { "tnz", { ST8_SHORTOFF_Y }, 0x906D },
  { "tnz", { ST8_LONGOFF_Y }, 0x904D },
  { "tnz", { ST8_SHORTOFF_SP }, 0x0D },
  { "tnz", { ST8_SHORTPTRW }, 0x923D },
  { "tnz", { ST8_LONGPTRW }, 0x723D },
  { "tnz", { ST8_SHORTPTRW_X }, 0x926D },
  { "tnz", { ST8_LONGPTRW_X }, 0x726D },
  { "tnz", { ST8_SHORTPTRW_Y }, 0x916D },
  /* tnzw.  */
  { "tnzw", { ST8_REG_X }, 0x5D },
  { "tnzw", { ST8_REG_Y }, 0x905D },
  /* trap.  */
  { "trap", { ST8_END }, 0x83 },
  /* wfe.  */
  { "wfe", { ST8_END }, 0x728F },
  /* wfi.  */
  { "wfi", { ST8_END }, 0x8F },
  /* xor.  */
  { "xor", { ST8_REG_A, ST8_BYTE }, 0xA8 },
  { "xor", { ST8_REG_A, ST8_SHORTMEM }, 0xB8 },
  { "xor", { ST8_REG_A, ST8_LONGMEM }, 0xC8 },
  { "xor", { ST8_REG_A, ST8_INDX }, 0xF8 },
  { "xor", { ST8_REG_A, ST8_SHORTOFF_X }, 0xE8 },
  { "xor", { ST8_REG_A, ST8_LONGOFF_X }, 0xD8 },
  { "xor", { ST8_REG_A, ST8_INDY }, 0x90F8 },
  { "xor", { ST8_REG_A, ST8_SHORTOFF_Y }, 0x90E8 },
  { "xor", { ST8_REG_A, ST8_LONGOFF_Y }, 0x90D8 },
  { "xor", { ST8_REG_A, ST8_SHORTOFF_SP }, 0x18 },
  { "xor", { ST8_REG_A, ST8_SHORTPTRW }, 0x92C8 },
  { "xor", { ST8_REG_A, ST8_LONGPTRW }, 0x72C8 },
  { "xor", { ST8_REG_A, ST8_SHORTPTRW_X }, 0x92D8 },
  { "xor", { ST8_REG_A, ST8_LONGPTRW_X }, 0x72D8 },
  { "xor", { ST8_REG_A, ST8_SHORTPTRW_Y }, 0x91D8 },
  { NULL, { ST8_END }, 0 },
};

int
stm8_num_opcode_operands (const struct stm8_opcodes_s *opcode)
{
  int i;
  for (i = 0; opcode->constraints[i]; i++)
    ;
  return i;
}

unsigned int
stm8_opcode_size (unsigned int opcode)
{
  if (opcode > 0xffffffU)
    return 4;
  if (opcode > 0xffffU)
    return 3;
  if (opcode > 0xffU)
    return 2;
  return 1;
}

int
stm8_compute_insn_size (const struct stm8_opcodes_s *opcode)
{
  int i, ret = 0;
  for (i = 0; opcode->constraints[i] != ST8_END; i++)
    {
      switch (opcode->constraints[i])
	{
	case ST8_PCREL:
	case ST8_SHORTMEM:
	case ST8_BYTE:
	case ST8_SHORTPTRW_Y:
	case ST8_SHORTPTRW_X:
	case ST8_SHORTPTRW:
	case ST8_SHORTOFF_X:
	case ST8_SHORTOFF_Y:
	case ST8_SHORTOFF_SP:
	  ret++;
	  break;
	case ST8_LONGPTRE_Y:
	case ST8_LONGPTRE_X:
	case ST8_LONGPTRW_Y:
	case ST8_LONGPTRW_X:
	case ST8_LONGPTRE:
	case ST8_LONGPTRW:
	case ST8_LONGOFF_X:
	case ST8_LONGOFF_Y:
	case ST8_LONGMEM:
	case ST8_WORD:
	  ret += 2;
	  break;
	case ST8_EXTMEM:
	case ST8_EXTOFF_X:
	case ST8_EXTOFF_Y:
	  ret += 3;
	  break;
	case ST8_END:
	case ST8_BIT_0:
	case ST8_BIT_1:
	case ST8_BIT_2:
	case ST8_BIT_3:
	case ST8_BIT_4:
	case ST8_BIT_5:
	case ST8_BIT_6:
	case ST8_BIT_7:
	case ST8_REG_CC:
	case ST8_REG_A:
	case ST8_REG_X:
	case ST8_REG_Y:
	case ST8_REG_SP:
	case ST8_REG_XL:
	case ST8_REG_XH:
	case ST8_REG_YL:
	case ST8_REG_YH:
	case ST8_INDX:
	case ST8_INDY:
	  break;
	}
    }
  ret += stm8_opcode_size (opcode->bin_opcode);
  return ret;
}
