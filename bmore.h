/*  BMORE.H
 *
 * 1996-02-28  V 1.0.0
 * 1999-01-21  V 1.1.0
 * 1999-03-17  V 1.1.1
 * 1999-07-01  V 1.2.0 beta
 * 1999-08-21  V 1.2.0 final
 * 2000-05-31  V 1.3.0 beta
 * 2000-10-04  V 1.3.0 final
 *
 *  NOTE: Edit this file with tabstop=4 !
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <setjmp.h>

#ifdef __MSDOS__
#	include "patchlev.h"
#	include "dosconf.h"
#   include <alloc.h>
#	include <conio.h>
#	include <bios.h>
#else
#	include "patchlevel.h"
#	include "config.h"
#	include <unistd.h>
#if HAVE_NCURSES_H
#   include <ncurses.h>
#else
#   include <curses.h>
#endif 
#include <term.h>
#endif

/* defines for filemode */
#define	ERROR				-1
#define REGULAR				0
#define NEW					1
#define DIRECTORY			2
#define CHARACTER_SPECIAL	3
#define BLOCK_SPECIAL		4
#define PARTIAL             5

/* regular expressions */
#define END     0
#define ONE     1
#define STAR    2

#define ASCII		1
#define FORWARD		0
#define BACKWARD	1
#define CR			'\r'
#define NL			'\n'
#define BS			8
#define	ESC			27
#define SEARCH		0
#define REPLACE		1
#ifndef CTRL
#	define CTRL(n)		(n&0x1f)
#endif

#ifndef NULL
#	define NULL		((void *)0)
#endif

#ifndef TRUE
#	define TRUE		1
#	define FALSE	0
#endif

#ifdef __MSDOS__
#	define ANSI
#	define PTR		char huge *
#	define off_t	long
#   define DELIM	'\\'
#   define  strncasecmp strnicmp
#   define  strcasecmp	stricmp
#else
#	define PTR		char *
#   define DELIM	'/'
#endif

#define MAXCMD	255
#define BUFFER	1024


#ifdef DEBUG
	extern FILE *debug_fp;
#endif

extern	char	*version;

#ifdef NO_SYSERRL
	extern	char	*sys_errlist[];
#endif


extern	int		maxx, maxy;
extern	int		ignore_case, magic;
extern	int		no_tty, no_intty;




#ifdef ANSI
	void	initterm(void), set_tty(void), reset_tty(void);
	void	clreol(void), clrscr(void), highvideo(void), normvideo(void);
	void	/* beep(void), */ home(void);
	void	doshell(char *), emsg(char *);
	void	do_next(int);
	void	open_file(char *);
	void	bmsearch(int);
	void	pushback(int, char *);
	int		printout(int), rdline(int);
	int		nextchar(void), vgetc(void);
	int     sbracket(int, char *, int);
	int     bmregexec(char *);
	int		ascii_comp(char *, char *), hex_comp(char *, char *);
	void    putline(char *, int);
#else
	void	initterm(), set_tty(), reset_tty();
	void	clreol(), clrscr(), highvideo(), normvideo();
	void	/* beep(), */ home();
	void	doshell(), emsg();
	void	do_next();
	void	open_file();
	void	bmsearch();
	void	pushback();
	int		printout(), rdline();
	int		nextchar(), vgetc();
	int     sbracket();
	int     bmregexec();
	int		ascii_comp(), hex_comp();
	void    putline();
#endif
