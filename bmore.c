/* BMORE - binary more
 *
 * 1990-01-31  V 1.0.0
 * 1990-09-04  V 1.1.0
 * 2000-05-31  V 1.3.0 beta
 * 2000-10-18  V 1.3.0 final
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1990-2000 by Gerhard Buergmann
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


#include <sys/types.h>

#ifdef HAVE_LOCALE_H
#	include <locale.h>
#endif

#ifdef	__MSDOS__
#	define PRINTF	cprintf
#else
#	define PRINTF	printf
#ifndef HELPFILE
#	define HELPFILE "/usr/local/lib/bmore.help"
#endif
#endif

#include "bmore.h"

char	*copyright  = "Copyright (C) 1990-2000 by Gerhard Buergmann";

int		maxx, maxy;
char	*name = NULL;
char	sstring[MAXCMD] = "";
char	string[MAXCMD];
FILE	*curr_file = NULL, *help_file;
int		AnzAdd;
long	precount = -1;	/* number preceding command */

char	**files;		/* list of input files */
int		numfiles;		/* number of input files */
int		file_nr = 0;	/* number of current input file */

int		arrnum = 0;
char	numarr[64];		/* string for collecting number */
char	addr_form[15];

int		ascii_flag = 0;
int		d_flag = 0;
int		init_search = 0;
char	buffer1[MAXCMD], buffer2[MAXCMD];
int		out_len;
int		corr = 0, to_print;
off_t	init_byte = 0;
off_t	last_search = 0;
off_t	screen_home, filesize;
off_t	bytepos, oldpos;
int		prompt = 1;
char	helppath[MAXCMD];

static	char	*progname;
static	char	cmdbuf[MAXCMD];
static	int		cnt = 0;
static	int		icnt = 0;
static	int		smode;

char	search_pat[BUFFER];	/* / or ? command */
char	*emptyclass = "Empty byte class '[]' or '[^]'";


/* -a   ASCII mode
 * -d   beginners mode
 * -i   ignore case
 */
