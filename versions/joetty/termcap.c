/* TERMCAP database interface
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
#include "types.h"
#include "cap.h"

static C flgs[]="\
EPHDLCMTNLOPUCambsbwcadadbeognhchshzinkmmimsmtncnsosptulxbxnxoxrxsxtxvxx\
";

static C nums[]="\
codBdCdFdNdTdVitknlilmpbsgsstwugvtws\
";

static C strs[]="\
ALCCCMDCDLDOICK1K2K3K4K5LERISFSRUPaealasbcblbtcdcechclcmcrcsctcvdcdldmdodseced\
eiesfffsgegogthdhohui1i2i3iPicifimipisk0k1k2k3k4k5k6k7k8k9kAkCkDkEkFkHkIkLkMkN\
kPkRkSkTkakbkdkekhklkokrksktkul0l1l2l3l4l5l6l7l8l9lellmambmdmemhmkmlmmmompmrmu\
ndnlnwpOpcpfpkplpopspxr1r2r3rcrfrirprssascsesfsosrsttatitetitsucueupusvbvevivs\
wi\
";

CAP *getcap(name)
C *name;
{
C *tp, *pp;
I x,y,c,z;
FILE *fd;
C namebuf[512], *npbuf[32];
CAP *cap=(CAP *)malloc(sizeof(CAP));
memset(cap->nums,-1,sizeof(cap->nums));
memset(cap->flgs,0,sizeof(cap->flgs));
memset(cap->strs,0,sizeof(cap->strs));
if(!name) if(!(name=getenv("TERM")))
 {
 free(cap);
 return 0;
 }

tp=getenv("TERMCAP");

if(tp?tp[0]=='/':0)
 strcpy(namebuf,tp), cap->tbuf[0]=0;
else
 {
 if(tp) strcpy(cap->tbuf,tp); else cap->tbuf[0]=0;
 if(tp=getenv("TERMPATH"))
  strcpy(namebuf,tp);
 else if(tp=getenv("HOME"))
  strcpy(namebuf,tp), strcat(namebuf,"/"), strcat(namebuf,TERMPATH);
 else
  strcpy(namebuf,TERMPATH);
 }

for(x=0,y=0;namebuf[x];)
 {
 while(namebuf[x]==' ' || namebuf[x]=='\t') ++x;
 npbuf[y++]=namebuf+x;
 while(namebuf[x] && namebuf[x]!=' ' && namebuf[x]!='\t') ++x;
 if(namebuf[x]) namebuf[x++]=0;
 }
npbuf[y]=0;

y=0; fd=0; tp=cap->tbuf;
goto match;

nextfile:
if(fd) fclose(fd), fd=0;
if(!npbuf[y])
 {
 free(cap);
 return 0;
 }
/* printf("Opening file %s\n",npbuf[y]); */
fd=fopen(npbuf[y++],"r");

nextline:
if(!fd) goto nextfile;
x=0;

while((c=getc(fd))!= -1)
 if(c=='\n')
  if(x?tp[x-1]=='\\':0) --x;
  else break;
 else if(c=='\r') ;
 else
  if(tp-cap->tbuf+x!=ENTRYLEN-2) tp[x++]=c;

if(!x) goto nextfile;
else tp[x]=0;

match:
/* Check if any names at tp matches */
if(*tp=='#') goto nextline;
x=0;
matchloop:
for(z=x;tp[z] && tp[z]!='|' && tp[z]!=':';++z);
c= tp[z]; tp[z]=0;
if(strcmp(name,tp+x))
 {
 if(!c || c==':') goto nextline;
 tp[z]=c; x=z+1; goto matchloop;
 }
tp[z]=c;

/* printf("Found match\n"); */

x=strlen(tp);
do
 {
 tp[x]=0;
 while(x) if(tp[--x]==':') break;
 }
 while(x && (!tp[x+1] || tp[x+1]==':'));

if(tp[x+1]=='t' && tp[x+2]=='c' && tp[x+3]=='=')
 {
 name=tp+x+4;
 tp=name+strlen(name)+1;
 if(y) --y;
/*  printf("Found tc=\n"); */
 goto nextfile;
 }

