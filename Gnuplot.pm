package Term::Gnuplot;

=head1 NAME

Term::Gnuplot - lowlevel graphics using gnuplot drawing routines.

=head1 SYNOPSIS

  use Term::Gnuplot ':ALL';
  list_terms();
  change_term('dumb') or die "Cannot set terminal.\n";
  init();
  graphics();
  $xmax = xmax();
  $ymax = ymax();
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
  text();
  Term::Gnuplot::reset();

=head1 EXPORTS

None by default.

=head2 Exportable

  change_term test_term init_terminal list_terms  set_gnuplot_fh
  LEFT CENTRE RIGHT 
  name description xmax ymax v_char h_char v_tic h_tic
  init scale graphics linetype move vector point text_angle
  justify_text put_text arrow text

=head2 Export tags

C<:ALL> for all stuff, C<:SETUP> for the first row above, C<:JUSTIFY>
for the second, C<:FIELDS> for the third, C<:METHODS> for the rest.

=head1 DESCRIPTION

Below I include the contents of the file F<term/README> from gnuplot
distribution. It explains the meaning of the above methods. All is
supported under Perl but the C<options> method. The discription below
includes underscores, that are deleted in the perl interface.

The only functions that are not included in the description below are
C<change_term($newname)>, test_term() and init_terminal(), that should
be self-explanatory. Currently it is impossible to find names of
supported terminals, this would require patch to gnuplot.  However, it
is possible to print them out using list_terms().

One can set the output filehandle by calling set_gnuplot_fh().

=head1 gnuplot F<term/README>

DOCUMENTATION FOR GNUPLOT TERMINAL DRIVER WRITERS

By Russell Lang 1/90

