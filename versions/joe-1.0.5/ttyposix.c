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
#include "config.h"
#include "heap.h"
#include "tty.h"

#ifndef HZ
#define HZ 10
#endif

static struct termios oldterm;

char *obuf=0;
int obufp=0;
int obufsiz;
unsigned long upc;
unsigned baud;
int have=0;
int leave=0;
static int ttymode=0;
static char havec;
FILE *term=0;

static speed_t speeds[]=
{
B50,50,B75,75,B110,110,B134,134,B150,150,B200,200,B300,300,B600,600,B1200,1200,
B1800,1800,B2400,2400,B4800,4800,B9600,9600,EXTA,19200,EXTB,38400,B19200,19200,
B38400,38400
};

void esignal(a,b)
int a;
void (*b)();
{
struct sigaction action;
sigemptyset(&action.sa_mask);
action.sa_flags=0;
action.sa_handler=b;
sigaction(a,&action,NULL);
}

void sigjoe()
{
esignal(SIGHUP,ttsig);
esignal(SIGTERM,ttsig);
esignal(SIGPIPE,SIG_IGN);
esignal(SIGINT,SIG_IGN);
esignal(SIGQUIT,SIG_IGN);
}

void signrm()
{
esignal(SIGHUP,SIG_DFL);
esignal(SIGTERM,SIG_DFL);
esignal(SIGQUIT,SIG_DFL);
esignal(SIGPIPE,SIG_DFL);
esignal(SIGINT,SIG_DFL);
}

void ttopen()
{
sigjoe();
ttopnn();
}

void ttopnn()
{
int x;
speed_t bd;
struct termios newterm;
if(!term && !(term=fopen("/dev/tty","r+")))
 {
 fprintf(stderr,"Couldn\'t open tty\n");
 exit(1);
 }
if(ttymode) return;
else ttymode=1;
fflush(term);
tcdrain(fileno(term));
tcgetattr(fileno(term),&oldterm);
newterm=oldterm;
newterm.c_lflag&=0;
newterm.c_iflag&=~(ICRNL|IGNCR|INLCR);
newterm.c_oflag&=0;
newterm.c_cc[VMIN]=1;
newterm.c_cc[VTIME]=0;
tcsetattr(fileno(term),TCSANOW,&newterm);
upc=0;
bd=cfgetospeed(&newterm);
baud=9600;
for(x=0;x!=34;x+=2)
 if(bd==speeds[x])
  {
  baud=speeds[x+1];
  break;
  }
{
char *bs=getenv("BAUD");
if(bs)
 {
 sscanf(bs,"%u",&baud);
 }
}
upc=DIVIDEND/baud;
if(obuf) free(obuf);
if(!(TIMES*upc)) obufsiz=4096;
else
 {
 obufsiz=1000000/(TIMES*upc);
 if(obufsiz>4096) obufsiz=4096;
 }
if(!obufsiz) obufsiz=1;
obuf=(char *)malloc(obufsiz);
}

void ttclose()
{
ttclsn();
signrm();
}

void ttclsn()
{
int oleave=leave;
if(ttymode) ttymode=0;
else return;
leave=1;
ttflsh();
tcsetattr(fileno(term),TCSANOW,&oldterm);
leave=oleave;
}

static int yep;

static void dosig() { yep=1; } 

int ttflsh()
{
if(obufp)
 {
 struct itimerval a,b;
 unsigned long usec=obufp*upc;
 if(usec>=500000/HZ && baud<38400)
  {
  a.it_value.tv_sec=usec/1000000;
  a.it_value.tv_usec=usec%1000000;
  a.it_interval.tv_usec=0;
  a.it_interval.tv_sec=0;
  esignal(SIGALRM,dosig);
  yep=0;
  sigsetmask(sigmask(SIGALRM));
  setitimer(ITIMER_REAL,&a,&b);
  write(fileno(term),obuf,obufp);
  while(!yep) sigpause(0);
  esignal(SIGALRM,SIG_DFL);
  }
 else write(fileno(term),obuf,obufp);
 obufp=0;
 }
if(!have && !leave)
 {
 fcntl(fileno(term),F_SETFL,O_NDELAY);
 if(read(fileno(term),&havec,1)==1) have=1;
 fcntl(fileno(term),F_SETFL,0);
 }
return 0;
}

int ttgetc()
{
ttflsh();
if(have) have=0;
else if(read(fileno(term),&havec,1)<1) ttsig(0);
return havec;
}

void ttputs(s)
char *s;
{
while(*s)
 {
 obuf[obufp++]= *(s++);
 if(obufp==obufsiz) ttflsh();
 }
}

void ttgtsz(x,y)
int *x, *y;
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
if(ioctl(fileno(term),TIOCGSIZE,&getit)!= -1)
 {
 *x=getit.ts_cols;
 *y=getit.ts_lines;
 }
#else
#ifdef TIOCGWINSZ
if(ioctl(fileno(term),TIOCGWINSZ,&getit)!= -1)
 {
 *x=getit.ws_col;
 *y=getit.ws_row;
 }
#endif
#endif
}

void ttshell(cmd)
char *cmd;
{
int x,omode=ttymode;
char *s=getenv("SHELL");
if(!s) return;
ttclsn();
if(x=fork())
 {
 if(x!= -1) wait(0);
 if(omode) ttopnn();
 }
else
 {
 signrm();
 if(cmd) execl(s,s,"-c",cmd,NULL);
 else
  {
  fprintf(stderr,"You are at the command shell.  Type 'exit' to return\n");
  execl(s,s,NULL);
  }
 _exit(0);
 }
}

static int gotsig;

static void dosi()
{
gotsig=1;
}

void ttsusp()
{
#ifdef SIGCONT
int omode=ttymode;
ttclsn();
gotsig=0;
fprintf(stderr,"You have suspended the program.  Type \'fg\' to return\n");
esignal(SIGCONT,dosi);
sigsetmask(sigmask(SIGCONT));
kill(0,SIGTSTP);
while(!gotsig) sigpause(0);
esignal(SIGCONT,SIG_DFL);
if(omode) ttopnn();
#else
ttshell(NULL);
#endif
}

char *getcwd();
char *pwd()
{
static char buf[1024];
return getcwd(buf,1024);
}
