/* Device independant tty interface for JOE
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

/* Number of quick updates to buffer before doing a brute-force update */

#define NQUICK 10

/* Number of key sequences */

#define NKEYS 20

/* Each terminal has one of these */

struct scrn
 {
 CAP *cap;		/* Termcap/Terminfo data */

 I li;			/* Screen height */
 I co;			/* Screen width */

 C *pad;		/* Padding string to use */

 C *ti;			/* Initialization string */
 C *te;			/* Restoration string */

 I hz;			/* Terminal can't print ~s */
 I os;			/* Terminal overstrikes */
 I eo;			/* Can use blank to erase even if os */
 I ul;			/* _ overstrikes */
 I am;			/* Terminal has autowrap, but not magic wrap */
 I xn;			/* Terminal has magicwrap */

 C *so;			/* Enter standout (inverse) mode */
 C *se;			/* Exit standout mode */

 C *us;			/* Enter underline mode */
 C *ue;			/* Exit underline mode */
 C *uc;			/* Single time underline character */

 C *mb;			/* Enter blinking mode */
 C *md;			/* Enter bold mode */
 C *mh;			/* Enter dim mode */
 C *mr;			/* Enter inverse mode */
 C *me;			/* Exit above modes */

 I ms;			/* Ok to move when in standout/underline mode */

 I da, db;		/* Extra lines exist above, below */
 I ns;			/* If LF does not scroll */
 C *al, *dl, *AL, *DL;	/* Insert/delete lines */
 C *cs;			/* Set scrolling region */
 C *sf, *SF, *sr, *SR;	/* Scroll */

 C *bs;			/* Move cursor left 1 */
 I cbs;
 C *lf;			/* Move cursor down 1 */
 I clf;
 C *up;			/* Move cursor up 1 */
 I cup;

 C *ta;			/* Move cursor to next tab stop */
 I cta;
 C *bt;			/* Move cursor to previous tab stop */
 I cbt;
 I tw;			/* Tab width */

 C *ho;			/* Home cursor to upper left */
 I cho;
 C *ll;			/* Home cursor to bottom left */
 I cll;
 C *cr;			/* Move cursor to left edge */
 I ccr;
 C *RI;			/* Move cursor right n */
 I cRI;
 C *LE;			/* Move cursor left n */
 I cLE;
 C *UP;			/* Move cursor up n */
 I cUP;
 C *DO;			/* Move cursor down n */
 I cDO;
 C *ch;			/* Set cursor column */
 I cch;
 C *cv;			/* Set cursor row */
 I ccv;
 C *cm;			/* Set cursor row and column */
 I ccm;

 C *ce;			/* Clear to end of line */
 I cce;
 C *cd;			/* Clear to end of screen */
 I ccd;

 struct
  {
  C *s;			/* Key string */
  I l;			/* Key string length */
  I n;			/* Value which should be returned for this string */
  } ktab[NKEYS];

 I tabsize;		/* Number of entries in table */

 C kbuf[32];		/* Keyboard buffer */
 I kbufp;		/* Pointer */

 I (*out)();		/* Function to output a character to the terminal */
 C *outptr;		/* Pointer to pass to 'out' */

 I div;			/* Baud rate divisor: 100000/baud */

 I *scrn;		/* Current contents of screen */
 I *screen;		/* What you want it to look like */
 I *sary;		/* Scroll array */

 I quickies;		/* Number of buffered simple updates */
 struct
  {
  I ch,c,x,y,l,attr;
  C *s;
  } quick[NQUICK];	/* Buffered simple updates */

 I x,y;			/* Current cursor position (-1 for unknown) */
 I top,bot;		/* Current scrolling region */
 I attrib;		/* Current character attributes */
 I avattr;		/* Bits set for available attributes */
 I tattr;		/* Requested attributes */
 I upd;			/* Set if screen needs to be updated from scratch */
 I cnt;			/* Set if screen update needs to be continued */
 I updy;		/* Place where screen update should continue from */
 I placex, placey;	/* Where you want the cursor to be */
 };

