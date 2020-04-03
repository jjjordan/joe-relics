/* TTY interface for POSIX
   Copyright (C) 1991 Joseph H. Allen
   (Contributed by Mike Lijewski)

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  

You should have received a copy of the GNU General Public License along with
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 675
Mass Ave, Cambridge, MA 02139, USA.  */ 

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <termios.h>
#include <unistd.h>
#include "types.h"
#include "tty.h"

#ifndef HZ
#define HZ 10
#endif

static struct termios oldterm;

static C *obuf=0;
static U obufp=0;
static U obufsiz;
unsigned long upc;
unsigned long baud;
I have=0;
I leave=0;
static C havec;

static speed_t speeds[]=
{
B50,50,B75,75,B110,110,B134,134,B150,150,B200,200,B300,300,B600,600,B1200,1200,
B1800,1800,B2400,2400,B4800,4800,B9600,9600,EXTA,19200,EXTB,38400,B19200,19200,
B38400,38400
};

V esignal(a,b)
I a;
V (*b)();
{
struct sigaction action;
sigemptyset(&actions.sa_mask);
action.sa_handler=b;
sigaction(a,&action,NULL);
}

V sigjoe()
{
esignal(SIGHUP,tsignal);
esignal(SIGTERM,tsignal);
esignal(SIGPIPE,SIG_IGN);
esignal(SIGINT,SIG_IGN);
esignal(SIGQUIT,SIG_IGN);
}

V signorm()
{
esignal(SIGHUP,SIG_DFL);
esignal(SIGTERM,SIG_DFL);
esignal(SIGQUIT,SIG_DFL);
esignal(SIGPIPE,SIG_DFL);
esignal(SIGINT,SIG_DFL);
}

V aopen()
{
I x;
speed_t bd;
struct termios newterm;
fflush(stdout);
tcdrain(STDOUT_FILENO);
tcgetattr(STDIN_FILENO,&oldterm);
newterm=oldterm;
newterm.c_lflag&=0;
newterm.c_iflag&=~(ICRNL|IGNCR|INLCR);
newterm.c_oflag&=0;
newterm.c_cc[VMIN]=1;
newterm.c_cc[VTIME]=0;
tcsetattr(STDIN_FILENO,TCSANOW,&newterm);
upc=0;
bd=cfgetospeed(&newterm);
baud=9600;
for(x=0;x!=34;x+=2)
 if(bd==speeds[x])
  {
  upc=DIVIDEND/speeds[x+1];
  baud=speeds[x+1];
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
tcsetattr(STDIN_FILENO,TCSANOW,&oldterm);
}

static I yep;

static V dosig() { yep=1; } 

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
  action.sa_handler=dosig;
  esignal(SIGALRM,dosig);
  yep=0;
  sigsetmask(sigmask(SIGALRM));
  setitimer(ITIMER_REAL,&a,&b);
  write(fileno(stdout),obuf,obufp);
  while(!yep) sigpause(0);
  esignal(SIGALRM,SIG_DFL);
  }
 else write(fileno(stdout),obuf,obufp);
 obufp=0;
 }
if(!have && !leave)
 {
 fcntl(STDIN_FILENO,F_SETFL,O_NDELAY);
 if(read(STDIN_FILENO,&havec,1)==1) have=1;
 fcntl(STDIN_FILENO,F_SETFL,0);
 }
}

C anext()
{
aflush();
if(have) have=0;
else if(read(STDIN_FILENO,&havec,1)<1) tsignal(0);
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
