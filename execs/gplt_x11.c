#ifndef lint
static char *RCSid() { return RCSid("$Id: gplt_x11.c,v 1.9 1999/10/29 18:49:24 lhecking Exp $"); }
#endif

/* GNUPLOT - gplt_x11.c */

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


/* lph changes:
 * (a) make EXPORT_SELECTION the default and specify NOEXPORT to undefine
 * (b) append X11 terminal number to resource name
 * (c) change cursor for active terminal
 */

/*-----------------------------------------------------------------------------
 *   gnuplot_x11 - X11 outboard terminal driver for gnuplot 3.3
 *
 *   Requires installation of companion inboard x11 driver in gnuplot/term.c
 *
 *   Acknowledgements: 
 *      Chris Peterson (MIT)
 *      Dana Chee (Bellcore) 
 *      Arthur Smith (Cornell)
 *      Hendri Hondorp (University of Twente, The Netherlands)
 *      Bill Kucharski (Solbourne)
 *      Charlie Kline (University of Illinois)
 *      Yehavi Bourvine (Hebrew University of Jerusalem, Israel)
 *      Russell Lang (Monash University, Australia)
 *      O'Reilly & Associates: X Window System - Volumes 1 & 2
 *
 *   This code is provided as is and with no warranties of any kind.
 *
 * drd: change to allow multiple windows to be maintained independently
 *       
 * There is a mailing list for gnuplot users. Note, however, that the
 * newsgroup 
 *	comp.graphics.apps.gnuplot 
 * is identical to the mailing list (they
 * both carry the same set of messages). We prefer that you read the
 * messages through that newsgroup, to subscribing to the mailing list.
 * (If you can read that newsgroup, and are already on the mailing list,
 * please send a message to majordomo@dartmouth.edu, asking to be
 * removed from the mailing list.)
 *
 * The address for mailing to list members is
 *	   info-gnuplot@dartmouth.edu
 * and for mailing administrative requests is 
 *	   majordomo@dartmouth.edu
 * The mailing list for bug reports is 
 *	   bug-gnuplot@dartmouth.edu
 * The list of those interested in beta-test versions is
 *	   info-gnuplot-beta@dartmouth.edu
 *---------------------------------------------------------------------------*/

/* drd : export the graph via ICCCM primary selection. well... not quite
 * ICCCM since we dont support full list of targets, but this
 * is a start.  define EXPORT_SELECTION if you want this feature
 */

/*lph: add a "feature" to undefine EXPORT_SELECTION
   The following makes EXPORT_SELECTION the default and 
   defining NOEXPORT over-rides the default
 */

/* Petr Mikulik and Johannes Zellner: added mouse support (October 1999)
 * Implementation and functionality is based on os2/gclient.c; see mousing.c
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef EXPORT_SELECTION
# undef EXPORT_SELECTION
#endif /* EXPORT SELECTION */
#ifndef NOEXPORT
# define EXPORT_SELECTION XA_PRIMARY
#endif /* NOEXPORT */


#if !(defined(VMS) || defined(CRIPPLED_SELECT))
# define DEFAULT_X11
#endif

#if defined(VMS) && defined(CRIPPLED_SELECT)
Error. Incompatible options.
#endif


#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <signal.h>

#ifdef HAVE_SYS_BSDTYPES_H
# include <sys/bsdtypes.h>
#endif /* HAVE_SYS_BSDTYPES_H */

#ifdef __EMX__
/* for gethostname ... */
# include <netdb.h>
#endif

#if defined(HAVE_SYS_SELECT_H) && !defined(VMS)
# include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H && !VMS */

#ifndef FD_SET
# define FD_SET(n, p)    ((p)->fds_bits[0] |= (1 << ((n) % 32)))
# define FD_CLR(n, p)    ((p)->fds_bits[0] &= ~(1 << ((n) % 32)))
# define FD_ISSET(n, p)  ((p)->fds_bits[0] & (1 << ((n) % 32)))
# define FD_ZERO(p)      memset((char *)(p),'\0',sizeof(*(p)))
#endif /* not FD_SET */

#include "plot.h"

#if defined(HAVE_SYS_SYSTEMINFO_H) && defined(HAVE_SYSINFO)
# include <sys/systeminfo.h>
# define SYSINFO_METHOD "sysinfo"
# define GP_SYSTEMINFO(host) sysinfo (SI_HOSTNAME, (host), MAXHOSTNAMELEN)
#else
# define SYSINFO_METHOD "gethostname"
# define GP_SYSTEMINFO(host) gethostname ((host), MAXHOSTNAMELEN)
#endif /* HAVE_SYS_SYSTEMINFO_H && HAVE_SYSINFO */

#ifdef VMS
# ifdef __DECC
#  include <starlet.h>
# endif				/* __DECC */
# define EXIT(status) sys$delprc(0,0)	/* VMS does not drop itself */
#else
# define EXIT(status) exit(status)
#endif

#ifdef OSK
# define EINTR	E_ILLFNC
#endif


#ifdef USE_MOUSE
#if defined(OS2) && !defined(GNUPMDRV)
  #define INCL_DOSSEMAPHORES
  #include <os2.h>
#endif
#include "os2/dialogs.h"
#include "mousing.c"
unsigned long gnuplotXID = 0; /* WINDOWID of gnuplot */
#define IGNORE_MOUSE (useMouse==0 || !gp4mouse.graph)
  /* don't react to mouse in the event handler */
int force_mouse_graph = 0;
  /* Can be used to force a certain type of gp4mouse.graph after (re)plot, 
     mostly to avoid the automatic graph type change in mousing.c
   */
#endif


/* information about one window/plot */

typedef struct plot_struct {
    Window window;
    Pixmap pixmap;
    unsigned int posn_flags;
    int x, y;
    unsigned int width, height;	/* window size */
    unsigned int px, py;	/* pointsize */
    int ncommands, max_commands;
    char **commands;
#ifdef USE_MOUSE
    Window msgwin;
    char mouse;     /* 0: don't use mouse, 1: use mouse         */
    int  button;    /* buttons which are currently pressed      */
    int  pointer_x; /* current pointer position                 */
    int  pointer_y;
    int  motion_x;  /* for pointer motions. (see ButtonMotion1) */
    int  motion_y;
    char str[0xff]; /* last displayed string                    */
    int  zoom[4];   /* two points spanning current zoom region, */
    Time time;      /* time of last button press event          */
#endif
} plot_struct;

#ifdef USE_MOUSE
enum { no = 0, yes = 1 };
enum { NOT_AVAILABLE = -1 };
enum { button1 = (1 << 0), button2 = (1 << 1), button3 = (1 << 2) };
#define ESC ''

#ifndef OS2
/* sunos 4 uses on_exit() in place of atexit(). If both are missing,
 * we can probably survive since gnuplot_x11 should detect EOF on
 * the pipe. Unfortunately, the handlers take different parameters.
 */

#ifdef NO_ATEXIT
# define HANDLER_PROTO  __PROTO((int x, void *y))
# define HANDLER_DECL (x,y) int x; void *y;
# define HANDLER_PARAMS (0,NULL)
# ifdef HAVE_ON_EXIT
#  define atexit(x) on_exit(x, NULL)
# else
#  define atexit(x)		/* nowt */
# endif
#else /* !NO_ATEXIT */
# define HANDLER_PROTO __PROTO((void))
# define HANDLER_DECL   ()
# define HANDLER_PARAMS ()
#endif

static void X11_mouse_atexit HANDLER_PROTO;

#endif /* !OS2 */

#endif /* USE_MOUSE */

void store_command __PROTO((char *line, plot_struct * plot));
void prepare_plot __PROTO((plot_struct * plot, int term_number));
void delete_plot __PROTO((plot_struct * plot));

int record __PROTO((void));
void process_event __PROTO((XEvent * event));	/* from Xserver */

void mainloop __PROTO((void));

void display __PROTO((plot_struct * plot));
void UpdateWindow __PROTO((plot_struct* plot));
#ifdef USE_MOUSE
void GetMouseXY __PROTO((int* x, int* y, plot_struct* plot));
void EventuallyDrawRuler __PROTO((plot_struct* plot));
void EventuallyDrawMouseAddOns __PROTO((plot_struct* plot));
void alert __PROTO((void));
void ZoomNext __PROTO((plot_struct* plot));
void ZoomPrevious __PROTO((plot_struct* plot));
void ZoomUnzoom __PROTO((plot_struct* plot));
void DrawBox __PROTO((plot_struct* plot, int x0, int y0, int x1, int y1));
void AnnotatePoint __PROTO((plot_struct* plot, int x, int y));
void DrawLine __PROTO((plot_struct* plot, int x1, int y1, int x2, int y2));
void DrawEllipse __PROTO((plot_struct* plot, int x, int y, unsigned int a, unsigned int b));
void DrawCircle __PROTO((plot_struct* plot, int x, int y, unsigned int r));
void DrawStringAt __PROTO((plot_struct* plot, int x, int y, char* str));
void ButtonPress3 __PROTO((plot_struct* plot, int x, int y));
void ButtonRelease3 __PROTO((plot_struct* plot, int x, int y));
void ZoomStart __PROTO((plot_struct* plot, int x, int y));
void ZoomRegionFinish __PROTO((plot_struct* plot, int x, int y));
void ZoomUpdate __PROTO((plot_struct* plot, int x, int y));
int ZoomActive __PROTO((plot_struct* plot));
void ZoomCancel __PROTO((plot_struct* plot));
void SetPointer __PROTO((plot_struct* plot, int x, int y));
int SetTime __PROTO((plot_struct* plot, Time t));
void DoubleClick1 __PROTO((plot_struct* plot, int x, int y));
void ButtonPress2 __PROTO((plot_struct* plot, int x, int y));
void ButtonMotion2 __PROTO((plot_struct* plot, int x, int y));
void ButtonRelease2 __PROTO((plot_struct* plot, int x, int y));
int ChangeViewActive __PROTO((plot_struct* plot));
void ChangeView __PROTO((int x_incr, int z_incr, double zoom_incr, double z_scale_incr));
void ButtonPress1_graph3d __PROTO((plot_struct* plot, int x, int y));
void ButtonRelease1_graph3d __PROTO((plot_struct* plot, int x, int y));
void ButtonPress2_graph3d __PROTO((plot_struct* plot, int x, int y));
void ButtonRelease2_graph3d __PROTO((plot_struct* plot, int x, int y));
void MotionChangeView __PROTO((plot_struct* plot, int x, int y));
void GetGCXor __PROTO((GC* gc, Window window));
void GetGCXorDashed __PROTO((GC* gc, Window window));
void GetGCBlackAndWhite __PROTO((GC* gc, Pixmap pixmap, int mode));
void DisplayHelpWindow __PROTO((plot_struct * plot));
int SplitAt __PROTO((char** args, int maxargs, char* buf, char splitchar));
void DisplayMessageAt __PROTO((plot_struct* plot, char* msg, int x, int y));
void xfree(void* fred);
void EraseCoords __PROTO((plot_struct* plot));
void MousePosToGraphPos __PROTO((
	double* real_x, double* real_y,
	plot_struct* plot,
	int mx, int my, int mouse_mode
	));
void
PxPosToMousePos __PROTO((int* mx, int* my, plot_struct* plot, int px, int py));
void DrawCoords __PROTO((plot_struct* plot, int x, int y));
void DisplayCoords __PROTO((plot_struct * plot, int x, int y));
#endif

void reset_cursor __PROTO((void));

void preset __PROTO((int argc, char *argv[]));
char *pr_GetR __PROTO((XrmDatabase db, char *resource));
void pr_color __PROTO((void));
void pr_dashes __PROTO((void));
void pr_font __PROTO((void));
void pr_geometry __PROTO((void));
void pr_pointsize __PROTO((void));
void pr_width __PROTO((void));
Window pr_window __PROTO((unsigned int flags, int x, int y, unsigned int width, unsigned height));
void pr_raise __PROTO((void));
void pr_persist __PROTO((void));

#ifdef EXPORT_SELECTION
void export_graph __PROTO((plot_struct * plot));
void handle_selection_event __PROTO((XEvent * event));
#endif

#define FallbackFont "fixed"

#define Ncolors 13
unsigned long colors[Ncolors];

