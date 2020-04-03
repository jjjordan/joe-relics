/* Device independant TTY interface for JOE
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
#include "cap.h"
#include "tty.h"
#include "scrn.h"

/* Table of key sequences which we will translate to single codes */

struct
 {
 I seq;
 I code;
 } seqs[NKEYS]=
{
 { CAPkd, KEYDOWN },
 { CAPku, KEYUP },
 { CAPkl, KEYLEFT },
 { CAPkr, KEYRIGHT },
 { CAPk0, KEYF0 },
 { CAPk1, KEYF1 },
 { CAPk2, KEYF2 },
 { CAPk3, KEYF3 },
 { CAPk4, KEYF4 },
 { CAPk5, KEYF5 },
 { CAPk6, KEYF6 },
 { CAPk7, KEYF7 },
 { CAPk8, KEYF8 },
 { CAPk9, KEYF9 },
 { CAPkD, KEYDEL },
 { CAPkI, KEYINS },
 { CAPkh, KEYHOME },
 { CAPkH, KEYEND },
 { CAPkN, KEYPGDN },
 { CAPkP, KEYPGUP }
};

/* Set attributes */

V attr(t,c)
SCRN *t;
I c;
{
I e=(t->attrib&~c);
if(e&UNDERLINE)
 {
 if(t->ue) texec(t->ue,t->out,t,t->pad,t->div,1), e&=~UNDERLINE;
 t->attrib&=~UNDERLINE;
 }
if(e&INVERSE)
 {
 if(t->se) texec(t->se,t->out,t,t->pad,t->div,1), e&=~INVERSE;
 else if(t->me) texec(t->me,t->out,t,t->pad,t->div,1), e=0, t->attrib=0;
 t->attrib&=~INVERSE;
 }
if(e)
 {
 if(t->me) texec(t->me,t->out,t,t->pad,t->div,1);
 t->attrib=0;
 }
e=(c&~t->attrib);
if(e&INVERSE)
 if(t->mr) texec(t->mr,t->out,t,t->pad,t->div,1);
 else if(t->so) texec(t->so,t->out,t,t->pad,t->div,1);
if(e&UNDERLINE)
 if(t->us) texec(t->us,t->out,t,t->pad,t->div,1);
if(e&BLINK)
 if(t->mb) texec(t->mb,t->out,t,t->pad,t->div,1);
if(e&BOLD)
 if(t->md) texec(t->md,t->out,t,t->pad,t->div,1);
if(e&DIM)
 if(t->mh) texec(t->mh,t->out,t,t->pad,t->div,1);
t->attrib=c;
}

/* Output a character with attributes */

V outatr(t,c,flg)
SCRN *t;
I c,flg;
{
I ch=c&255;
if(t->am && t->x==t->co-1 && t->y==t->li-1) return;
if((c&~255)!=t->attrib) attr(t,c&~255);
if((c&UNDERLINE) && !t->us && t->uc)
 {
 if(!flg)
  {
  if(t->hz && ch=='~') ch='\\';
  t->out(t,ch);
  ++t->x;
  cpos(t,t->x-1,t->y);
  }
 texec(t->uc,t->out,t,t->pad,t->div,0);
 ++t->x;
 }
else
 {
 if(t->hz && ch=='~') ch='\\';
 t->out(t,ch);
 ++t->x;
 }
if(t->x==t->co)
 if(t->am) t->x=0, ++t->y;
 else if(t->xn) t->x= -1, t->y= -1;
 else --t->x;
}

/* Set scrolling region */

V setregn(t,top,bot)
SCRN *t;
I top,bot;
{
if(!t->cs)
 {
 t->top=top;
 t->bot=bot;
 return;
 }
if(t->top!=top || t->bot!=bot)
 {
 t->top=top;
 t->bot=bot;
 texec(t->cs,t->out,t,t->pad,t->div,1,top,bot-1);
 t->x= -1; t->y= -1;
 }
}

V out(t,c)
C *t;
C c;
{
eputc(c);
}

