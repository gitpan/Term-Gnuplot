use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

$gnuplotdir = './gnuterm';

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot',
    'VERSION_FROM'	=> 'Gnuplot.pm',
    'MYEXTLIB' => $gnuplotdir . '/libgnuterm$(LIB_EXT)',
    'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket -L. -lgnuterm'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket '
    #		    . "-L$gnuplotdir -lgnuplot"],   # e.g., '-lm' 
    'INC'	=> "-I$gnuplotdir",     # e.g., '-Idir' 
    #'DEFINE'	=> '-DNO_JUNK',     # e.g., '-DHAVE_SOMETHING' 
    'dist' => {COMPRESS=>'gzip -9f', SUFFIX=>'gz'},
);

sub MY::postamble {
"GNUPLOT_DIR = $gnuplotdir" .   '

$(MYEXTLIB)  : FORCE
	cd $(GNUPLOT_DIR); make libgnuterm$(LIB_EXT) $(PASTHRU)

';
}
