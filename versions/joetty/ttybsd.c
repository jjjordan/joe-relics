/* TTY interface for BSD UNIX
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

#include <sgtty.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "types.h"
#include "tty.h"

#ifndef HZ
#define HZ 10			/* Clock ticks/second */
#endif

/* The original tty state */

static struct sgttyb oarg;
static struct tchars otarg;
static struct ltchars oltarg;

/* The output buffer, index and size. */

static C *obuf=0;
static U obufp=0;
static U obufsiz;

/* The baud rate */

unsigned long baud;
unsigned long upc;

/* Code to baud-rate conversion table */

static U speeds[]=
{
B50,50,B75,75,B110,110,B134,134,B150,150,B200,200,B300,300,B600,600,B1200,1200,
B1800,1800,B2400,2400,B4800,4800,B9600,9600,EXTA,19200,EXTB,38400
};

/* Input buffer, typeahead indication flag and editor is about to exit flag */

I have=0;
static C havec;
I leave=0;

V sigjoe()
{
signal(SIGHUP,tsignal);
signal(SIGTERM,tsignal);
signal(SIGINT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
}

V signorm()
{
signal(SIGHUP,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
}

V aopen()
{
I x;
struct sgttyb arg;
struct tchars targ;
struct ltchars ltarg;
fflush(stdout);
ioctl(fileno(stdin),TIOCGETP,&arg);
ioctl(fileno(stdin),TIOCGETC,&targ);
ioctl(fileno(stdin),TIOCGLTC,&ltarg);
oarg=arg; otarg=targ; oltarg=ltarg;
arg.sg_flags=( (arg.sg_flags&~(ECHO|CRMOD) ) | CBREAK) ;
targ.t_intrc= -1;
targ.t_quitc= -1;
targ.t_eofc= -1;
targ.t_brkc= -1;
ltarg.t_suspc= -1;
ltarg.t_dsuspc= -1;
ltarg.t_rprntc= -1;
ltarg.t_flushc= -1;
ltarg.t_werasc= -1;
ltarg.t_lnextc= -1;
ioctl(fileno(stdin),TIOCSETN,&arg);
ioctl(fileno(stdin),TIOCSETC,&targ);
ioctl(fileno(stdin),TIOCSLTC,&ltarg);
baud=9600;
upc=0;
for(x=0;x!=30;x+=2)
 if(arg.sg_ospeed==speeds[x])
  {
  baud=speeds[x+1];
  upc=DIVIDEND/speeds[x+1];
  break;
  }
if(obuf) free(obuf);
if(!(TIMES*upc)) obufsiz=4096;
else
 {
 obufsiz=1000000/(TIMES*upc);
 if(obufsiz>4096) obufsiz=4096;
 }
if(!obufsiz) obufsiz=1;
obuf=(C *)malloc(obufsiz);
}

V aclose()
{
aflush();
ioctl(fileno(stdin),TIOCSETN,&oarg);
ioctl(fileno(stdin),TIOCSETC,&otarg);
ioctl(fileno(stdin),TIOCSLTC,&oltarg);
}

static I yep;
static dosig() { yep=1; } 

V aflush()
{
if(obufp)
 {
 struct itimerval a,b;
 unsigned long usec=obufp*upc;
 if(usec>=500000/HZ)
  {
  a.it_value.tv_sec=usec/1000000;
  a.it_value.tv_usec=usec%1000000;
  a.it_interval.tv_usec=0;
  a.it_interval.tv_sec=0;
  signal(SIGALRM,dosig);
  yep=0;
  sigsetmask(sigmask(SIGALRM));
  setitimer(ITIMER_REAL,&a,&b);
  write(fileno(stdout),obuf,obufp);
  while(!yep) sigpause(0);
  signal(SIGALRM,SIG_DFL);
  }
 else write(fileno(stdout),obuf,obufp);
 obufp=0;
 }
if(!have && !leave)
 {
 fcntl(fileno(stdin),F_SETFL,FNDELAY);
 if(read(fileno(stdin),&havec,1)==1) have=1;
 fcntl(fileno(stdin),F_SETFL,0);
 }
}

C anext()
{
aflush();
if(have) have=0;
else if(read(fileno(stdin),&havec,1)<1) tsignal(0);
return havec;
}

V eputc(c)
C c;
{
obuf[obufp++]=c;
if(obufp==obufsiz) aflush();
}

V eputs(s)
C *s;
{
while(*s)
 {
 obuf[obufp++]= *(s++);
 if(obufp==obufsiz) aflush();
 }
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