SCRN *topen(flg)
I flg;
{
SCRN *t=(SCRN *)malloc(sizeof(SCRN));
I x,y;
C *p;
if(!(t->cap=getcap(NULL)))
 {
 free(t);
 printf("Couldn't load termcap/terminfo entry\n");
 return 0;
 }

if(!flg) sigjoe();
aopen();

if(t->cap->nums[CAPli]<1) t->li=24; else t->li=t->cap->nums[CAPli];
if(t->cap->nums[CAPco]<2) t->co=80; else t->co=t->cap->nums[CAPco];
getsize(&x,&y);
if(x>3 && y>3) t->li=y, t->co=x;
x=0; y=0;
if(p=getenv("ROWS")) sscanf(p,"%d",&y);
if(p=getenv("COLS")) sscanf(p,"%d",&x);
if(x>3) t->co=x;
if(y>3) t->li=y;

t->pad=t->cap->strs[CAPpc];
t->div=100000/baud;
t->out=out;

t->hz=t->cap->flgs[CAPhz];
t->os=t->cap->flgs[CAPos];
t->eo=t->cap->flgs[CAPeo];
if(t->cap->flgs[CAPhc]) t->os=1;
if(t->os || t->cap->flgs[CAPul]) t->ul=1;

if(t->cap->flgs[CAPxn]) t->xn=1;
else t->xn=0;

if(t->cap->flgs[CAPam]) t->am=1;
else t->am=0;

t->ti=0;
if(t->cap->strs[CAPti]) t->ti=t->cap->strs[CAPti];

t->te=0;
if(t->cap->strs[CAPte]) t->te=t->cap->strs[CAPte];

if(!(t->me=t->cap->strs[CAPme])) goto oops;
if(t->mb=t->cap->strs[CAPmb]) t->avattr|=BLINK;
if(t->md=t->cap->strs[CAPmd]) t->avattr|=BOLD;
if(t->mh=t->cap->strs[CAPmh]) t->avattr|=DIM;
if(t->mr=t->cap->strs[CAPmr]) t->avattr|=INVERSE;
oops:

t->so=0; t->se=0;
if(t->cap->nums[CAPsg]<=0 && !t->mr && t->cap->strs[CAPse])
 {
 if(t->so=t->cap->strs[CAPso]) t->avattr|=INVERSE;
 t->se=t->cap->strs[CAPse];
 }
if(t->cap->flgs[CAPxs] || t->cap->flgs[CAPxt]) t->so=0;

t->us=0; t->ue=0;
if(t->cap->nums[CAPug]<=0 && t->cap->strs[CAPue])
 {
 if(t->us=t->cap->strs[CAPus]) t->avattr|=UNDERLINE;
 t->ue=t->cap->strs[CAPue];
 }

if(!(t->uc=t->cap->strs[CAPuc])) if(t->ul) t->uc="_";
if(t->uc) t->avattr|=UNDERLINE;

t->ms=t->cap->flgs[CAPms];

t->da=t->cap->nums[CAPda];
t->db=t->cap->nums[CAPdb];
t->cs=t->cap->strs[CAPcs];
t->sf=t->cap->strs[CAPsf];
t->sr=t->cap->strs[CAPsr];
t->SF=t->cap->strs[CAPSF];
t->SR=t->cap->strs[CAPSR];
t->al=t->cap->strs[CAPal];
t->dl=t->cap->strs[CAPdl];
t->AL=t->cap->strs[CAPAL];
t->DL=t->cap->strs[CAPDL];
if(!t->cap->nums[CAPns] && !t->sf) t->sf="\12";

t->bs=0;
if(t->cap->strs[CAPbc]) t->bs=t->cap->strs[CAPbc];
else if(t->cap->strs[CAPle]) t->bs=t->cap->strs[CAPle]; 
if(t->cap->flgs[CAPbs]) t->bs="\10";

if(t->bs) t->cbs=tcost(t->bs,t->pad,t->div,1,2,2); else t->cbs=10000;

t->lf="\12";
if(t->cap->strs[CAPdo]) t->lf=t->cap->strs[CAPdo];

if(t->lf) t->clf=tcost(t->lf,t->pad,t->div,1,2,2); else t->clf=10000;

t->up=t->cap->strs[CAPup];

if(t->up) t->cup=tcost(t->up,t->pad,t->div,1,2,2); else t->cup=10000;

t->tw=8;
if(t->cap->nums[CAPit]>0) t->tw=t->cap->nums[CAPit];
else if(t->cap->nums[CAPtw]>0) t->tw=t->cap->nums[CAPtw];

if(t->cap->strs[CAPta]) t->ta=t->cap->strs[CAPta];
else if(t->cap->flgs[CAPpt]) t->ta="\11";
t->bt=t->cap->strs[CAPbt];
if(t->cap->flgs[CAPxt]) t->ta=0, t->bt=0;

if(t->ta) t->cta=tcost(t->ta,t->pad,t->div,1,2,2); else t->cta=10000;
if(t->bt) t->cbt=tcost(t->bt,t->pad,t->div,1,2,2); else t->cbt=10000;

t->ho=t->cap->strs[CAPho];
if(t->ho) t->cho=tcost(t->ho,t->pad,t->div,1,2,2); else t->cho=10000;
t->ll=t->cap->strs[CAPll];
if(t->ll) t->cll=tcost(t->ll,t->pad,t->div,1,2,2); else t->cll=10000;

t->cr="\15";
if(t->cap->strs[CAPcr]) t->cr=t->cap->strs[CAPcr];
if(t->cap->flgs[CAPnc] || t->cap->flgs[CAPxr]) t->cr=0;
if(t->cr) t->ccr=tcost(t->cr,t->pad,t->div,1,2,2); else t->ccr=10000;

if(t->RI=t->cap->strs[CAPRI]) t->cRI=tcost(t->RI,t->pad,t->div,1,2,2);
else t->cRI=10000;
if(t->LE=t->cap->strs[CAPLE]) t->cLE=tcost(t->LE,t->pad,t->div,1,2,2);
else t->cLE=10000;
if(t->UP=t->cap->strs[CAPUP]) t->cUP=tcost(t->UP,t->pad,t->div,1,2,2);
else t->cUP=10000;
if(t->DO=t->cap->strs[CAPDO]) t->cDO=tcost(t->DO,t->pad,t->div,1,2,2);
else t->cDO=10000;
if(t->ch=t->cap->strs[CAPch]) t->cch=tcost(t->ch,t->pad,t->div,1,2,2);
else t->cch=10000;
if(t->cv=t->cap->strs[CAPcv]) t->ccv=tcost(t->cv,t->pad,t->div,1,2,2);
else t->ccv=10000;
if(t->cm=t->cap->strs[CAPcm]) t->ccm=tcost(t->cm,t->pad,t->div,1,2,2);
else t->ccm=10000;

if(t->ce=t->cap->strs[CAPce]) t->cce=tcost(t->ce,t->pad,t->div,1,2,2);
else t->cce=10000;
if(t->cd=t->cap->strs[CAPcd]) t->ccd=tcost(t->cd,t->pad,t->div,1,2,2);
else t->ccd=10000;

x=0;
for(y=0;y!=NKEYS;++y)
 if(t->cap->strs[seqs[y].seq])
  {
  I l;
  C *s=tcompile(t->cap->strs[seqs[y].seq],0,0,&l);
  if(l)
   {
   t->ktab[x].s=(C *)malloc(l);
   memcpy(t->ktab[x].s,s,l);
   t->ktab[x].l=l;
   t->ktab[x].n=seqs[y].code;
   ++x;
   }
  }
t->tabsize=x;
t->kbufp=0;

/* Make sure terminal can do absolute positioning */
if(t->cm) goto ok;
if(t->ch && t->cv) goto ok;
if(t->ho && (t->lf || t->DO || t->cv)) goto ok;
if(t->ll && (t->up || t->UP || t->cv)) goto ok;
if(t->cr && t->cv) goto ok;
leave=1;
aclose();
signorm();
printf("Sorry, your terminal can't do absolute cursor positioning\n");
printf("It\'s broken\n");
return 0;
ok:

t->scrn=(I *)malloc(t->li*t->co*sizeof(I));
t->screen=(I *)malloc(t->li*t->co*sizeof(I));
t->sary=(I *)calloc(t->li,sizeof(I));
memset(t->scrn,-1,t->li*t->co*sizeof(I));
for(x=0;x!=t->li*t->co;++x) t->screen[x]=' ';
for(x=0;x!=NQUICK;++x) t->quick[x].s=(C *)malloc(t->co);
t->quickies=0;
t->x= -1; t->y= -1;
t->top=t->li; t->bot=0;
t->attrib= -1;
t->tattr=0;
t->placex=0;
t->placey=0;
t->cnt=0;
t->upd=1;

if(t->ti) texec(t->ti,t->out,t,t->pad,t->div,1);

return t;
} 

