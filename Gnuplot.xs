#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "Gnuplot.h"

#define change_term_address() ((IV)&change_term)
#define term_tbl_address() ((IV)term_tbl)
#define set_gnuplot_fh(file) (outfile = PerlIO_exportFILE(file,0))
typedef PerlIO *OutputStream;

MODULE = Term::Gnuplot		PACKAGE = Term::Gnuplot

void
set_gnuplot_fh(file)
    OutputStream file

IV
change_term_address()

IV
term_tbl_address()

int
test_term()

void
list_terms()

int
change_term(name,length=strlen(name))
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
getdata()
   PPCODE:
    {
      if (term<0) {
	croak("No terminal specified");
      }
      EXTEND(sp, 8);
      PUSHs(sv_2mortal(newSVpv(term_tbl[term].name,0)));
      PUSHs(sv_2mortal(newSVpv(term_tbl[term].description,0)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].xmax)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].ymax)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].v_char)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].h_char)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].v_tic)));
      PUSHs(sv_2mortal(newSViv(term_tbl[term].h_tic)));
    }

BOOT:
     outfile = stdout;
