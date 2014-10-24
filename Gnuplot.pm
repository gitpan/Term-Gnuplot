package Term::Gnuplot;

=head1 NAME

Term::Gnuplot - lowlevel graphics using gnuplot drawing routines.

=head1 SYNOPSIS

  use Term::Gnuplot ':ALL';
  list_terms();
  change_term('dumb') or die "Cannot set terminal.\n";
  term_init();				# init()
  term_start_plot();			# graphics();
  $xmax = scaled_xmax();
  $ymax = scaled_ymax();
  linetype(-2);
  move(0,0);
  vector($xmax-1,0);
  vector($xmax-1,$ymax-1);
  vector(0,$ymax-1);
  vector(0,0);
  justify_text(LEFT);
  put_text(h_char()*5, $ymax - v_char()*3,"Terminal Test, Perl");
  $x = $xmax/4;
  $y = $ymax/4;
  $xl = h_tic()*5;
  $yl = v_tic()*5;
  linetype(2);
  arrow($x,$y,$x+$xl,$y,1);
  arrow($x,$y,$x+$xl/2,$y+$yl,1);
  arrow($x,$y,$x,$y+$yl,1);
  arrow($x,$y,$x-$xl/2,$y+$yl,0);
  arrow($x,$y,$x-$xl,$y,1);
  arrow($x,$y,$x-$xl,$y-$yl,1);
  arrow($x,$y,$x,$y-$yl,1);
  arrow($x,$y,$x+$xl,$y-$yl,1);
  term_end_plot();			# text();
  Term::Gnuplot::reset();

=head1 EXPORTS

None by default.

=head2 Exportable and export tags:

=over

=item C<:SETUP>

  change_term test_term init_terminal list_terms  plot_outfile_set
  term_init term_start_plot term_end_plot
  term_start_multiplot term_end_multiplot plotsizes_scale
  setcanvas

=item C<:JUSTIFY>

  LEFT CENTRE RIGHT

=item C<:FIELDS>

  name description xmax ymax v_char h_char v_tic h_tic
  scaled_xmax scaled_ymax

=item C<:METHODS>

  init scale graphics linetype move vector point text_angle
  justify_text put_text arrow text

=item %description

Hash of C<name =E<gt> description> pairs.  In particular, C<keys
%description> contains names of supported terminals.

=item C<:ALL>

All of the above.

=back

=head1 DESCRIPTION

Below I include the contents of the file F<term/README> from gnuplot
distribution (see L<gnuplot F<term/README>>). It explains
the meaning of the methods of L<"SYNOPSIS">.

All methods are supported under Perl, the C<options> method is available as
set_options(). The discription below includes underscores, that are
deleted in the perl interface.

The functions which are not included in the description below:

=over

=item C<change_term($newname)>

=item test_term()

=item init_terminal()

self-explanatory. 

=item list_terms()

Print names/descriptions of supported terminals.

=item C<plot_outfile_set($filename)>

set the output file.  This should be done before setting the terminal type.

=item C<plotsizes_scale($xfactor, $yfactor)>

set the size of the output device (such as a graphic file) in fractions
of the default size.
Some output drivers may ignore this request.  The request is active until the
next request.

Does not change sizes reported by xmax() and ymax()!  After such a call
one should use the following calls:

=item scaled_xmax(), scaled_ymax()

Report xmax() and ymax() corrected by the scaling factors given to
plotsizes_scale().

=item term_init(), term_start_plot(), term_end_plot()

higher-level functions variants of init(), graphics(), text().  Should
be prefered over init(), graphics(), text() if the output
file is changed, since they take into account resetting of output file
mode (if needed).

=item term_start_multiplot(), term_end_multiplot()

Interfaces to C functions with the same names.  How to use them
outside of C<gnuplot> proper is not clear.

=item _term_descrs()

Returns hash of names/descriptions of supported terminals (available as
%Term::Gnuplot::description too).

=item setcanvas($ptk_canvas)

Sets the Tk widget for direct-draw operations (may be used with
terminal 'tkcanvas' with the option 'tkperl_canvas').

=back

B<NOTE.> Some terminals I<require> calling set_options() before init()!

=head1 gnuplot F<term/README>

DOCUMENTATION FOR GNUPLOT TERMINAL DRIVER WRITERS