#define Nwidths 10
unsigned int widths[Nwidths] = { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define Ndashes 10
char dashes[Ndashes][5];

#define MAX_WINDOWS 16

#define XC_crosshair 34


struct plot_struct plot_array[MAX_WINDOWS];


Display *dpy;
int scr;
Window root;
Visual *vis;
GC gc = (GC) 0;
GC gc_xor = (GC) 0;
GC gc_xor_dashed = (GC) 0;
XFontStruct *font;
int do_raise = 1, persist = 0;
KeyCode keycode[0xff]; /* well, a bit overdimensioned. */
/* the following don't work for ascii codes < 'a' (e.g. ' '). (joze) */
/* #define KEYCODE(x) keycode[(x) - 'a'] */
/* but this is wasting memory */
#define KEYCODE(x) keycode[(x)]
Cursor cursor;

int windows_open = 0;

int gX = 100, gY = 100;
unsigned int gW = 640, gH = 450;
unsigned int gFlags = PSize;

unsigned int BorderWidth = 2;
unsigned int D;			/* depth */

Bool Mono = 0, Gray = 0, Rv = 0, Clear = 0;
char Name[64] = "gnuplot";
char Class[64] = "Gnuplot";

int cx = 0, cy = 0, vchar;
double xscale, yscale, pointsize;
#define X(x) (int) ((x) * xscale)
#define Y(y) (int) ((4095-(y)) * yscale)

#define Nbuf 1024
char buf[Nbuf], **commands = (char **) 0;
static int buffered_input_available = 0;

FILE *X11_ipc;
char X11_ipcpath[32];

/* when using an ICCCM-compliant window manager, we can ask it
 * to send us an event when user chooses 'close window'. We do this
 * by setting WM_DELETE_WINDOW atom in property WM_PROTOCOLS
 */

Atom WM_PROTOCOLS, WM_DELETE_WINDOW;

XPoint Diamond[5], Triangle[4];
XSegment Plus[2], Cross[2], Star[4];

/*-----------------------------------------------------------------------------
 *   main program 
 *---------------------------------------------------------------------------*/

int
main(argc, argv)
int argc;
char *argv[];
{


#ifdef OSK
    /* malloc large blocks, otherwise problems with fragmented mem */
    _mallocmin(102400);
#endif
#ifdef __EMX__
    /* close open file handles */
    fcloseall();
#endif

    FPRINTF((stderr, "gnuplot_X11 starting up\n"));

#if defined(USE_MOUSE) && !defined(OS2)
    /* install an exit handler which tells gnuplot
     * to terminate the connectinon */
    atexit(X11_mouse_atexit);
#endif

    preset(argc, argv);

/* set up the alternative cursor */
    cursor = XCreateFontCursor(dpy, XC_crosshair);

    mainloop();

    if (persist) {
	FPRINTF((stderr, "waiting for %d windows\n", windows_open));
	/* read x events until all windows have been quit */
	while (windows_open > 0) {
	    XEvent event;
	    XNextEvent(dpy, &event);
	    process_event(&event);
	}
    }
    XCloseDisplay(dpy);

    FPRINTF((stderr, "exiting\n"));

    EXIT(0);
}

/*-----------------------------------------------------------------------------
 *   mainloop processing - process X events and input from gnuplot
 *
 *   Three different versions of main loop processing are provided to support
 *   three different platforms.
 * 
 *   DEFAULT_X11:     use select() for both X events and input on stdin 
 *                    from gnuplot inboard driver
 *
 *   CRIPPLED_SELECT: use select() to service X events and check during 
 *                    select timeout for temporary plot file created
 *                    by inboard driver
 *
 *   VMS:             use XNextEvent to service X events and AST to
 *                    service input from gnuplot inboard driver on stdin 
 *---------------------------------------------------------------------------*/


#ifdef DEFAULT_X11
/*-----------------------------------------------------------------------------
 *    DEFAULT_X11 mainloop
 *---------------------------------------------------------------------------*/

void
mainloop()
{
    int nf, cn = ConnectionNumber(dpy), in;
    fd_set_size_t nfds;
    struct timeval timeout, *timer = (struct timeval *) 0;
    fd_set tset;

    X11_ipc = stdin;
    in = fileno(X11_ipc);

#ifdef ISC22
/* Added by Robert Eckardt, RobertE@beta.TP2.Ruhr-Uni-Bochum.de */
    timeout.tv_sec = 0;		/* select() in ISC2.2 needs timeout */
    timeout.tv_usec = 300000;	/* otherwise input from gnuplot is */
    timer = &timeout;		/* suspended til next X event. */
#endif /* ISC22   (0.3s are short enough not to be noticed */

    while (1) {

	/* XNextEvent does an XFlush() before waiting. But here.
	 * we must ensure that the queue is flushed, since we
	 * dont call XNextEvent until an event arrives. (I have
	 * twice wasted quite some time over this issue, so now
	 * I am making sure of it !
	 */

	XFlush(dpy);

	FD_ZERO(&tset);
	FD_SET(cn, &tset);

	/* Don't wait for events if we know that input is
	 * already sitting in a buffer.  Also don't wait for
	 * input to become available.
	*/
	if (buffered_input_available) {
	    timeout.tv_sec  = 0;
	    timeout.tv_usec = 0;
	    timer = &timeout;
	} else {
	    timer = (struct timeval *) 0;
	    FD_SET(in, &tset);
	}

	nfds = (cn > in) ? cn + 1 : in + 1;

	nf = select(nfds, SELECT_FD_SET_CAST &tset, 0, 0, timer);

	if (nf < 0) {
	    if (errno == EINTR)
		continue;
	    fprintf(stderr, "gnuplot: select failed. errno:%d\n", errno);
	    EXIT(1);
	}

	if (nf > 0)
	    XNoOp(dpy);

	if (FD_ISSET(cn, &tset)) {
	    /* used to use CheckMaskEvent() but that cannot receive
	     * maskable events such as ClientMessage. So now we do
	     * one event, then return to the select.
	     * And that almost works, except that under some Xservers
	     * running without a window manager (e.g. Hummingbird Exceed under Win95)
	     * a bogus ConfigureNotify is sent followed by a valid ConfigureNotify
	     * when the window is maximized.  The two events are queued, apparently
	     * in a single I/O because select() above doesn't see the second, valid
	     * event.  This little loop fixes the problem by flushing the
	     * event queue completely.
	     */
	    XEvent xe;
	    do {
		XNextEvent(dpy, &xe);
		process_event(&xe);
	    } while (XPending(dpy));
	}

	if (FD_ISSET(in, &tset) || buffered_input_available) {
	    if (!record())	/* end of input */
		return;
	}
    }
}


#elif defined(CRIPPLED_SELECT)
/*-----------------------------------------------------------------------------
 *    CRIPPLED_SELECT mainloop
 *---------------------------------------------------------------------------*/

void
mainloop()
{
    fd_set_size_t nf, nfds, cn = ConnectionNumber(dpy);
    struct timeval timeout, *timer;
    fd_set tset;
    unsigned long all = (unsigned long) (-1L);
    XEvent xe;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    timer = &timeout;
    sprintf(X11_ipcpath, "/tmp/Gnuplot_%d", getppid());
    nfds = cn + 1;

    while (1) {
	XFlush(dpy);		/* see above */

	FD_ZERO(&tset);
	FD_SET(cn, &tset);

	/* Don't wait for events if we know that input is
	 * already sitting in a buffer.  Also don't wait for
	 * input to become available.
	*/
	if (buffered_input_available) {
	    timeout.tv_sec  = 0;
	    timeout.tv_usec = 0;
	    timer = &timeout;
	} else {
	    timer = (struct timeval *) 0;
	    FD_SET(in, &tset);
	}

	nfds = (cn > in) ? cn + 1 : in + 1;

	nf = select(nfds, SELECT_FD_SET_CAST &tset, 0, 0, timer);

	if (nf < 0) {
	    if (errno == EINTR)
		continue;
	    fprintf(stderr, "gnuplot: select failed. errno:%d\n", errno);
	    EXIT(1);
	}

	if (nf > 0)
	    XNoOp(dpy);

	if (FD_ISSET(cn, &tset)) {
	    while (XCheckMaskEvent(dpy, all, &xe)) {
		process_event(&xe);
	    }
	}
	if ((X11_ipc = fopen(X11_ipcpath, "r"))) {
	    unlink(X11_ipcpath);
	    record();
	    fclose(X11_ipc);
	}
    }
}


#elif defined(VMS)
/*-----------------------------------------------------------------------------
 *    VMS mainloop - Yehavi Bourvine - YEHAVI@VMS.HUJI.AC.IL
 *---------------------------------------------------------------------------*/

/*  In VMS there is no decent Select(). hence, we have to loop inside
 *  XGetNextEvent for getting the next X window event. In order to get input
 *  from the master we assign a channel to SYS$INPUT and use AST's in order to
 *  receive data. In order to exit the mainloop, we need to somehow make
 *  XNextEvent return from within the ast. We do this with a XSendEvent() to
 *  ourselves !
 *  This needs a window to send the message to, so we create an unmapped window
 *  for this purpose. Event type XClientMessage is perfect for this, but it
 *  appears that such messages come from elsewhere (motif window manager,
 *  perhaps ?) So we need to check fairly carefully that it is the ast event
 *  that has been received.
 */

#include <iodef.h>
char STDIIN[] = "SYS$INPUT:";
short STDIINchannel, STDIINiosb[4];
struct {
    short size, type;
    char *address;
} STDIINdesc;
char STDIINbuffer[64];
int status;

ast()
{
    int status = sys$qio(0, STDIINchannel, IO$_READVBLK, STDIINiosb, record,
			 0, STDIINbuffer, sizeof(STDIINbuffer) - 1, 0, 0, 0, 0);
    if ((status & 0x1) == 0)
	EXIT(status);
}

Window message_window;

void
mainloop()
{
    /* dummy unmapped window for receiving internally-generated terminate
     * messages
     */
    message_window = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 1, 0, 0);

    STDIINdesc.size = strlen(STDIIN);
    STDIINdesc.type = 0;
    STDIINdesc.address = STDIIN;
    status = sys$assign(&STDIINdesc, &STDIINchannel, 0, 0, 0);
    if ((status & 0x1) == 0)
	EXIT(status);
    ast();

    for (;;) {
	XEvent xe;
	XNextEvent(dpy, &xe);
	if (xe.type == ClientMessage && xe.xclient.window == message_window) {
	    if (xe.xclient.message_type == None &&
		xe.xclient.format == 8 &&
		strcmp(xe.xclient.data.b, "die gnuplot die") == 0) {
		FPRINTF((stderr, "quit message from ast\n"));
		return;
	    } else {
		FPRINTF((stderr, "Bogus XClientMessage event from window manager ?\n"));
	    }
	}
	process_event(&xe);
    }
}
#else /* !(DEFAULT_X11 || CRIPPLED_SELECT || VMS */
You lose. No mainloop.
#endif				/* !(DEFAULT_X11 || CRIPPLED_SELECT || VMS */

/* delete a window / plot */

void
delete_plot(plot)
plot_struct *plot;
{
    int i;

    FPRINTF((stderr, "Delete plot %d\n", plot - plot_array));

    for (i = 0; i < plot->ncommands; ++i)
	free(plot->commands[i]);
    plot->ncommands = 0;

    if (plot->window) {
	FPRINTF((stderr, "Destroy window 0x%x\n", plot->window));
	XDestroyWindow(dpy, plot->window);
	plot->window = None;
	--windows_open;
    }
    if (plot->pixmap) {
	XFreePixmap(dpy, plot->pixmap);
	plot->pixmap = None;
    }
    /* but preserve geometry */
}


/* prepare the plot structure */

void
prepare_plot(plot, term_number)
plot_struct *plot;
int term_number;
{
    int i;
    char *term_name;

    for (i = 0; i < plot->ncommands; ++i)
	free(plot->commands[i]);
    plot->ncommands = 0;

    if (!plot->posn_flags) {
	/* first time this window has been used - use default or -geometry
	 * settings
	 */
	plot->posn_flags = gFlags;
	plot->x = gX;
	plot->y = gY;
	plot->width = gW;
	plot->height = gH;
    }
    if (!plot->window) {
	plot->window = pr_window(plot->posn_flags, plot->x, plot->y, plot->width, plot->height);
	++windows_open;

	/* append the X11 terminal number (if greater than zero) */

	if (term_number) {
	    char new_name[60];
	    XFetchName(dpy, plot->window, &term_name);
	    FPRINTF((stderr, "Window title is %s\n", term_name));

	    sprintf(new_name, "%.55s%3d", term_name, term_number);
	    FPRINTF((stderr, "term_number  is %d\n", term_number));

	    XStoreName(dpy, plot->window, new_name);

	    sprintf(new_name, "gplt%3d", term_number);
	    XSetIconName(dpy, plot->window, new_name);
	}
#ifdef USE_MOUSE
	/**
	 * set all mouse parameters
	 * to a well-defined state.
	 */
	plot->msgwin = (Window) 0;
	useMouse = 0;
	plot->button = 0;
	plot->pointer_x = NOT_AVAILABLE;
	plot->pointer_y = NOT_AVAILABLE;
	plot->x = NOT_AVAILABLE;
	plot->y = NOT_AVAILABLE;
	plot->str[0] = '\0';
	{
	    int i;
	    for (i = 0; i < 4; i++) {
		plot->zoom[i] = NOT_AVAILABLE;
	    }
	}
	plot->time = 0; /* XXX how should we initialize this ? XXX */
#endif
    }
/* We don't know that it is the same window as before, so we reset the
 * cursors for all windows and then define the cursor for the active
 * window 
 */
    reset_cursor();
    XDefineCursor(dpy, plot->window, cursor);

}

/* store a command in a plot structure */

void
store_command(buffer, plot)
char *buffer;
plot_struct *plot;
{
    char *p;

    FPRINTF((stderr, "Store in %d : %s", plot - plot_array, buffer));

    if (plot->ncommands >= plot->max_commands) {
	plot->max_commands = plot->max_commands * 2 + 1;
	plot->commands = (plot->commands)
	    ? (char **) realloc(plot->commands, plot->max_commands * sizeof(char *))
	: (char **) malloc(sizeof(char *));
    }
    p = (char *) malloc((unsigned) strlen(buffer) + 1);
    if (!plot->commands || !p) {
	fputs("gnuplot: can't get memory. X11 aborted.\n", stderr);
	EXIT(1);
    }
    plot->commands[plot->ncommands++] = strcpy(p, buffer);
}