/* Calculate cost of positioning the cursor using only relative cursor
 * positioning functions: t->(lf, DO, up, UP, bs, LE, RI, ta, bt) and rewriting
 * characters (to move right)
 *
 * This doesn't use the am and bw capabilities although it probably could.
 */

I relcost(t,x,y,ox,oy)
register SCRN *t;
register I x,y,ox,oy;
{
I cost=0, c;

/* If we don't know the cursor position, force use of absolute positioning */
if(oy== -1 || ox== -1) return 10000;

/* First adjust row */
if(y>oy)
 /* Have to go down */
 if(t->lf)
  if(t->cDO<(c=(y-oy)*t->clf)) cost+=t->cDO;
  else cost+=c;
 else if(t->DO) cost+=t->cDO;
 else return 10000;
else if(y<oy)
 /* Have to go up */
 if(t->up)
  if(t->cUP<(c=(oy-y)*t->cup)) cost+=t->cUP;
  else cost+=c;
 else if(t->UP) cost+=t->cUP;
 else return 10000;

/* Now adjust column */

/* Use tabs */
if(x>ox && t->ta)
 {
 I ntabs=(x-ox+ox%t->tw)/t->tw;
 I cstunder=x%t->tw+t->cta*ntabs, cstover;
 if(x+t->tw<t->co && t->bs) cstover=t->cbs*(t->tw-x%t->tw)+t->cta*(ntabs+1);
 else cstover=10000;
 if(cstunder<t->cRI && cstunder<x-ox && cstover>cstunder)
  return cost+cstunder;
 else if(cstover<t->cRI && cstover<x-ox) return cost+cstover;
 }
else if(x<ox && t->bt)
 {
 I ntabs=(ox-x+t->tw-ox%t->tw)/t->tw;
 I cstunder,cstover;
 if(t->bs) cstunder=t->cbt*ntabs+t->cbs*(t->tw-x%t->tw); else cstunder=10000;
 if(x-t->tw>=0) cstover=t->cbt*(ntabs+1)+x%t->tw; else cstover=10000;
 if(cstunder<t->cLE && (t->bs?cstunder<(ox-x)*t->cbs:1) && cstover>cstunder)
  return cost+cstunder;
 else if(cstover<t->cRI && (t->bs?cstover<(ox-x)*t->cbs:1)) return cost+cstover;
 }

/* Use simple motions */
if(x<ox)
 /* Have to go left */
 if(t->bs) 
  if(t->cLE<(c=(ox-x)*t->cbs)) cost+=t->cLE;
  else cost+=c;
 else if(t->LE) cost+=t->cLE;
 else return 10000;
else if(x>ox)
 /* Have to go right */
 /* Hmm.. this should take into account possible attribute changes */
 if(t->cRI<x-ox) cost+=t->cRI;
 else cost+=x-ox;

return cost;
}

/* Find optimal set of cursor positioning commands to move from the current
 * cursor row and column (either or both of which might be unknown) to the
 * given new row and column and execute them.
 */

