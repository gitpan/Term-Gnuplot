/*
 * $Id: syscfg.h,v 1.24 2002/03/09 17:48:39 lhecking Exp $
 */

/* GNUPLOT - syscfg.h */

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

/* This header file provides system dependent definitions. New features
 * and platforms should be added here.
 */

#ifndef SYSCFG_H
#define SYSCFG_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sys/types.h"
#include "sys/stat.h"
/*
 * Define operating system dependent constants [default value]:
 *
 * OS:       [""] Name of OS; only required if system has no uname(2) call
 * HELPFILE: ["docs/gnuplot.gih"] Location of helpfile - overridden by Makefile
 * HOME:     ["HOME"] Name of environment variable which points to
 *           the directory where gnuplot's config file is found.
 * PLOTRC:   [".gnuplot"] Name of the gnuplot startup file.
 * SHELL:    ["/bin/sh"] Name, and in some cases, full path to the shell
 *           that is used to run external commands.
 * DIRSEP1:  ['/'] Primary character which separates path components.
 * DIRSEP2:  ['\0'] Secondary character which separates path components.
 * PATHSEP:  [':'] Character which separates path names
 *        
 */

#if defined(AMIGA_SC_6_1) || defined(AMIGA_AC_5) || defined(__amigaos__)
# ifndef __amigaos__
#  define OS "Amiga"
#  define HELPFILE "S:gnuplot.gih"
#  define HOME     "GNUPLOT"
#  define SHELL    "NewShell"
#  define DIRSEP2  ':'
#  define PATHSEP  ';'
# endif
# ifndef AMIGA
#  define AMIGA
# endif
/* Fake S_IFIFO for SAS/C
 * See stdfn.h for details
 */
# ifdef AMIGA_SC_6_1
#  define S_IFIFO S_IREAD
# endif
#endif /* Amiga */

#ifdef ATARI
# define OS      "TOS"
# define HOME     "GNUPLOT"
# define PLOTRC   "gnuplot.ini"
# define SHELL    "gulam.prg"
# define DIRSEP1  '\\'
# ifdef MTOS
#  define DIRSEP2 '/'
# endif
#endif /* Atari */
/* FIXME: may need to be ifdef'd for ATARI/MTOS */
#ifdef __PUREC__
# define sscanf purec_sscanf
# define GP_MATHERR purec_matherr
#endif

#ifdef DOS386
# define OS       "DOS 386"
# define HELPFILE "gnuplot.gih"
# define HOME     "GNUPLOT"
# define PLOTRC   "gnuplot.ini"
# define SHELL    "\\command.com"
# define DIRSEP1  '\\'
# define PATHSEP  ';'
#endif /* DOS386 */

#if defined(__NeXT__) || defined(NEXT)
# ifndef NEXT
#  define NEXT
# endif
#endif /* NeXT */

#ifdef OS2
# define OS       "OS/2"
# undef HELPFILE
# define HELPFILE "gnuplot.gih"
# define HOME     "GNUPLOT"
# define PLOTRC   "gnuplot.ini"
# define SHELL    "c:\\os2\\cmd.exe"
# define DIRSEP1  '\\'
# define PATHSEP  ';'
#endif /* OS/2 */

#ifdef OSK
# define OS    "OS-9"
# define SHELL "/dd/cmds/shell"
#endif /* OS-9 */

#if defined(vms) || defined(VMS)
# define OS "VMS"
# ifndef VMS
#  define VMS
# endif
# define HOME   "sys$login:"
# define PLOTRC "gnuplot.ini"
# ifdef NO_GIH
   /* for show version long */
#  define HELPFILE "GNUPLOT$HELP"
# endif
# if !defined(VAXCRTL) && !defined(DECCRTL)
#  define VAXCRTL VAXCRTL_AND_DECCRTL_UNDEFINED
#  define DECCRTL VAXCRTL_AND_DECCRTL_UNDEFINED
# endif
/* avoid some IMPLICITFUNC warnings */
# ifdef __DECC
#  include <starlet.h>
# endif  /* __DECC */
#endif /* VMS */

#if defined(_WINDOWS) || defined(_Windows)
# ifndef _Windows
#  define _Windows
# endif
# ifdef WIN32
#  define OS "MS-Windows 32 bit"
/* introduced by Pedro Mendes, prm@aber.ac.uk */
#  define far
/* Fix for broken compiler headers
 * See stdfn.h
 */