By Russell Lang 1/90

Updated for new file layout by drd 4/95

Paragraphs about inclusion of TERM_HELP added by rcc 1/96

No change to the interface between gnuplot and the terminal drivers,
but we would like to make the terminal drivers standalone

1) in order move the support for the terminal drivers outside of the
   support for the main program, thereby encouraging a library of
   contributed drivers
2) To make it easy for users to add contributed drivers, by adding
   a single #include line to term.h
3) To allow individual compilation on DOS, to save the overlay
   manager from having to load _all_ drivers together.

CORRECTION - scale() interface is no longer supported, since it
is incompatible with multiplot.

Whole of terminal driver should be contained in one <driver>.trm file,
with a fairly strict layout as detailed below - this allows the
gnuplot maintainers to change the way the terminal drivers are
compiled without having to change the drivers themselves.

term.h, and therefore each file.trm file, may be loaded more than once,
with different sections selected by macros.

Each driver provides all the functions it needs, and a table of
function pointers and other data to interface to gnuplot.
The table entry is currently defined as follows in plot.h:

  struct TERMENTRY {

  /* required entries */

	char *name;
	char *description;
	unsigned int xmax,ymax,v_char,h_char,v_tic,h_tic;

	void (*options) __PROTO((void));
	void (*init) __PROTO((void));
	void (*reset) __PROTO((void));
	void (*text) __PROTO((void));
	int (*scale) __PROTO((double, double));
	void (*graphics) __PROTO((void));
	void (*move) __PROTO((unsigned int, unsigned int));
	void (*vector) __PROTO((unsigned int, unsigned int));
	void (*linetype) __PROTO((int));
	void (*put_text) __PROTO((unsigned int, unsigned int,char*));

  /* optional entries */

	int (*text_angle) __PROTO((int));
	int (*justify_text) __PROTO((enum JUSTIFY));
	void (*point) __PROTO((unsigned int, unsigned int,int));
	void (*arrow) __PROTO((unsigned int, unsigned int, unsigned int,
			   unsigned int, int));
   int (*set_font) __PROTO((char *font));  /* "font,size" */
	void (*set_pointsize) __PROTO((double size)); /* notification of set pointsize */
	int flags; /* various flags */
   void (*suspend) __PROTO((void)); /* after one plot of multiplot */
   void (*resume) __PROTO((void));  /* before subsequent plot of multiplot */
   void (*boxfill) __PROTO((int style, unsigned int x1, unsigned int y1, unsigned int width, unsigned int height)); /* clear part of multiplot */
   void (*linewidth) __PROTO((double linewidth));
   void (*pointsize) __PROTO((double pointsize));
  };

One consequence of (1) is that we would like drivers to be backwards
compatible - drivers in the correct form below should work in future
versions of gnuplot without change. C compilers guarantee to fill
unitialised members of a structure to zero, so gnuplot can detect old
drivers, in which fields have not been initalised, and can point
new interface entry pointers to dummy functions.

We can add fields to the terminal structure, but only at the end of the list.
If you design a terminal that cant work without a new interface being defined,
and consequent changes to the main gnuplot source, please contact
bug-gnuplot@dartmouth.edu simply to ensure that you have the most
up to date defn of the terminal structure. Also, please ensure that
the 'set term' command checks for 0 values in added fields when an
old driver is selected, and a pointer to a suitable 'cant do' function
is provided. It is therefore not required (and in fact not possible)
to add padding fields to the end of all drivers.

Similarly, if you add an optional field to an old driver, take care
to ensure that all intervening fields are padded with zeros.

Some of the above fields are required - this should not be a problem,
since they were all required in earlier releases of gnuplot.
The later fields are interfaces to capabilities that not all devices
can do, or for which the generic routines provided should be adequate.
There are several null ('cant do') functions provided by term.c which
a driver can reference in the table. Similarly, for bitmap devices, there
are generic routines for lines and text provided by bitmap.c



Here's a brief description of each variable:

The char *name is a pointer to a string containing the name
of the terminal.  This name is used by the 'set terminal' and 
'show terminal' commands.  
The name must be unique and must not be confused with an abbreviation 
of another name.  For example if the name "postscript" exists, it is not
possible to have another name "postscript2".
Keep the name under 15 characters.

