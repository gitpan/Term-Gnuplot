/* -*- C -*-
 * FILE: "/disk01/home/joze/pub/gnuplot-Oct-31-99/src/ipc.h"
 * LAST MODIFICATION: "Sun Oct 31 04:13:41 1999 (joze)"
 * 1999 by Johannes Zellner, <johannes@zellner.org>
 * $Id:$
 */

#ifndef _IPC_H
#define _IPC_H

/*
 * currently only used for X11 && USE_MOUSE, but in principle
 * every term could use this file descriptor to write back
 * commands to gnuplot.  Note, that terminals using this fd
 * should set it to a negative value when closing. (joze)
 */
#ifdef _TERM_C
    int ipc_back_fd = -1;
#else
    extern int ipc_back_fd;
#endif

#if defined(USE_MOUSE) && !defined(OS2)
char* readline_ipc __PROTO((const char*, int* from_ipc));
#else
char* readline_ipc __PROTO((const char*));
#endif

#endif /* _IPC_H */