#ifndef VMS

/* Handle input.  Use read instead of fgets because stdio buffering
* causes trouble when combined with calls to select.
*/
int
read_input (int howmany)
{
    static int rdbuf_size = 10*Nbuf;
    static char rdbuf[10*Nbuf-1];
    static int total_chars;
    static int rdbuf_offset;
    static int buf_offset;
    static int partial_read = 0;
    int fd = fileno (X11_ipc);

    if (! partial_read)
	buf_offset = 0;

    if (! buffered_input_available) {
	total_chars = read (fd, rdbuf, rdbuf_size);
	buffered_input_available = 1;
	partial_read = 0;
	rdbuf_offset = 0;
	if (total_chars < 0)
	    return -1;
    }

    if (rdbuf_offset < total_chars) {
	while (rdbuf_offset < total_chars && buf_offset < Nbuf) {
	    char c = rdbuf[rdbuf_offset++];
	    buf[buf_offset++] = c;
	    if (howmany < 0 && c == '\n' || buf_offset == howmany)
		break;
	}

	if (buf_offset == Nbuf) {
	    fputs("\
\n\
gnuplot: buffer overflow in read_input!\n\
gnuplot: X11 aborted.\n", stderr);
	    EXIT(1);
	} else
	    buf[buf_offset] = NUL;
    }

    if (rdbuf_offset == total_chars) {
	buffered_input_available = 0;
	if (buf[buf_offset-1] != '\n')
	    partial_read = 1;
    }

    return partial_read;
}


/*-----------------------------------------------------------------------------
 *   record - record new plot from gnuplot inboard X11 driver (Unix)
 *---------------------------------------------------------------------------*/

int
record()
{
    static plot_struct *plot = plot_array;

    while (1) {
	int status = read_input (-1);
	if (status != 0)
	    return status;

	switch (*buf) {
	case 'G':		/* enter graphics mode */
	    {
		int plot_number = atoi(buf + 1);    /* 0 if none specified */

		if (plot_number < 0 || plot_number >= MAX_WINDOWS)
		    plot_number = 0;

#ifdef USE_MOUSE
		/* receive information about real plot coord. */

		/*fread(&gp4mouse, sizeof(gp4mouse), 1, X11_ipc);*/
		read_input(sizeof(gp4mouse));
		memcpy((void*)&gp4mouse, (void*)buf, sizeof(gp4mouse));
#ifdef OS2
		{ /* get pid of gnuplot.exe */
		int p; fread(&p, sizeof(p), 1, X11_ipc); ppidGnu = p;
		if (!input_from_PM_Terminal) { /* get shared memory */
		  sprintf( mouseShareMemName, "\\SHAREMEM\\GP%i_Mouse_Input", (int)ppidGnu );
		  if (DosGetNamedSharedMem(&input_from_PM_Terminal, mouseShareMemName, PAG_WRITE))
		    DosBeep(1440L,1000L); /* indicates error */
		  semInputReady = 0;
		}
		}
#endif
		/* get XID of gnuplot window (<space> then raises it up) */
		/*fread(&gnuplotXID, sizeof(unsigned long), 1, X11_ipc);*/
		read_input(sizeof(unsigned long));
		memcpy((void*)&gnuplotXID, (void*)buf, sizeof(unsigned long));

#if 0
		MouseDebugShow_gp4mouse();
#endif
		if (ruler.on) recalc_ruler_pos();
#endif

		FPRINTF((stderr, "plot for window number %d\n", plot_number));
		plot = plot_array + plot_number;
		prepare_plot(plot, plot_number);
		continue;
	    }
	case 'E':		/* leave graphics mode / suspend */
	    display(plot);
	    return 1;
	case 'R':		/* leave x11 mode */
	    reset_cursor();
	    return 0;
#ifdef USE_MOUSE
	case '#':		/* do_3dplot() and boundary3d() changed [xleft..ytop] */
	    {
	    /*fread(&gp4mouse.xleft, 4*sizeof(gp4mouse.xleft), 1, X11_ipc);*/
	    read_input(4*sizeof(gp4mouse.xleft));
	    memcpy((void*)&gp4mouse.xleft, (void*)buf, 4*sizeof(gp4mouse.xleft));
	    if (force_mouse_graph) {
		gp4mouse.graph = force_mouse_graph;
		force_mouse_graph = 0;
	    }
	    if (ruler.on) recalc_ruler_pos();
	    continue;
	    }
#endif
	default:
	    store_command(buf, plot);
	    continue;
	}
    }
}

#else /* VMS */
/*-----------------------------------------------------------------------------
 *   record - record new plot from gnuplot inboard X11 driver (VMS)
 *---------------------------------------------------------------------------*/

record()
{
    static plot_struct *plot = plot_array;

    int status;

    if ((STDIINiosb[0] & 0x1) == 0)
	EXIT(STDIINiosb[0]);
    STDIINbuffer[STDIINiosb[1]] = '\0';
    strcpy(buf, STDIINbuffer);

    switch (*buf) {
    case 'G':			/* enter graphics mode */
	{
	    int plot_number = atoi(buf + 1);	/* 0 if none specified */
	    if (plot_number < 0 || plot_number >= MAX_WINDOWS)
		plot_number = 0;
	    FPRINTF((stderr, "plot for window number %d\n", plot_number));
	    plot = plot_array + plot_number;
	    prepare_plot(plot, plot_number);
	    break;
	}
    case 'E':			/* leave graphics mode */
	display(plot);
	break;
    case 'R':			/* exit x11 mode */
	FPRINTF((stderr, "received R - sending ClientMessage\n"));
	reset_cursor();
	sys$cancel(STDIINchannel);
	/* this is ridiculous - cook up an event to ourselves,
	 * in order to get the mainloop() out of the XNextEvent() call
	 * it seems that window manager can also send clientmessages,
	 * so put a checksum into the message
	 */
	{
	    XClientMessageEvent event;
	    event.type = ClientMessage;
	    event.send_event = True;
	    event.display = dpy;
	    event.window = message_window;
	    event.message_type = None;
	    event.format = 8;
	    strcpy(event.data.b, "die gnuplot die");
	    XSendEvent(dpy, message_window, False, 0, (XEvent *) & event);
	    XFlush(dpy);
	}
	return;			/* no ast */
    default:
	store_command(buf, plot);
	break;
    }
    ast();
}
#endif /* VMS */


/*-----------------------------------------------------------------------------
 *   display - display a stored plot
 *---------------------------------------------------------------------------*/

void
display(plot)
plot_struct *plot;
{
    int n, x, y, sw, sl, lt = 0, width = 0, type = LineSolid, point, px, py;
    int user_width = 1;		/* as specified by plot...linewidth */
    char *buffer, *str;
    enum JUSTIFY jmode;

    FPRINTF((stderr, "Display %d ; %d commands\n", plot - plot_array, plot->ncommands));

    if (plot->ncommands == 0)
	return;

    /* set scaling factor between internal driver & window geometry */
    xscale = plot->width / 4096.0;
    yscale = (plot->height
#if 0
/* PM !!!! avoid this (now) --- all heights needed to be corrected afterwards */
#ifdef USE_MOUSE
	- (useMouse ? vchar : 0) /* add space for displaying coordinates */
#endif
#endif
	     ) / 4096.0;

    /* initial point sizes, until overridden with P7xxxxyyyy */
    px = (int) (xscale * pointsize);
    py = (int) (yscale * pointsize);

    /* create new pixmap & GC */
    if (gc)
	XFreeGC(dpy, gc);

    if (!plot->pixmap) {
	FPRINTF((stderr, "Create pixmap %d : %dx%dx%d\n", plot - plot_array, plot->width,
		 plot->height, D));
	plot->pixmap = XCreatePixmap(dpy, root, plot->width, plot->height, D);
    }
    gc = XCreateGC(dpy, plot->pixmap, 0, (XGCValues *) 0);

    XSetFont(dpy, gc, font->fid);

    /* set pixmap background */
    XSetForeground(dpy, gc, colors[0]);
    XFillRectangle(dpy, plot->pixmap, gc, 0, 0, plot->width, plot->height);
    XSetBackground(dpy, gc, colors[0]);

    if (!plot->window) {
	plot->window = pr_window(plot->posn_flags, plot->x, plot->y, plot->width, plot->height);
	++windows_open;
    }
    /* top the window but don't put keyboard or mouse focus into it. */
    if (do_raise)
	XMapRaised(dpy, plot->window);

    /* momentarily clear the window first if requested */
    if (Clear) {
	XClearWindow(dpy, plot->window);
	XFlush(dpy);
    }
    /* loop over accumulated commands from inboard driver */
    for (n = 0; n < plot->ncommands; n++) {
	buffer = plot->commands[n];
	/* fprintf (stderr, "(display) buffer = |%s|\n", buffer); */

	/*   X11_vector(x,y) - draw vector  */
	if (*buffer == 'V') {
	    sscanf(buffer, "V%4d%4d", &x, &y);
	    XDrawLine(dpy, plot->pixmap, gc, X(cx), Y(cy), X(x), Y(y));
	    cx = x;
	    cy = y;
	}
	/*   X11_move(x,y) - move  */
	else if (*buffer == 'M')
	    sscanf(buffer, "M%4d%4d", &cx, &cy);

	/*   X11_put_text(x,y,str) - draw text   */
	else if (*buffer == 'T') {
	    sscanf(buffer, "T%4d%4d", &x, &y);
	    str = buffer + 9;
	    sl = strlen(str) - 1;
	    sw = XTextWidth(font, str, sl);

	    switch (jmode) {
	    case LEFT:
		sw = 0;
		break;
	    case CENTRE:
		sw = -sw / 2;
		break;
	    case RIGHT:
		sw = -sw;
		break;
	    }

	    XSetForeground(dpy, gc, colors[2]);
	    XDrawString(dpy, plot->pixmap, gc, X(x) + sw, Y(y) + vchar / 3, str, sl);
	    XSetForeground(dpy, gc, colors[lt + 3]);
	} else if (*buffer == 'F') {	/* fill box */
	    int style, xtmp, ytmp, w, h;

	    if (sscanf(buffer + 1, "%4d%4d%4d%4d%4d", &style, &xtmp, &ytmp, &w, &h) == 5) {
		/* gnuplot has origin at bottom left, but X uses top left
		 * There may be an off-by-one (or more) error here.
		 * style ignored here for the moment
		 */
		ytmp += h;	/* top left corner of rectangle to be filled */
		w *= xscale;
		h *= yscale;
		XSetForeground(dpy, gc, colors[0]);
		XFillRectangle(dpy, plot->pixmap, gc, X(xtmp), Y(ytmp), w, h);
		XSetForeground(dpy, gc, colors[lt + 3]);
	    }
	}
	/*   X11_justify_text(mode) - set text justification mode  */
	else if (*buffer == 'J')
	    sscanf(buffer, "J%4d", (int *) &jmode);

	/*  X11_linewidth(width) - set line width */
	else if (*buffer == 'W')
	    sscanf(buffer + 1, "%4d", &user_width);

	/*   X11_linetype(type) - set line type  */
	else if (*buffer == 'L') {
	    sscanf(buffer, "L%4d", &lt);
	    lt = (lt % 8) + 2;
	    /* default width is 0 {which X treats as 1} */
	    width = widths[lt] ? user_width * widths[lt] : user_width;
	    if (dashes[lt][0]) {
		type = LineOnOffDash;
		XSetDashes(dpy, gc, 0, dashes[lt], strlen(dashes[lt]));
	    } else {
		type = LineSolid;
	    }
	    XSetForeground(dpy, gc, colors[lt + 3]);
	    XSetLineAttributes(dpy, gc, width, type, CapButt, JoinBevel);
	}
	/*   X11_point(number) - draw a point */
	else if (*buffer == 'P') {
	    /* linux sscanf does not like %1d%4d%4d" with Oxxxxyyyy */
	    /* sscanf(buffer, "P%1d%4d%4d", &point, &x, &y); */
	    point = buffer[1] - '0';
	    sscanf(buffer + 2, "%4d%4d", &x, &y);
	    if (point == 7) {
		/* set point size */
		px = (int) (x * xscale * pointsize);
		py = (int) (y * yscale * pointsize);
	    } else {
		if (type != LineSolid || width != 0) {	/* select solid line */
		    XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinBevel);
		}
		switch (point) {
		case 0:	/* dot */
		    XDrawPoint(dpy, plot->pixmap, gc, X(x), Y(y));
		    break;
		case 1:	/* do diamond */
		    Diamond[0].x = (short) X(x) - px;
		    Diamond[0].y = (short) Y(y);
		    Diamond[1].x = (short) px;
		    Diamond[1].y = (short) -py;
		    Diamond[2].x = (short) px;
		    Diamond[2].y = (short) py;
		    Diamond[3].x = (short) -px;
		    Diamond[3].y = (short) py;
		    Diamond[4].x = (short) -px;
		    Diamond[4].y = (short) -py;

		    /*
		     * Should really do a check with XMaxRequestSize()
		     */
		    XDrawLines(dpy, plot->pixmap, gc, Diamond, 5, CoordModePrevious);
		    XDrawPoint(dpy, plot->pixmap, gc, X(x), Y(y));
		    break;
		case 2:	/* do plus */
		    Plus[0].x1 = (short) X(x) - px;
		    Plus[0].y1 = (short) Y(y);
		    Plus[0].x2 = (short) X(x) + px;
		    Plus[0].y2 = (short) Y(y);
		    Plus[1].x1 = (short) X(x);
		    Plus[1].y1 = (short) Y(y) - py;
		    Plus[1].x2 = (short) X(x);
		    Plus[1].y2 = (short) Y(y) + py;

		    XDrawSegments(dpy, plot->pixmap, gc, Plus, 2);
		    break;
		case 3:	/* do box */
		    XDrawRectangle(dpy, plot->pixmap, gc, X(x) - px, Y(y) - py, (px + px), (py + py));
		    XDrawPoint(dpy, plot->pixmap, gc, X(x), Y(y));
		    break;
		case 4:	/* do X */
		    Cross[0].x1 = (short) X(x) - px;
		    Cross[0].y1 = (short) Y(y) - py;
		    Cross[0].x2 = (short) X(x) + px;
		    Cross[0].y2 = (short) Y(y) + py;
		    Cross[1].x1 = (short) X(x) - px;
		    Cross[1].y1 = (short) Y(y) + py;
		    Cross[1].x2 = (short) X(x) + px;
		    Cross[1].y2 = (short) Y(y) - py;

		    XDrawSegments(dpy, plot->pixmap, gc, Cross, 2);
		    break;
		case 5:	/* do triangle */
		    {
			short temp_x, temp_y;

			temp_x = (short) (1.33 * (double) px + 0.5);
			temp_y = (short) (1.33 * (double) py + 0.5);

			Triangle[0].x = (short) X(x);
			Triangle[0].y = (short) Y(y) - temp_y;
			Triangle[1].x = (short) temp_x;
			Triangle[1].y = (short) 2 *py;
			Triangle[2].x = (short) -(2 * temp_x);
			Triangle[2].y = (short) 0;
			Triangle[3].x = (short) temp_x;
			Triangle[3].y = (short) -(2 * py);

			XDrawLines(dpy, plot->pixmap, gc, Triangle, 4, CoordModePrevious);
			XDrawPoint(dpy, plot->pixmap, gc, X(x), Y(y));
		    }
		    break;
		case 6:	/* do star */
		    Star[0].x1 = (short) X(x) - px;
		    Star[0].y1 = (short) Y(y);
		    Star[0].x2 = (short) X(x) + px;
		    Star[0].y2 = (short) Y(y);
		    Star[1].x1 = (short) X(x);
		    Star[1].y1 = (short) Y(y) - py;
		    Star[1].x2 = (short) X(x);
		    Star[1].y2 = (short) Y(y) + py;
		    Star[2].x1 = (short) X(x) - px;
		    Star[2].y1 = (short) Y(y) - py;
		    Star[2].x2 = (short) X(x) + px;
		    Star[2].y2 = (short) Y(y) + py;
		    Star[3].x1 = (short) X(x) - px;
		    Star[3].y1 = (short) Y(y) + py;
		    Star[3].x2 = (short) X(x) + px;
		    Star[3].y2 = (short) Y(y) - py;

		    XDrawSegments(dpy, plot->pixmap, gc, Star, 4);
		    break;
		}
		if (type != LineSolid || width != 0) {	/* select solid line */
		    XSetLineAttributes(dpy, gc, width, type, CapButt, JoinBevel);
		}
	    }
	}
    }

