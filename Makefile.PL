use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

$try_libs = ' -L/usr/openwin/lib -lX11 -lm  -lsocket -lvga -llinuxvga -lgd -lpng -lz -ljmgraph -lvesa';

@ARGV = grep !(/^TRY_LIBS=(.*)$/ and $try_libs = $1), @ARGV;

$common::try_libs = $try_libs;

$gnuplotdir = './gnuterm';

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot',
    'VERSION_FROM'	=> 'Gnuplot.pm',
    'MYEXTLIB'	=> $gnuplotdir . '/libgnuterm$(LIB_EXT)',
    'LIBS'	=> [ $try_libs ],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket -L. -lgnuterm'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket '
    #		    . "-L$gnuplotdir -lgnuplot"],   # e.g., '-lm' 
    'INC'	=> "-I$gnuplotdir",     # e.g., '-Idir' 
    #'DEFINE'	=> '-DNO_JUNK',     # e.g., '-DHAVE_SOMETHING' 
    'dist' => {COMPRESS=>'gzip -9f', SUFFIX=>'gz'},
	 # Let us hope that boot_Term__Gnuplot should not be mangled
    'FUNCLIST'	=> ['get_term_ftable', 'boot_Term__Gnuplot'],
);

sub MY::postamble {
  # First, take a possibility to check for the presence of gnuplot_x11
  my $self = shift;
  if ($self->maybe_command_in_dirs(['gnuplot_x11'],[$self->path])) {
    # Touch a file as a way of inter-process communication...
    open TMP, '>test_x11' or warn;
    close TMP or warn;
  }
"GNUPLOT_DIR = $gnuplotdir" .   '

$(MYEXTLIB)  : FORCE
	cd $(GNUPLOT_DIR); make libgnuterm$(LIB_EXT) $(PASTHRU)

gnuterm_dynamic  : FORCE
	cd $(GNUPLOT_DIR); make dynamic $(PASTHRU)

';
}
