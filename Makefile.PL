use File::Basename;
use ExtUtils::MakeMaker;
use Config;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

$common::x = "$ENV{X11ROOT}/XFree86"
  if exists $ENV{X11ROOT} and -d "$ENV{X11ROOT}/XFree86/lib" and -d "$ENV{X11ROOT}/XFree86/include";

$try_libs = $common::x ? "-L$common::x/lib" : '';
$try_libs .= ' -L/usr/openwin/lib -lX11 -lm  -lsocket -lvga -llinuxvga -lgd -lpng -lz -ljmgraph -lvesa';

@ARGV = grep !(/^TRY_LIBS=(.*)$/ and $try_libs = $1), @ARGV;

$common::try_libs = $try_libs;

$common::pm_exe = 'gplpmdrb';			# Usual: gnupmdrv
$common::x_exe = 'gnuplot_x11b';		# Usual: without b
$common::defines = '-DPM3D -DUSE_ULIG_FILLEDBOXES';

# No IPC is needed without mouse enabled
if ($^O eq 'os2') {
 # $common::defines .= ' -DOS2_IPC';
} else {
 # $common::defines .= ' -DPIPE_IPC';
}

$common::cflags = '';

# Not used: inline, STDC_HEADERS, NOCWDRC, HAVE_ERF, HAVE_GAMMA,
# HAVE_GETCWD, HAVE_LGAMMA, HAVE_SETVBUF, HAVE_SYSINFO, HAVE_TCGETATTR, 
# HAVE_SGTTY_H, HAVE_SYS_BSDTYPES_H, HAVE_SYS_SYSTEMINFO_H

# Cannot find: DOS32, EXTERN_ERRNO, HAVE_LIBGD, HAVE_LIBPNG, LINUXVGA,
# MSDOS, NO_ERRNO_H, READLINE, X11, HAVE_SLEEP, 

# Cannot find, probably important: NO_ATEXIT, HAVE_ON_EXIT, PIPES, HAVE_LIBC_H
$common::cflags .= ' -DNO_STDLIB_H' unless $Config{i_stdlib};	# not used now
$common::cflags .= ' -DNO_FLOAT_H' unless $Config{i_float};

$common::cflags .= ' -DHAVE_UNISTD_H' if $Config{i_unistd};
$common::cflags .= ' -DHAVE_FLOAT_H'  if $Config{i_float};
$common::cflags .= ' -DHAVE_LIMITS_H' if $Config{i_limits};
$common::cflags .= ' -DHAVE_LOCALE_H' if $Config{i_locale};
$common::cflags .= ' -DHAVE_MATH_H'   if $Config{i_math};
$common::cflags .= ' -DHAVE_MALLOC_H'   if $Config{i_malloc};
$common::cflags .= ' -DHAVE_STDLIB_H' if $Config{i_stdlib};
$common::cflags .= ' -DHAVE_UNISTD_H' if $Config{i_unistd};
$common::cflags .= ' -DHAVE_STRING_H' if $Config{i_string};
$common::cflags .= ' -DHAVE_TERMIOS_H' if $Config{i_termios};
$common::cflags .= ' -DHAVE_VALUES_H' if $Config{i_values};
$common::cflags .= ' -DHAVE_SYS_TYPES_H' if $Config{i_systypes};
$common::cflags .= ' -DHAVE_SYS_STAT_H' if $Config{i_sysstat};
$common::cflags .= ' -DHAVE_SYS_PARAM_H' if $Config{i_sysparam};

$common::cflags .= ' -DNO_TIME_H'	# Is it reasonable?
  unless $Config{i_time} or $Config{timeincl} and $Config{timeincl} =~ /\W/;

$common::cflags .= ' -DOLD_SELECT' unless $Config{i_sysselct};
$common::cflags .= ' -DNO_MEMCPY' unless $Config{d_memcpy};
$common::cflags .= ' -DNO_MEMSET' unless $Config{d_memset};
$common::cflags .= ' -DNO_STRCHR' unless $Config{d_strchr};
$common::cflags .= ' -DHAVE_STRERROR' if $Config{d_strerror};
$common::cflags .= ' -DANSI_C' if $Config{'prototype'}; # Will -DPROTOTYPES automatically
$common::cflags .= ' -Dconst=' unless $Config{d_const};
$common::cflags .= ' -DNOCOPY' unless $Config{d_bcopy};
$common::cflags .= ' -Dsize_t=unsigned' unless $Config{sizetype} eq 'size_t';
$common::cflags .= ' -Dtime_t=long' unless $Config{timetype} eq 'time_t';

# $common::cflags .= ' -DGETCWD -DHAVE_STRNICMP -DPIPES -DHAVE_STRERROR -DNOGAMMA' if $^O eq 'os2';
if ($^O eq 'os2' or $^O eq 'MSWin32') {
  $common::cflags .= ' -DHAVE_STRNICMP';
} else {
  $common::cflags .= ' -DHAVE_STRNCASECMP';
}

$gnuplotdir = './gnuterm';

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot',
    'DEFINE'	=> $common::defines,
    'VERSION_FROM'	=> 'Gnuplot.pm',
    'MYEXTLIB'	=> $gnuplotdir . '/libgnuterm$(LIB_EXT)',
    'LIBS'	=> [ $try_libs ],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket -L. -lgnuterm'],
    #'LIBS'	=> [' -L/usr/openwin/lib -lX11 -lm  -lsocket '
    #		    . "-L$gnuplotdir -lgnuplot"],   # e.g., '-lm' 
    'INC'	=> "-I$gnuplotdir",     # e.g., '-Idir' 
    #'DEFINE'	=> '-DNO_JUNK',     # e.g., '-DHAVE_SOMETHING' 
    'dist' => {COMPRESS=>'bzip2 -f', SUFFIX=>'bz2'},
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
