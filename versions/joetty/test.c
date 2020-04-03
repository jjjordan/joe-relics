#include "types.h"
#include "tty.h"
#include "cap.h"
#include "scrn.h"

main()
{
SCRN *t=topen(0);	/* Open the screen.  This sets up everything */
int x=1,y=1,c;
if(!t) return;		/* Abort if there was an error... */

tbox(t,1,1,t->co-2,t->li-2);	/* Draw a box around the screen */

do
 {
 tsetpos(t,x,y);		/* Set the displayed cursor position */
 c=tgetc(t);			/* Get a character from the keyboard */
 if(c==KEYUP && y) --y;		/* Check for arrow keys... */
 else if(c==KEYDOWN && y!=t->li-1) ++y;
 else if(c==KEYRIGHT && x!=t->co-1) ++x;
 else if(c==KEYLEFT && x) --x;
 else if(c=='X') tclose(t,0);	/* Check for exiting... */
 else if(c=='!')
  {
  if(!(t=tsusp(t))) exit(1);	/* Check for shell escape... */
  }
 else if(c>=' ' && c<='~' && x!=t->co-1)
  tputc(t,x,y,c), ++x;		/* Write normal chars to screen */
 }
 while(!leave);
}
