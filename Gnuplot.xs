#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define GNUPLOT_OUTLINE_STDOUT
#define DONT_POLLUTE_INIT
#include "Gnuplot.h"

#define change_term_address() ((IV)&change_term)
/* #define term_tbl_address() ((IV)term_tbl) */  /* Not used any more */
#define term_tbl_address() 0

/* #define set_gnuplot_fh(file) (outfile = PerlIO_exportFILE(file,0)) */

#define int_change_term(s,l) (change_term(s,l) != 0)
typedef PerlIO *OutputStream;

/* This sets the tokens for the options */
static void
set_tokens(SV **svp, int n, SV* acc)
{
    int tk = 0;

    c_token = 0;
    num_tokens = n;
    if (num_tokens > MAX_TOKENS) {
	char buf[80];
	sprintf(buf, "panic: more than %d tokens for options: %d",
		MAX_TOKENS, num_tokens);
	croak(buf);
    }
    while (num_tokens > tk) {
	SV *elt = *svp++;
	char buf[80];

	sv_catpvn(acc, " ", 1);
        token[tk].start_index = SvCUR(acc);
	if (SvIOKp(elt)) {
	    token[tk].is_token = 0;
	    token[tk].l_val.type = INTGR;
	    token[tk].l_val.v.int_val = SvIV(elt);
	    sprintf(buf, "%d", SvIV(elt));
	    sv_catpv(acc, buf);
	    token[tk].length = strlen(buf);
	} else if (SvNOKp(elt)) {
	    token[tk].is_token = 0;
	    token[tk].l_val.type = CMPLX;
	    token[tk].l_val.v.cmplx_val.real = SvNV(elt);
	    token[tk].l_val.v.cmplx_val.imag = 0;
	    sprintf(buf, "%g", SvNV(elt));
	    sv_catpv(acc, buf);
	    token[tk].length = strlen(buf);
	} else {
	    token[tk].is_token = 1;
	    token[tk].length = SvCUR(elt);
	    sv_catsv(acc, elt);
	}
	tk++;
    }
}

void
set_options(SV **svp, int n)
{
    SV *sv = newSVpvn("", 0);	/* For error reporting in options() only */

    sv_2mortal(sv);
    set_tokens(svp,n,sv);
    input_line = SvPVX(sv);
    options();
    input_line = Nullch;
    c_token = num_tokens = 0;
}

long
plot_outfile_set(char *s) { 
    int normal = (strcmp(s,"-") == 0);

    /* Delegate all the hard work to term_set_output() */

    if (normal) 
	term_set_output(NULL);
    else {				/* term_set_output() needs
					   a malloced string */
	char *s1 = (char*) malloc(strlen(s) + 1);

	strcpy(s1,s);
	term_set_output(s1);
    }
    return 1; 
}

MODULE = Term::Gnuplot		PACKAGE = Term::Gnuplot		PREFIX = int_

long
plot_outfile_set(s)
    char *s

IV
change_term_address()

IV
term_tbl_address()

int
test_term()

void
list_terms()

void
term_start_plot()

void
term_end_plot()

void
term_start_multiplot()

void
term_end_multiplot()

void
term_init()

int
int_change_term(name,length=strlen(name))
char *	name
int	length

IV
int_get_term_ftable()

void
int_set_term_ftable(a)
	IV a

int
init_terminal()

# set_term is unsupported without junk

MODULE = Term::Gnuplot	PACKAGE = Term::Gnuplot  PREFIX=gptable_

void
gptable_init()

MODULE = Term::Gnuplot	PACKAGE = Term::Gnuplot

void
reset()

void
text()

void
graphics()

void
set_options(...)
    CODE:
    {
	set_options(&(ST(0)),items);
    }

void
linetype(lt)
     int	lt

int
justify_text(mode)
     int	mode

int
text_angle(ang)
     int	ang

int
scale(xs,ys)
     double	xs
     double	ys

void
move(x,y)
     unsigned int	x
     unsigned int	y

void
vector(x,y)
     unsigned int	x
     unsigned int	y

void
put_text(x,y,str)
     int	x
     int	y
     char *	str

void
point(x,y,point)
     unsigned int	x
     unsigned int	y
     int	point

void
arrow(sx,sy,ex,ey,head)
     int	sx
     int	sy
     int	ex
     int	ey
     int	head

void
resume()

void
suspend()

void
linewidth(w)
    double w

void
setpointsize(w)
    double w

int
set_font(s)
    char *s

void
fillbox(sx,sy,ex,ey,head)
     int	sx
     unsigned int	sy
     unsigned int	ex
     unsigned int	ey
     unsigned int	head

void
getdata()
   PPCODE:
    {
      if (!term) {
	croak("No terminal specified");
      }
      EXTEND(sp, 8);
      PUSHs(sv_2mortal(newSVpv(term->name,0)));
      PUSHs(sv_2mortal(newSVpv(term->description,0)));
      PUSHs(sv_2mortal(newSViv(term->xmax)));
      PUSHs(sv_2mortal(newSViv(term->ymax)));
      PUSHs(sv_2mortal(newSViv(term->v_char)));
      PUSHs(sv_2mortal(newSViv(term->h_char)));
      PUSHs(sv_2mortal(newSViv(term->v_tic)));
      PUSHs(sv_2mortal(newSViv(term->h_tic)));
    }

bool
cannot_multiplot()

bool
can_multiplot()

bool
is_binary()

void
plotsizes_scale(x,y)
    double x
    double y

double
scaled_xmax()

double
scaled_ymax()

BOOT:
    setup_gpshim();
    plot_outfile_set("-");