The char *description is a pointer to a string containing a
description of the terminal, which is displayed in response
to the 'set terminal' command.  
Keep the description under 60 characters.

xmax is the maximum number of points in the x direction.  
The range of points used by gnuplot is 0 to xmax-1.

ymax is the maximum number of points in the y direction.  
The range of points used by gnuplot is 0 to ymax-1.

v_char is the height of characters, in the same units as xmax and ymax.
The border for labelling at the top and bottom of the plot is 
calculated using v_char.  
v_char is used as the vertical line spacing for characters.

h_char is the width of characters, in the same units as xmax and ymax.
The border for labelling at the left and right of the plot is 
calculated using h_char, for example.
If the _justify_text function returns FALSE, h_char is used to justify 
text right or centre.  If characters are not fixed width, then the 
_justify_text function must correctly justify the text.

v_tic is the vertical size of tics along the x axis, 
in the same units as ymax.

h_tic is the horizontal size of tics along the y axis, 
in the same units as xmax.

v_tic and h_tic should give tics of the same physical size on the
output. The ratio of these two quantities is used by gnuplot to set the
aspect ratio to 1 so that circles appear circular when 'set size square'
is active.

All the above values need not be static - values can be substituted
into the table during terminal initialisation, based on options for
example.



Here's a brief description of what each term.c function does:

_options()  Called when terminal type is selected.  
This procedure should parse options on the command line.  A list of the 
currently selected options should be stored in term_options[] in a form 
suitable for use with the set term command.  term_options[] is used by 
the save command.  Use options_null() if no options are available. 

_init()  Called once, when the device is first selected.  This procedure
should set up things that only need to be set once, like handshaking and
character sets etc...
There is a global variable 'pointsize' which you might want to use here.
If set pointsize is issued after init has been called, the set_pointsize()
function is called.

_reset()  Called when gnuplot is exited, the output device changed or
the terminal type changed.  This procedure should reset the device, 
possibly flushing a buffer somewhere or generating a form feed.

_scale(xs,ys) Called just before _graphics(). This takes the x and y
scaling factors as information. If the terminal would like to do its
own scaling, it returns TRUE. Otherwise, it can ignore the information
and return FALSE: do_plot will do the scaling for you. null_scale is
provided to do just this, so most drivers can ignore this function
entirely. The Latex driver is currently the only one providing its own
scaling. PLEASE DO NOT USE THIS INTERFACE - IT IS NOT COMPATIBLE WITH
MULTIPLOT.

_graphics()  Called just before a plot is going to be displayed.  This
procedure should set the device into graphics mode.  Devices which can't
be used as terminals (like plotters) will probably be in graphics mode 
always and therefore won't need this.

_text()  Called immediately after a plot is displayed.  This procedure 
should set the device back into text mode if it is also a terminal, so
that commands can be seen as they're typed.  Again, this will probably
do nothing if the device can't be used as a terminal. This call can
be used to trigger conversion and output for bitmap devices.

_move(x,y)  Called at the start of a line.  The cursor should move to the
(x,y) position without drawing.

_vector(x,y)  Called when a line is to be drawn.  This should display a line
from the last (x,y) position given by _move() or _vector() to this new (x,y)
position.

_linetype(lt)  Called to set the line type before text is displayed or
line(s) plotted.  This procedure should select a pen color or line
style if the device has these capabilities.  
lt is an integer from -2 to 0 or greater.  
An lt of -2 is used for the border of the plot.
An lt of -1 is used for the X and Y axes.  
lt 0 and upwards are used for plots 0 and upwards.
If _linetype() is called with lt greater than the available line types, 
it should map it to one of the available line types.
Most drivers provide 9 different linetypes (lt is 0 to 8).

_put_text(x,y,str)  Called to display text at the (x,y) position, 
while in graphics mode.   The text should be vertically (with respect 
to the text) justified about (x,y).  The text is rotated according 
to _text_angle and then horizontally (with respect to the text)
justified according to _justify_text.


The following are optional


_text_angle(ang)  Called to rotate the text angle when placing the y label.
If ang = 0 then text is horizontal.  If ang = 1 then text is vertically
upwards.  Returns TRUE if text can be rotated, FALSE otherwise.
[But you must return TRUE if called with ang=0]

