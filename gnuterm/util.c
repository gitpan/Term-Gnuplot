#ifndef lint
static char *RCSid() { return RCSid("$Id: util.c,v 1.35 2002/09/02 21:03:27 mikulik Exp $"); }
#endif

/* GNUPLOT - util.c */

/*[
 * Copyright 1986 - 1993, 1998   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the complete modified source code.  Modifications are to
 * be distributed as patches to the released version.  Permission to
 * distribute binaries produced by compiling modified sources is granted,
 * provided you
 *   1. distribute the corresponding source modifications from the
 *    released version in the form of a patch file along with the binaries,
 *   2. add special version identification to distinguish your version
 *    in addition to the base release version number,
 *   3. provide your name and address as the primary contact for the
 *    support of your modified version, and
 *   4. retain our contact information in regard to use of the base
 *    software.
 * Permission to distribute the released version of the source code along
 * with corresponding source modifications in the form of a patch file is
 * granted with same provisions 2 through 4 for binary distributions.
 *
 * This software is provided "as is" without express or implied warranty
 * to the extent permitted by applicable law.
]*/

#include <sys/types.h>
#include <dirent.h>

#include "util.h"

#include "alloc.h"
#include "command.h"
#include "datafile.h"		/* for df_showdata */
#include "misc.h"
#include "plot.h"
/*  #include "setshow.h" */		/* for month names etc */
#include "term_api.h"		/* for term_end_plot() used by graph_error() */

/* Exported (set-table) variables */

/* decimal sign */
char *decimalsign = NULL;


/* internal prototypes */

static void parse_esc __PROTO((char *instr));
static void mant_exp __PROTO((double, double, TBOOLEAN, double *, int *, const char *));

/*
 * chr_in_str() compares the characters in the string of token number t_num
 * with c, and returns TRUE if a match was found.
 */
int
chr_in_str(t_num, c)
int t_num;
int c;
{
    register int i;

    if (!token[t_num].is_token)
	return (FALSE);		/* must be a value--can't be equal */
    for (i = 0; i < token[t_num].length; i++) {
	if (input_line[token[t_num].start_index + i] == c)
	    return (TRUE);
    }
    return FALSE;
}


/*
 * equals() compares string value of token number t_num with str[], and
 *   returns TRUE if they are identical.
 */
int
equals(t_num, str)
int t_num;
const char *str;
{
    register int i;

    if (!token[t_num].is_token)
	return (FALSE);		/* must be a value--can't be equal */
    for (i = 0; i < token[t_num].length; i++) {
	if (input_line[token[t_num].start_index + i] != str[i])
	    return (FALSE);
    }
    /* now return TRUE if at end of str[], FALSE if not */
    return (str[i] == NUL);
}



/*
 * almost_equals() compares string value of token number t_num with str[], and
 *   returns TRUE if they are identical up to the first $ in str[].
 */
int
almost_equals(t_num, str)
int t_num;
const char *str;
{
    register int i;
    register int after = 0;
    register int start = token[t_num].start_index;
    register int length = token[t_num].length;

    if (!str)
	return FALSE;
    if (!token[t_num].is_token)
	return FALSE;		/* must be a value--can't be equal */
    for (i = 0; i < length + after; i++) {
	if (str[i] != input_line[start + i]) {
	    if (str[i] != '$')
		return (FALSE);
	    else {
		after = 1;
		start--;	/* back up token ptr */
	    }
	}
    }

    /* i now beyond end of token string */

    return (after || str[i] == '$' || str[i] == NUL);
}



int
isstring(t_num)
int t_num;
{

    return (token[t_num].is_token &&
	    (input_line[token[t_num].start_index] == '\'' ||
	     input_line[token[t_num].start_index] == '"'));
}


int
isanumber(t_num)
int t_num;
{
    return (!token[t_num].is_token);
}


int
isletter(t_num)
int t_num;
{
    return (token[t_num].is_token &&
	    ((isalpha((unsigned char) input_line[token[t_num].start_index])) ||
	     (input_line[token[t_num].start_index] == '_')));
}


