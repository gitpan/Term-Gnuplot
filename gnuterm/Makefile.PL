use ExtUtils::MakeMaker;
use Config;

my $cflags = '-DNOCWDRC';
$cflags .= ' -DGETCWD -DPIPES' if $^O eq 'os2';

WriteMakefile(
    'NAME'	=> 'Term::Gnuplot::gnuplot',
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