#  define S_IFIFO  _S_IFIFO
# else
#  define OS "MS-Windows"
#  ifndef WIN16
#   define WIN16
#  endif
# endif /* WIN32 */
# define HOME    "GNUPLOT"
# define PLOTRC  "gnuplot.ini"
# define SHELL   "\\command.com"
# define DIRSEP1 '\\'
# define PATHSEP ';'
#endif /* _WINDOWS */

#if defined(MSDOS) && !defined(_Windows)
# if !defined(DOS32) && !defined(DOS16)
#  define DOS16
# endif
/* should this be here ? */
# ifdef MTOS
#  define OS "TOS & MiNT & MULTITOS & Magic - "
# endif /* MTOS */
# define OS       "MS-DOS"
# undef HELPFILE
# define HELPFILE "gnuplot.gih"
# define HOME     "GNUPLOT"
# define PLOTRC   "gnuplot.ini"
# define SHELL    "\\command.com"
# define DIRSEP1  '\\'
# define PATHSEP  ';'
# ifdef __DJGPP__
#  define DIRSEP2 '/'
# endif
#endif /* MSDOS */

/* End OS dependent constants; fall-through defaults
 * for the constants defined above are following.
 */

#ifndef HELPFILE
# define HELPFILE "docs/gnuplot.gih"
#endif

#ifndef HOME
# define HOME "HOME"
#endif

#ifndef PLOTRC
# define PLOTRC ".gnuplot"
#endif

#ifndef SHELL
# define SHELL "/bin/sh"    /* used if SHELL env variable not set */
#endif

#ifndef DIRSEP1
# define DIRSEP1 '/'
#endif

#ifndef DIRSEP2
# define DIRSEP2 NUL
#endif

#ifndef PATHSEP
# define PATHSEP ':'
#endif

#ifndef FAQ_LOCATION
#define FAQ_LOCATION "http://www.gnuplot.info/faq/"
/*
#define FAQ_LOCATION "http://www.gnuplot.info/gnuplot-faq.html"
#define FAQ_LOCATION "http://www.ucc.ie/gnuplot/gnuplot-faq.html"
*/
#endif

#ifndef CONTACT
# define CONTACT "bug-gnuplot@dartmouth.edu"
#endif

#ifndef HELPMAIL
# define HELPMAIL "info-gnuplot@dartmouth.edu"
#endif
/* End fall-through defaults */

/* Need this before any headers are incldued */
#ifdef PROTOTYPES
# define __PROTO(proto) proto
#else
# define __PROTO(proto) ()
#endif

/* Atari stuff. Moved here from command.c, plot2d.c, readline.c */
#if defined(ATARI) || defined(MTOS)
# ifdef __PUREC__
#  include <ext.h>
#  include <tos.h>
#  include <aes.h>
# else
#  include <osbind.h>
#  include <aesbind.h>
#  include <support.h>
# endif                         /* __PUREC__ */
#endif /* ATARI || MTOS */


/* DOS/Windows stuff. Moved here from command.c */
#if defined(MSDOS) || defined(DOS386)

# ifdef DJGPP
#  include <dos.h>
#  include <dir.h>              /* HBB: for setdisk() */
# else
#  include <process.h>
# endif                         /* !DJGPP */

# ifdef __ZTC__
#  define HAVE_SLEEP 1
#  define P_WAIT 0

# elif defined(__TURBOC__)
#  include <dos.h>		/* for sleep() prototype */
#  ifndef _Windows
#   define HAVE_SLEEP 1
#   include <conio.h>
#   include <dir.h>            /* setdisk() */
#  endif                       /* _Windows */
#  ifdef WIN32
#   define HAVE_SLEEP 1
#  endif

# else                         /* must be MSC */
#  if !defined(__EMX__) && !defined(DJGPP)
#   ifdef __MSC__
#    include <direct.h>        /* for _chdrive() */
#   endif                      /* __MSC__ */
#  endif                       /* !__EMX__ && !DJGPP */
# endif                        /* !ZTC */

#endif /* MSDOS */

/* Watcom's compiler; this should probably be somewhere
 * in the Windows section
 */
#ifdef __WATCOMC__
# include <direct.h>
# define HAVE_GETCWD 1
#endif


/* Misc platforms */
#ifdef apollo
# ifndef APOLLO
#  define APOLLO
# endif
# define GPR
#endif

#if defined(APOLLO) || defined(alliant)
# undef HAVE_LIMITS_H
#endif

#ifdef sequent
# undef HAVE_LIMITS_H
# undef HAVE_STRCHR
#endif

#ifdef unixpc
# ifndef UNIXPC
#  define UNIXPC
# endif
#endif