doline:
/* printf("%s\n",tp); */
/* Process line at tp */
pp=tp;
loop:
while(*pp && *pp!=':') ++pp;
if(*pp)
 {
 *pp=0;
 ++pp;
 if(pp[0]==' ' || pp[0]=='\t') goto loop;
 if(pp[2]=='#')
  {
  x=0; z= -1;
  y=sizeof(nums)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>nums[z*2]*256+nums[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<nums[z*2]*256+nums[z*2+1]) y=z;
   else
    {
    cap->nums[z]=atoi(pp+3);
    goto loop;
    }
   }
  }
 else if(pp[2]=='=')
  {
  x=0; z= -1;
  y=sizeof(strs)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>strs[z*2]*256+strs[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<strs[z*2]*256+strs[z*2+1]) y=z;
   else
    {
    cap->strs[z]=pp+3;
    goto loop;
    }
   }
  }
 else if(pp[2]=='@')
  {
  x=0; z= -1; y=sizeof(flgs)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>flgs[z*2]*256+flgs[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<flgs[z*2]*256+flgs[z*2+1]) y=z;
   else
    {
    cap->flgs[z]=0;
    goto loop;
    }
   }
  x=0; z= -1; y=sizeof(strs)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>strs[z*2]*256+strs[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<strs[z*2]*256+strs[z*2+1]) y=z;
   else
    {
    cap->strs[z]=0;
    goto loop;
    }
   }
  x=0; z= -1; y=sizeof(nums)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>nums[z*2]*256+nums[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<nums[z*2]*256+nums[z*2+1]) y=z;
   else
    {
    cap->nums[z]= -1;
    goto loop;
    }
   }
  }
 else
  {
  x=0; z= -1;
  y=sizeof(flgs)/2;
  while(z!=(x+y)/2)
   {
   z=(x+y)/2;
   if(pp[0]*256+pp[1]>flgs[z*2]*256+flgs[z*2+1]) x=z;
   else if(pp[0]*256+pp[1]<flgs[z*2]*256+flgs[z*2+1]) y=z;
   else
    {
    cap->flgs[z]=1;
    goto loop;
    }
   }
  }
/* printf("Unknown capability: %c%c\n",pp[0],pp[1]); */
 goto loop;
 }

if(tp!=cap->tbuf)
 {
 for(--tp;tp!=cap->tbuf;--tp) if(!tp[-1]) break;
 goto doline;
 }
return cap; /* We're done! */
}

static C escape(s)
C **s;
{
C c= *(*s)++;
if(c=='^' && **s)
 if(**s!='?') return 037&*(*s)++;
 else return (*s)++, 127;
else if(c=='\\' && **s)
 switch(c= *((*s)++))
  {
 case '0': case '1': case '2': case'3': case '4': case '5': case '6': case '7':
           c-='0';
           if(**s>='0' && **s<='7') c=(c<<3)+*((*s)++)-'0';
           if(**s>='0' && **s<='7') c=(c<<3)+*((*s)++)-'0';
           return c;
 case 'e':
 case 'E': return 27;
 case 'n':
 case 'l': return 10;
 case 'r': return 13;
 case 't': return 9;
 case 'b': return 8;
 case 'f': return 12;
 case 's': return 32;
 default: return c;
  }
else return c;
}


V texec(s,out,outptr,pad,div,l,x,y)
C *s, *pad, *outptr;
I div,l,x,y;
V (*out)();
{
I c, tenth=0;
while(*s>='0' && *s<='9') tenth=tenth*10+*(s++)-'0';
tenth*=10;
if(*s=='.') ++s, tenth+= *(s++)-'0';
if(*s=='*') ++s, tenth*=l;
while(c= *s++)
 if(c=='%' && *s)
  switch(c= escape(&s))
   {
  case '+': if(*s) x+= escape(&s);
  case '.': out(outptr,x); x=y; break;
  case 'd': if(x<10) goto one;
  case '2': if(x<100) goto two;
  case '3': c='0'; while(x>=100) ++c, x-=100; out(outptr,c);
       two: c='0'; while(x>=10) ++c, x-=10; out(outptr,c);
       one: out(outptr,'0'+x); x=y; break;
  case 'r': c=x; x=y; y=c; break;
  case 'i': ++x; ++y; break;
  case 'n': x^=0140; y^=0140; break;
  case 'D': x=x-2*(x&15); break;
  case 'B': x=16*(x/10)+x%10; break;
  case '>': if(x>escape(&s)) x+=escape(&s); else escape(&s);
   default: out(outptr,'%'); out(outptr,c);
   }
 else --s, out(outptr,escape(&s));
if(pad) while(tenth>=div) for(s=pad;*s;++s) out(outptr,*s), tenth-=div;
else while(tenth>=div) out(outptr,0), tenth-=div;
}

static I total;

static V cst()
{
++total;
}

I tcost(s,pad,div,l,x,y)
C *s, *pad;
I div,l,x,y;
{
total=0;
if(!s) return 0;
texec(s,cst,NULL,pad,div,l,x,y);
return total;
}

V rmcap(cap)
CAP *cap;
{
free(cap);
}

static C ss[1024];
static C *ssp;
static V cpl(ptr,c)
C *ptr;
C c;
{
*ssp++=c;
}

C *tcompile(s,x,y,l)
C *s;
I x,y,*l;
{
*l=0;
ssp=ss;
texec(s,cpl,NULL,NULL,10000,0,x,y);
*ssp=0;
*l=ssp-ss;
return ss;
}

V tshow(s)
C *s;
{
while(*s)
 {
 if(*s>=128) printf("M-"), *s-=128;
 if(*s>=32 && *s<=126) printf("%c",*s);
 else if(*s<32) printf("^%c",*s+'@');
 else if(*s==127) printf("^?");
 s++;
 }
printf("\n");
}
