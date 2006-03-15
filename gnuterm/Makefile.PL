use ExtUtils::MakeMaker;
use Config;

my $cflags = "$common::defines -DRETSIGTYPE=$Config{signal_t} -DGPLT_X11_MODE -DPIPES -DX11 -DX11_DRIVER_DIR=\\\".\\\"";

$cflags .= $common::cflags;

$try_libs = $common::try_libs;

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