#if 0
#ifdef USE_MOUSE
    /* if (ruler.on) EventuallyDrawRuler(plot); */
    /* XXX NO: UpdateWindow() calls this XXX */
#endif
#endif
#ifdef EXPORT_SELECTION
    export_graph(plot);
#endif

    UpdateWindow(plot);
}

void
UpdateWindow(plot_struct* plot)
{
#ifdef USE_MOUSE
    XEvent event;
#endif

    XSetWindowBackgroundPixmap(dpy, plot->window, plot->pixmap);
    XClearWindow(dpy, plot->window);

#ifdef USE_MOUSE
    EventuallyDrawMouseAddOns(plot);

    XFlush(dpy);

    /* XXX discard expose events. This is a kludge for
     * preventing the event dispatcher calling UpdateWindow()
     * and the latter again generating expose events, which
     * again would trigger the event dispatcher ... (joze) XXX */
    while (XCheckWindowEvent(dpy, plot->window, ExposureMask, &event))
	/* EMPTY */;
#endif
}


#ifdef USE_MOUSE

#ifndef OS2
static void
X11_mouse_atexit HANDLER_DECL
{
    /* tell gnuplot to terminate the connection
     * (see readline.c) */
    printf("@\n");
}
#endif

/**
 * draw a crosshair ruler using a GXxor, if ruler.on
 * Uses ruler.px, ruler.px --- coordinates are in viewport units/pixels
 */
void
EventuallyDrawRuler(plot_struct* plot)
{
    if (ruler.on) {
	int mx, my;
	recalc_ruler_pos();
	/* XXX */
	PxPosToMousePos(&mx, &my, plot, ruler.px, ruler.py);

	if (!gc_xor) {
	    /* create a gc for `rubberbanding' (well ...) */
	    GetGCXor(&gc_xor, plot->window);
	}
	if (ruler.on) { /* draw new ruler */
	    /* XGrabServer(dpy); */
	    /* vertical line */
	    /* XDrawLine(dpy, plot->window, gc_xor, ruler.px, 0, ruler.px, plot->height); */
	    XDrawLine(dpy, plot->window, gc_xor, mx, 0, mx, plot->height);
	    /* horizontal line */
	    /* XDrawLine(dpy, plot->window, gc_xor, 0, ruler.py, plot->width, ruler.py); */
	    XDrawLine(dpy, plot->window, gc_xor, 0, my, plot->width, my);
	    /* XUngrabServer(dpy); */
	}
    }
}

void
EventuallyDrawMouseAddOns(plot_struct* plot)
{
    EventuallyDrawRuler(plot);
    if (!IGNORE_MOUSE) {
	DrawCoords(plot, plot->pointer_x, plot->pointer_y);
    }

    /*
	TODO more ...
    */
}

void
alert(void)
{
    fprintf(stderr, "\a");
}

void
ZoomNext(plot_struct* plot)
{
    if (zoom_now == NULL || zoom_now->next == NULL)
	alert();
      else
	apply_zoom( zoom_now->next );
}

void
ZoomPrevious(plot_struct* plot)
{
    if (zoom_now == NULL || zoom_now->prev == NULL)
	alert();
      else
	apply_zoom( zoom_now->prev );
}

void
ZoomUnzoom(plot_struct* plot)
{
    if (zoom_head == NULL || zoom_now == zoom_head)
	alert();
      else
	apply_zoom( zoom_head );
}

/**
 * draw a box using the gc with the GXxor function.
 * This can be used to turn on *and off* a box. The
 * corners of the box are annotated with the plot
 * coordinates.
 */
void
DrawBox(plot_struct* plot, int x0, int y0, int x1, int y1)
{
    int width;
    int height;
    int X0 = x0;
    int Y0 = y0;
    int X1 = x1;
    int Y1 = y1;

    if (!gc_xor_dashed) {
	GetGCXorDashed(&gc_xor_dashed, plot->window);
    }

    if (X1 < X0) {
	int tmp = X1;
	X1 = X0;
	X0 = tmp;
    }

    if (Y1 < Y0) {
	int tmp = Y1;
	Y1 = Y0;
	Y0 = tmp;
    }

    width = X1 - X0;
    height = Y1 - Y0;

    XDrawRectangle(dpy, plot->window, gc_xor_dashed, X0, Y0, width, height);
    AnnotatePoint(plot, x0, y0);
    AnnotatePoint(plot, x1, y1);
}

/**
 * draw the plot coordinates centered horizontally
 * and vertically at the point x, y. Use the GXxor
 * as usually, so that we can also remove the coords
 * later.
 */
void
AnnotatePoint(plot_struct* plot, int x, int y)
{
    double real_x, real_y;
    char xstr[0x20];
    char ystr[0x20];
    int xlen, ylen;
    int xwidth, ywidth, width;

    MousePosToGraphPos(&real_x, &real_y, plot, x, y, mouse_mode);
    sprintf(xstr, "x = %g", real_x);
    xlen = strlen(xstr);
    xwidth = XTextWidth(font, xstr, xlen);
    sprintf(ystr, "y = %g", real_y);
    ylen = strlen(ystr);
    ywidth = XTextWidth(font, ystr, ylen);
    width = xwidth > ywidth ? xwidth : ywidth;
    if (!gc_xor) {
	GetGCXor(&gc_xor, plot->window);
    }
    x -= width / 2;
    XDrawString(dpy, plot->window, gc_xor, x, y - 3, xstr, xlen);
    XDrawString(dpy, plot->window, gc_xor, x, y + vchar, ystr, ylen);
}

void
DrawEllipse(plot_struct* plot, int x, int y,
    unsigned int a, unsigned int b)
{
    if (!gc_xor) {
	GetGCXor(&gc_xor, plot->window);
    }
    XDrawArc(dpy, plot->window, gc_xor,
	x - a, y - b, 2 * a, 2 * b, 0, 23040 /* 360 deg */);
}

void
DrawCircle(plot_struct* plot, int x, int y, unsigned int r)
{
    DrawEllipse(plot, x, y, r, r);
}

void
DrawLine(plot_struct* plot, int x1, int y1, int x2, int y2)
{
    if (!gc_xor) {
	GetGCXor(&gc_xor, plot->window);
    }
    XDrawLine(dpy, plot->window, gc_xor, x1, y1, x2, y2);
}

void
DrawStringAt(plot_struct* plot, int x, int y, char* str)
{
    if (!gc_xor) {
	GetGCXor(&gc_xor, plot->window);
    }
    XDrawString(dpy, plot->window, gc_xor, x, y, str, strlen(str));
}

void
ButtonPress3(plot_struct* plot, int x, int y)
{
    plot->button |= button3;
    ZoomStart(plot, x, y);
}

/* called after releasing button 3 */
void
ButtonRelease3(plot_struct* plot, int x, int y)
{
    plot->button &= ~button3;
}

void
ZoomStart(plot_struct* plot, int x, int y)
{
    plot->zoom[0] = x;
    plot->zoom[1] = y;
    plot->zoom[2] = x; /* `last' zoom corner */
    plot->zoom[3] = y; /* `last' zoom corner */
    DrawBox(plot, x, y, x, y); /* turn on a (zero size) box */
}

void
ZoomUpdate(plot_struct* plot, int x, int y)
{
    /* erase current zoom box */
    DrawBox(plot, plot->zoom[0], plot->zoom[1], plot->zoom[2], plot->zoom[3]);
    /* draw new box */
    DrawBox(plot, plot->zoom[0], plot->zoom[1], x, y);
    plot->zoom[2] = x;
    plot->zoom[3] = y;
}

int
ZoomActive(plot_struct* plot)
{
    if (NOT_AVAILABLE == plot->zoom[0])
	return 0;
    else
	return 1;
}

void
ZoomCancel(plot_struct* plot)
{
    int i;
    /* erase current zoom box */
    DrawBox(plot, plot->zoom[0], plot->zoom[1], plot->zoom[2], plot->zoom[3]);
    /* X11 specific */
    for (i = 0; i < 4; i++) {
	plot->zoom[i] = NOT_AVAILABLE;
    }
}

void
dump_gp4mouse(void)
{
    fprintf(stderr, "(dump_gp4mouse) xmin, xmax, ymin, ymax = %f, %f, %f, %f\n",
	gp4mouse.xmin, gp4mouse.xmax, gp4mouse.ymin, gp4mouse.ymax);
    fprintf(stderr, "(dump_gp4mouse) xleft, ybot, xright, ytop = %d, %d, %d, %d\n",
	gp4mouse.xleft, gp4mouse.ybot, gp4mouse.xright, gp4mouse.ytop);
#if 0
    int /*TBOOLEAN*/ is_log_x, is_log_y; /* are x and y axes log? */
    double base_log_x, base_log_y; /* bases of log */
    double log_base_log_x, log_base_log_y; /* log of bases */
    int has_grid; /* grid on? */
#endif
}

/*
 * called after pression B1, which sets
 * the second point of the zoom region.
 */