/*
 * is_definition() returns TRUE if the next tokens are of the form
 *   identifier =
 *              -or-
 *   identifier ( identifer {,identifier} ) =
 */
int
is_definition(t_num)
int t_num;
{
    /* variable? */
    if (isletter(t_num) && equals(t_num + 1, "="))
	return 1;

    /* function? */
    /* look for dummy variables */
    if (isletter(t_num) && equals(t_num + 1, "(") && isletter(t_num + 2)) {
	t_num += 3;		/* point past first dummy */
	while (equals(t_num, ",")) {
	    if (!isletter(++t_num))
		return 0;
	    t_num += 1;
	}
	return (equals(t_num, ")") && equals(t_num + 1, "="));
    }
    /* neither */
    return 0;
}



/*
 * copy_str() copies the string in token number t_num into str, appending
 *   a null.  No more than max chars are copied (including \0).
 */
void
copy_str(str, t_num, max)
char *str;
int t_num;
int max;
{
    register int i = 0;
    register int start = token[t_num].start_index;
    register int count = token[t_num].length;

    if (count >= max) {
	count = max - 1;
	FPRINTF((stderr, "str buffer overflow in copy_str"));
    }

    do {
	str[i++] = input_line[start++];
    } while (i != count);
    str[i] = NUL;

}

/* length of token string */
size_t
token_len(t_num)
int t_num;
{
    return (size_t)(token[t_num].length);
}

/*
 * quote_str() does the same thing as copy_str, except it ignores the
 *   quotes at both ends.  This seems redundant, but is done for
 *   efficency.
 */
void
quote_str(str, t_num, max)
char *str;
int t_num;
int max;
{
    register int i = 0;
    register int start = token[t_num].start_index + 1;
    register int count;

    if ((count = token[t_num].length - 2) >= max) {
	count = max - 1;
	FPRINTF((stderr, "str buffer overflow in quote_str"));
    }
    if (count > 0) {
	do {
	    str[i++] = input_line[start++];
	} while (i != count);
    }
    str[i] = NUL;
    /* convert \t and \nnn (octal) to char if in double quotes */
    if (input_line[token[t_num].start_index] == '"')
	parse_esc(str);
}


/*
 * capture() copies into str[] the part of input_line[] which lies between
 * the begining of token[start] and end of token[end].
 */
void
capture(str, start, end, max)
char *str;
int start, end;
int max;
{
    register int i, e;

    e = token[end].start_index + token[end].length;
    if (e - token[start].start_index >= max) {
	e = token[start].start_index + max - 1;
	FPRINTF((stderr, "str buffer overflow in capture"));
    }
    for (i = token[start].start_index; i < e && input_line[i] != NUL; i++)
	*str++ = input_line[i];
    *str = NUL;
}


/*
 * m_capture() is similar to capture(), but it mallocs storage for the
 * string.
 */
void
m_capture(str, start, end)
char **str;
int start, end;
{
    register int i, e;
    register char *s;

    e = token[end].start_index + token[end].length;
    *str = gp_realloc(*str, (e - token[start].start_index + 1), "string");
    s = *str;
    for (i = token[start].start_index; i < e && input_line[i] != NUL; i++)
	*s++ = input_line[i];
    *s = NUL;
}


/*
 * m_quote_capture() is similar to m_capture(), but it removes
 * quotes from either end of the string.
 */
void
m_quote_capture(str, start, end)
char **str;
int start, end;
{
    register int i, e;
    register char *s;

    e = token[end].start_index + token[end].length - 1;
    *str = gp_realloc(*str, (e - token[start].start_index + 1), "string");
    s = *str;
    for (i = token[start].start_index + 1; i < e && input_line[i] != NUL; i++)
	*s++ = input_line[i];
    *s = NUL;

    if (input_line[token[start].start_index] == '"')
	parse_esc(*str);

}


