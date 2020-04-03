/* Cruddy but portable terminal interface
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
#include <signal.h>
#include "types.h"
#include "tty.h"

I have=0;
I leave=0;
unsigned long baud=38400;
unsigned long upc=DIVIDEND/baud;

V eputs(s)
C *s;
{
fputs(s,stdout);
}

V eputc(c)
C c;
{
putchar(c);
}

V sigjoe()
{
signal(SIGHUP,tsignal);
signal(SIGTERM,tsignal);
signal(SIGQUIT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGINT,SIG_IGN);
}

V signorm()
{
signal(SIGHUP,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGINT,SIG_DFL);
}

V aopen()
{
fflush(stdout);
system("/bin/stty raw -echo");
}

V aclose()
{
fflush(stdout);
system("/bin/stty cooked echo");
}

V aflush()
{
fflush(stdout);
}

C anext()
{
C c;
if(read(fileno(stdin),&c,1)<1) tsignal(0);
return c;
}

V getsize(x,y)
I *x, *y;
{
#ifdef TIOCGSIZE
struct ttysize getit;
#else
#ifdef TIOCGWINSZ
struct winsize getit;
#endif
#endif
*x=0; *y=0;
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
 *x=getit.ws_col;
 *y=getit.ws_row;
 }
#endif
#endif
}

V shell()
{
I x;
C *s=getenv("SHELL");
if(!s) return;
printf("You are at the command shell.  Type 'exit' to continue\n");
if(x=fork())
 {
 if(x!= -1) wait(0);
 }
else
 {
 signorm();
 execl(s,s,NULL);
 _exit(0);
 }
}

static I gotsig;

static V dosi()
{
gotsig=1;
}

V susp()
{
#ifdef SIGCONT
gotsig=0;
printf("You have suspended the program.  Type \'fg\' to continue\n");
signal(SIGCONT,dosi);
sigsetmask(sigmask(SIGCONT));
kill(0,SIGTSTP);
while(!gotsig) sigpause(0);
signal(SIGCONT,SIG_DFL);
#else
shell();
#endif
}
