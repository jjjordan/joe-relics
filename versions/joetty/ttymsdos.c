/* TTY interface for MSDOS using TURBO-C
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

#include <stdio.h>
#include <conio.h>
#include <bios.h>
#include "types.h"
#include "tty.h"

unsigned long baud=38400;
unsigned long upc;
I have=0;
I leave=0;

V eputs(s)
C *s;
{
fputs(s,stdout);
if(!have) have=bioskey(1);
}

V eputc(c)
C c;
{
putchar(c);
if(!have) have=bioskey(1);
}

V sigjoe()
{
}

V signorm()
{
}

V aopen()
{
fflush(stdout);
upc=DIVIDEND/baud;
}

V aclose()
{
aflush();
}

V aflush()
{
fflush(stdout);
if(!have) have=bioskey(1);
}

C anext()
{
return getch();
}

V getsize(x,y)
I *x, *y;
{
*x=0; *y=0;
#ifdef TIOCGSIZE
struct ttysize getit;
#else
#ifdef TIOCGWINSZ
struct winsize getit;
#endif
#endif
#ifdef TIOCGSIZE
if(ioctl(fileno(stdout),TIOCGSIZE,&getit)!= -1)
 {
 *x=getit.ts_cols;
 *y=getit.ts_lines;
 }
#else
#ifdef TIOCGWINSZ
if(ioctl(fileno(stdout),TIOCGWINSZ,&getit)!= -1)
 {
 *x=getit.ts_cols;
 *y=getit.ts_lines;
 }
#endif
#endif
}

V shell()
{
C *s=getenv("COMSPEC");
if(s) system(s);
}

V susp()
{
#ifdef SIGCONT
gotsig=0;
printf("You have suspended the program.  Type \'fg\' to continue\n");
signal(SIGCONT,dosig);
sigsetmask(sigmask(SIGCONT));
kill(0,SIGTSTP);
while(!gotsig) sigpause(0);
signal(SIGCONT,SIG_DFL);
#else
shell();
#endif
}
