use strict;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

if (not defined $common::try_libs) {
  $common::try_libs = ' -L/usr/openwin/lib -lX11';
}

BEGIN {
  push @INC, '../utils';
}
use FindLib;

{
  my $loadlibs = FindLib::findlib( $common::try_libs );
  print "...Found libs: '$loadlibs'\n";
  unless ($loadlibs =~ /-lX11/) {
    ExtUtils::MakeMaker::WriteEmptyMakefile();
    return;
  }
}

my $cflags = '';
if ($^O eq 'os2' or $^O eq 'MSWin32') {
  $cflags .= ' -DHAVE_STRNICMP';
} else {
  $cflags .= ' -DHAVE_STRNCASECMP';
}

my $gnuplot_x11_exe = 'gnuplot_x11b';

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot::plotx',
    'VERSION_FROM'	=> '../Gnuplot.pm',
    'LIBS'	=> [ $common::try_libs ],
    'INC'	=> "-I../gnuterm -I.",     # e.g., '-Idir' 
    'EXE_FILES'	=> [$gnuplot_x11_exe],     # e.g., '-Idir' 
    'SKIP'	=> [qw( distclean test dist makeaperl xs_o static dynamic_bs
			dynamic_lib linkext manifypods )],
    'DEFINE'	=> $cflags,     # e.g., '-DHAVE_SOMETHING' 
    #'FIXIN'	=> '@$(NOOP)',	# Not a script, an executable...
);

sub MY::postamble {
  my $self = shift;
  $self->{FIXIN} = '@$(NOOP)';	# Does not work...
"GNUPLOT_X11_EXE = $gnuplot_x11_exe" .
'

#linkext :: $(LINKTYPE)
linkext ::

manifypods :

test:

static::

$(GNUPLOT_X11_EXE): gplt_x11$(OBJ_EXT)
	$(CC) $(OPTIMIZE) $(LDFLAGS) -o $@ gplt_x11$(OBJ_EXT) $(LDLOADLIBS)
';
}