void
ZoomRegionFinish(plot_struct* plot, int x, int y)
{
    int i;
    int xfrom,yfrom,xto,yto;
    double x1,y1,x2,y2; /* result---request this zooming */
    /* erase current zoom box */
    DrawBox(plot, plot->zoom[0], plot->zoom[1], plot->zoom[2], plot->zoom[3]);
    plot->button &= ~button3;
#if 0
    fprintf (stderr, "(ZoomRegionFinish) [%d, %d] -- [%d, %d]\n",
	plot->zoom[0], plot->zoom[1], plot->zoom[2], plot->zoom[3]);
#endif
    /* convert from zoom[0],[1], zoom[2],[3] so that from < to */
    if (plot->zoom[0] <= plot->zoom[2]) {
	xfrom = plot->zoom[0]; xto = plot->zoom[2]; }
      else {
	xfrom = plot->zoom[2]; xto = plot->zoom[0]; }
    if (plot->zoom[1] <= plot->zoom[3]) {
	yfrom = plot->zoom[1]; yto = plot->zoom[3]; }
      else {
	yfrom = plot->zoom[3]; yto = plot->zoom[1]; }
    /* X11 specific */
    for (i = 0; i < 4; i++) {
	plot->zoom[i] = NOT_AVAILABLE;
    }
    /* Return if the selected area is too small (8 pixels for canceling) */
    if (xto - xfrom <= 8 || yto - yfrom <= 8)
      return;
    /* Transform mouse->real graph coordinates */
    MousePosToGraphPos( &x1, &y1, plot, xfrom,yfrom, MOUSE_COORDINATES_REAL );
    MousePosToGraphPos( &x2, &y2, plot, xto,yto, MOUSE_COORDINATES_REAL );
    /* Make zoom of the box [x1,y1]..[x2,y2] */
#if 0
    fprintf(stderr,"yfrom .. yto = %d %d\n", yfrom, yto);
    fprintf(stderr,"GO FOR ZOOM: (%g,%g) .. (%g,%g)\n", x1,y1,x2,y2);
    dump_gp4mouse();
#endif
    /* X11's y-axis points downwards, that's why y1 <-> y2 */
    do_zoom( x1, y2, x2, y1);
}

void
SetPointer(plot_struct* plot, int x, int y)
{
    plot->pointer_x = x;
    plot->pointer_y = y;
}

/* returns 1, if doubleclicked (< 300 millisec separation), else 0.  */
int
SetTime(plot_struct* plot, Time t)
{
    /* fprintf (stderr, "(SetTime) difftime = %ld\n", t - plot->time); */
    long int diff = t - plot->time;
    int ret = 0;
    if (diff > 0 && diff < 300 /* millisec, hardcoded, this is ugly! */)
	ret = 1;
    plot->time = t;
    return ret;
}

void
DoubleClick1(plot_struct* plot, int x, int y)
{
    double real_x, real_y;
    char s[256];
    if (mouseSprintfFormat<0 || mouseSprintfFormat>=nMouseSprintfFormats) mouseSprintfFormat = 1;
    MousePosToGraphPos(&real_x, &real_y, plot, x, y, mouse_mode);
    sprintf( s, MouseSprintfFormats[mouseSprintfFormat], real_x,real_y );
    fprintf(stderr, "MOUSE CLICKED AT x,y = %s\n", s);
}

void
ButtonPress2(plot_struct* plot, int x, int y)
{
    double real_x, real_y;
    char s[100];
    const int offset = 4;
    const int offset2 = 2;
    plot->button |= button2;
    MousePosToGraphPos(&real_x, &real_y, plot, x, y, mouse_mode);
    GetAnnotateString(s,real_x,real_y,mouse_mode);
    /* draw string in the first quadrant separated
     * (offset2 x offset2) pixels from the actual mouse position */
    DrawStringAt(plot, x + offset2, y - offset2, s);
    /* draw cross at the mouse position */
    DrawLine(plot, x - offset, y, x + offset, y);
    DrawLine(plot, x, y - offset, x, y + offset);
    /* DrawCircle(plot, x, y, 3); */
}

void
ButtonMotion2(plot_struct* plot, int x, int y)
{
#if 0
    /* erase string */
    DrawStringAt(plot, plot->pointer_x, plot->pointer_y, GREETING);
    /* draw string */
    DrawStringAt(plot, x, y, GREETING);
#endif
}

void
ButtonRelease2(plot_struct* plot, int x, int y)
{
    plot->button &= ~button2;
#if 0
    /* erase string */
    DrawStringAt(plot, plot->pointer_x, plot->pointer_y, GREETING);
#endif
}

int
ChangeViewActive(plot_struct* plot)
{
    return plot->button & button1;
}

void
ButtonPress1_graph3d(plot_struct* plot, int x, int y)
{
    plot->button |= button1;
    plot->motion_x = x;
    plot->motion_y = y;
}

void
ButtonRelease1_graph3d(plot_struct* plot, int x, int y)
{
    plot->button &= ~button1;
}

void
ButtonPress2_graph3d(plot_struct* plot, int x, int y)
{
    plot->button |= button2;
    plot->motion_x = x;
    plot->motion_y = y;
}

void
ButtonRelease2_graph3d(plot_struct* plot, int x, int y)
{
    plot->button &= ~button2;
}

void
ChangeView(int x_incr, int z_incr, double zoom_incr, double z_scale_incr)
{
    static int rot_x = 60, rot_z = 30;
    static double zoom = (double) 1.0, z_scale = (double) 1.0;
    static char cmd[0x40];

    rot_x += x_incr;
    rot_z += z_incr;
    zoom += zoom_incr;
    z_scale += z_scale_incr;

    /* rot_z is cyclic */
    if (rot_z < 0)
	rot_z += 360;
    else if (rot_z > 360)
	rot_z -= 360;

    /* confirm to 0 <= rot_x <= 180 degrees */
    if (rot_x < 0)
	rot_x = 0;
    else if (rot_x > 180)
	rot_x = 180;

    /* confirm to 0 < zoom */
    if (zoom <= 0)
	zoom -= zoom_incr;

    /* confirm to 0 <= z_scale */
    if (z_scale <= 0)
	z_scale -= z_scale_incr;

    sprintf(cmd, "set view %d, %d, %f, %f; replot", rot_x, rot_z, zoom, z_scale);
    force_mouse_graph = graph3d; /* avoid switching to map for 'set view 90*n,0' */
    gp_execute(cmd);
}

void
MotionChangeView(plot_struct* plot, int x, int y)
{
#define rotation_factor ((double) 0.25)
#define zoom_factor ((double) 0.005)
    int x_incr = 0, z_incr = 0;
    double zoom_incr = (double) 0, z_scale_incr = 0;
    int xdist = x - plot->motion_x;
    int ydist = y - plot->motion_y;

    if (abs(xdist) < 4 && abs(ydist) < 4) {
	return;
    } else {
	plot->motion_x = x;
	plot->motion_y = y;
    }

    if (plot->button & button1) {
	if (abs(xdist) > abs(ydist)) {
	    z_incr = (int) ((double) -xdist * rotation_factor);
	} else {
	    x_incr = (int) ((double) -ydist * rotation_factor);
	}
    } else if (plot->button & button2) {
	if (abs(xdist) > abs(ydist)) {
	    z_scale_incr = ((double) xdist * zoom_factor);
	} else {
	    zoom_incr = ((double) ydist * zoom_factor);
	}
    } else {
	/* button 3 not bound yet */
	return;
    }
    ChangeView(x_incr, z_incr, zoom_incr, z_scale_incr);
}

void
GetGCXor(GC* gc, Window window)
    /* returns the newly created gc      */
    /* window: where the gc will be used */
{
    XGCValues values;
    unsigned long mask = 0;

    mask = GCForeground | GCBackground | GCFunction | GCFont;
    values.foreground = WhitePixel(dpy, scr);
    values.background = BlackPixel(dpy, scr);
    values.function = GXxor;
    values.font = font->fid;

    *gc = XCreateGC(dpy, window, mask, &values);

}

void
GetGCXorDashed(GC* gc, Window window)
{
    GetGCXor(gc, window);
    XSetLineAttributes(dpy, *gc,
	0, /* line width, X11 treats 0 as a `thin' line */
	LineOnOffDash, /* also: LineDoubleDash */
	CapNotLast,    /* also: CapButt, CapRound, CapProjecting */
	JoinMiter      /* also: JoinRound, JoinBevel */);
}

void
GetGCBlackAndWhite(GC* gc, Pixmap pixmap, int mode)
    /* returns the newly created gc      */
    /* pixmap: where the gc will be used */
    /* mode == 0 --> black on white      */
    /* mode == 1 --> white on black      */
{
    XGCValues values;
    unsigned long mask = 0;

    mask = GCForeground | GCBackground | GCFont | GCFunction;
    if (!mode) {
	values.foreground = BlackPixel(dpy, scr);
	values.background = WhitePixel(dpy, scr);
    } else {
	/**
	 * swap colors
	 */
	values.foreground = WhitePixel(dpy, scr);
	values.background = BlackPixel(dpy, scr);
    }
    values.function = GXcopy;
    values.font = font->fid;

    *gc = XCreateGC(dpy, pixmap, mask, &values);
}

/**
 * display help.
 */
void
DisplayHelpWindow(plot_struct* plot)
{
    switch (gp4mouse.graph) {
	case graph3d:
	    {
		static char helptext[] =
		    "    Mousing                              Other hotkeys\n"
		    "    -------                              -------------\n"
		    "m   toggle mouse on/off              l   toggle lin/log y\n"
		    "                                     g   toggle grid on/off\n"
		    "                                     a   set autoscale + replot\n"
		    "                                     e   replot\n"
		    "                                     h   this help\n"
		    "                                     q   quit (close) X11 terminal\n"
		    "                                     <space>  raise gnuplot's window\n"
		    "<Up> <Down> <Left> <Right>  change view (not implemented yet)\n"
		    "Button 1 motion:            change view (rotation)\n"
		    "Button 2 motion:            change view (scale, z-scale)\n"
		    "\n"
		    "<hit any key to continue>\n";
		DisplayMessageAt(plot, helptext, -1, -1);
	    }
	    break;
	default:
	    {
		static char helptext[] =
		    "    Mousing                              Other hotkeys\n"
		    "    -------                              -------------\n"
		    "m   toggle mouse on/off              l   toggle lin/log y\n"
		    "r   toggle ruler on/off              g   toggle grid on/off\n"
		    "u   unzoom (head of zoom history)    a   set autoscale + replot\n"
		    "p   previous zoom                    e   replot\n"
		    "n   next zoom                        h   this help\n"
		    "                                     q   quit (close) X11 terminal\n"
		    "                                     <space>  raise gnuplot's window\n"
		    "1,2 step over coordinates: real,pixel,screen,xdate,xtime,xdate/time\n"
		    "3,4 step over formats for MB1 double click\n"
		    "5   show pointer-ruler distance in polar coordinates too\n"
		    "Button 1 2x    print coordinates (format according to hotkeys 3,4)\n"
		    "Button 2       temporarily annotate the graph\n"
		    "Button 3       mark zoom region\n"
		    "\n"
		    "<hit any key to continue>\n";
		DisplayMessageAt(plot, helptext, -1, -1);
	    }
	    break;
    }
}

/**
 * split a string at `splitchar'.
 */
int
SplitAt(char** args, int maxargs, char* buf, char splitchar)
{
    int argc = 0;

    while (*buf != '\0' && argc < maxargs) {

        if ((*buf == splitchar))
            *buf++ = '\0';

        if (!(*buf)) /* don't count the terminating NULL */
            break;

        /* Save the argument.  */
        *args++ = buf;
        argc++;

        /* Skip over the argument */
        while ((*buf != '\0') && (*buf != splitchar))
            buf++;
    }

    *args = '\0'; /* terminate */
    return argc;
}

void
xfree(void* fred)
{
    if (fred)
	free(fred);
}

/**
 * display a message in a subwindow at x, y.
 * If -1 == x, display the message window centered
 * on the parent.
 */
void
DisplayMessageAt(plot_struct* plot, char* msg, int sub_x, int sub_y)
{
    static GC gc = (GC) NULL; /* gc for drawing */
    static GC clear_gc = (GC) NULL; /* gc for clearing the pixmap */
    Pixmap pixmap = XCreatePixmap(dpy, root, plot->width, plot->height, D);
    /* int font_height = font->max_bounds.ascent + font->max_bounds.descent; */
    int font_height = vchar;
    int y = 1.5 * font_height;
    int padding = 10;
    int i;
    int max_width = 0;
    char* msg_cpy = strdup(msg); /* SplitAt writes to msg */

    char* argv[0x40]; /* up to 64 lines, should be sufficient */
    int argc = SplitAt(argv, sizeof (argv), msg_cpy, '\n');

    if (!gc) {

	/* only if we're the first time in here ...  */

	/* first we create a gc for clearing the pixmap */
	GetGCBlackAndWhite(&clear_gc, pixmap, 1);

	/* get a simple gc with a high contrast */
	GetGCBlackAndWhite(&gc, pixmap, 0);
    }

    /* clear pixmap */
    XFillRectangle(dpy, pixmap, clear_gc, 0, 0, plot->width, plot->height);

    if (!plot->window) {
	plot->window = pr_window(plot->posn_flags, plot->x, plot->y, plot->width, plot->height);
	++windows_open;
    }

    /* top the window but don't put keyboard or mouse focus into it. */
    if (do_raise)
	XMapRaised(dpy, plot->window);

    for (i = 0; i < argc; i++) {
	int len = strlen(argv[i]);
	int width = XTextWidth(font, argv[i], len);
	if (width > max_width)
	    max_width = width;
	XDrawString(dpy, pixmap, gc, padding, y, argv[i], len);
	y += font_height * 1.2;
    }

    if (!plot->msgwin) {

	int width = max_width + 2 * padding;
	int height = y;

	if (width > plot->width) {
	    width = plot->width;
	    sub_x = 0;
	} else if (sub_x < 0) {
	    sub_x = (plot->width - width) / 2;
	}
	if (height > plot->height) {
	    height = plot->height;
	    sub_y = 0;
	} else if (sub_y < 0) {
	    sub_y = (plot->height - height) / 2;
	}

	/* create subwindow */
	plot->msgwin = XCreateSimpleWindow(
	    dpy,                         /* display                    */
	    plot->window,                /* parent                     */
	    sub_x, sub_y, width, height, /* x, y, width, height        */
	    1,                           /* border width               */
	    BlackPixel(dpy, scr),        /* (unsigned long) border     */
	    WhitePixel(dpy, scr)         /* (unsigned long) background */);

	XMapWindow(dpy, plot->msgwin);
    }

    /* set new pixmap as window background */
    XSetWindowBackgroundPixmap(dpy, plot->msgwin, pixmap);

    /* trigger exposure of background pixmap */
    XClearWindow(dpy, plot->msgwin);

    xfree(msg_cpy);

    /*
    XFlush(dpy);
    */
}