V cpos(t,x,y)
register SCRN *t;
register I x,y;
{
if(y==t->y)
 {
 if(x==t->x) return;
 if(x>t->x && x-t->x<4)
  {
  I *s=t->screen+t->x+t->co*t->y;
  do outatr(t,*s++,1); while(x!=t->x);
  return;
  }
 }
if(!t->ms && t->attrib&(INVERSE|UNDERLINE))
 attr(t,t->attrib&~(INVERSE|UNDERLINE));
if(y<t->top || y>=t->bot) setregn(t,0,t->li);
cposs(t,x,y);
}

V cposs(t,x,y)
register SCRN *t;
register I x,y;
{
register I bestcost,cost;
I bestway;

/* Assume best way is with only using relative cursor positioning */

bestcost=relcost(t,x,y,t->x,t->y); bestway=0;

/* Now check if combinations of absolute cursor positioning functions are
 * better (or necessary in case one or both cursor positions are unknown)
 */

if(t->ccr<bestcost)
 {
 cost=relcost(t,x,y,0,t->y)+t->ccr;
 if(cost<bestcost) bestcost=cost, bestway=1;
 }
if(t->cho<bestcost)
 {
 cost=relcost(t,x,y,0,0)+t->cho;
 if(cost<bestcost) bestcost=cost, bestway=2;
 }
if(t->cll<bestcost)
 {
 cost=relcost(t,x,y,0,t->li-1)+t->cll;
 if(cost<bestcost) bestcost=cost, bestway=3;
 }
if(t->cch<bestcost && x!=t->x)
 {
 cost=relcost(t,x,y,x,t->y)+tcost(t->ch,t->pad,t->div,1,x);
 if(cost<bestcost) bestcost=cost, bestway=4;
 }
if(t->ccv<bestcost && y!=t->y)
 {
 cost=relcost(t,x,y,t->x,y)+tcost(t->cv,t->pad,t->div,1,y);
 if(cost<bestcost) bestcost=cost, bestway=5;
 }
if(t->ccm<bestcost)
 {
 cost=tcost(t->cm,t->pad,t->div,1,y,x);
 if(cost<bestcost) bestcost=cost, bestway=6;
 }
if(t->cch+t->ccv<bestcost && x!=t->x && y!=t->y)
 {
 cost=tcost(t->cv,t->pad,t->div,1,y)+tcost(t->ch,t->pad,t->div,1,x);
 if(cost<bestcost) bestcost=cost, bestway=7;
 }
if(t->ccv+t->ccr<bestcost && y!=t->y)
 {
 cost=tcost(t->cv,t->pad,t->div,1,y)+tcost(t->cr,t->pad,t->div,1)+
      relcost(t,x,y,0,y);
 if(cost<bestcost) bestcost=cost, bestway=8;
 }
if(t->cll+t->cch<bestcost)
 {
 cost=tcost(t->ll,t->pad,t->div,1)+tcost(t->ch,t->pad,t->div,x)+
      relcost(t,x,y,x,t->li-1);
 if(cost<bestcost) bestcost=cost, bestway=9;
 }
if(t->cll+t->ccv<bestcost)
 {
 cost=tcost(t->ll,t->pad,t->div,1)+tcost(t->cv,t->pad,t->div,y)+
      relcost(t,x,y,0,y);
 if(cost<bestcost) bestcost=cost, bestway=10;
 }
if(t->cho+t->cch<bestcost)
 {
 cost=tcost(t->ho,t->pad,t->div,1)+tcost(t->ch,t->pad,t->div,x)+
      relcost(t,x,y,x,t->li-1);
 if(cost<bestcost) bestcost=cost, bestway=11;
 }
if(t->cho+t->ccv<bestcost)
 {
 cost=tcost(t->ho,t->pad,t->div,1)+tcost(t->cv,t->pad,t->div,y)+
      relcost(t,x,y,0,y);
 if(cost<bestcost) bestcost=cost, bestway=12;
 }

/* Do absolute cursor positioning if we don't know the cursor position or
 * if it is faster than doing only relative cursor positioning
 */

switch(bestway)
 {
case 1: texec(t->cr,t->out,t,t->pad,t->div,1); t->x=0; break;
case 2: texec(t->ho,t->out,t,t->pad,t->div,1); t->x=0; t->y=0; break;
case 3: texec(t->ll,t->out,t,t->pad,t->div,1); t->x=0; t->y=t->li-1; break;
case 9: texec(t->ll,t->out,t,t->pad,t->div,1); t->x=0; t->y=t->li-1; goto doch;
case 11: texec(t->ho,t->out,t,t->pad,t->div,1); t->x=0; t->y=0;
  doch:
case 4: texec(t->ch,t->out,t,t->pad,t->div,1,x); t->x=x; break;
case 10: texec(t->ll,t->out,t,t->pad,t->div,1); t->x=0; t->y=t->li-1; goto docv;
case 12: texec(t->ho,t->out,t,t->pad,t->div,1); t->x=0; t->y=0; goto docv;
case 8: texec(t->cr,t->out,t,t->pad,t->div,1); t->x=0;
  docv:
case 5: texec(t->cv,t->out,t,t->pad,t->div,1,y); t->y=y; break;
case 6: texec(t->cm,t->out,t,t->pad,t->div,1,y,x); t->y=y, t->x=x; break;
case 7: texec(t->cv,t->out,t,t->pad,t->div,1,y); t->y=y;
        texec(t->ch,t->out,t,t->pad,t->div,1,x); t->x=x;
        break;
 }

/* Use relative cursor position functions if we're not there yet */

/* First adjust row */
if(y>t->y)
 /* Have to go down */
 if(!t->lf || t->cDO<(y-t->y)*t->clf)
  texec(t->DO,t->out,t,t->pad,t->div,1,y-t->y), t->y=y;
 else while(y>t->y) texec(t->lf,t->out,t,t->pad,t->div,1), ++t->y;
else if(y<t->y)
 /* Have to go up */
 if(!t->up || t->cUP<(t->y-y)*t->cup)
  texec(t->UP,t->out,t,t->pad,t->div,1,t->y-y), t->y=y;
 else while(y<t->y) texec(t->up,t->out,t,t->pad,t->div,1), --t->y;

/* Use tabs */
if(x>t->x && t->ta)
 {
 I ntabs=(x-t->x+t->x%t->tw)/t->tw;
 I cstunder=x%t->tw+t->cta*ntabs, cstover;
 if(x+t->tw<t->co && t->bs) cstover=t->cbs*(t->tw-x%t->tw)+t->cta*(ntabs+1);
 else cstover=10000;
 if(cstunder<t->cRI && cstunder<x-t->x && cstover>cstunder)
  {
  t->x=x-x%t->tw;
  while(ntabs--) texec(t->ta,t->out,t,t->pad,t->div,1);
  }
 else if(cstover<t->cRI && cstover<x-t->x)
  {
  t->x=t->tw+x-x%t->tw;
  ++ntabs;
  while(ntabs--) texec(t->ta,t->out,t,t->pad,t->div,1);
  }
 }
else if(x<t->x && t->bt)
 {
 I ntabs=(t->x-x+t->tw-t->x%t->tw)/t->tw;
 I cstunder,cstover;
 if(t->bs) cstunder=t->cbt*ntabs+t->cbs*(t->tw-x%t->tw); else cstunder=10000;
 if(x-t->tw>=0) cstover=t->cbt*(ntabs+1)+x%t->tw; else cstover=10000;
 if(cstunder<t->cLE && (t->bs?cstunder<(t->x-x)*t->cbs:1) && cstover>cstunder)
  {
  t->x=x+t->tw-x%t->tw;
  while(ntabs--) texec(t->bt,t->out,t,t->pad,t->div,1);
  }
 else if(cstover<t->cRI && (t->bs?cstover<(t->x-x)*t->cbs:1))
  {
  t->x=x-x%t->tw; ++ntabs;
  while(ntabs--) texec(t->bt,t->out,t,t->pad,t->div,1);
  }
 }

/* Now adjust column */
if(x<t->x)
 /* Have to go left */
 if(!t->bs || t->cLE<(t->x-x)*t->cbs)
  texec(t->LE,t->out,t,t->pad,t->div,1,t->x-x), t->x=x;
 else while(x<t->x) texec(t->bs,t->out,t,t->pad,t->div,1), --t->x;
else if(x>t->x)
 /* Have to go right */
 /* Hmm.. this should take into account possible attribute changes */
 if(t->cRI<x-t->x) texec(t->RI,t->out,t,t->pad,t->div,1,x-t->x), t->x=x;
 else while(x>t->x) outatr(t,t->screen[t->x+t->y*t->co],1);
}

