/* TTY interface header file
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

/* V aopen(V);  Open the tty (attached to stdin) for use inside of JOE
 *
 * (1) fflush(stdout)
 *
 * (2) Save the current state of the tty
 *
 * (3) Disable CR/LF/NL input translations,
 *     Disable all output processing,
 *     Disable echo and line editing, and
 *     Place tty in character at a time mode.
 *     (basically, disable all processing except for XON/XOFF if it's set)
 *
 * (4) Set this new tty state without loosing any typeahead
 *
 * (5) Store the baud rate in the global variable 'baud'
 *
 * (6) Divide the baud rate into the constant DIVIDEND and store the result
 *     in the global variable 'upc'.  This should come out to the number
 *     of microseconds needed to send each character.  The constant 'DIVIDEND'
 *     should be chosen so that 'upc' reflects the real throughput of the
 *     tty, not the theoretical best throughput.
 *
 * (7) Create an output buffer of a size which depends on 'upc' and the
 *     constant 'TIMES'.  'TIMES' is the number of times per second JOE
 *     should check for typeahead.  Since we only check for typehead after
 *     the output buffer is flushed, 'upc' and the size of the output buffer
 *     determine how often this occurs.  So for example if 'upc'==1000 (~9600
 *     baud) and 'TIMES'==3, the output buffer size is set to 333 characters.
 *     Each time this buffer is completely flushed, 1/3 of a second will go by.
 */
V aopen();
extern unsigned long upc;
extern unsigned long baud;

#define TIMES 3
#define DIVIDEND 11000000

/* V aclose(V);  Restore the tty back to its original mode.
 *
 * (1) aflush()
 *
 * (2) Restore the original tty mode which aopen() had saved.  Do this without
 *     loosing any typeahead.
 */
V aclose();

/* V aflush(V);  Flush the output buffer and check for typeahead.
 *
 * (1) write() any character in the output buffer to the tty.  Sleep for the
 *     amount of time it should take for all of these characters to get
 *     to the tty.  This is so that any buffering between the editor and the
 *     tty is defeated.  If this is not done, the screen update will not be
 *     able to defer for typeahead.
 *
 *     The best way to do this (and it's currently only possible in BSD) is to
 *     set a timer for the necessary amount, write the characters to the tty,
 *     and then sleep until the timer expires.
 *
 *     If this can't be done, it's usually ok to 'write' and then to sleep for
 *     the necessary amount of time.  However, you will notice delays in the
 *     screen update if the 'write' actually takes any significant amount of
 *     time to execute (it usually takes none since all it usually does is
 *     write to an operating system buffer).
 *
 * (2) If the global variable 'leave' is not set and if the global variable
 *     'have' is not set, check for typeahead.  If there is any, set the global
 *     variable 'have'.  This absolutely must not read any characters from the
 *     'tty' if 'leave' is set or typeahead will be lost when the editor exists
 *     or does a shell escape.
 */
V aflush();

extern I have;
extern I leave;

/* C anext(V);  Flush the output and get the next character from the tty
 *
 * (1) aflush()
 *
 * (2) Read the next input character
 *     If the input closed, call 'tsignal' with 0 as its argument.
 *
 * (3) Clear 'have'
 */
C anext();

/* V eputc(C c);  Write a character to the output buffer.  If it becomes
 * full, call aflush()
 */
V eputc();

/* V eputs(C *s);  Write a string to the output buffer.  Any time the
 * output buffer gets full, call aflush()
 */
V eputs();

/* V sigjoe(V);  Set the signal handling for joe.  I.E., ignore all
 * signals the user can generate from the keyboard and trap the software
 * terminate and hangup signals.
 */
V sigjoe();

/* V signorm(V);  Set the signal handling for the shell.  I.E., set the
 * signals back to their default actions.
 */
V signorm();

/* V tsignal(I n);  Signal handler.  This is called if the editor gets a
 * hangup signal, termination signal or if the input closes.  It is called
 * with 'n' set to the number of the caught signal or 0 if the input closed.
 */
V tsignal();

/* V getsize(I *x,I *y);  Get size of screen from ttysize/winsize structure */
V getsize();

/* V shell(V);  Run the shell in the environment variable SHELL */
V shell();

/* V susp(V);  Suspend the process, of the UNIX can't do it, call shell() */
V susp();