/* HBB 20000416: stuff moved from plot.h to here. It's system-dependent,
 * so it belongs here, IMHO */

/* To access curves larger than 64k, MSDOS needs to use huge pointers */
#if (defined(__TURBOC__) && defined(MSDOS)) || defined(WIN16)
# define GPHUGE huge
# define GPFAR far
#else /* not TurboC || WIN16 */
# define GPHUGE /* nothing */
# define GPFAR /* nothing */
#endif /* not TurboC || WIN16 */

#if defined(DOS16) || defined(WIN16)
typedef float coordval;		/* memory is tight on PCs! */
# define COORDVAL_FLOAT 1
#else
typedef double coordval;
#endif

/* Set max. number of arguments in a user-defined function */
#ifdef DOS16
# define MAX_NUM_VAR	3
#else
# define MAX_NUM_VAR	5
#endif

/* HBB 20010223: Moved VERYLARGE definition to stdfn.h: it can only be
 * resolved correctly after #include <float.h>, which is done there,
 * not here. */

#ifdef VMS
# define is_comment(c) ((c) == '#' || (c) == '!')
# define is_system(c) ((c) == '$')
/* maybe configure could check this? */
# define BACKUP_FILESYSTEM 1
#else /* not VMS */
# define is_comment(c) ((c) == '#')
# define is_system(c) ((c) == '!')
#endif /* not VMS */

#ifndef RETSIGTYPE
/* assume ANSI definition by default */
# define RETSIGTYPE void
#endif

#ifndef SIGFUNC_NO_INT_ARG
typedef RETSIGTYPE (*sigfunc)__PROTO((int));
#else
typedef RETSIGTYPE (*sigfunc)__PROTO((void));
#endif

#ifdef HAVE_SIGSETJMP
# define SETJMP(env, save_signals) sigsetjmp(env, save_signals)
# define LONGJMP(env, retval) siglongjmp(env, retval)
# define JMP_BUF sigjmp_buf
#else
# define SETJMP(env, save_signals) setjmp(env)
# define LONGJMP(env, retval) longjmp(env, retval)
# define JMP_BUF jmp_buf
#endif

/* generic pointer type. For old compilers this has to be changed to char *,
 * but I don't know if there are any CC's that support void and not void *
 */
#define generic void

/* HBB 20010720: removed 'sortfunc' --- it's no longer used */
/* FIXME HBB 20010720: Where is SORTFUNC_ARGS supposed to be defined?  */
#ifndef SORTFUNC_ARGS
#define SORTFUNC_ARGS const generic *
#endif

/* Macros for string concatenation */
#ifdef HAVE_STRINGIZE
/* ANSI version */
# define CONCAT(x,y) x##y
# define CONCAT3(x,y,z) x##y##z
#else
/* K&R version */
# define CONCAT(x,y) x/**/y
# define CONCAT3(x,y,z) x/**/y/**/z
#endif

/* Windows needs to redefine stdin/stdout functions */
#if defined(_Windows) && !defined(WINDOWS_NO_GUI)
# include "win/wtext.h"
#endif

#ifndef GP_EXCEPTION_NAME
# define GP_EXCEPTION_NAME exception
#endif

#ifndef GP_MATHERR
# define GP_MATHERR matherr
#endif

#ifdef HAVE_STRUCT_EXCEPTION_IN_MATH_H
# define STRUCT_EXCEPTION_P_X struct GP_EXCEPTION_NAME *x
#else
# define STRUCT_EXCEPTION_P_X /* nothing */
#endif

/* if GP_INLINE has not yet been defined, set to __inline__ for gcc,
 * nothing. I'd prefer that any other compilers have the defn in
 * the makefile, rather than having a huge list of compilers here.
 * But gcc is sufficiently ubiquitous that I'll allow it here !!!
 */
#ifndef GP_INLINE
# ifdef __GNUC__
#  define GP_INLINE __inline__
# else
#  define GP_INLINE /*nothing*/
# endif
#endif

/* avoid precompiled header conflict with redefinition */
#ifdef NEXT
# include <mach/boolean.h>
#else
/* Sheer, raging paranoia */
# ifdef TRUE
#  undef TRUE
# endif
# ifdef FALSE
#  undef FALSE
# endif
# define TRUE 1
# define FALSE 0
#endif

#ifndef __cplusplus
#undef bool
typedef unsigned int bool;
#endif

/* TRUE or FALSE */
#define TBOOLEAN bool

#endif /* !SYSCFG_H */