/* Determine the best places to use cd and ce */

V clreol(t,cs,s)
SCRN *t;
register I *cs,*s;
{
I best=0,bestx= -1;
register gain= -t->cce, ldist=3, gdist=3, x=t->co;
s+=t->co; cs+=t->co;
if(t->ce)
 do
  {
  --x; --s; --cs;
  if(ldist!=3) ++ldist;
  if(gdist!=3) ++gdist;
  if(*cs!=' ')
   if(*s==' ') gain+=gdist, gdist=0;
   else if(*s== *cs) gain-=ldist, ldist=0;
   else gdist=0, ldist=0;
  else if(*s!=' ') gdist=0, ldist=0;
  if(gain>best) best=gain, bestx=x;
  }
  while(x);
return bestx;
}

V clreos(t)
SCRN *t;
{
I x,y=t->li,z=t->li*t->co,cst=0;
do
 {
 I lcst=0;
 --y;
 x=t->co;
 do
  {
  --x, --z;
  if(t->scrn[z]!=' ') ++lcst;
  if(t->screen[z]!=' ')
   {
   ++x; ++z;
   if(x==t->co) x=0, ++y;
   goto done;
   }
  } while(x);
 cst+=lcst;
 }
 while(y);
done:
if(cst>t->cce) return y;
else return -1;
}

V udline(t,y,cs,s)
register SCRN *t;
I y;
register I *cs,*s;
{
register I x,bestx=clreol(t,cs,s),destx=t->co,z;
for(x=0;x!=destx;++cs,++s,++x)
 {
 if(have) return;
 if(x==bestx)
  {
  cpos(t,x,y);
  attr(t,0);
  texec(t->ce,t->out,t,t->pad,t->div,1);
  for(z=0;z!=t->co-x;++z) cs[z]=' ';
  }
 if(*cs!= *s)
  {
  if(t->os && t->eo && (*cs&255!=' ' || *cs&~*s&~255))
   cpos(t,x,y), outatr(t,' ',0);
  else if(*s&255=='_')
   cpos(t,x,y), outatr(t,' ',0);
  cpos(t,x,y), outatr(t,*cs= *s,0);
  }
 }
}