_justify_text(mode)  Called to justify text left, right or centre.
If mode = LEFT then text placed by _put_text is flushed left against (x,y).
If mode = CENTRE then centre of text is at (x,y).  
If mode = RIGHT then text is placed flushed right against (x,y).
Returns TRUE if text can be justified
Returns FALSE otherwise and then _put_text assumes text is flushed left;
justification of text is then performed by calculating the text width
using strlen(text) * h_char.

_point(x,y,point)  Called to place a point at position (x,y).
point is -1 or an integer from 0 upwards.  
At least 6 point types (numbered 0 to 5) are normally provided.  
Point type -1 is a dot.
If point is more than the available point types then it should 
be mapped back to one of the available points.
Two _point() functions called do_point() and line_and_point() are 
provided in term.c and should be suitable for most drivers.  
do_point() draws the points in the current line type.
If your driver uses dotted line types (generally because it is
monochrome), you should use line_and_point() which changes to 
line type 0 before drawing the point.  line type 0 should be solid.

There is a global variable 'pointsize' which is controlled by the
set pointsize command. If possible, use that. pointsize should be
examined at terminal init. If it is subsequently changed, the
set_pointsize() fn will be called.


_arrow(sx,sy,ex,ey,head)  Called to draw an arrrow from (sx,sy) to (ex,ey).
A head is drawn on the arrow if head = TRUE.
An _arrow() function called do_arrow() is provided in term.c which will
draw arrows using the _move() and _vector() functions.  
Drivers should use do_arrow unless it causes problems.

_set_font() is called to set the font of labels, etc [new 3.7 feature]
fonts are selected as strings "name,size"

_pointsize() is used to set the pointsize for subsequent points

_flags stores various flags describing driver capabilities. Currently
 three bits are allocated
  - TERM_CAN_MULTIPLOT - driver can do multiplot
  fully-interactively when output is not redirected. ie text and graphics
  go to different places, or driver can flip using suspend.
  - TERM_CANT_MULTIPLOT - driver cannot multiplot, even if output
  is redirected.
  - TERM_BINARY - output file must be opened in binary mode
  Another bit is earmarked for VMS_PASTHRU, but not yet implemented.

_suspend() - called before gnuplot issues a prompt in multiplot mode
   linux vga driver uses this entry point to flip from graphics to
   text mode. X11 driver will take this opportunity to paint the window
   on the display.

_resume() - called after suspend(), before subsequent plots of a multiplot.

_boxfill() - fills a box in given style (currently unimplemented - always
           background colour at present). used by 'clear' in multiplot for
           support of inset graphs

_linewidth() - sets the linewidth

The following should illustrate the order in which calls to these
routines are made:

 _options()
  _init()
    _scale(xs,ys)
    _graphics()
      _linewidth(lw)
      _linetype(lt)
      _move(x,y)
      _vector(x,y)
      _pointsize(size)
      _point(x,y,point)
      _text_angle(angle)
      _justify(mode)
      _set_font(font)
      _put_text(x,y,text)
      _arrow(sx,sy,ex,ey)
    _text()
    _graphics()
      .
    _suspend()
    _set_pointsize()
    _resume()
      .
    _text()
  _reset()


------------------------------------

BITMAP DEVICES

A file bitmap.c is provided, implementing a generic set of bitmap
routines. It provides all the routines required to generate a
bitmap in memory, drawing lines and writing text. A simple driver
need provide only a text() entry point, which converts and outputs
the stored bitmap in the format required by the device.

Internally, the bitmap is built of one or more planes of 1
bit per pixel. In fact, I think the library would be easier to
use if it offered one or more planes of pixels with 1,2,4 or 8
bits per pixel, since not all bitmap devices are based on planes,
and the planes have to be recombined at the end at present.
In general, a device would use either planes or bits-per-pixel,
though I guess a 24-bit bitmap could use 3 planes of 8 bits
per pixel..?


The pixels are currently organised horizontally packed into bytes.

ie

  ********%%%%%%%%$$$$$$$$!!!!!!!! etc
  ^^^^^^^^@@@@@@@@########++++++++ etc

where like symbols are stored in one byte. Vertical packing can be
arranged by reversing x and y dimensions and setting the global
b_rastermode to TRUE.  (eg epson 8-pin dot-matrix printer)