/* Our own version of strdup()
 * Make copy of string into gp_alloc'd memory
 * As with all conforming str*() functions,
 * it is the caller's responsibility to pass
 * valid parameters!
 */
char *
gp_strdup(s)
const char *s;
{
    char *d;

#ifndef HAVE_STRDUP
    d = gp_alloc(strlen(s) + 1, "gp_strdup");
    if (d)
	memcpy (d, s, strlen(s) + 1);
#else
    d = strdup(s);
#endif
    return d;
}


/* HBB 20020405: moved these functions here from axis.c, where they no
 * longer truly belong. */
/*{{{  mant_exp - split into mantissa and/or exponent */
/* HBB 20010121: added code that attempts to fix rounding-induced
 * off-by-one errors in 10^%T and similar output formats */
static void
mant_exp(log10_base, x, scientific, m, p, format)
    double log10_base, x;
    TBOOLEAN scientific;	/* round to power of 3 */
    double *m;			/* results */
    int *p;			
    const char *format;		/* format string for fixup */
{
    int sign = 1;
    double l10;
    int power;
    double mantissa;

    /*{{{  check 0 */
    if (x == 0) {
	if (m)
	    *m = 0;
	if (p)
	    *p = 0;
	return;
    }
    /*}}} */
    /*{{{  check -ve */
    if (x < 0) {
	sign = (-1);
	x = (-x);
    }
    /*}}} */

    l10 = log10(x) / log10_base;
    power = floor(l10);
    mantissa = pow(10.0, (l10 - power) * log10_base);

    /* round power to an integer multiple of 3, to get what's
     * sometimes called 'scientific' or 'engineering' notation. Also
     * useful for handling metric unit prefixes like 'kilo' or 'micro'
     * */
    /* HBB 20010121: avoid recalculation of mantissa via pow() */
    if (scientific) {
	int temp_power  = 3 * floor(power / 3.0);
	switch (power - temp_power) {
	case 2:
	    mantissa *= 100; break;
	case 1:
	    mantissa *= 10; break;
	case 0:
	    break;
	default:
	    int_error (NO_CARET, "Internal error in scientific number formatting");
	}
	power = temp_power;
    }

    /* HBB 20010121: new code for decimal mantissa fixups.  Looks at
     * format string to see how many decimals will be put there.  Iff
     * the number is so close to an exact power of ten that it will be
     * rounded up to 10.0e??? by an sprintf() with that many digits of
     * precision, increase the power by 1 to get a mantissa in the
     * region of 1.0.  If this handling is not wanted, pass NULL as
     * the format string */
    if (format) {
	double upper_border = scientific ? 1000 : 10;
	int precision = 0;

	format = strchr (format, '.');
	if (format != NULL)
	    /* a decimal point was found in the format, so use that 
	     * precision. */
	    precision = strtol(format + 1, NULL, 10);
	
	/* See if mantissa would be right on the border.  All numbers
	 * greater than that will be rounded up to 10, by sprintf(), which
	 * we want to avoid. */
	if (mantissa > (upper_border - pow(10.0, -precision) / 2)
	    ) {
	    mantissa /= upper_border;
	    power += (scientific ? 3 : 1);
	}
    }

    if (m)
	*m = sign * mantissa;
    if (p)
	*p = power;
}

/*}}} */

/*
 * Kludge alert!!
 * Workaround until we have a better solution ...
 * Note: this assumes that all calls to sprintf in gprintf have
 * exactly three args. Lars
 */
#ifdef HAVE_SNPRINTF
# define sprintf(str,fmt,arg) \
    if (snprintf((str),count,(fmt),(arg)) > count) \
      fprintf (stderr,"%s:%d: Warning: too many digits for format\n",__FILE__,__LINE__)
#endif

/*{{{  gprintf */
/* extended s(n)printf */
/* HBB 20010121: added code to maintain consistency between mantissa
 * and exponent across sprintf() calls.  The problem: format string
 * '%t*10^%T' will display 9.99 as '10.0*10^0', but 10.01 as
 * '1.0*10^1'.  This causes problems for people using the %T part,
 * only, with logscaled axes, in combination with the occasional
 * round-off error. */