void
usage()
{
	fprintf(stderr, "Usage: %s [-adi] [+linenum | +/pattern] name1 name2 ...\n", progname);
	exit(1);
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int		ch, ch1;
	int		colon = 0, last_ch = 0;
	long	last_pre = 0;
	int		lflag, repeat;
	long	count;
	int		i, n = 1;
	int		d_line, r_line, z_line;


#ifdef __MSDOS__
	char	*poi;

	strcpy(helppath, argv[0]);
	poi = strrchr(helppath, '\\');
	*poi = '\0';
	strcat(helppath, "\\MORE.HLP");
#else
	strncpy(helppath, HELPFILE, MAXCMD - 1);
#endif

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif

	progname = argv[0];

	while (n < argc) {
		switch (argv[n][0]) {
		case '-':
			i = 1;
			while (argv[n][i] != '\0') {
				switch (argv[n][i]) {
				case 'a':	ascii_flag++;
							break;
				case 'd':	d_flag++;
							break;
				case 'i':	ignore_case++;
							break;
				default:	usage();
				}
				i++;
			}
			n++;
			break;
		case '+':			/* +cmd */
			if (argv[n][1] == '/' || argv[n][1] == '\\') {
				init_search = argv[n][1];
				strcpy(sstring, &argv[n][2]);
			} else {
				if (argv[n][1] == '0') {
					init_byte = (off_t)strtol(argv[n] + 1, NULL, 16);
				} else {
					init_byte = (off_t)strtol(argv[n] + 1, NULL, 10);
				}
			}
			n++;
			break;
		default:			/* must be a file name */
			name = strdup(argv[n]);
			files = &(argv[n]);
			numfiles = argc - n;
			n = argc;
			break;
		}
	}
	if (numfiles == 0) {
		curr_file = stdin;
		if (isatty(fileno(stdin)) != 0) usage();
	} else {
		open_file(name);
		file_nr = 1;
		fseek(curr_file, init_byte, SEEK_SET);
		bytepos += init_byte;
	}
	screen_home = bytepos;

	initterm();
	set_tty();
	maxy -= 2;
	z_line = maxy;
	d_line = maxy / 2;
	r_line = 1;

	AnzAdd = 10;
	strcpy(addr_form,  "%08lX  ");

	if (ascii_flag)
		 out_len = ((maxx - AnzAdd - 1) / 4) * 4;
	else
		out_len = ((maxx - AnzAdd - 1) / 16) * 4;

	if (init_search)
		bmsearch(init_search);

	if (no_tty) {
		while(!printout(1));
		reset_tty();
		exit(0);
	}
	if (printout(maxy)) {
		do_next(1);
	}
	/* main loop */
	do {
		to_print = 0;
		if (prompt) {
			highvideo();
			PRINTF("--More--");
			if (prompt == 2) {
				PRINTF("(Next file: %s)", name);
			} else if (!no_intty && filesize) {
				PRINTF("(%d%%)", (int)((bytepos * 100) / filesize));
			}

			if (d_flag) PRINTF("[Press space to continue, 'q' to quit]");
			normvideo();
			fflush(stdout);
		}
		ch = vgetc();
		if (prompt == 2) {
			open_file(name);
		}
		prompt = 1;
		PRINTF("\r");
		while (ch >= '0' && ch <= '9') {
			numarr[arrnum++] = ch;
			ch = vgetc();
		}
		numarr[arrnum] = '\0';
		if (arrnum != 0) precount = strtol(numarr, (char **)NULL, 10);
			else precount = -1;
		lflag = arrnum = 0;

		if (ch == '.') {
			precount = last_pre;
			ch = last_ch;
			repeat = 1;
		} else {
			last_pre = precount;
			last_ch = ch;
			if (ch == ':') colon = vgetc();
			repeat = 0;
		}

		switch (ch) {
		case ' ':	/*  Display next k lines of text [current screen size] */
					if (precount > 0) to_print = precount;
						else to_print = maxy;
					break;
		case 'z':	/* Display next k lines of bytes [current screen size]* */
					if (precount > 0) z_line = precount;
					to_print = z_line;
					break;
		case '\r':
		case '\n':	/* Display next k lines of text [current screen size]* */
					if (precount > 0) r_line = precount;
					to_print = r_line;
					break;
		case 'q':
		case 'Q':
					clreol();
					reset_tty();
					exit(0);
		case ':' :	
					switch (colon) {
					case 'f':
						prompt = 0;
						if (!no_intty)
							PRINTF("\"%s\" line %lu", name,
								(unsigned long)(bytepos - out_len));
						else
							PRINTF("[Not a file] line %lu",
								(unsigned long)(bytepos - out_len));
						fflush(stdout);
						break;
					case 'n':
						if (precount < 1) precount = 1;
						do_next(precount);
						PRINTF("\r");
						clreol();
						PRINTF("\n...Skipping to file %s\r\n\r\n", name);
						prompt = 2;
						break;
					case 'p':
						if (precount < 1) precount = 1;
						do_next(-precount);
						PRINTF("\r");
						clreol();
						PRINTF("\n...Skipping back to file %s\r\n\r\n", name);
						prompt = 2;
						break;
					case 'q':
						clreol();
						reset_tty();
						exit(0);
						break;
					case '!':
						if (!no_intty) {
							clreol();
							if (rdline(ch)) break;
							doshell(sstring);
							PRINTF("------------------------\r\n");
							break;
						}
					default:
						beep();
					}
					break;
		case 'd':	/* Scroll k lines [current scroll size, initially 11]* */
		case CTRL('D'):
					if (precount > 0) d_line = precount;
					to_print = d_line;
					break;
		case CTRL('L'):   	/*** REDRAW SCREEN ***/
					if (no_intty) {
						beep();
					} else {
						clrscr();
						to_print = maxy + 1;
						fseek(curr_file, screen_home, SEEK_SET);
						bytepos = screen_home;
					}
					break;
		case 'b':		/* Skip backwards k screenfuls of text [1] */
		case CTRL('B'):
					if (no_intty) {
						beep();
					} else {
						if (precount < 1) precount = 1;
						PRINTF("...back %ld page", precount);
						if (precount > 1) {
							PRINTF("s\r\n");
						} else {
							PRINTF("\r\n");
						}
						screen_home -= (maxy + 1) * out_len;
						if (screen_home < 0) screen_home = 0;
						fseek(curr_file, screen_home, SEEK_SET);
						bytepos = screen_home;
						to_print = maxy + 1;
					}
					break;
		case 'f':		/* Skip forward k screenfuls of bytes [1] */
		case 's':		/* Skip forward k lines of bytes [1] */
					if (precount < 1) precount = 1;
					if (ch == 'f') {
						count = maxy * precount;
					} else {
						count = precount;
					}
					putchar('\r');
					clreol();
					PRINTF("\n...skipping %ld line", count);
					if (count > 1) {
						PRINTF("s\r\n\r\n");
					} else {
						PRINTF("\r\n\r\n");
					}
					screen_home += (count + maxy) * out_len;
					fseek(curr_file, screen_home, SEEK_SET);
					bytepos = screen_home;
					to_print = maxy;
					break;
		case '\\':  
					if (ascii_flag) {
						beep();
						break;
					}
		case '/':	/**** Search String ****/
					if (!repeat) {
						if (rdline(ch)) break;
					}
		case 'n': 		/**** Search Next ****/
		case 'N':   
					bmsearch(ch);
					/*
					to_print--;
					*/
					break;
		case '\'':   
					if (no_intty) {
						beep();
					} else {
						bytepos = last_search;
						fseek(curr_file, bytepos, SEEK_SET);
						screen_home = bytepos;
						to_print = maxy;
						PRINTF("\r");
						clreol();
						PRINTF("\n\r\n***Back***\r\n\r\n");
					}
					break;
		case '=':
					prompt = 0;
					clreol();
					PRINTF("%lX hex  %lu dec", (unsigned long)bytepos,
						(unsigned long)bytepos);
					fflush(stdout);
					break;
		case '?':
		case 'h':
					if ((help_file = fopen(helppath, "r")) == NULL) {
						emsg("Can't open help file");
						break;
					}
					while ((ch1 = getc(help_file)) != EOF)
					    putchar(ch1);
					fclose(help_file);
					to_print = 0;
					break;
		case 'w':
		case 'v':
					if (!no_intty) {
						clreol();
						if (ch == 'v') {
							sprintf(string, "bvi +%lu %s", 
								(unsigned long)(screen_home + 
								(maxy + 1) / 2 * out_len), name);
						} else {
							if (precount < 1) precount = bytepos - screen_home;
							sprintf(string, "bvi -b %lu -s %lu %s",
								(unsigned long)screen_home, 
								(unsigned long)precount, name);
						}
						doshell(string);
						to_print = maxy + 1;
						break;
					}
		default :
					if (d_flag) {
						emsg("[Press 'h' for instructions.]");
					} else {
						beep();
					}
					break;
		}
		if (to_print) {
			if (printout(to_print)) {
				do_next(1);
			}
		}
	} while (1);
}


