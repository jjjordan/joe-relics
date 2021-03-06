/* Single-key query windows
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */ 

#ifndef _Iquery
#define _Iquery 1

#include "config.h"

typedef struct query QW;
struct query
 {
 void (*func)();		/* Func. which gets called when key is hit */
 char *prompt;			/* Prompt string */
 int promptlen;			/* Width of prompt string */
 int promptofst;		/* Prompt scroll offset */
 };

#define TYPEQW 0x1000

extern CONTEXT cquery, cquerya, cquerysr;

/* W *mkqw(W *w,char *prompt,void (*func)());
 * Create a query window for the given window
 */
W *mkqw();
W *mkqwna();
W *mkqwnsr();

void uabortqw();
void utypeqw();

#endif