void
gprintf(dest, count, format, log10_base, x)
    char *dest, *format;
    size_t count;
    double log10_base, x;
{
    char temp[MAX_LINE_LEN + 1];
    char *t;
    TBOOLEAN seen_mantissa = FALSE; /* memorize if mantissa has been
                                       output, already */
    int stored_power = 0;	/* power that matches the mantissa
                                   output earlier */

    for (;;) {
	/*{{{  copy to dest until % */
	while (*format != '%')
	    if (!(*dest++ = *format++))
		return;		/* end of format */
	/*}}} */

	/*{{{  check for %% */
	if (format[1] == '%') {
	    *dest++ = '%';
	    format += 2;
	    continue;
	}
	/*}}} */

	/*{{{  copy format part to temp, excluding conversion character */
	t = temp;
	*t++ = '%';
	/* dont put isdigit first since sideeffect in macro is bad */
	while (*++format == '.' || isdigit((unsigned char) *format)
	       || *format == '-' || *format == '+' || *format == ' ')
	    *t++ = *format;
	/*}}} */

	/*{{{  convert conversion character */
	switch (*format) {
	    /*{{{  x and o */
	case 'x':
	case 'X':
	case 'o':
	case 'O':
	    t[0] = *format;
	    t[1] = 0;
	    sprintf(dest, temp, (int) x);
	    break;
	    /*}}} */
	    /*{{{  e, f and g */
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
	    t[0] = *format;
	    t[1] = 0;
	    sprintf(dest, temp, x);
	    break;
	    /*}}} */
	    /*{{{  l --- mantissa to current log base */
	case 'l':
	    {
		double mantissa;

		t[0] = 'f';
		t[1] = 0;
		mant_exp(log10_base, x, FALSE, &mantissa, &stored_power, temp);
		seen_mantissa = TRUE;
		sprintf(dest, temp, mantissa);
		break;
	    }
	    /*}}} */
	    /*{{{  t --- base-10 mantissa */
	case 't':
	    {
		double mantissa;

		t[0] = 'f';
		t[1] = 0;
		mant_exp(1.0, x, FALSE, &mantissa, &stored_power, temp);
		seen_mantissa = TRUE;
		sprintf(dest, temp, mantissa);
		break;
	    }
	    /*}}} */
	    /*{{{  s --- base-1000 / 'scientific' mantissa */
	case 's':
	    {
		double mantissa;

		t[0] = 'f';
		t[1] = 0;
		mant_exp(1.0, x, TRUE, &mantissa, &stored_power, temp);
		seen_mantissa = TRUE;
		sprintf(dest, temp, mantissa);
		break;
	    }
	    /*}}} */
	    /*{{{  L --- power to current log base */
	case 'L':
	    {
		int power;

		t[0] = 'd';
		t[1] = 0;
		if (seen_mantissa)
		    power = stored_power;
		else
		    mant_exp(log10_base, x, FALSE, NULL, &power, "%.0f");
		sprintf(dest, temp, power);
		break;
	    }
	    /*}}} */
	    /*{{{  T --- power of ten */
	case 'T':
	    {
		int power;

		t[0] = 'd';
		t[1] = 0;
		if (seen_mantissa)
		    power = stored_power;
		else 
		    mant_exp(1.0, x, FALSE, NULL, &power, "%.0f");
		sprintf(dest, temp, power);
		break;
	    }
	    /*}}} */
	    /*{{{  S --- power of 1000 / 'scientific' */
	case 'S':
	    {
		int power;

		t[0] = 'd';
		t[1] = 0;
		if (seen_mantissa)
		    power = stored_power;
		else 
		    mant_exp(1.0, x, TRUE, NULL, &power, "%.0f");
		sprintf(dest, temp, power);
		break;
	    }
	    /*}}} */
	    /*{{{  c --- ISO decimal unit prefix letters */
	case 'c':
	    {
		int power;

		t[0] = 'c';
		t[1] = 0;
		if (seen_mantissa)
		    power = stored_power;
		else 
		    mant_exp(1.0, x, TRUE, NULL, &power, "%.0f");

		if (power >= -18 && power <= 18) {
		    /* -18 -> 0, 0 -> 6, +18 -> 12, ... */
		    /* HBB 20010121: avoid division of -ve ints! */
		    power = (power + 18) / 3;
		    sprintf(dest, temp, "afpnum kMGTPE"[power]);
		} else {
		    /* please extend the range ! */
		    /* name  power   name  power
		       -------------------------
		       atto   -18    Exa    18
		       femto  -15    Peta   15
		       pico   -12    Tera   12
		       nano    -9    Giga    9
		       micro   -6    Mega    6
		       milli   -3    kilo    3   */

		    /* for the moment, print e+21 for example */
		    sprintf(dest, "e%+02d", (power - 6) * 3);
		}

		break;
	    }
	    /*}}} */
	    /*{{{  P --- multiple of pi */
	case 'P':
	    {
		t[0] = 'f';
		t[1] = 0;
		sprintf(dest, temp, x / M_PI);
		break;
	    }
	    /*}}} */
	default:
	    int_error(NO_CARET, "Bad format character");
	} /* switch */
	/*}}} */

    /* change decimal `.' to the actual entry in decimalsign */
	if (decimalsign != NULL) {
	    char *dotpos1 = dest, *dotpos2;
	    size_t newlength = strlen(decimalsign);

	    /* replace every `.' by the contents of decimalsign */
	    while ((dotpos2 = strchr(dotpos1,'.')) != NULL) {
		size_t taillength = strlen(dotpos2);

		dotpos1 = dotpos2 + newlength;
		/* test if the new value for dest would be too long */
		if (dotpos1 - dest + taillength > count)
		    int_error(NO_CARET,
			      "format too long due to long decimalsign string");
		/* move tail end of string out of the way */
		memmove(dotpos1, dotpos2 + 1, taillength);
		/* insert decimalsign */
		memcpy(dotpos2, decimalsign, newlength);
	    }
	    /* clear temporary variables for safety */
	    dotpos1=NULL;
	    dotpos2=NULL;
	}

	/* this was at the end of every single case, before: */
	dest += strlen(dest);
	++format;
    } /* for ever */
}

