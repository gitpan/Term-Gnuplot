use ExtUtils::MakeMaker;
use Config;

my $cflags = "$common::defines -DRETSIGTYPE=$Config{signal_t} -DGPLT_X11_MODE -DPIPES -DX11 -DX11_DRIVER_DIR=\\\".\\\"";

$cflags .= $common::cflags;

BEGIN {
  push @INC, '../utils';
}
use FindLib;

{
  my $loadlibs = FindLib::findlib( $common::try_libs );
  print "...Found libs: '$loadlibs'\n";
  if ($loadlibs =~ /-lgd\b/) {
    my $have;
    # Need to check for gdImageCreate, if not found, abort...  How?!!!
    my $cppf = $Config{cppflags};
    $cppf =~ s/-Zmt\b// if $^O eq 'os2';
    if (open IN, "$Config{cpp} $cppf trygd.h |") {
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

$cflags .= " -DEXENAME_X11=$common::x_exe -DEXENAME_PM=$common::pm_exe -DCANT_ACCEPT_COMMANDS -DX11_FORCED_PATH";

# version.c is used to output "produced by version" to some output formats
# bitmap.c is used by hpljii.trm
# util.c is used to parse the string in many places (though probably we
# do not touch this code yet)

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot::gnuterm',
    'LINKTYPE'	=> 'static',
     'OBJECT'	=> 'term$(OBJ_EXT) util$(OBJ_EXT) version$(OBJ_EXT) bitmap$(OBJ_EXT) stdfn$(OBJ_EXT) getcolor$(OBJ_EXT) my_utils$(OBJ_EXT) shim_gnuplot$(OBJ_EXT)', 
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

