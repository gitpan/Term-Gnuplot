use ExtUtils::MakeMaker;
use Config;

my $cflags = '-DNOCWDRC';
$cflags .= ' -DGETCWD -DHAVE_STRNICMP -DPIPES -DHAVE_STRERROR -DNOGAMMA' if $^O eq 'os2';

# version.c is used to output "produced by version" to some output formats
# bitmap.c is used by hpljii.trm
# util.c is used to parse the string in many places (though probably we
# do not touch this code yet)

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot::gnuterm',
    'LINKTYPE'	=> 'static',
    'OBJECT'	=> 'term$(OBJ_EXT) util$(OBJ_EXT) version$(OBJ_EXT) bitmap$(OBJ_EXT)', 
    'DEFINE'	=> $cflags,
    'INC'	=> '-Iterm',
    'SKIP'	=> [qw( distclean test dist makeaperl xs_o static)],
    'clean'	=> {'FILES' => 'libgnuterm$(LIB_EXT)'}
);

sub MY::top_targets {
  '
all :: libgnuterm$(LIB_EXT)

static ::       libgnuterm$(LIB_EXT)

libgnuterm$(LIB_EXT): $(OBJECT)
	$(AR) $(ADD_AR_OPT) cr libgnuterm$(LIB_EXT) $(OBJECT)
	$(RANLIB) libgnuterm$(LIB_EXT)

';
}

