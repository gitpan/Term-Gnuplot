use ExtUtils::MakeMaker;
use Config;

my $cflags = "-DRETSIGTYPE=$Config{signal_t} -DPIPES -DX11";

# Not used: inline, STDC_HEADERS, NOCWDRC, HAVE_ERF, HAVE_GAMMA,
# HAVE_GETCWD, HAVE_LGAMMA, HAVE_SETVBUF, HAVE_SYSINFO, HAVE_TCGETATTR, 
# HAVE_SGTTY_H, HAVE_SYS_BSDTYPES_H, HAVE_SYS_SYSTEMINFO_H

# Cannot find: DOS32, EXTERN_ERRNO, HAVE_LIBGD, HAVE_LIBPNG, LINUXVGA,
# MSDOS, NO_ERRNO_H, READLINE, X11, HAVE_SLEEP, 

# Cannot find, probably important: NO_ATEXIT, HAVE_ON_EXIT, PIPES, HAVE_LIBC_H
$cflags .= ' -DHAVE_UNISTD_H' if $Config{i_unistd};
$cflags .= ' -DNO_FLOAT_H' unless $Config{i_float};
$cflags .= ' -DNO_LIMITS_H' unless $Config{i_limits};
$cflags .= ' -DNO_LOCALE_H' unless $Config{i_locale};
$cflags .= ' -DNO_STDLIB_H' unless $Config{i_stdlib};
$cflags .= ' -DNO_STRING_H' unless $Config{i_string};
$cflags .= ' -DHAVE_STRING_H' if $Config{i_string};
$cflags .= ' -DHAVE_TERMIOS_H' if $Config{i_termios};
$cflags .= ' -DHAVE_VALUES_H' if $Config{i_values};
$cflags .= ' -DNO_SYS_TYPES_H' unless $Config{i_systypes};

$cflags .= ' -DNO_TIME_H'	# Is it reasonable?
  unless $Config{i_time} or $Config{timeincl} and $Config{timeincl} =~ /\W/;

$cflags .= ' -DOLD_SELECT' unless $Config{i_sysselct};
$cflags .= ' -DNO_MEMCPY' unless $Config{d_memcpy};
$cflags .= ' -DNO_MEMSET' unless $Config{d_memset};
$cflags .= ' -DNO_STRCHR' unless $Config{d_strchr};
$cflags .= ' -DHAVE_STRERROR' if $Config{d_strerror};
$cflags .= ' -DANSI_C' if $Config{'prototype'}; # Will -DPROTOTYPES automatically
$cflags .= ' -Dconst=' unless $Config{d_const};
$cflags .= ' -DNOCOPY' unless $Config{d_bcopy};
$cflags .= ' -Dsize_t=unsigned' unless $Config{sizetype} eq 'size_t';
$cflags .= ' -Dtime_t=long' unless $Config{timetype} eq 'time_t';

# $cflags .= ' -DGETCWD -DHAVE_STRNICMP -DPIPES -DHAVE_STRERROR -DNOGAMMA' if $^O eq 'os2';
if ($^O eq 'os2' or $^O eq 'MSWin32') {
  $cflags .= ' -DHAVE_STRNICMP';
} else {
  $cflags .= ' -DHAVE_STRNCASECMP';
}

use ExtUtils::Liblist;

{
  print "...Looking for libs, expect some warnings...\n";
  local @ExtUtils::Liblist::ISA = @ExtUtils::Liblist::ISA;
  push @ExtUtils::Liblist::ISA, 'xxx';
#  *xxx::lsdir = 'ExtUtils::MakeMaker'->can('lsdir');
  *xxx::lsdir = 'MY'->can('lsdir');
  *xxx::file_name_is_absolute = 'MY'->can('file_name_is_absolute');

#  my $loadlibs = (ExtUtils::Liblist->ext('-lvga -lvesa -ljmgraph -llinuxvga -lgd -lpng -lz'))[2];
  my $loadlibs = (ExtUtils::Liblist->ext( $common::try_libs ))[2];
  print "...Found libs: '$loadlibs'\n";
  if ($loadlibs =~ /-lgd\b/) {
    my $have;
    # Need to check for gdImageCreate, if not found, abort...  How?!!!
    if (open IN, "$Config{cpp} $Config{cppflags} trygd.h |") {
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
  }
  my $addflags = ' ';
  $addflags .= ' -DHAVE_LIBGD'
	if $loadlibs =~ /-lgd\b/;
  $addflags .= ' -DHAVE_LIBPNG'
	if $loadlibs =~ /-lpng\b/ and $loadlibs =~ /-lz\b/;
  $addflags .= ' -DLINUXVGA'
	if $loadlibs =~ /-llinuxvga\b/;
  $addflags .= ' -DEMXVESA'
	if $loadlibs =~ /-lvesa\b/ and $loadlibs =~ /-ljmgraph\b/;
  if ($addflags =~ /\S/) {
    print "...Adding flags '$addflags'\n";
    $cflags .= $addflags;
  }
}

# version.c is used to output "produced by version" to some output formats
# bitmap.c is used by hpljii.trm
# util.c is used to parse the string in many places (though probably we
# do not touch this code yet)

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot::gnuterm',
    'LINKTYPE'	=> 'static',
    'OBJECT'	=> 'term$(OBJ_EXT) util$(OBJ_EXT) version$(OBJ_EXT) bitmap$(OBJ_EXT) stdfn$(OBJ_EXT) shim_gnuplot$(OBJ_EXT)', 
    'DEFINE'	=> $cflags,
    'INC'	=> '-I. -Iterm -Igd -I..',
    'SKIP'	=> [qw( distclean test dist makeaperl xs_o static)],
    'clean'	=> {'FILES' => 'libgnuterm$(LIB_EXT)'},
    'FUNCLIST'	=> ['get_term_ftable']
);

sub MY::top_targets {
  '
all :: libgnuterm$(LIB_EXT)

static ::       libgnuterm$(LIB_EXT)

test:

libgnuterm$(LIB_EXT): $(OBJECT)
	$(AR) $(ADD_AR_OPT) cr libgnuterm$(LIB_EXT) $(OBJECT)
	$(RANLIB) libgnuterm$(LIB_EXT)

';
}

sub MY::const_config		# Mostly from Tk
{
 my $self = shift;
 my $flags = $self->{'CCCDLFLAGS'};
 $flags =~ s/(-[fK]?\s*)pic\b/${1}PIC/; 
 $self->{'CCCDLFLAGS'} = $flags;
 return $self->MM::const_config;
}