V tdoquick(t)
SCRN *t;
{
I n,x;
for(n=0;n!=t->quickies;++n)
 {
 I *s=t->screen+t->quick[n].y*t->co+t->quick[n].x,
  *cs=t->scrn+t->quick[n].y*t->co+t->quick[n].x;
 if(t->quick[n].ch)
  {
  *s=t->quick[n].c|t->quick[n].attr;
  if(*s != *cs)
   {
   if(t->os && t->eo && (*cs&255!=' ' || *cs&~*s&~255))
    cpos(t,t->quick[n].x,t->quick[n].y), outatr(t,' ',0);
   else if(*s&255=='_')
    cpos(t,t->quick[n].x,t->quick[n].y), outatr(t,' ',0);
   cpos(t,t->quick[n].x,t->quick[n].y), outatr(t,*cs= *s,0);
   }
  }
 else
  for(x=0;x!=t->quick[n].l;++x,++s,++cs)
   {
   *s=t->quick[n].s[x]|t->quick[n].attr;
   if(*s != *cs)
    {
    if(t->os && t->eo && (*cs&255!=' ' || *cs&~*s&~255))
     cpos(t,t->quick[n].x+x,t->quick[n].y), outatr(t,' ',0);
    else if(*s&255=='_')
     cpos(t,t->quick[n].x+x,t->quick[n].y), outatr(t,' ',0);
    cpos(t,t->quick[n].x+x,t->quick[n].y), outatr(t,*cs= *s,0);
    }
   }
 }
t->quickies=0;
}

V tdumpq(t)
SCRN *t;
{
I n,x;
for(n=0;n!=t->quickies;++n)
 {
 I *s=t->screen+t->quick[n].y*t->co+t->quick[n].x;
 if(t->quick[n].ch)
  *s=t->quick[n].c|t->quick[n].attr;
 else
  for(x=0;x!=t->quick[n].l;++x,++s)
   *s=t->quick[n].s[x]|t->quick[n].attr;
 }
t->quickies=0;
}

V trefresh(t)
SCRN *t;
{
I y,z,q;
I *cs, *s;

if(t->quickies && !t->upd && !t->cnt) tdoquick(t);
if(t->quickies && (t->upd || t->cnt)) tdumpq(t);
if(!t->upd && !t->cnt) goto check;

t->cnt=0;
if(!t->upd && t->updy>=t->placey) goto after;
if(!t->upd) goto before;

/* First, do any necessary scrolling */
for(y=0;y!=t->li;++y)
 {
 q=t->sary[y];
 if(have)
  {
  cpos(t,t->placex,t->placey);
  return;
  }
 if(q && q!=t->li)
  {
  for(z=y;z!=t->li && t->sary[z]==q;++z);
  if(q>0) doscrollup(t,y,z+q,q), y=z-1;
  else doscrolldn(t,y+q,z,-q), y=z-1;
  }
 }
for(y=0;y!=t->li;++y) t->sary[y]=0;

y=t->placey;
after:
cs=t->scrn+t->co*y;
s=t->screen+t->co*y;
for(;y!=t->li;++y, s+=t->co, cs+=t->co)
 if(have)
  {
  t->updy=y;
  t->cnt=1;
  cpos(t,t->placex,t->placey);
  return;
  }
 else udline(t,y,cs,s);

y=0;
before:
cs=t->scrn+t->co*y;
s=t->screen+t->co*y;
for(;y!=t->placey;++y,s+=t->co,cs+=t->co)
 if(have)
  {
  t->updy=y;
  t->cnt=1;
  cpos(t,t->placex,t->placey);
  return;
  }
 else udline(t,y,cs,s);

check:
if(t->x!=t->placex || t->y!=t->placey) cpos(t,t->placex,t->placey);
t->upd=0;
}

V tclose(t,flg)
SCRN *t;
I flg;
{
I x;
leave=1;
setregn(t,0,t->li);
if(t->te) texec(t->te,t->out,t,t->pad,t->div,1);
aclose();
if(!flg) signorm();
rmcap(t->cap);
free(t->scrn);
free(t->screen);
free(t->sary);
for(x=0;x!=NQUICK;++x) free(t->quick[x].s);
for(x=0;x!=t->tabsize;++x) free(t->ktab[x].s);
free(t);
}

V tsignal(sig)
I sig;
{
C buf[128];
if(sig) sprintf(buf,"\rProgram killed by signal %d\r\n",sig);
else sprintf(buf,"\rProgram killed because input closed\r\n");
write(1,buf,strlen(buf));
_exit(1);
}

V tprintf(t,x,y,str,a1,a2,a3,a4,a5,a6,a7)
SCRN *t;
C *str;
I x,y;
{
if(t->quickies==NQUICK) tdumpq(t);
t->quick[t->quickies].ch=0;
t->quick[t->quickies].x=x;
t->quick[t->quickies].y=y;
sprintf(t->quick[t->quickies].s,str,a1,a2,a3,a4,a5,a6,a7);
t->quick[t->quickies].l=strlen(t->quick[t->quickies].s);
++t->quickies;
}

