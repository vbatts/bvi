/* BM_DOS.C - DOS specific subroutines for BMORE
 *
 * 2000-05-10 V 1.3.0 alpha
 * 2000-07-07 V 1.3.0 final
 * 2001-12-07 V 1.3.1
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2000 by Gerhard Buergmann
 * Gerhard.Buergmann@altavista.net
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * See file COPYING for information on distribution conditions.
 */

/*
#define DEBUG 1
*/

#include "bmore.h"

#define TBUFSIZ 1024


extern	off_t	bytepos, screen_home;
extern	FILE	*curr_file;
/*
extern	int		screenlines;
*/

int		got_int;
int		no_intty, no_tty;

#ifdef DEBUG
FILE *dbug;
#endif


void
initterm()
{
	maxx = 80;
	maxy = 25;
}

void
set_tty ()
{
}

void
reset_tty ()
{
}


void
sig()
{
	signal(SIGINT, sig);

	got_int = TRUE;
}


/*
 * doshell() - run a command or an interactive shell
 */
void
doshell(cmd)
char	*cmd;
{
	system(cmd);
	printf("\r");
	clrscr();
	fseek(curr_file, screen_home, SEEK_SET);
	bytepos = screen_home;
}


void
home()
{
	/*
	tputs(Home, 1, putch);

	screenlines = 0;
	*/
	gotoxy(1, 1);
}


int
vgetc()
{
	 return ((char)bioskey(0));
}


/*
 * force clear to end of line
 */
cleareol()
{
	/*
	tputs(erase_ln, 1, putch);
	*/
	clreol();
}