/*}}} */
#ifdef HAVE_SNPRINTF
# undef sprintf
#endif

/* some macros for the error and warning functions below
 * may turn this into a utility function later
 */
#define PRINT_SPACES_UNDER_PROMPT \
{ register size_t i; \
  for (i = 0; i < sizeof(PROMPT) - 1; i++) \
   (void) fputc(' ', stderr); \
}

#define PRINT_SPACES_UPTO_TOKEN \
{ register int i; \
   for (i = 0; i < token[t_num].start_index; i++) \
    (void) fputc((input_line[i] == '\t') ? '\t' : ' ', stderr); \
}

#define PRINT_CARET fputs("^\n",stderr);

#define PRINT_FILE_AND_LINE \
 if (!interactive) { \
        if (infile_name != NULL) \
            fprintf(stderr, "\"%s\", line %d: ", infile_name, inline_num); \
        else fprintf(stderr, "line %d: ", inline_num); \
 }

/* TRUE if command just typed; becomes FALSE whenever we
 * send some other output to screen.  If FALSE, the command line
 * will be echoed to the screen before the ^ error message.
 */
TBOOLEAN screen_ok;

#if defined(VA_START) && defined(STDC_HEADERS)
void
os_error(int t_num, const char *str,...)
#else
void
os_error(t_num, str, va_alist)
int t_num;
const char *str;
va_dcl
#endif
{
#ifdef VA_START
    va_list args;
#endif
#ifdef VMS
    static status[2] = { 1, 0 };		/* 1 is count of error msgs */
#endif /* VMS */

    /* reprint line if screen has been written to */

    if (t_num == DATAFILE) {
	df_showdata();
    } else if (t_num != NO_CARET) {	/* put caret under error */
	if (!screen_ok)
	    fprintf(stderr, "\n%s%s\n", PROMPT, input_line);
	
	PRINT_SPACES_UNDER_PROMPT;
	PRINT_SPACES_UPTO_TOKEN;
	PRINT_CARET;
    }
    PRINT_SPACES_UNDER_PROMPT;

#ifdef VA_START
    VA_START(args, str);
# if defined(HAVE_VFPRINTF) || _LIBC
    vfprintf(stderr, str, args);
# else
    _doprnt(str, args, stderr);
# endif
    va_end(args);
#else
    fprintf(stderr, str, a1, a2, a3, a4, a5, a6, a7, a8);
#endif
    putc('\n', stderr);

    PRINT_SPACES_UNDER_PROMPT;
    PRINT_FILE_AND_LINE;

#ifdef VMS
    status[1] = vaxc$errno;
    sys$putmsg(status);
    (void) putc('\n', stderr);
#else /* VMS */
    perror("util.c");
    putc('\n', stderr);
#endif /* VMS */

    bail_to_command_line();
}