int
rdline(ch)
	int	ch;
{
	int	i = 0;
	int	ch1 = 0;
	
	clreol();
	putchar(ch);
	fflush(stdout);

	while (i < MAXCMD) {
		ch1 = vgetc();
		if (ch1 == '\n' || ch1 == '\r' || ch1 == ESC) {
			break;
		} else if (ch1 == 8) {
			if (i) {
				sstring[--i] = '\0';
				PRINTF("\r%c%s", ch, sstring);
				clreol();
			} else {
				ch1 = ESC;
				break;
			}
		} else {
			putchar(ch1);
			sstring[i++] = ch1;
		}
		fflush(stdout);
	}
	if (ch1 == ESC) {
		putchar('\r');
		clreol();
		return 1;
	}
	if (i) sstring[i] = '\0';
	return 0;
}


void
do_next(n)
	int	n;
{
	if (numfiles) {
		if (n == 1 && file_nr == numfiles) {
			reset_tty();
			exit(0);
		}
		if ((file_nr + n) > numfiles)
			file_nr = numfiles;
		else if ((file_nr + n) < 1)
			file_nr = 1;
		else
			file_nr += n;
		prompt = 2;
		free(name);
		name = strdup(*(files + file_nr - 1));
	} else {
		reset_tty();
		exit(0);
	}
}