Functions provided are

(internal fns ? - should probably be static, not external ?)

  b_setpixel(x,y,value)     
  b_setmaskpixel(x,y,value)
  b_putc(x,y,char,angle)
  b_setvalue(size)

setting up stuff

  b_makebitmap(x,y,planes)  - make a bitmap of size x * y
  b_freebitmap()            - free bitmap
  b_charsize(size)


gnuplot driver interface functions  (can go straight into gnuplot structure)

  b_setlinetype(linetype)
  b_move(x,y)
  b_vector(x,y)
  b_put_text(x,y,*str)
  b_text_angle(ang)



I think that the library could be made easier to use if we defined
a structure which described the bitmap (raster mode, planes, bits-per-pixel,
colours, etc) and then added to the gnuplot term struct a pointer to
this structure. Then we could have b_graphics() routine which did all
the initialisation that presently has to be done by the driver graphics()
entry point.  Also, one day I would like to have parsing, including
terminal driver options, table-driven, but I'm getting ahead of myself
here.


At present, bitmap.c is linked into gnuplot unconditionally. Perhaps
it should be put into a library, so that it is linked in only if
any of the user-selected drivers require bitmap support.

There may be scope to do similar things with some of the other
stuff that is shared by several drivers. Rather than requiring,
for example, that LATEX driver is required if EMTEX is to be used,
the shared routines could be extracted to a library and linked
if any of the drivers which use them are used.  Just a thought...

------------------------------------

FILE LAYOUT
-----------

I think a file layout like the following will leave most flexibility
to the gnuplot maintainers. I use REGIS for example.


  #include "driver.h"


  #ifdef TERM_REGISTER
  register_term(regis) /* no ; */
  #endif


  #ifdef TERM_PROTO
  TERM_PUBLIC void REGISinit __PROTO((void));
  TERM_PUBLIC void REGISgraphics __PROTO((void));
  /* etc */
  #define GOT_REGIS_PROTO
  #endif


  #ifndef TERM_PROTO_ONLY
  #ifdef TERM_BODY

  TERM_PUBLIC void REGISinit()
  {
    /* etc */
  }

  /* etc */

  #endif


  #ifdef TERM_TABLE

  TERM_TABLE_START(regis_driver)
    /* no { */
    "regis", "REGIS graphics language",
    REGISXMAX, /* etc */
    /* no } */
  TERM_TABLE_END(regis_driver)

  #undef LAST_TERM
  #define LAST_TERM regis_driver

  #endif /* TERM_TABLE */
  #endif /* TERM_PROTO_ONLY */




  #ifdef TERM_HELP
  START_HELP(regis)
  "1 regis",
  "?set terminal regis",
  "?regis",
  " The `regis` terminal device generates output in the REGIS graphics language.",
  " It has the option of using 4 (the default) or 16 colors.",
  "",
  " Syntax:",
  "         set term regis {4 | 16}"
  END_HELP(regis)
  #endif


--------------

The first three lines in the TERM_HELP section must contain the same
name as that specified by register_term, since this is the name that
will be entered into the list of available terminals.  If more than
one name is registered, the additional names should have their own
two "?" lines, but not the "1" line.

Each record is enclosed in double-quotes and (except for the last
record) followed by a comma.  The text is copied as a single string
into gnuplot.doc, so the syntax must obey the rules of that entity.
If the text includes double-quotes or backslashes, these must be
escaped by preceding each occurence with a backslash.

--------------

Rationale:

We may want to compile all drivers into term.c or one driver at a time
this layout should support both
TERM_PUBLIC will be static  if all modules are in term.c, or blank
otherwise.
Please make private support functions static if possible.


We may include term.h, and therefore all these files, one or more times.
If just once (all modules compiled into term.c) putting the four
parts in this order should make it work.

we may compile the table entries into either an array or a linked list
This organisation should support both

For separate compilation, we may write a program which
defines TERM_REGISTER and #include term.h to find out which drivers are
selected in term.h and thereby generate a makefile.



For a driver which depends on another (eg enhpost and pslatex on post)
the driver can do something like

  #ifndef GOT_POST_PROTO
  #define TERM_PROTO_ONLY
  #include "post.trm"
  #undef TERM_PROTO_ONLY
  #endif