V tputs(t,x,y,s)
SCRN *t;
I x,y;
C *s;
{
if(t->quickies==NQUICK) tdumpq(t);
t->quick[t->quickies].ch=0;
t->quick[t->quickies].x=x;
t->quick[t->quickies].y=y;
t->quick[t->quickies].l=strlen(s);
t->quick[t->quickies].attr=t->tattr;
strcpy(t->quick[t->quickies].s,s);
++t->quickies;
}

I tgetc(t)
SCRN *t;
{
I c,w,h,x;
trefresh(t);
up:
c=anext();
if(t->kbufp==32) t->kbufp=0;
t->kbuf[t->kbufp++]=c;
w=0;
for(h=0;h!=t->tabsize;++h)
 {
 for(x=0;x!=t->kbufp && x!=t->ktab[h].l;++x)
  if(t->ktab[h].s[x]!=t->kbuf[x]) goto nomatch;
 if(x==t->ktab[h].l)
  {
  c=t->ktab[h].n;
  goto found;
  }
 else if(x==t->kbufp) w=1;
 nomatch:;
 }
if(w) goto up;
found:
t->kbufp=0;

/* getsize(&w,&h);
if((w!=t->co || h!=t->li) && w>=3 && h>=3) tchsize(t,w,h); */
return c;
}

V tsetpos(t,x,y)
SCRN *t;
I x,y;
{
t->placex=x;
t->placey=y;
}

V tgetpos(t,x,y)
SCRN *t;
I *x,*y;
{
*x=t->placex;
*y=t->placey;
}

I boxul='+', boxur='+', boxll='+', boxlr='+', boxl='|', boxr='|', boxt='-',
  boxb='-';

V tbox(t,x,y,w,h)
SCRN *t;
I x,y,w,h;
{
I z;
tdumpq(t);
t->screen[x+(y-1)*t->co-1]=(boxul|t->tattr);
t->screen[x+w+(y-1)*t->co]=(boxur|t->tattr);
t->screen[x+(y+h)*t->co-1]=(boxll|t->tattr);
t->screen[x+w+(y+h)*t->co]=(boxlr|t->tattr);
for(z=0;z!=w;++z) t->screen[x+z+(y-1)*t->co]=(boxt|t->tattr);
for(z=0;z!=w;++z) t->screen[x+z+(y+h)*t->co]=(boxb|t->tattr);
for(z=0;z!=h;++z) t->screen[x+(y+z)*t->co-1]=(boxl|t->tattr);
for(z=0;z!=h;++z) t->screen[x+w+(y+z)*t->co]=(boxr|t->tattr);
t->upd=1;
}

V trect(t,x,y,w,h,c)
SCRN *t;
I x,y,w,h,c;
{
I i,j;
tdumpq(t);
c|=t->tattr;
for(j=0;j!=h;++j)
 for(i=0;i!=w;++i)
  t->screen[x+i+(y+j)*t->co]=c;
t->upd=1;
}

