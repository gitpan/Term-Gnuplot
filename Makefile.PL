use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

$gnuplotdir = '$(HOME)/get/gnuplot.3.5';

($checkdir = $gnuplotdir) =~ s/\$\((\w+)\)/$ENV{$1}/g;
die <<"EOD;" unless -d $checkdir;
Cannot find Gnuplot directory '$checkdir'.
Please edit the location at start of $0.
EOD;

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot',
    'VERSION'	=> '0.2',
    #'OBJECT' => '$(BASEEXT).o ' . "$gnuplotdir/term.o",
    'MYEXTLIB' => 'libgnuterm.a',
    'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket -L. -lgnuterm'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket '
    #		    . "-L$gnuplotdir -lgnuplot"],   # e.g., '-lm' 
    'DEFINE'	=> "-I$gnuplotdir",     # e.g., '-DHAVE_SOMETHING' 
    #'DEFINE'	=> '-I$gnuplotdir -DUSE_JUNK',     # e.g., '-DHAVE_SOMETHING' 
    'INC'	=> '',     # e.g., '-I/usr/include/other' 
);
#ar r libgnuterm.a ../gnuplot/term.o ../gnuplot/bitmap.o ../gnuplot/util.o ../gnuplot/version.o
sub MY::postamble {
"GNUPLOT_DIR = $gnuplotdir" .   '

$(MYEXTLIB)  : FORCE
	ar r $(MYEXTLIB) $(GNUPLOT_DIR)/term.o $(GNUPLOT_DIR)/bitmap.o $(GNUPLOT_DIR)/util.o $(GNUPLOT_DIR)/version.o

';
}