this is probably needed only in the TERM_TABLE section only, but may
also be used in the body. The TERM_PROTO_ONLY means that we pick up
only the protos from post.trm, even if current driver is being compiled
with TERM_BODY or TERM_TABLE

If we do it the linked-list way, the arg to TERM_TABLE_START will be
the name of the variable, so any valid, unique name is fine.
The TERM_TABLE_START macro will do all the work of linking the entries
together, probably using LAST_TERM

The inclusion of the TERM_HELP section (and removal of terminal documentation
from the master gnuplot.doc file) means that the online help will include
discussions of only those terminals available to the user.  For generation
of the printed manual, all can be included.


Please make as many things as possible static, but do still try to use unique
names since all drivers may all be compiled into term.o

The bit in the PROTO section is basically what you would put into a .h
file if we had them - everything that is needed by the TABLE_ENTRY
should be defined in this part. In particular, dont forget all the maxes
and character sizes and things for the table entry.

Dont forget to put TERM_PUBLIC in the defns of the fns as well as the
prototypes. It will probably always expand to 'static' except for pcs.

=head1 Using Term::Gnuplot from C libraries

The interface of this module to B<gnuplot> version 3.7 is going via a
translation layer in F<Gnuplot.h>.  This layer isolates low-level drawing
routines from B<gnuplot> program.  (In doing this unsupported job it does
some nasty thing, in particular F<Gnuplot.h> cannot be included in more than
one C compilation unit.)

In fact F<Gnuplot.h> can be used by any C program or library which
wants to use device-independent plotting routines of B<gnuplot>.

C library should use the same syntax of calls as C<Term::Gnuplot>, with
the only difference that to call low-level _init() method one calls
gptable_init(), to set terminal one calls C<termset(name)>,
and to get a property of terminal (say C<xmax>) one uses C<termprop(xmax)>.

To set options one can setup the array C<token> and data C<c_token>,
C<num_tokens>, C<input_line>, then call options().  Alternately, one
can define C<SET_OPTIONS_FROM_STRING>, then a call
C<set_options_from(string)> is avalable.  This call will set up all
these variables and will call options().  However, the logic of the
parsing of the string is very primitive.

B<NOTES.>

=over

=item *

To initialize the facilities of F<Gnuplot.h> from C one needs to call
setup_gpshim().  This call can be made as many times as wanted, only the
first call will do anything.

=item *

C<NULL> argument to term_set_output() means "reset back to stdout".

=item *

F<Gnuplot.h> expects that the macro/function C<croak(...)> is defined.
This function should have the same syntax as printf(), and should not return.
It is used as an error-reporting function.

=item *

One should define functions C<int StartOutput()>, C<int EndOutput()> and
C<int OutLine(char *s)> which will be used by B<gnuplot> for error messages
and for terminal listing.  Alternatively, one can define
C<GNUPLOT_OUTLINE_STDOUT>, and B<gnuplot> will put these messages to C<stdout>.

=back

=head2 Runtime link with B<gnuplot> DLL

There are two different ways to use the plotting calls via F<Gnuplot.h>.
One can either establish the link to B<gnuplot> library at I<compile/link
time>, or to postpone this link to I<run time>.  By default F<Gnuplot.h>
provides the first way, to switch to the second way define C<DYNAMIC_GNUPLOT>
before including F<Gnuplot.h>.

To establish a link at I<run time>, one needs to load a dynamic library
which is compiled from F<Gnuplot.h> - but without C<DYNAMIC_GNUPLOT> defined.
Feed the result of the call to get_term_ftable() as an argument to
set_term_ftable(), as in (with error-condition checking disabled):

  typedef struct t_ftable *(*g_ftable)(void);
  g_ftable g_ftable_addr;
  void *handle = dlopen("gnuterm.dll", mode);

  g_ftable_addr = (g_ftable) dlsym(handle, "get_term_ftable");
  set_term_ftable((*g_ftable_addr)());

This means that any C library/program which uses the API provided by
F<Gnuplot.h> does not I<need> to be even linked with B<gnuplot>,
neither it I<requires> include files of B<gnuplot>.  If plotting
is done in some situations only, one will not need the overhead of gnuplot
plotting library (F<gnuterm.dll> in the above example) unless plotting
is requested.

=head2 Runtime link of a Perl module with C<Term::Gnuplot>