/* SCRN *topen(I flg);
 *
 * Open the screen (sets TTY mode so that screen may be used immediatly after
 * the 'topen').
 *
 * If 'flg' is set, 'topen' doesn't mess with the signals.  Usually topen is
 * called with 'flg' set to zero.  The only time to have it set is if you're
 * implementing shell escapes.
 */
SCRN *topen();

/* V tclose(SCRN *t,I flg);
 *
 * Close the screen and restore TTY to initial state.
 *
 * if 'flg' is set, tclose doesn't mess with the signals.
 */
V tclose();

/* I tgetc(SCRN *t);
 *
 * Get next input character.  The screen is updated if it needs to be.
 * Arrow keys are translated into the integer codes shown below.
 */
I tgetc();

#define KEYUP 256	/* Arrow keys */			/* ku */
#define KEYDOWN 257						/* kd */
#define KEYLEFT 258						/* kl */
#define KEYRIGHT 259						/* kr */
#define KEYF0 260	/* Function keys (if F0 really F10?) */ /* k0 */
#define KEYF1 261						/* k1 */
#define KEYF2 262
#define KEYF3 263
#define KEYF4 264
#define KEYF5 265
#define KEYF6 266
#define KEYF7 267
#define KEYF8 268
#define KEYF9 269						/* k9 */
#define KEYDEL 270 	/* Delete character */			/* kD */
#define KEYINS 271	/* Insert character */			/* kI */
#define KEYHOME 272	/* Home key */				/* kh */
#define KEYEND 273	/* End key */				/* kH */
#define KEYPGDN 274	/* Page down key */			/* kN */
#define KEYPGUP 275	/* Page up key */			/* kP */

/* V tputc(SCRN *t,I x,I y,I c);
 *
 * Write a character to the screen.  Warning:  This macro accesses parameter
 * 't' more than once.
 */
#define tputc(t,x,y,c) \
 ( \
 ( (t)->quickies==NQUICK ? tdumpq(t) : 0 ), \
 (t)->quick[(t)->quickies].ch=1, \
 (t)->quick[(t)->quickies].x=(x), \
 (t)->quick[(t)->quickies].y=(y), \
 (t)->quick[(t)->quickies].attr=(t)->tattr, \
 (t)->quick[(t)->quickies].c=(c), \
 ++(t)->quickies \
 )

/* V tputs(SCRN *t,I x,I y,C *s);
 *
 * Write a string to the screen.  The string must never go past the right edge
 * of the screen.
 */
V tputs();

/* V tprintf(SCRN *t,I x,I y,C *str,...);
 *
 * Printf to the screen using the current attributes.  The string should never
 * go past the right edge of the screen.
 */
V tprintf();

/* V tsetattr(SCRN *t,I c);
 *
 * Set the attribute value.  This value is ORed with the characters written
 * by 'tputs', 'tputc', 'tprintf', 'tbox' and 'trect' before being sent to the
 * screen.
 */
#define tsetattr(t,c) ((t)->tattr=(c))

/* Character attribute bits */

#define INVERSE 256
#define UNDERLINE 512
#define BOLD 1024
#define BLINK 2048
#define DIM 4096

/* I tgetattr(SCRN *t);
 *
 * Get the current attributes
 */
#define tgetattr(t) ((t)->tattr)

/* I tavattr(SCRN *t);
 *
 * Get attributes available with this terminal.  I.E., the returned word
 * is the some of the attribute bit values for the attributes which are
 * available.
 */
#define tavattr(t) ((t)->avattr)

/* V tsetpos(SCRN *t,I x,I y);
 *
 * Set the cursor position on the screen.  This is where the cursor will
 * be left after calling 'trefresh' or 'tgetc'.
 */
V tsetpos();