void
open_file(name)
	char *name;
{
	struct	stat	buf;

	if (stat(name, &buf) > -1) {
		filesize = buf.st_size;
	}
	if (numfiles > 1) {
		PRINTF("\r");
		clreol();
		PRINTF("\n::::::::::::::\r\n%s\r\n::::::::::::::\r\n", name);
		corr = 2;
	}
	if (curr_file != NULL) fclose(curr_file);
	if ((curr_file = fopen(name, "rb")) == NULL) {
		reset_tty();
		perror(name);
		exit(1);
	}
	bytepos = screen_home = 0;
}


void
putline(buf, num)
	char	*buf;
	int		num;
{
	int			print_pos;
	unsigned	char	ch;

	PRINTF(addr_form, bytepos);
	for (print_pos = 0; print_pos < num; print_pos++) {
		ch = buf[print_pos];
		if (!ascii_flag) {
		    PRINTF("%02X ", ch);
		}
		++bytepos;
		if ((ch > 31) && (ch < 127))
			*(string + print_pos) = ch;
		else
			*(string + print_pos) = '.';
	}
	*(string + num) = '\0';
	PRINTF("%s\r\n", string);
}


int
printout(lns)
	int lns;
{
	int			c, num;
	int			doub = 0;
	static		int		flag;
	
	if (corr && (lns > maxy - 2)) lns -= corr;
	corr = 0;
	do {
		for (num = 0; num < out_len; num++) {
			if ((c = nextchar()) == -1) break;
			buffer1[num] = c;
		}
		if (!num) return 1;
		if (memcmp(buffer1, buffer2, num) || !bytepos ) {
			memcpy(buffer2, buffer1, num);
			putline(buffer2, num);
			if (!no_tty) flag = TRUE;
			lns--;
		} else {
			if (flag) {
				clreol();
				PRINTF("*\r\n");
				lns--;
			} else {
				doub++;
			}
			flag = FALSE;
			bytepos += num;
		}
		if (lns == 0) {
			screen_home = bytepos - ((maxy + 1 + doub) * out_len);
			if (screen_home < 0) screen_home = 0;
			return 0;
		}
	} while(num);
	return 1;
}


int
nextchar()
{
	if (cnt == 0) return fgetc(curr_file);
	cnt--;
	return cmdbuf[icnt++] & 0xff;
}


void
pushback(n, where)
	int	n;
	char	*where;
{
	if (cnt) memmove(cmdbuf + n, cmdbuf, n);
	memcpy(cmdbuf, where, n);
	icnt = 0;
	cnt += n;
}




/* Return:
 * -1	EOF
 *  0	not found at current position
 *  1   found
 */