To facilitate I<run time> link the C<Term::Gnuplot> Perl module provides
a Perl subroutine get_term_ftable() which is a variant of C get_term_ftable()
which returns an integer instead of an address.  One can feed this integer
to a C function C<v_set_term_ftable(void*)>, which will establish a runtime
link.  Thus the external XS library which wants to use Term::Gnuplot
at runtime can put this in F<.xs> file:

  #define int_set_term_ftable(a) (v_set_term_ftable((void*)a))
  extern  void v_set_term_ftable(void *a);
  ...

  void
  int_set_term_ftable(a)
    IV a

include F<Gnuplot.h> (with C<DYNAMIC_GNUPLOT> defined) into any C file,
and define this Perl subroutine

  sub link_gnuplot {
    eval 'use Term::Gnuplot 0.56; 1' or die;
    int_set_term_ftable(Term::Gnuplot::get_term_ftable());
  }

in F<.pm> file.  

Now if it needs to do plotting, it calls link_gnuplot(), then does the
plotting - without a need to interact with B<gnuplot> at compile/link
time, and having the additional burden of low-level plotting code
loaded in the executable/DLL.

After a call link_gnuplot(), all the F<Gnuplot.h> API calls made from
the above C file will be directed through the vtables of the Perl
module C<Term::Gnuplot>.

=head2 Using different Plotting libraries

In fact F<Gnuplot.h> knows almost nothing about B<gnuplot>, with a notable
exceptions that there is an API call C<term = change_term(name)>, which
returns a table of methods.  Thus I<in principle> F<Gnuplot.h> can establish
a runtime-link with any plotting library which supports (or can be
coerced to support) this call.  (The table is assumed to
be compatible with gnuplot layout of C<struct termentry>.)

F<Gnuplot.h> uses also a handful of other gnuplot APIs (such as
changing output file and several initialization shortcuts), but they
should be easy to be ignored if an interface to another plotting library
is needed.

=head1 Examples

=head2 High-level plotting

  sub ploth {
    my ($xmin, $xmax, $sub) = (shift, shift, shift);
    my $wpoints = scaled_xmax() - 1;
    my $hpoints = scaled_ymax() - 1;
    my $delta = ($xmax - $xmin)/$wpoints;
    my (@ys, $y);
    my ($ymin, $ymax) = (1e300, -1e300);
    my $x = $xmin;
    for my $i (0 .. $wpoints) {
      $y = $sub->($x);
      push @ys, $y;
      $ymin = $y if $ymin > $y;
      $ymax = $y if $ymax < $y;
      $x += $delta;
    }
    my $deltay = ($ymax - $ymin)/$hpoints;
    $_ = ($_ - $ymin)/$deltay + $yfix for @ys;

    term_start_plot();

    linetype -2;
    move 0, $yfix;
    vector 0, $hpoints + $yfix;
    vector $wpoints, $hpoints + $yfix;
    vector $wpoints, $yfix;
    vector 0, $yfix;
  
    linetype -1;
    if ($xmin < 0 && $xmax > 0) {
      my $xzero = -$xmin/$delta;
      move $xzero, $yfix;
      vector $xzero, $hpoints + $yfix;
    }
    if ($ymin < 0 && $ymax > 0) {
      my $yzero = -$ymin/$deltay + $yfix;
      move 0, $yzero;
      vector $wpoints, $yzero;
    }

    linetype 0;
    move 0, int($ys[0] + 0.5);
    linetype 0;
    for my $i (1 .. $wpoints) {
      $x += $delta;
      vector $i, int($ys[$i] + 0.5);
    }
    term_end_plot();
  }

This function should be used as in

  ploth(-1,3, sub { sin( (shift)**6 ) })

$yfix should be set to 1 for C<gif> terminal to compensate for off-by-one bug.

=head2 Multifile plotting

  use strict;
  use Term::Gnuplot qw(:ALL);

  my ($yfix, $ext) = (1, 'gif');
  plot_outfile_set "manypl1.$ext";		# Need to set before plotterm()

  change_term "gif";
  set_options "size", 300, ',', 200;
  term_init();

  for my $k (1..6) {
    plot_outfile_set "manypl$k.$ext" unless $k == 1;
    ploth( -1, 3, sub { sin((shift)**$k) } );
  }

