use File::Basename;
use ExtUtils::MakeMaker;
use Config;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

$try_libs = ' -L/usr/openwin/lib -lX11 -lm  -lsocket -lvga -llinuxvga -lgd -lpng -lz -ljmgraph -lvesa';

@ARGV = grep !(/^TRY_LIBS=(.*)$/ and $try_libs = $1), @ARGV;

use ExtUtils::Liblist;

{
  print "...Looking for libs, expect some warnings...\n";
  local @ExtUtils::Liblist::ISA = @ExtUtils::Liblist::ISA;
  push @ExtUtils::Liblist::ISA, 'xxx';
#  *xxx::lsdir = 'ExtUtils::MakeMaker'->can('lsdir');
  *xxx::lsdir = 'MY'->can('lsdir');
  *xxx::file_name_is_absolute = 'MY'->can('file_name_is_absolute');

#  my $loadlibs = (ExtUtils::Liblist->ext('-lvga -lvesa -ljmgraph -llinuxvga -lgd -lpng -lz'))[2];
  my $loadlibs = (ExtUtils::Liblist->ext( $try_libs ))[2];
  print "...Found libs: '$loadlibs'\n";
  if ($loadlibs =~ /-lgd\b/) {
    my $have;
    # Need to check for gdImageCreate, if not found, abort...  How?!!!
    my $cppflags = $Config{cppflags};
    $cppflags =~ s/-Zmt\b// if $^O eq 'os2';	# Better fixed in Configure...
    if (open IN, "$Config{cpp} $cppflags trygd.h |") {
      local $/;
      my $in = <IN>;
      $have = ($in =~ /gdImageCreate/);
      if ($have) {
	if ($in =~ /gdImageGif/) {
	  print "...Found gdImageCreate and gdImageGif in gd.h, keeping -lgd.\n";
	} else {
	  print "...No gdImageGif in gd.h, removing -lgd.\n";
	  $have = 0;
	}
      } else {
	print "...No gdImageCreate in gd.h, removing -lgd.\n";
      }
    } else {
      print "...Did not find gd.h, removing -lgd.\n";
    }
    $loadlibs =~ s/-lgd\b\S*//g unless $have; # //g just in case
    $try_libs =~ s/-lgd\b\S*//g unless $have; # //g just in case
    if (open IN, "$Config{cpp} $cppflags trypng.h |") {
      local $/;
      my $in = <IN>;
      $have = ($in =~ /\bpng_read_image\b/);
      if ($have) {
	print "...Found png_read_image in png.h, keeping -lpng.\n";
      } else {
	print "...No png_read_image in png.h, removing -lpng.\n";
      }
    } else {
      print "...Did not find png.h, removing -lpng.\n";
      $have = 0;
    }
    $loadlibs =~ s/-lpng\b\S*//g unless $have; # //g just in case
    $try_libs =~ s/-lpng\b\S*//g unless $have; # //g just in case
  }
  $common::loadlibs = $loadlibs;
}

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
# Not supported by CPAN.pm (thus by PAUSE) as of Nov 2005
#    'dist' => {COMPRESS=>'bzip2 -f', SUFFIX=>'bz2'},
	 # Let us hope that boot_Term__Gnuplot should not be mangled
    'FUNCLIST'	=> ['get_term_ftable', 'boot_Term__Gnuplot'],
    clean     => { FILES    => 'test_x11' },
);

sub MY::postamble {
  # First, take a possibility to check for the presence of gnuplot_x11
  my $self = shift;
  if ($self->MY::_maybe_command_in_dirs(['gnuplot_x11'],[$self->path])) {
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

# This subroutine disappeared at some version of perl...
sub MY::_maybe_command_in_dirs {	# $ver is optional argument if looking for perl
# Ilya's suggestion. Not yet used, want to understand it first, but at least the code is here
    my($self, $names, $dirs, $trace, $ver) = @_;
    my($name, $dir);
    foreach $dir (@$dirs){
	next unless defined $dir; # $self->{PERL_SRC} may be undefined
	foreach $name (@$names){
	    my($abs,$tryabs);
	    if ($self->file_name_is_absolute($name)) { # /foo/bar
		$abs = $name;
	    } elsif ($self->canonpath($name) eq $self->canonpath(basename($name))) { # bar
		$abs = $self->catfile($dir, $name);
	    } else { # foo/bar
		$abs = $self->catfile($self->curdir, $name);
	    }
	    print "Checking $abs for $name\n" if ($trace >= 2);
	    next unless $tryabs = $self->maybe_command($abs);
	    print "Substituting $tryabs instead of $abs\n"
		if ($trace >= 2 and $tryabs ne $abs);
	    $abs = $tryabs;
	    if (defined $ver) {
		print "Executing $abs\n" if ($trace >= 2);
		if (`$abs -e 'require $ver; print "VER_OK\n" ' 2>&1` =~ /VER_OK/) {
		    print "Using PERL=$abs\n" if $trace;
		    return $abs;
		}
	    } else { # Do not look for perl
		return $abs;
	    }
	}
    }
}