#if defined(VA_START) && defined(STDC_HEADERS)
void
int_error(int t_num, const char *str,...)
#else
void
int_error(t_num, str, va_alist)
int t_num;
const char str[];
va_dcl
#endif
{
#ifdef VA_START
    va_list args;
#endif

    /* reprint line if screen has been written to */

    if (t_num == DATAFILE) {
        df_showdata();
    } else if (t_num != NO_CARET) { /* put caret under error */
	if (!screen_ok)
	    fprintf(stderr, "\n%s%s\n", PROMPT, input_line);

	PRINT_SPACES_UNDER_PROMPT;
	PRINT_SPACES_UPTO_TOKEN;
	PRINT_CARET;
    }
    PRINT_SPACES_UNDER_PROMPT;
    PRINT_FILE_AND_LINE;

#ifdef VA_START
    VA_START(args, str);
# if defined(HAVE_VFPRINTF) || _LIBC
    vfprintf(stderr, str, args);
# else
    _doprnt(str, args, stderr);
# endif
    va_end(args);
#else
    fprintf(stderr, str, a1, a2, a3, a4, a5, a6, a7, a8);
#endif
    fputs("\n\n", stderr);

    bail_to_command_line();
}

/* Warn without bailing out to command line. Not a user error */
#if defined(VA_START) && defined(STDC_HEADERS)
void
int_warn(int t_num, const char *str,...)
#else
void
int_warn(t_num, str, va_alist)
int t_num;
const char str[];
va_dcl
#endif
{
#ifdef VA_START
    va_list args;
#endif

    /* reprint line if screen has been written to */

    if (t_num == DATAFILE) {
        df_showdata();
    } else if (t_num != NO_CARET) { /* put caret under error */
	if (!screen_ok)
	    fprintf(stderr, "\n%s%s\n", PROMPT, input_line);

	PRINT_SPACES_UNDER_PROMPT;
	PRINT_SPACES_UPTO_TOKEN;
	PRINT_CARET;
    }
    PRINT_SPACES_UNDER_PROMPT;
    PRINT_FILE_AND_LINE;

    fputs("warning: ", stderr);
#ifdef VA_START
    VA_START(args, str);
# if defined(HAVE_VFPRINTF) || _LIBC
    vfprintf(stderr, str, args);
# else
    _doprnt(str, args, stderr);
# endif
    va_end(args);
#else
    fprintf(stderr, str, a1, a2, a3, a4, a5, a6, a7, a8);
#endif
    putc('\n', stderr);
}				/* int_warn */

/*{{{  graph_error() */
/* handle errors during graph-plot in a consistent way */
/* HBB 20000430: move here, from graphics.c */
#if defined(VA_START) && defined(STDC_HEADERS)
void
graph_error(const char *fmt, ...)
#else
void
graph_error(fmt, va_alist)
    const char *fmt;
    va_dcl