Here we use the function ploth() from L<High-level plotting>, it should
be in the same scope so that $yfix is visible there.  Note that C<gif>
terminal requires $yfix to be 1 to circumvent a bug, and requires a literal
C<','> in the set_options call.

If a terminal does not support direct setting of the size of the output device,
one may set global rescaling factors by calling plotsizes_scale():

  plotsizes_scale(300/xmax(), 200/ymax());

=head1 BUGS and LIMITATIONS

The following C macros are set to reasonable values, no peeking is
performed to get correct values, this may break Gnuplot on some systems:

  NO_ATEXIT HAVE_ON_EXIT PIPES HAVE_LIBC_H

No testing for

  X11

macros is performed either, however, this may only increase size of
the executable (since C<X11> module is not making any direct C<X> calls,
but calls an external program to serve the requests).

Apparently C<gif> terminal has off-by-one error: yrange is C<1..ymax()>.
All the bugs of B<gnuplot> low-level plotting show in this module as well.

=head1 SEE ALSO

L<Term::GnuplotTerminals>.

=cut

require Exporter;
require DynaLoader;

$VERSION = '0.5703';

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
@EXPORT_OK = qw(
		change_term test_term init_terminal list_terms
		term_init term_start_plot term_end_plot
		term_start_multiplot term_end_multiplot plotsizes_scale
		LEFT CENTRE RIGHT 
		name description xmax ymax v_char h_char v_tic h_tic
		scaled_xmax scaled_ymax
		init scale graphics linetype move vector point text_angle
		justify_text put_text arrow text set_options
		set_font pointsize suspend resume is_binary cannot_multiplot 
		can_multiplot fillbox linewidth plot_outfile_set setcanvas
		%description);
%EXPORT_TAGS = ('JUSTIFY' => [qw(LEFT CENTRE RIGHT)],
		'SETUP' => [qw(change_term test_term init_terminal
			       term_init term_start_plot term_end_plot
			       term_start_multiplot term_end_multiplot
			       list_terms plot_outfile_set plotsizes_scale
			       setcanvas)],
		'FIELDS'  => [qw(name description xmax ymax
				 is_binary cannot_multiplot can_multiplot
				 v_char h_char v_tic h_tic
				 scaled_xmax scaled_ymax)],
		'METHODS' => [qw(init scale graphics linetype move vector 
				 point text_angle justify_text put_text arrow
				 text set_options
				 set_font pointsize suspend resume 
				 fillbox linewidth)],
		);
$EXPORT_TAGS{'ALL'} = [@{$EXPORT_TAGS{'JUSTIFY'}},
		       @{$EXPORT_TAGS{'SETUP'}},
		       @{$EXPORT_TAGS{'FIELDS'}},
		       @{$EXPORT_TAGS{'METHODS'}}, '%description'];

*pointsize = \&setpointsize;	# To simplify C macros

bootstrap Term::Gnuplot;

%description = _term_descrs();

# Preloaded methods go here.

sub LEFT {0}
sub CENTRE {1}
sub RIGHT {2}

sub name {(getdata())[0]}
sub description {(getdata())[1]}
sub xmax {(getdata())[2]}
sub ymax {(getdata())[3]}
sub v_char {(getdata())[4]}
sub h_char {(getdata())[5]}
sub v_tic {(getdata())[6]}
sub h_tic {(getdata())[7]}

sub canvas_sizes {
  my $canvas = shift;
  return $canvas->gnuplot_sizes if $canvas->can('gnuplot_sizes');
  my ($w,$h) = map $canvas->winfo($_), "width", "height";
  my $b = $canvas->cget('border');
  my $ci = $canvas->createText(1,1);
  my $f = $canvas->itemcget($ci,-font);
  require Tk::Font;
  my %m = $f->metrics;
  my $fh = $m{-linespace};
  my $s = '0123456789';			#'The quick brown fox';
  my $fw = int(0.5 + $f->measure($s)/length($s));
  my $vt = int (($fh + 3)/4);
  my $ht = int (($fw*2 + 2)/3);
  $canvas->delete($ci);
  ($w,$h,$b,$b,$fw,$fh,$ht,$vt);
}

# Autoload methods go after __END__, and are processed by the autosplit program.

1;
__END__
