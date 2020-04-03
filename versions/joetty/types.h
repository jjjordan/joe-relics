/* TYPDEF FILE
   Copyright (C) 1991 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  

You should have received a copy of the GNU General Public License
along with JOE; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Basic Types */

typedef unsigned char C;	/* A character */
typedef int I;			/* Small number/Status return */
typedef unsigned int U;		/* Max difference between two pointers */
typedef unsigned long SZ;	/* Max size of file */
typedef int V;			/* Void */

/* Structures */

typedef struct cap CAP;
typedef struct scrn SCRN;

V *malloc();
V *calloc();
V *realloc();
C *getenv();
C *strcpy();
I strcmp();
C *strcat();