/* erase the last displayed position string */
void
EraseCoords(plot_struct* plot)
{
    if (!gc_xor) {
	GetGCXor(&gc_xor, plot->window);
    }

    if (graph3d != gp4mouse.graph) {
	if (plot->str[0]) {
	    XDrawString(dpy, plot->window, gc_xor, 1, plot->height - 1,
		plot->str, strlen(plot->str));
	}
    }
    /*
    XFlush(dpy);
    */
}

/**
 * convert pixel coordinates to plot coordinates.
 */
void
MousePosToGraphPos(
	double* real_x, double* real_y,
	plot_struct* plot,
	int mx, int my, int mouse_mode)
{
  if (mouse_mode==MOUSE_COORDINATES_PIXELS) {
    *real_x = mx;
    *real_y = my;
    return;
  }
  if (mouse_mode==MOUSE_COORDINATES_SCREEN) {
    *real_x = (double)mx/plot->width;
    *real_y = (double)(plot->height - my)/plot->height;
    return;
  }

  /* fixed resolution, see x11.trm:
     #define X11_XMAX 4096, #define X11_YMAX 4096
  */
  *real_x = mx * 4096.0/plot->width;
  *real_y = (plot->height - my) * 4096.0/plot->height;
  /* main job of transformation, which is not device dependent */
  MousePosToGraphPosReal( real_x, real_y );
}

void
PxPosToMousePos(int* mx, int* my, plot_struct* plot, int px, int py)
{
    *mx = (int) ((double) px * (double) plot->width / 4096.0);
    *my = (int) ((double) plot->height * ((double) 1. - (double) py / 4096.0));
}

void
DrawCoords(plot_struct* plot, int x, int y)
{
    static char str[0xff]; /* XXX */
    double real_x, real_y;

    if (graph3d != gp4mouse.graph) {
	if (!gc_xor) {
	    GetGCXor(&gc_xor, plot->window);
	}

	MousePosToGraphPos(&real_x, &real_y, plot, x, y, mouse_mode);
	sprintf(str, "[%g, %g]", real_x, real_y);
	if (ruler.on) {
	    char p[128];
	    GetRulerString( p, real_x, real_y );
	    strcat(str,p);
	}

	XDrawString(dpy, plot->window, gc_xor, 1, plot->height - 1, str, strlen(str));

	/*
	   XFlush(dpy);
	 */
	strcpy(plot->str, str); /* save string */
    }
}

/* display the mouse position in the lower left corner of the window. */
void
DisplayCoords(plot_struct* plot, int x, int y)
{
    EraseCoords(plot);
    DrawCoords(plot, x, y);
}

#endif

/*---------------------------------------------------------------------------
 *  reset all cursors (since we dont have a record of the previous terminal #)
 *---------------------------------------------------------------------------*/

void
reset_cursor()
{
    int plot_number;
    plot_struct *plot = plot_array;

    for (plot_number = 0, plot = plot_array;
	 plot_number < MAX_WINDOWS;
	 ++plot_number, ++plot) {
	if (plot->window) {
	    FPRINTF((stderr, "Window for plot %d exists\n", plot_number));
	    XUndefineCursor(dpy, plot->window);;
	}
    }

    FPRINTF((stderr, "Cursors reset\n"));
    return;
}

/*-----------------------------------------------------------------------------
 *   resize - rescale last plot if window resized
 *---------------------------------------------------------------------------*/

plot_struct *
find_plot(window)
Window window;
{
    int plot_number;
    plot_struct *plot = plot_array;

    for (plot_number = 0, plot = plot_array;
	 plot_number < MAX_WINDOWS;
	 ++plot_number, ++plot) {
	if (plot->window == window) {
	    FPRINTF((stderr, "Event for plot %d\n", plot_number));
	    return plot;
	}
    }

    FPRINTF((stderr, "Bogus window 0x%x in event !\n", window));
    return NULL;
}