Information on each terminal device driver is contained in term.c and
the term/*.trm files.  Each driver is contained in a .trm file and is 
#include'd into term.c.  Each driver has a set of initialisers in 
term.c for term_tbl[], an array of struct termentry.

Here is the definition of the struct termentry from plot.h:

  struct termentry {
	char *name;
	char *description;
	unsigned int xmax,ymax,v_char,h_char,v_tic,h_tic;
	FUNC_PTR options,init,reset,text,scale,graphics,move,vector,linetype,
		put_text,text_angle,justify_text,point,arrow;
  };

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
calculated using h_char.  
If the _justify_text function returns FALSE, h_char is used to justify 
text right or centre.  If characters are not fixed width, then the 
_justify_text function must correctly justify the text.

v_tic is the vertical size of tics along the x axis, 
in the same units as ymax.

h_tic is the horizontal size of tics along the y axis, 
in the same units as xmax.


Here's a brief description of what each term.c function does:

_options()  Called when terminal type is selected.  
This procedure should parse options on the command line.  A list of the 
currently selected options should be stored in term_options[] in a form 
suitable for use with the set term command.  term_options[] is used by 
the save command.  Use options_null() if no options are available. 

_init()  Called once, when the device is first selected.  This procedure
should set up things that only need to be set once, like handshaking and
character sets etc...

_reset()  Called when gnuplot is exited, the output device changed or
the terminal type changed.  This procedure should reset the device, 
possibly flushing a buffer somewhere or generating a form feed.

_scale(xs,ys) Called just before _graphics(). This takes the x and y
scaling factors as information. If the terminal would like to do its
own scaling, it returns TRUE. Otherwise, it can ignore the information
and return FALSE: do_plot will do the scaling for you. null_scale is
provided to do just this, so most drivers can ignore this function
entirely. The Latex driver is currently the only one providing its own
scaling.

_graphics()  Called just before a plot is going to be displayed.  This
procedure should set the device into graphics mode.  Devices which can't
be used as terminals (like plotters) will probably be in graphics mode 
always and therefore won't need this.

_text()  Called immediately after a plot is displayed.  This procedure 
should set the device back into text mode if it is also a terminal, so
that commands can be seen as they're typed.  Again, this will probably
do nothing if the device can't be used as a terminal.

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

_text_angle(ang)  Called to rotate the text angle when placing the y label.
If ang = 0 then text is horizontal.  If ang = 1 then text is vertically
upwards.  Returns TRUE if text can be rotated, FALSE otherwise.

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
6 point types (numbered 0 to 5) are normally provided.  
Point type -1 is a dot.
If point is more than the available point types then it should 
be mapped back to one of the available points.
Two _point() functions called do_point() and line_and_point() are 
provided in term.c and should be suitable for most drivers.  
do_point() draws the points in the current line type.
If your driver uses dotted line types (generally because it is
monochrome), you should use line_and_point() which changes to 
line type 0 before drawing the point.  line type 0 should be solid.

_arrow(sx,sy,ex,ey,head)  Called to draw an arrrow from (sx,sy) to (ex,ey).
A head is drawn on the arrow if head = TRUE.
An _arrow() function called do_arrow() is provided in term.c which will
draw arrows using the _move() and _vector() functions.  
Drivers should use do_arrow unless it causes problems.

The following should illustrate the order in which calls to these
routines are made:

  _init()
    _scale(xs,ys)
    _graphics()
      _linetype(lt)
      _move(x,y)
      _vector(x,y)
	  _point(x,y,point)
      _text_angle(angle)
      _justify(mode)
      _put_text(x,y,text)
      _arrow(sx,sy,ex,ey)
    _text()
    _graphics()
      .
      .
    _text()
  _reset()

=head1 Using Term::Gnuplot from C libraries

The interface of this module to B<gnuplot> version 3.5 is going via a
translation layer in F<Gnuplot.h>.  It isolates low-level drawing
routines from B<gnuplot> program.  (In doing this unsupported job it does
some nasty thing, in particular it cannot be included in more than one
C compilation unit.)

This header file knows nothing about B<gnuplot> except that there is an API
call C<term = change_term(name)>, and there is an (indexed by C<term>)
array C<term_tbl> of tables of methods.

This means that any C library which uses the API provided by
F<Gnuplot.h> does not I<need> to be even linked with B<gnuplot>,
neither it I<requires> include files of B<gnuplot>.  It can establish
a runtime-link with any plotting library which supports (or can be
coerced to support) the above interface.

To enable this I<dynamic> link to plotting libraries make sure that
preprocessor macro C<DYNAMIC_GNUPLOT> is defined when you include
F<Gnuplot.h>.  At runtime call C<set_term_funcp(&change_term,
term_tbl)> before doing any drawing, and the link will be established.

To facilitate this the C<Term::Gnuplot> Perl module provides two Perl
routines change_term_address() and term_tbl_address() which return
addresses of B<gnuplot>s routine/table as integers.  Thus the external library which wants to use Term::Gnuplot at runtime can put this in F<.xs> file:

  typedef int (*FUNC_PTR)();
  #define set_gnuterm(a,b) \
     set_term_funcp((FUNC_PTR)(a),(struct termentry *)(b))
  ...

  void
  set_gnuterm(a,b)
    IV a
    IV b

and define this

  sub link_gnuplot {
    eval 'use Term::Gnuplot 0.4; 1' or die;
    set_gnuterm(Term::Gnuplot::change_term_address(), 
	        Term::Gnuplot::term_tbl_address());
  }

in F<.pm> file.  

Now if it needs to do plotting, it calls link_gnuplot(), then does the
plotting - without a need to interact with B<gnuplot> at compile/link
time, and having the additional burden of low-level plotting code
loaded in the executable.

=head1 BUGS and LIMITATIONS

options() call is not implemented, since it will pull all the
B<gnuplot> code with it.  Some hand-made substitute parser of options
setting may be needed...

The following C macros are set to reasonable values, no peeking is
performed to get correct values, this may break Gnuplot on some systems:

  NO_ATEXIT HAVE_ON_EXIT PIPES HAVE_LIBC_H

No testing for 

  HAVE_LIBGD HAVE_LIBPNG LINUXVGA X11

macros is performed either, however, this may only diminish
functionality or (in the case of C<X11>) increase size (since gnuplot
is not making any direct C<X> calls, but may call an external
program to serve the requests).

=cut

require Exporter;
require DynaLoader;

$VERSION = 0.52;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
@EXPORT_OK = qw(
		change_term test_term init_terminal list_terms set_gnuplot_fh
		LEFT CENTRE RIGHT 
		name description xmax ymax v_char h_char v_tic h_tic
		init scale graphics linetype move vector point text_angle
		justify_text put_text arrow text
);
%EXPORT_TAGS = ('JUSTIFY' => [qw(LEFT CENTRE RIGHT)],
		'SETUP' => [qw(change_term test_term init_terminal list_terms set_gnuplot_fh)],
		'FIELDS'  => [qw(name description xmax ymax
				 v_char h_char v_tic h_tic)],
		'METHODS' => [qw(init scale graphics linetype move vector 
				 point text_angle justify_text put_text arrow
				 text)],
		);
$EXPORT_TAGS{'ALL'} = [@{$EXPORT_TAGS{'JUSTIFY'}},
		       @{$EXPORT_TAGS{'SETUP'}},
		       @{$EXPORT_TAGS{'FIELDS'}},
		       @{$EXPORT_TAGS{'METHODS'}}];
		      
bootstrap Term::Gnuplot;

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

# Autoload methods go after __END__, and are processed by the autosplit program.

1;
__END__