V doscrollup(t,top,bot,amnt)
SCRN *t;
I top,bot,amnt;
{
I a=amnt, x;
if(!amnt) return;
if(top==0 && bot==t->li && (t->sf || t->SF))
 {
 setregn(t,0,t->li);
 cpos(t,0,t->li-1);
 if(amnt==1 && t->sf || !t->SF) while(a--) texec(t->sf,t->out,t,t->pad,t->div,1);
 else texec(t->SF,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if(bot==t->li && (t->dl || t->DL))
 {
 setregn(t,0,t->li);
 cpos(t,0,top);
 if(amnt==1 && t->dl || !t->DL) while(a--) texec(t->dl,t->out,t,t->pad,t->div,1);
 else texec(t->DL,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if(t->cs && ( t->sf || t->SF ))
 {
 setregn(t,top,bot);
 cpos(t,0,bot-1);
 if(amnt==1 && t->sf || !t->SF) while(a--) texec(t->sf,t->out,t,t->pad,t->div,1);
 else texec(t->SF,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if((t->dl || t->DL) && (t->al || t->AL))
 {
 cpos(t,0,top);
 if(amnt==1 && t->dl || !t->DL) while(a--) texec(t->dl,t->out,t,t->pad,t->div,1);
 else texec(t->DL,t->out,t,t->pad,t->div,a,a);
 a=amnt;
 cpos(t,0,bot-amnt);
 if(amnt==1 && t->al || !t->AL) while(a--) texec(t->al,t->out,t,t->pad,t->div,1);
 else texec(t->AL,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
return;
done:
for(x=(top+amnt)*t->co;x!=bot*t->co;++x) t->scrn[x-amnt*t->co]=t->scrn[x];
if(bot==t->li && t->db)
 for(x=0;x!=amnt*t->co;++x) t->scrn[x+t->li*t->co-amnt*t->co]= -1;
else for(x=0;x!=amnt*t->co;++x) t->scrn[x+bot*t->co-amnt*t->co]=' ';
}

V doscrolldn(t,top,bot,amnt)
SCRN *t;
I top,bot,amnt;
{
I a=amnt,x;
if(!amnt) return;
if(top==0 && bot==t->li && (t->sr || t->SR))
 {
 setregn(t,0,t->li);
 cpos(t,0,0);
 if(amnt==1 && t->sr || !t->SR)
  while(a--) texec(t->sr,t->out,t,t->pad,t->div,1);
 else texec(t->SR,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if(bot==t->li && (t->al || t->AL))
 {
 setregn(t,0,t->li);
 cpos(t,0,top);
 if(amnt==1 && t->al || !t->AL)
  while(a--) texec(t->al,t->out,t,t->pad,t->div,1);
 else texec(t->AL,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if(t->cs && (t->sr || t->SR))
 {
 setregn(t,top,bot);
 cpos(t,0,top);
 if(amnt==1 && t->sr || !t->SR)
  while(a--) texec(t->sr,t->out,t,t->pad,t->div,1);
 else texec(t->SR,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
if((t->dl || t->DL) && (t->al || t->AL))
 {
 cpos(t,0,bot-amnt);
 if(amnt==1 && t->dl || !t->DL)
  while(a--) texec(t->dl,t->out,t,t->pad,t->div,1);
 else texec(t->DL,t->out,t,t->pad,t->div,a,a);
 a=amnt;
 cpos(t,0,top);
 if(amnt==1 && t->al || !t->AL)
  while(a--) texec(t->al,t->out,t,t->pad,t->div,1);
 else texec(t->AL,t->out,t,t->pad,t->div,a,a);
 goto done;
 }
return;
done:
for(x=bot*t->co;x!=top*t->co+amnt*t->co;--x)
 t->scrn[x-1]=t->scrn[x-t->co*amnt-1];
if(!top && t->da) for(x=0;x!=amnt*t->co;++x) t->scrn[x]= -1;
else for(x=0;x!=amnt*t->co;++x) t->scrn[t->co*top+x]=' ';
}

V tscrolldn(t,top,bot,amnt)
SCRN *t;
I top,bot,amnt;
{
I x;
for(x=bot*t->co;x!=top*t->co+amnt*t->co;--x)
 t->screen[x-1]=t->screen[x-t->co*amnt-1];
for(x=bot;x!=top+amnt;--x)
 t->sary[x-1]=(t->sary[x-amnt-1]==t->li?t->li:t->sary[x-amnt-1]-amnt);
for(x=0;x!=amnt*t->co;++x) t->screen[t->co*top+x]=' ';
for(x=0;x!=amnt;++x) t->sary[top+x]= t->li;
t->upd=1;
}

V tscrollup(t,top,bot,amnt)
SCRN *t;
I top,bot,amnt;
{
I x;
for(x=(top+amnt)*t->co;x!=bot*t->co;++x) t->screen[x-amnt*t->co]=t->screen[x];
for(x=top+amnt;x!=bot;++x)
 t->sary[x-amnt]=(t->sary[x]==t->li?t->li:t->sary[x]+amnt);
for(x=0;x!=amnt*t->co;++x)
 t->screen[x+bot*t->co-amnt*t->co]=' ';
for(x=0;x!=amnt;++x) t->sary[bot-amnt+x]= t->li;
t->upd=1;
}

/*
V tchsize(t,w,h)
SCRN *t;
I w,h;
{
I x,y,*tmp;
if(w==t->co && h==t->li) return;

t->scrn=(I *)realloc(t->scrn,w*h*sizeof(I));
memset(t->scrn,-1,w*h*sizeof(I));

tmp=(I *)malloc(w*h*sizeof(I));
for(x=0;x!=w*h;++x) tmp[x]=' ';
for(y=0;y!=t->li;++y)
 {
 if(y==h) break;
 for(x=0;x!=t->co;++x)
  {
  if(x==w) break;
  tmp[x+y*h]=t->screen[x+y*t->co];
  }
 }
free(t->screen); t->screen=tmp;

if(t->li!=h) t->sary=(I *)realloc(t->sary,h*sizeof(I));
memset(t->sary,0,h*sizeof(I));
t->x= -1;
t->y= -1;
t->top=h;
t->bot=0;
t->co=w; t->li=h;
if(t->placex>=w) t->placex=w-1;
if(t->placey>=h) t->placey=h-1;
t->upd=1;
}
*/

I *tsave(t,x,y,w,h)
SCRN *t;
I x,y,w,h;
{
I *tmp=(I *)malloc(w*h*sizeof(I));
I i,j;
for(j=0;j!=h;++j)
 for(i=0;i!=w;++i)
  tmp[i+j*w]=t->screen[x+i+(j+y)*t->co];
return tmp;
}

V trestore(t,x,y,w,h,s)
SCRN *t;
I x,y,w,h,*s;
{
I i,j;
tdumpq(t);
for(j=0;j!=h;++j)
 for(i=0;i!=w;++i)
  t->screen[x+i+(j+y)*t->co]=s[i+j*w];
t->upd=1;
}

SCRN *tsusp(t)
SCRN *t;
{
I *sv, sva, xpos, ypos;
sv=tsave(t,0,0,t->co,t->li);
sva=tgetattr(t);
tgetpos(t,&xpos,&ypos);
leave=1;
tclose(t,1);
susp();
leave=0;
t=topen(1);
if(t)
 {
 trestore(t,0,0,t->co,t->li,sv);
 tsetattr(t,sva);
 tsetpos(t,xpos,ypos);
 }
free(sv);
return t;
}

V tredraw(t)
SCRN *t;
{
memset(t->scrn,-1,t->li*t->co*sizeof(I));
t->x= -1;
t->y= -1;
t->top=t->li;
t->bot=0;
t->attrib= -1;
t->upd=1;
}