void
process_event(event)
XEvent *event;
{
    plot_struct* plot;
    FPRINTF((stderr, "Event 0x%x\n", event->type));

    switch (event->type) {
    case ConfigureNotify:
	plot = find_plot(event->xconfigure.window);
	/*
	   fprintf (stderr, "(process_event) x = %d\n", event->xconfigure.x);
	   fprintf (stderr, "(process_event) y = %d\n", event->xconfigure.y);
	   fprintf (stderr, "(process_event) width = %d\n", event->xconfigure.width);
	   fprintf (stderr, "(process_event) height = %d\n", event->xconfigure.height);
	 */
	    if (plot) {
		int w = event->xconfigure.width, h = event->xconfigure.height;

		/* store settings in case window is closed then recreated */
		plot->x = event->xconfigure.x;
		plot->y = event->xconfigure.y;
		plot->posn_flags = (plot->posn_flags & ~PPosition) | USPosition;

		if (w > 1 && h > 1 && (w != plot->width || h != plot->height)) {
		    plot->width = w;
		    plot->height = h;
		    plot->posn_flags = (plot->posn_flags & ~PSize) | USSize;
		    if (plot->pixmap) {
			/* it is the wrong size now */
			FPRINTF((stderr, "Free pixmap %d\n", 0));
			XFreePixmap(dpy, plot->pixmap);
			plot->pixmap = None;
		    }
		    display(plot);
		}
	    }
	    break;
    case KeyPress:
	plot = find_plot(event->xkey.window);
#ifdef USE_MOUSE
	plot->pointer_x = event->xkey.x;
	plot->pointer_y = event->xkey.y;
#if 0
	fprintf(stderr,"KEY: %i\t",(int)event->xkey.keycode);
#endif
	/* *any* key will cancel zooming, but as ESC is the
	 * correct key to abort zooming all other keys will
	 * ring the terminal bell.
	 */
	if (ZoomActive(plot)) {
	    ZoomCancel(plot);
	    if (event->xkey.keycode != KEYCODE(ESC)) {
		alert();
	    }
	    break; /* no further event processing */
	}
	if (plot->msgwin) {
	    /**
	     * if the help window is displayed, any key
	     * removes the help window and restores the
	     * plot.
	     */
	    XDestroyWindow(dpy, plot->msgwin);
	    plot->msgwin = (Window) 0;
	    UpdateWindow(plot);
	} else if (event->xkey.keycode == KEYCODE('h')) {
	    DisplayHelpWindow(plot);
	} else if (event->xkey.keycode == KEYCODE('m')) {
	    if (useMouse) {
		useMouse = 0;
		plot->button = 0;
		plot->zoom[0] = NOT_AVAILABLE;
		plot->str[0] = '\0';
		display(plot);
	    } else {
		useMouse = 1;
		display(plot);
		/* DisplayCoords(plot, event->xkey.x, event->xkey.y); */
	    }
	} else if (event->xkey.keycode == KEYCODE('n')) {
	    ZoomNext(plot);
	} else if (event->xkey.keycode == KEYCODE('p')) {
	    ZoomPrevious(plot);
	} else if (event->xkey.keycode == KEYCODE('u')) {
	    ZoomUnzoom(plot);
	} else if (event->xkey.keycode == KEYCODE('a')) {
	    gp_execute("set autoscale; replot");
	} else if (event->xkey.keycode == KEYCODE('e')) {
	    gp_execute("replot");
	} else if (event->xkey.keycode == KEYCODE('l')) {
	    if (gp4mouse.is_log_y) gp_execute("set nolog; replot");
	      else gp_execute("set log y; replot");
	} else if (event->xkey.keycode == KEYCODE('g')) {
#if 0
	    Bool propagate = True;
	    XKeyEvent event;
	    event.keycode = KEYCODE('q');
	    XSetInputFocus(dpy, gnuplotXID, 0 /* revert_to ?? */, CurrentTime);
	    /* XFlush(dpy); */
	    XSendEvent(dpy, gnuplotXID, propagate, 0, (XEvent*) &event);
	    /* XFlush(dpy); */
#endif
	    if (gp4mouse.has_grid)
		gp_execute("set nogrid; replot");
	    else
		gp_execute("set mxtics 2; set mytics 2; set grid x y mx my; replot");
	} else if (event->xkey.keycode == KEYCODE(' ')) {
	    if (gnuplotXID) {
		XMapRaised(dpy, gnuplotXID);
		XSetInputFocus(dpy, gnuplotXID, 0 /* revert_to ?? */, CurrentTime);
		XFlush(dpy);
	    }
	} else if (event->xkey.keycode == KEYCODE('1')) {
	    mouse_mode--;
	    if (mouse_mode < MOUSE_COORDINATES_REAL)
		mouse_mode = MOUSE_COORDINATES_XDATETIME;
	    fprintf(stderr, "Mouse coordinates: %s\n",MouseCoordinatesHelpStrings[mouse_mode]);
	    if (!IGNORE_MOUSE)
		DisplayCoords(plot, event->xkey.x, event->xkey.y);
	} else if (event->xkey.keycode == KEYCODE('2')) {
	    mouse_mode++;
	    if (mouse_mode > MOUSE_COORDINATES_XDATETIME)
		mouse_mode = MOUSE_COORDINATES_REAL;
	    fprintf(stderr, "Mouse coordinates: %s\n",MouseCoordinatesHelpStrings[mouse_mode]);
	    if (!IGNORE_MOUSE)
		DisplayCoords(plot, event->xkey.x, event->xkey.y);
	} else if (event->xkey.keycode == KEYCODE('3')) {
	    mouseSprintfFormat--;
	    if (mouseSprintfFormat < 0)
		mouseSprintfFormat = nMouseSprintfFormats - 1;
	    fprintf(stderr, "Format for double click is: %s\n", MouseSprintfFormats[mouseSprintfFormat] );
	} else if (event->xkey.keycode == KEYCODE('4')) {
	    mouseSprintfFormat++;
	    if (mouseSprintfFormat >= nMouseSprintfFormats)
		mouseSprintfFormat = 0;
	    fprintf(stderr, "Format for double click is: %s\n", MouseSprintfFormats[mouseSprintfFormat] );
	} else if (event->xkey.keycode == KEYCODE('5')) {
	    mousePolarDistance = !mousePolarDistance ;
	    fprintf(stderr,"Turning distance in polar coordinates %s\n",(mousePolarDistance ? "on":"off"));
	    if (!IGNORE_MOUSE && ruler.on)
		DisplayCoords(plot, event->xkey.x, event->xkey.y);
	} else if (event->xkey.keycode == KEYCODE('r')) {
	    /* ruler can be on even when mouse is turned off */
	    if (!ruler.on) {
		ruler.on = 1;
		/* create new ruler */
		ruler.px = event->xkey.x;
		ruler.py = event->xkey.y;
		MousePosToGraphPos( &ruler.x, &ruler.y, plot, /* mouse->real graph coordinates */
			event->xkey.x, event->xkey.y, MOUSE_COORDINATES_REAL );
		EventuallyDrawRuler(plot);
		if (!IGNORE_MOUSE)
		    DisplayCoords(plot, event->xkey.x, event->xkey.y);
	    } else {
		/* erase existing ruler */
		ruler.on = 0;
		plot->str[0] = '\0';
		UpdateWindow(plot);
	    }
	} else
#endif
	    /**
	     * close X window
	     */
	    if (event->xkey.keycode == KEYCODE('q')) {
	    if (plot)
		delete_plot(plot);
	}
	break;
    case ClientMessage:
	if (event->xclient.message_type == WM_PROTOCOLS &&
	    event->xclient.format == 32 &&
	    event->xclient.data.l[0] == WM_DELETE_WINDOW) {
	    plot = find_plot(event->xclient.window);
	    if (plot) {
		delete_plot(plot);
	}
	}
	break;
#ifdef USE_MOUSE
    case Expose:
	/*
	 * we need to handle expose events here, because
	 * there might stuff like rulers which has to
	 * be redrawn. Well. It's not really hard to handle
	 * this. Note that the x and y fields are not used
	 * to update the pointer pos because the pointer
	 * might be on another window which generates the
	 * expose events. (joze)
	 */
	plot = find_plot(event->xexpose.window);
	if (!event->xexpose.count) {
	    /* XXX jitters display while resizing */
	    UpdateWindow(plot);
	}
	break;
    case MotionNotify:
	plot = find_plot(event->xmotion.window);
	if (!IGNORE_MOUSE) {
	    switch (gp4mouse.graph) {
		case graph3d: /* splot */
		    MotionChangeView(plot, event->xbutton.x, event->xbutton.y);
		default:
		    if (ZoomActive(plot)) {
			ZoomUpdate(plot, event->xmotion.x, event->xmotion.y);
			break; /* no further event processing */
		    }
		    if (plot->button & button2) {
			ButtonMotion2(plot, event->xbutton.x, event->xbutton.y);
		    }
		    DisplayCoords(plot, event->xmotion.x, event->xmotion.y);
		    /* must be done *after* dispatching the button events */
		    SetPointer(plot, event->xmotion.x, event->xmotion.y);
		    break;
	    }
	}
	break;
    case ButtonPress:
	plot = find_plot(event->xmotion.window);
	if (!IGNORE_MOUSE) {

	    int doubleclick = SetTime(plot, event->xbutton.time);

	    switch (gp4mouse.graph) {
		case graph3d: /* splot */
		    switch (event->xbutton.button) {
			case 1:
			    ButtonPress1_graph3d(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 2:
			    ButtonPress2_graph3d(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 3:
			    break;
		    }
	break;
		default:
		    switch (event->xbutton.button) {
			case 1:
			    if (ZoomActive(plot)) {
				ZoomRegionFinish(plot, event->xbutton.x, event->xbutton.y);
			    } else if (doubleclick) {
				DoubleClick1(plot, event->xbutton.x, event->xbutton.y);
			    }
			    break;
			case 2:
			    if (ZoomActive(plot)) {
				/* ZoomCancel(plot); */
				ZoomRegionFinish(plot, event->xbutton.x, event->xbutton.y);
				break; /* no further event processing */
			    }
			    ButtonPress2(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 3:
			    if (ZoomActive(plot)) {
				/* ZoomCancel(plot); */
				ZoomRegionFinish(plot, event->xbutton.x, event->xbutton.y);
				break; /* no further event processing */
			    }
			    ButtonPress3(plot, event->xbutton.x, event->xbutton.y);
			    break;
		    }
		    break;
	    }
	    /* must be done *after* dispatching the button events */
	    SetPointer(plot, event->xbutton.x, event->xbutton.y);
	}
	break;
    case ButtonRelease:
	plot = find_plot(event->xmotion.window);
	if (!IGNORE_MOUSE) {
	    SetPointer(plot, event->xbutton.x, event->xbutton.y);
	    switch (gp4mouse.graph) {
		case graph3d: /* splot */
		    switch (event->xbutton.button) {
			case 1:
			    ButtonRelease1_graph3d(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 2:
			    ButtonRelease2_graph3d(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 3:
			    break;
		    }
		    break;
		default:
		    switch (event->xbutton.button) {
			case 1:
			    break;
			case 2:
			    ButtonRelease2(plot, event->xbutton.x, event->xbutton.y);
			    break;
			case 3:
			    ButtonRelease3(plot, event->xbutton.x, event->xbutton.y);
			    break;
		    }
		    break;
	    }
	}
	break;
#endif /* USE_MOUSE */
#ifdef EXPORT_SELECTION
    case SelectionNotify:
    case SelectionRequest:
	handle_selection_event(event);
	break;
#endif
    }
}

/*-----------------------------------------------------------------------------
 *   preset - determine options, open display, create window
 *---------------------------------------------------------------------------*/
/*
#define On(v) ( !strcmp(v,"on") || !strcmp(v,"true") || \
                !strcmp(v,"On") || !strcmp(v,"True") || \
                !strcmp(v,"ON") || !strcmp(v,"TRUE") )
*/
#define On(v) ( !strncasecmp(v,"on",2) || !strncasecmp(v,"true",4) )

#define AppDefDir "/usr/lib/X11/app-defaults"
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

static XrmDatabase dbCmd, dbApp, dbDef, dbEnv, db = (XrmDatabase) 0;

char *getenv(), *type[20];
XrmValue value;

static XrmOptionDescRec options[] = {
    {"-mono", ".mono", XrmoptionNoArg, (caddr_t) "on"},
    {"-gray", ".gray", XrmoptionNoArg, (caddr_t) "on"},
    {"-clear", ".clear", XrmoptionNoArg, (caddr_t) "on"},
    {"-tvtwm", ".tvtwm", XrmoptionNoArg, (caddr_t) "on"},
    {"-pointsize", ".pointsize", XrmoptionSepArg, (caddr_t) NULL},
    {"-display", ".display", XrmoptionSepArg, (caddr_t) NULL},
    {"-name", ".name", XrmoptionSepArg, (caddr_t) NULL},
    {"-geometry", "*geometry", XrmoptionSepArg, (caddr_t) NULL},
    {"-background", "*background", XrmoptionSepArg, (caddr_t) NULL},
    {"-bg", "*background", XrmoptionSepArg, (caddr_t) NULL},
    {"-foreground", "*foreground", XrmoptionSepArg, (caddr_t) NULL},
    {"-fg", "*foreground", XrmoptionSepArg, (caddr_t) NULL},
    {"-bordercolor", "*bordercolor", XrmoptionSepArg, (caddr_t) NULL},
    {"-bd", "*bordercolor", XrmoptionSepArg, (caddr_t) NULL},
    {"-borderwidth", ".borderwidth", XrmoptionSepArg, (caddr_t) NULL},
    {"-bw", ".borderwidth", XrmoptionSepArg, (caddr_t) NULL},
    {"-font", "*font", XrmoptionSepArg, (caddr_t) NULL},
    {"-fn", "*font", XrmoptionSepArg, (caddr_t) NULL},
    {"-reverse", "*reverseVideo", XrmoptionNoArg, (caddr_t) "on"},
    {"-rv", "*reverseVideo", XrmoptionNoArg, (caddr_t) "on"},
    {"+rv", "*reverseVideo", XrmoptionNoArg, (caddr_t) "off"},
    {"-iconic", "*iconic", XrmoptionNoArg, (caddr_t) "on"},
    {"-synchronous", "*synchronous", XrmoptionNoArg, (caddr_t) "on"},
    {"-xnllanguage", "*xnllanguage", XrmoptionSepArg, (caddr_t) NULL},
    {"-selectionTimeout", "*selectionTimeout", XrmoptionSepArg, (caddr_t) NULL},
    {"-title", ".title", XrmoptionSepArg, (caddr_t) NULL},
    {"-xrm", NULL, XrmoptionResArg, (caddr_t) NULL},
    {"-raise", "*raise", XrmoptionNoArg, (caddr_t) "on"},
    {"-noraise", "*raise", XrmoptionNoArg, (caddr_t) "off"},
    {"-persist", "*persist", XrmoptionNoArg, (caddr_t) "on"}
};

#define Nopt (sizeof(options) / sizeof(options[0]))

void
preset(argc, argv)
int argc;
char *argv[];
{
    int Argc = argc;
    char **Argv = argv;

#ifdef VMS
    char *ldisplay = (char *) 0;
#else
    char *ldisplay = getenv("DISPLAY");
#endif
    char *home = getenv("HOME");
    char *server_defaults, *env, buffer[256];

    /* avoid bus error when env vars are not set */
    if (ldisplay == NULL)
	ldisplay = "";
    if (home == NULL)
	home = "";

/*---set to ignore ^C and ^Z----------------------------------------------*/

    signal(SIGINT, SIG_IGN);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif

/*---prescan arguments for "-name"----------------------------------------*/

    while (++Argv, --Argc > 0) {
	if (!strcmp(*Argv, "-name") && Argc > 1) {
	    strncpy(Name, Argv[1], sizeof(Name) - 1);
	    strncpy(Class, Argv[1], sizeof(Class) - 1);
	    /* just in case */
	    Name[sizeof(Name)-1] = NUL;
	    Class[sizeof(Class)-1] = NUL;
	    if (Class[0] >= 'a' && Class[0] <= 'z')
		Class[0] -= 0x20;
	}
    }
    Argc = argc;
    Argv = argv;

/*---parse command line---------------------------------------------------*/

    XrmInitialize();
    XrmParseCommand(&dbCmd, options, Nopt, Name, &Argc, Argv);
    if (Argc > 1) {
	fprintf(stderr, "\n\
gnuplot: bad option: %s\n\
gnuplot: X11 aborted.\n", Argv[1]);
	EXIT(1);
    }
    if (pr_GetR(dbCmd, ".display"))
	ldisplay = (char *) value.addr;

/*---open display---------------------------------------------------------*/

    dpy = XOpenDisplay(ldisplay);
    if (!dpy) {
	fprintf(stderr, "\n\
gnuplot: unable to open display '%s'\n\
gnuplot: X11 aborted.\n", ldisplay);
	EXIT(1);
    }
    scr = DefaultScreen(dpy);
    vis = DefaultVisual(dpy, scr);
    D = DefaultDepth(dpy, scr);
    root = DefaultRootWindow(dpy);
    server_defaults = XResourceManagerString(dpy);

/*---get symcode for key q ---*/

    KEYCODE('q') = XKeysymToKeycode(dpy, XK_q);
#ifdef USE_MOUSE
    KEYCODE('a') = XKeysymToKeycode(dpy, XK_a);
    KEYCODE('c') = XKeysymToKeycode(dpy, XK_c);
    KEYCODE('e') = XKeysymToKeycode(dpy, XK_e);
    KEYCODE('g') = XKeysymToKeycode(dpy, XK_g);
    KEYCODE('h') = XKeysymToKeycode(dpy, XK_h);
    KEYCODE('l') = XKeysymToKeycode(dpy, XK_l);
    KEYCODE('m') = XKeysymToKeycode(dpy, XK_m);
    KEYCODE('n') = XKeysymToKeycode(dpy, XK_n);
    KEYCODE('p') = XKeysymToKeycode(dpy, XK_p);
    KEYCODE('r') = XKeysymToKeycode(dpy, XK_r);
    KEYCODE('u') = XKeysymToKeycode(dpy, XK_u);
    KEYCODE(' ') = XKeysymToKeycode(dpy, XK_space);
    KEYCODE('1') = XKeysymToKeycode(dpy, XK_1);
    KEYCODE('2') = XKeysymToKeycode(dpy, XK_2);
    KEYCODE('3') = XKeysymToKeycode(dpy, XK_3);
    KEYCODE('4') = XKeysymToKeycode(dpy, XK_4);
    KEYCODE('5') = XKeysymToKeycode(dpy, XK_5);
    KEYCODE(ESC) = XKeysymToKeycode(dpy, XK_Escape);
#endif

/**** atoms we will need later ****/

    WM_PROTOCOLS = XInternAtom(dpy, "WM_PROTOCOLS", False);
    WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);


/*---get application defaults--(subset of Xt processing)------------------*/

#ifdef VMS
    strcpy(buffer, "DECW$USER_DEFAULTS:GNUPLOT_X11.INI");
#elif defined OS2
/* for Xfree86 ... */
    {
	char *appdefdir = "XFree86/lib/X11/app-defaults";
	char *xroot = getenv("X11ROOT");
	sprintf(buffer, "%s/%s/%s", xroot, appdefdir, "Gnuplot");
    }
# else /* !OS/2 */
    strcpy(buffer, AppDefDir);
    strcat(buffer, "/");
    strcat(buffer, "Gnuplot");
#endif /* !VMS */

    dbApp = XrmGetFileDatabase(buffer);
    XrmMergeDatabases(dbApp, &db);

/*---get server or ~/.Xdefaults-------------------------------------------*/

    if (server_defaults)
	dbDef = XrmGetStringDatabase(server_defaults);
    else {
#ifdef VMS
	strcpy(buffer, "DECW$USER_DEFAULTS:DECW$XDEFAULTS.DAT");
#else
	strcpy(buffer, home);
	strcat(buffer, ".Xdefaults");
#endif
	dbDef = XrmGetFileDatabase(buffer);
    }
    XrmMergeDatabases(dbDef, &db);

/*---get XENVIRONMENT or  ~/.Xdefaults-hostname---------------------------*/

#ifndef VMS
    if ((env = getenv("XENVIRONMENT")) != NULL)
	dbEnv = XrmGetFileDatabase(env);
    else {
	char *p = NULL, host[MAXHOSTNAMELEN];

	if (GP_SYSTEMINFO(host) < 0) {
	    fprintf(stderr, "gnuplot: %s failed. X11 aborted.\n", SYSINFO_METHOD);
	    EXIT(1);
	}
	if ((p = strchr(host, '.')) != NULL)
	    *p = '\0';
	strcpy(buffer, home);
	strcat(buffer, "/.Xdefaults-");
	strcat(buffer, host);
	dbEnv = XrmGetFileDatabase(buffer);
    }
    XrmMergeDatabases(dbEnv, &db);
#endif /* not VMS */

/*---merge command line options-------------------------------------------*/

    XrmMergeDatabases(dbCmd, &db);

/*---set geometry, font, colors, line widths, dash styles, point size-----*/

    pr_geometry();
    pr_font();
    pr_color();
    pr_width();
    pr_dashes();
    pr_pointsize();
    pr_raise();
    pr_persist();
}

/*-----------------------------------------------------------------------------
 *   pr_GetR - get resource from database using "-name" option (if any)
 *---------------------------------------------------------------------------*/

char *
pr_GetR(xrdb, resource)
XrmDatabase xrdb;
char *resource;
{
    char name[128], class[128], *rc;

    strcpy(name, Name);
    strcat(name, resource);
    strcpy(class, Class);
    strcat(class, resource);
    rc = XrmGetResource(xrdb, name, class, type, &value)
    ? (char *) value.addr
    : (char *) 0;
    return (rc);
}

/*-----------------------------------------------------------------------------
 *   pr_color - determine color values
 *---------------------------------------------------------------------------*/

char color_keys[Ncolors][30] = {
    "background", "bordercolor", "text", "border", "axis",
    "line1", "line2", "line3", "line4",
    "line5", "line6", "line7", "line8"
};
char color_values[Ncolors][30] = {
    "white", "black", "black", "black", "black",
    "red", "green", "blue", "magenta",
    "cyan", "sienna", "orange", "coral"
};
char gray_values[Ncolors][30] = {
    "black", "white", "white", "gray50", "gray50",
    "gray100", "gray60", "gray80", "gray40",
    "gray90", "gray50", "gray70", "gray30"
};

void
pr_color()
{
    unsigned long black = BlackPixel(dpy, scr), white = WhitePixel(dpy, scr);
    char option[20], color[30], *v, *ctype;
    XColor xcolor;
    Colormap cmap;
    double intensity = -1;
    int n;

    pr_GetR(db, ".mono") && On(value.addr) && Mono++;
    pr_GetR(db, ".gray") && On(value.addr) && Gray++;
    pr_GetR(db, ".reverseVideo") && On(value.addr) && Rv++;

    if (!Gray && (vis->class == GrayScale || vis->class == StaticGray))
	Mono++;

    if (!Mono) {
	cmap = DefaultColormap(dpy, scr);
	ctype = (Gray) ? "Gray" : "Color";

	for (n = 0; n < Ncolors; n++) {
	    strcpy(option, ".");
	    strcat(option, color_keys[n]);
	    (n > 1) && strcat(option, ctype);
	    v = pr_GetR(db, option)
		? (char *) value.addr
		: ((Gray) ? gray_values[n] : color_values[n]);

	    if (sscanf(v, "%30[^,],%lf", color, &intensity) == 2) {
		if (intensity < 0 || intensity > 1) {
		    fprintf(stderr, "\ngnuplot: invalid color intensity in '%s'\n",
			    color);
		    intensity = 1;
		}
	    } else {
		strcpy(color, v);
		intensity = 1;
	    }

	    if (!XParseColor(dpy, cmap, color, &xcolor)) {
		fprintf(stderr, "\ngnuplot: unable to parse '%s'. Using black.\n",
			color);
		colors[n] = black;
	    } else {
		xcolor.red *= intensity;
		xcolor.green *= intensity;
		xcolor.blue *= intensity;
		if (XAllocColor(dpy, cmap, &xcolor)) {
		    colors[n] = xcolor.pixel;
		} else {
		    fprintf(stderr, "\ngnuplot: can't allocate '%s'. Using black.\n",
			    v);
		    colors[n] = black;
		}
	    }
	}
    } else {
	colors[0] = (Rv) ? black : white;
	for (n = 1; n < Ncolors; n++)
	    colors[n] = (Rv) ? white : black;
    }
}

/*-----------------------------------------------------------------------------
 *   pr_dashes - determine line dash styles 
 *---------------------------------------------------------------------------*/

char dash_keys[Ndashes][10] = {
    "border", "axis",
    "line1", "line2", "line3", "line4", "line5", "line6", "line7", "line8"
};

char dash_mono[Ndashes][10] = {
    "0", "16",
    "0", "42", "13", "44", "15", "4441", "42", "13"
};

char dash_color[Ndashes][10] = {
    "0", "16",
    "0", "0", "0", "0", "0", "0", "0", "0"
};

void
pr_dashes()
{
    int n, j, l, ok;
    char option[20], *v;

    for (n = 0; n < Ndashes; n++) {
	strcpy(option, ".");
	strcat(option, dash_keys[n]);
	strcat(option, "Dashes");
	v = pr_GetR(db, option)
	    ? (char *) value.addr
	    : ((Mono) ? dash_mono[n] : dash_color[n]);
	l = strlen(v);
	if (l == 1 && *v == '0') {
	    dashes[n][0] = (unsigned char) 0;
	    continue;
	}
	for (ok = 0, j = 0; j < l; j++) {
	    v[j] >= '1' && v[j] <= '9' && ok++;
	}
	if (ok != l || (ok != 2 && ok != 4)) {
	    fprintf(stderr, "gnuplot: illegal dashes value %s:%s\n", option, v);
	    dashes[n][0] = (unsigned char) 0;
	    continue;
	}
	for (j = 0; j < l; j++) {
	    dashes[n][j] = (unsigned char) (v[j] - '0');
	}
	dashes[n][l] = (unsigned char) 0;
    }
}

/*-----------------------------------------------------------------------------
 *   pr_font - determine font          
 *---------------------------------------------------------------------------*/

void
pr_font()
{
    char *fontname = pr_GetR(db, ".font");

    if (!fontname)
	fontname = FallbackFont;
    font = XLoadQueryFont(dpy, fontname);
    if (!font) {
	fprintf(stderr, "\ngnuplot: can't load font '%s'\n", fontname);
	fprintf(stderr, "gnuplot: using font '%s' instead.\n", FallbackFont);
	font = XLoadQueryFont(dpy, FallbackFont);
	if (!font) {
	    fprintf(stderr, "\
gnuplot: can't load font '%s'\n\
gnuplot: no useable font - X11 aborted.\n", FallbackFont);
	    EXIT(1);
	}
    }
    vchar = font->ascent + font->descent;
}

/*-----------------------------------------------------------------------------
 *   pr_geometry - determine window geometry      
 *---------------------------------------------------------------------------*/

void
pr_geometry()
{
    char *geometry = pr_GetR(db, ".geometry");
    int x, y, flags;
    unsigned int w, h;

    if (geometry) {
	flags = XParseGeometry(geometry, &x, &y, &w, &h);
	if (flags & WidthValue)
	    gW = w;
	if (flags & HeightValue)
	    gH = h;
	if (flags & (WidthValue | HeightValue))
	    gFlags = (gFlags & ~PSize) | USSize;

	if (flags & XValue)
	    gX = (flags & XNegative) ? x + DisplayWidth(dpy, scr) - gW - BorderWidth * 2 : x;

	if (flags & YValue)
	    gY = (flags & YNegative) ? y + DisplayHeight(dpy, scr) - gH - BorderWidth * 2 : y;

	if (flags & (XValue | YValue))
	    gFlags = (gFlags & ~PPosition) | USPosition;
    }
}

/*-----------------------------------------------------------------------------
 *   pr_pointsize - determine size of points for 'points' plotting style
 *---------------------------------------------------------------------------*/

void
pr_pointsize()
{
    if (pr_GetR(db, ".pointsize")) {
	if (sscanf((char *) value.addr, "%lf", &pointsize) == 1) {
	    if (pointsize <= 0 || pointsize > 10) {
		fprintf(stderr, "\ngnuplot: invalid pointsize '%s'\n", value.addr);
		pointsize = 1;
	    }
	} else {
	    fprintf(stderr, "\ngnuplot: invalid pointsize '%s'\n", value.addr);
	    pointsize = 1;
	}
    } else {
	pointsize = 1;
    }
}

/*-----------------------------------------------------------------------------
 *   pr_width - determine line width values
 *---------------------------------------------------------------------------*/

char width_keys[Nwidths][30] = {
    "border", "axis",
    "line1", "line2", "line3", "line4", "line5", "line6", "line7", "line8"
};

void
pr_width()
{
    int n;
    char option[20], *v;

    for (n = 0; n < Nwidths; n++) {
	strcpy(option, ".");
	strcat(option, width_keys[n]);
	strcat(option, "Width");
	if ((v = pr_GetR(db, option)) != NULL) {
	    if (*v < '0' || *v > '4' || strlen(v) > 1)
		fprintf(stderr, "gnuplot: illegal width value %s:%s\n", option, v);
	    else
		widths[n] = (unsigned int) atoi(v);
	}
    }
}

/*-----------------------------------------------------------------------------
 *   pr_window - create window 
 *---------------------------------------------------------------------------*/

Window
pr_window(flags, x, y, width, height)
unsigned int flags;
int x, y;
unsigned int width, height;
{
    char *title = pr_GetR(db, ".title");
    static XSizeHints hints;
    int Tvtwm = 0;

    Window win = XCreateSimpleWindow(dpy, root, x, y, width, height, BorderWidth,
				     colors[1], colors[0]);

    /* ask ICCCM-compliant window manager to tell us when close window
     * has been chosen, rather than just killing us
     */

    XChangeProperty(dpy, win, WM_PROTOCOLS, XA_ATOM, 32, PropModeReplace,
		    (unsigned char *) &WM_DELETE_WINDOW, 1);

    pr_GetR(db, ".clear") && On(value.addr) && Clear++;
    pr_GetR(db, ".tvtwm") && On(value.addr) && Tvtwm++;

    if (!Tvtwm) {
	hints.flags = flags;
    } else {
	hints.flags = (flags & ~USPosition) | PPosition;	/* ? */
    }
    hints.x = gX;
    hints.y = gY;
    hints.width = width;
    hints.height = height;

    XSetNormalHints(dpy, win, &hints);

    if (pr_GetR(db, ".iconic") && On(value.addr)) {
	XWMHints wmh;

	wmh.flags = StateHint;
	wmh.initial_state = IconicState;
	XSetWMHints(dpy, win, &wmh);
    }
    XStoreName(dpy, win, ((title) ? title : Class));

    XSelectInput(dpy, win, KeyPressMask | StructureNotifyMask
	| PointerMotionMask | ButtonPressMask | ButtonReleaseMask
	| ExposureMask);
    XMapWindow(dpy, win);

    return win;
}


/***** pr_raise ***/
void
pr_raise()
{
    if (pr_GetR(db, ".raise"))
	do_raise = (On(value.addr));
}


void
pr_persist()
{
    if (pr_GetR(db, ".persist"))
	persist = (On(value.addr));
}

/************ code to handle selection export *********************/

#ifdef EXPORT_SELECTION

/* bit of a bodge, but ... */
static struct plot_struct *exported_plot;

void
export_graph(plot)
struct plot_struct *plot;
{
    FPRINTF((stderr, "export_graph(0x%x)\n", plot));

    XSetSelectionOwner(dpy, EXPORT_SELECTION, plot->window, CurrentTime);
    /* to check we have selection, we would have to do a
     * GetSelectionOwner(), but if it failed, it failed - no big deal
     */
    exported_plot = plot;
}

void
handle_selection_event(event)
XEvent *event;
{
    switch (event->type) {
    case SelectionRequest:
	{
	    XEvent reply;

	    static Atom XA_TARGETS = (Atom) 0;
	    if (XA_TARGETS == 0)
		XA_TARGETS = XInternAtom(dpy, "TARGETS", False);

	    reply.type = SelectionNotify;
	    reply.xselection.send_event = True;
	    reply.xselection.display = event->xselectionrequest.display;
	    reply.xselection.requestor = event->xselectionrequest.requestor;
	    reply.xselection.selection = event->xselectionrequest.selection;
	    reply.xselection.target = event->xselectionrequest.target;
	    reply.xselection.property = event->xselectionrequest.property;
	    reply.xselection.time = event->xselectionrequest.time;

	    FPRINTF((stderr, "selection request\n"));

	    if (reply.xselection.target == XA_TARGETS) {
		static Atom targets[] = { XA_PIXMAP, XA_COLORMAP };

		FPRINTF((stderr, "Targets request from %d\n", reply.xselection.requestor));

		XChangeProperty(dpy, reply.xselection.requestor,
				reply.xselection.property, reply.xselection.target, 32, PropModeReplace,
				(unsigned char *) targets, 2);
	    } else if (reply.xselection.target == XA_COLORMAP) {
		Colormap cmap = DefaultColormap(dpy, 0);

		FPRINTF((stderr, "colormap request from %d\n", reply.xselection.requestor));

		XChangeProperty(dpy, reply.xselection.requestor,
				reply.xselection.property, reply.xselection.target, 32, PropModeReplace,
				(unsigned char *) &cmap, 1);
	    } else if (reply.xselection.target == XA_PIXMAP) {

		FPRINTF((stderr, "pixmap request from %d\n", reply.xselection.requestor));

		XChangeProperty(dpy, reply.xselection.requestor,
				reply.xselection.property, reply.xselection.target, 32, PropModeReplace,
				(unsigned char *) &(exported_plot->pixmap), 1);
	    } else {
		reply.xselection.property = None;
	    }

	    XSendEvent(dpy, reply.xselection.requestor, False, 0L, &reply);
	    /* we never block on XNextEvent(), so must flush manually
	     * (took me *ages* to find this out !)
	     */

	    XFlush(dpy);
	}
	break;
    }
}

#endif /* EXPORT_SELECTION */