/* V tgetpos(SCRN *t,I *x,I *y);
 *
 * Sets the variables at the addresses given with the real cursor position.
 */
V tgetpos();

/* V trect(SCRN *t,I x,I y,I w, I h,I c);
 *
 * Fill a rectangle defined by 'x', 'y', 'w' and 'h' with the character 'c'.
 * The current attributes (set with 'tattr') are ORed in with the character.
 */
V trect();

/* V tbox(SCRN *t,I x,I y,I w,I h);
 *
 * Draw a box on the screen.  'x', 'y', 'w' and 'h' give specify the rectangle
 * the box will surround.  The global variables 'boxul', 'boxur', 'boxll',
 * 'boxlr', 'boxl', 'boxr', 'boxt', 'boxb' specify the characters to use to
 * draw the box.  The current attributes (set with 'tattr') are ORed with
 * these characters before the box is drawn.  These characters are initialized
 * with '+', '-', and '|' and may be changed.
 */
V tbox();
extern I boxl, boxr, boxt, boxb, boxll, boxlr, boxul, boxur;

/* V tscrollup(SCRN *t,I top,I bot,I amnt);
 *
 * Scroll some lines up.  'top' and 'bot' indicate which lines to scroll.
 * 'bot' is the last line to scroll + 1.  'amnt' is distance in lines to
 * scroll.
 */
V tscrollup();

/* V tscrolldn(SCRN *t,I top,I bot,I amnt);
 *
 * Scroll some lines down.  'top' and 'bot' indicate which lines to scroll.
 * 'bot' is the last line to scroll + 1.  'amnt' is distance in lines to
 * scroll.
 */
V tscrolldn();

/* I *tsave(SCRN *t,I x,I y,I w,I h);
 *
 * Save a rectangle in an malloc block.  The address of the malloc block is
 * returned.  The format of the array is always: array[x+y*width]
 */
I *tsave();

/* V trestore(SCRN *t,I x,I y,I w,I h,I *s);
 *
 * Write an array (as returned by 'tsave') to the screen.  This routine does
 * not free the malloc block returned by 'tsave'.
 */
V trestore();

/* V trefresh(SCRN *t);
 *
 * Refresh the screen.  This should never have to be called except for
 * animation or 'PLEASE WAIT, READING FROM DISK' messages.  'tgetc'
 * automatically calls this.  If there is pending input, and if this can be
 * detected by the TTY driver, 'trefresh' aborts.
 */
I trefresh();

/* V tdumpq(SCRN *t);
 *
 * Dump buffered screen modifications to the screen buffer.  This routine
 * should be called before any direct access to 't->screen' array.  It will
 * make sure that any updates done with 'tputc', 'tputs' and 'tprintf' are
 * made to the screen array.  This way, you can write the array directly
 * messing up the order of the screen modifications
 */
V tdumpq();

/* V tchsize(SCRN *t,I w,I h);
 *
 * Change size of screen.  For example, call this when you find out that
 * the Xterm changed size.
 */
/* (this is not yet working in this version)
I tchsize();
*/

/* SCRN *tsusp(SCRN *t);
 *
 * Suspend the process.  This returns a new SCRN structure which must be used
 * instead of the original.  The new SCRN structure will be properly set
 * in case the user changed the baud rate.
 *
 * If your UNIX won't let you suspend, this routine calls tshell instead.
 */
SCRN *tsusp();

/* SCRN *tshell(SCRN *t);
 *
 * Run a shell.  See notes about tsusp about how it returns a new SCRN
 * structure.
 */
SCRN *tshell();

/* I TDIRECT(SCRN *t,I x,I y)
 *
 * Macro to directly access the screen buffer.
 */
#define TDIRECT(t,x,y) ((t)->screen[(x)+(y)*(t)->co])

/* V tredraw(SCRN *t);
 *
 * Invalidate all state variables for the terminal.  This way, everything gets
 * redrawn.
 */
V tredraw();