int
bmregexec(scan)
	char	*scan;
{
	char	*act;
	int		count, test;
	int		l;
	char	act_pat[MAXCMD];  /* found pattern */

	act = act_pat;
	l = 0;
	while (*scan != 0) {
		if ((test = nextchar()) == -1) return -1;
		*act++ = test;
		if (++l == MAXCMD) {
			pushback(l, act_pat);
			return 0;
		}
		if (ignore_case && smode == ASCII)	test = toupper(test);
		switch (*scan++) {
		case ONE:	/* exactly one character */
				count = *scan++;
				if (count == 1) {
					if (test != *scan) {
						bytepos++;
						if (l > 1) pushback(--l, act_pat + 1);
						return 0;
					}
					scan++;
				} else if (count > 1) {
					if (sbracket(test, scan, count)) {
						bytepos++;
						if (l > 1) pushback(--l, act_pat + 1);
						return 0;
					}
					scan += count;
				}
				break;
		case STAR:  /* zero or more characters */
				count = *scan++;
				if (count == 1) {	/* only one character, 0 - n times */
					while (test == *scan) {
						if ((test = nextchar()) == -1) return -2;
						*act++ = test;
						if (++l == MAXCMD) {
							pushback(l, act_pat);
							return 0;
						}
						if (ignore_case && smode == ASCII)
								test = toupper(test);
					}
					pushback(1, --act);
					l--;
					scan++;
				} else if (count > 1) {	/* characters in bracket */
					if (*scan == '^') {
						do {
/* If we found something matching the next part of the expression, we
 * abandon the search for not-matching characters. */
							if (bmregexec(scan + count)) {
								*act++ = test;	/* May be wrong case !! */
								l++;
								scan += count;
								bytepos--;
								break;
							}
							if (sbracket(test, scan, count)) {
								bytepos++;
								if (l > 1) pushback(--l, act_pat + 1);
								return 0;
							} else {
								if ((test = nextchar()) == -1) return -3;
								*act++ = test;
								if (++l == MAXCMD) {
									pushback(l, act_pat);
									return 0;
								}
								if (ignore_case && smode == ASCII)
										test = toupper(test);
							}
						} while(1);
					} else {
						while(!sbracket(test, scan, count)) {
							if ((test = nextchar()) == -1) return -4;
							*act++ = test;
							if (++l == MAXCMD) {
								pushback(l, act_pat);
								return 0;
							}
							if (ignore_case && smode == ASCII)
									test = toupper(test);
						}
						scan += count;
						pushback(1, --act);
						l--;
					}
				} else {	 /* ".*"  */
					do {
						if ((test = nextchar()) == -1) return -5;
						*act++ = test;
						if (++l == MAXCMD) {
							pushback(l, act_pat);
							return 0;
						}
						pushback(1, act - 1);
						bytepos--;
					} while (bmregexec(scan) == 0);
					bytepos++;
					act--;
					l--;
				}
				break;
		}
	}
	pushback(l, act_pat);
	return 1;	/* found */
}


int
sbracket(start, scan, count)
	int		start;
	char	*scan;
	int		count;
{
	if (*scan++ == '^') {
		if (!memchr(scan, start, --count)) return 0;
	} else {
		if (memchr(scan, start, --count)) return 0;
	}
	return 1;
}


void
bmsearch(ch)
	int	ch;
{
	int	i;

	if (sstring[0] == '\0') {
		emsg("No previous regular expression");
		return;
	}
	if (ch == '/') {
		if (ascii_comp(search_pat, sstring)) return;
	}
	if (ch == '\\') {
		if (hex_comp(search_pat, sstring)) return;
	}
	oldpos = bytepos;
	last_search = screen_home;
	if (precount < 1) precount = 1;
	while (precount--) {
		while ((i = bmregexec(search_pat)) == 0);
		if (i == 1) {
			screen_home = bytepos;
			to_print = maxy;
		} else {		/* i == -1 -> EOF */
			if (no_intty) {
				PRINTF("\r\nPattern not found\r\n");
				do_next(1);
			} else {
sprintf(string, "Pattern not found %d - %ul", i, (unsigned long)bytepos);
emsg(string);
/*
				emsg("Pattern not found");
*/
				bytepos = oldpos;
				fseek(curr_file, bytepos, SEEK_SET);
				break;
			}
		}
		if (precount) {
			nextchar();
			bytepos++;
		}
	}
	if (prompt) {
		PRINTF("\r\n...skipping\r\n");
	}
}


void
emsg(s)
	char	*s;
{
	putchar('\r');
	clreol();
	highvideo();
	PRINTF(s);
	normvideo();
	fflush(stdout);
	prompt = 0;
}