#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "Gnuplot.h"

#define change_term_address() ((IV)&change_term)
#define term_tbl_address() ((IV)term_tbl)

/* #define set_gnuplot_fh(file) (outfile = PerlIO_exportFILE(file,0)) */

#define int_change_term(s,l) (change_term(s,l) != 0)
typedef PerlIO *OutputStream;

#if 0
/* This sets the tokens for the options */
void
set_tokens(char *start)
{
    char *s = start;
    char *tstart;
    
    num_tokens = 0;
    while (num_tokens <= MAX_TOKENS) {
	while (*s == ' ' || *s == '\t' || *s == '\n')
	    s++;
	if (!*s)
	    return;
	token[num_tokens].is_token = (*s > '9' || *s < '0');
	tstart = s;
	while (*s && !(*s == ' ' || *s == '\t' || *s == '\n'))
	    s++;
	if (token[num_tokens].is_token) {
	    token[num_tokens].start_index = tstart - start;
	    token[num_tokens].length = s - tstart;
	} else {
	    token[num_tokens].value.type = INTGR;
	    token[num_tokens].value.v.int_val = atoi(tstart);
	}
	num_tokens++;
    }
    {
	char buf[80];
	sprintf(buf, "panic: more than %d tokens for options", MAX_TOKENS);
	croak(buf);    
    }
}
#endif

/* This sets the tokens for the options */
void
set_tokens(SV **svp, int n)
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
	STRLEN l;

	if (SvIOKp(elt)) {
	    token[tk].is_token = 0;
	    token[tk].l_val.type = INTGR;
	    token[tk].l_val.v.int_val = SvIV(elt);
	} else if (SvNOKp(elt)) {
	    token[tk].is_token = 0;
	    token[tk].l_val.type = CMPLX;
	    token[tk].l_val.v.cmplx_val.real = SvNV(elt);
	    token[tk].l_val.v.cmplx_val.imag = 0;
	} else {
	    token[tk].is_token = 1;
	    token[tk].start_index = SvPV(elt,l) - input_line;
	    token[tk].length = l;
	}
	tk++;
    }
}

void
set_options(SV **svp, int n)
{
    set_tokens(svp,n);
    options();
    c_token = num_tokens = 0;
}

int
StartOutput() {}

int
EndOutput() {}

int
OutLine(char *s)
{
   return fprintf(stdout, "%s", s);
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

int
int_change_term(name,length=strlen(name))
char *	name
int	length

int
init_terminal()

# set_term is unsupported without junk

MODULE = Term::Gnuplot	PACKAGE = Term::Gnuplot

void
init()

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

BOOT:
    plot_outfile_set("-");