#endif
{
#ifdef VA_START
    va_list args;
#endif

    multiplot = FALSE;
    term_end_plot();

#ifdef VA_START
    VA_START(args, fmt);
#if 0 
    /* HBB 20001120: this seems not to work at all. Probably because a
     * va_list argument, is, after all, something else than a varargs
     * list (i.e. a '...') */
    int_error(NO_CARET, fmt, args);
#else
    /* HBB 20001120: instead, copy the core code from int_error() to
     * here: */
    PRINT_SPACES_UNDER_PROMPT;
    PRINT_FILE_AND_LINE;

# if defined(HAVE_VFPRINTF) || _LIBC
    vfprintf(stderr, fmt, args);
# else
    _doprnt(fmt, args, stderr);
# endif
    va_end(args);
    fputs("\n\n", stderr);

    bail_to_command_line();
#endif /* 1/0 */
    va_end(args);
#else
    int_error(NO_CARET, fmt, a1, a2, a3, a4, a5, a6, a7, a8);
#endif

}

/*}}} */


/* Lower-case the given string (DFK) */
/* Done in place. */
void
lower_case(s)
char *s;
{
    register char *p = s;

    while (*p++) {
	if (isupper((unsigned char)*p))
	    *p = tolower((unsigned char)*p);
    }
}

/* Squash spaces in the given string (DFK) */
/* That is, reduce all multiple white-space chars to single spaces */
/* Done in place. */
void
squash_spaces(s)
char *s;
{
    register char *r = s;	/* reading point */
    register char *w = s;	/* writing point */
    TBOOLEAN space = FALSE;	/* TRUE if we've already copied a space */

    for (w = r = s; *r != NUL; r++) {
	if (isspace((unsigned char) *r)) {
	    /* white space; only copy if we haven't just copied a space */
	    if (!space) {
		space = TRUE;
		*w++ = ' ';
	    }			/* else ignore multiple spaces */
	} else {
	    /* non-space character; copy it and clear flag */
	    *w++ = *r;
	    space = FALSE;
	}
    }
    *w = NUL;			/* null terminate string */
}


static void
parse_esc(instr)
char *instr;
{
    char *s = instr, *t = instr;

    /* the string will always get shorter, so we can do the
     * conversion in situ
     */

    while (*s != NUL) {
	if (*s == '\\') {
	    s++;
	    if (*s == '\\') {
		*t++ = '\\';
		s++;
	    } else if (*s == 'n') {
		*t++ = '\n';
		s++;
	    } else if (*s == 'r') {
		*t++ = '\r';
		s++;
	    } else if (*s == 't') {
		*t++ = '\t';
		s++;
	    } else if (*s == '\"') {
		*t++ = '\"';
		s++;
	    } else if (*s >= '0' && *s <= '7') {
		int i, n;
		if (sscanf(s, "%o%n", &i, &n) > 0) {
		    *t++ = i;
		    s += n;
		} else {
		    /* int_error("illegal octal number ", c_token); */
		    *t++ = '\\';
		    *t++ = *s++;
		}
	    }
	} else {
	    *t++ = *s++;
	}
    }
    *t = NUL;
}

/* FIXME HH 20020915: This function does nothing if dirent.h and windows.h 
 * not available. */
TBOOLEAN 
existdir (name)
     const char *name;
{
#ifdef HAVE_DIRENT_H
    DIR *dp;
    if (! (dp = opendir(name) ) )
	return FALSE;
    
    closedir(dp);
    return TRUE;
#elif defined(_Windows) || defined(MY_Windows)
    HANDLE FileHandle;
    WIN32_FIND_DATA finddata;

    FileHandle = FindFirstFile(name, &finddata);
    if (FileHandle != INVALID_HANDLE_VALUE) {
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    return TRUE;
    }
    return FALSE;
#else
    int_warn(NO_CARET,
	     "Test on directory existence not supported\n\t('%s!')",
	     name);
    return FALSE;
#endif
}

