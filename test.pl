use Term::Gnuplot;
use integer;			# To get the same results as standard one

my ($n, $d);
# list_terms();
for $n (sort keys %Term::Gnuplot::description) {
  my $t = "\t" x (2 - int ((1 + length $n)/8));
  print " $n$t=> $Term::Gnuplot::description{$n}\n";
}

test_term("dumb");
if ($Term::Gnuplot::description{pm}) {
  test_term("pm");
} 
if ($ENV{DISPLAY} and $Term::Gnuplot::description{x11}) {
  &test_term("x11");
}
while (1) {
  $|=1;
  # list_terms();
  for $n (sort keys %Term::Gnuplot::description) {
    my $t = "\t" x (2 - int ((1 + length $n)/8));
    print " $n$t=> $Term::Gnuplot::description{$n}\n";
  }
  print "Type terminal name, 'file' to set output file(s), or ENTER to finish";
  print "\n  Or type 'pTk' to try the direct-to-Tk demo: ";
  $in = <>;
  chomp $in;
  if ($in eq 'file') {
    print "Output file name(s) for builtin and Perl tests? ";
    $file = <>;
    chomp $file;
    @files = split " ", $file;
    push @files, "perl$files[0]" if @files == 1;
    redo;
  }
  last unless $in;
  &test_term($in);
  @files = ();
}

my $ptk_canvas;
my $mw;
my $ptk;
my $ptk_waited;

sub test_term {
  my $name = shift;
  my $comment = "";
  $comment = ' - height set to 32' if $name eq 'dumb';
  $comment = ' - window name set to "Specially named terminal"'
    if $name eq 'pm';

  print("Output file $files[0]\n"), plot_outfile_set(shift @files) if @files;
  if ($name eq 'pTk') {
    $name = 'tkcanvas';
    $ptk = 1;
    $ptk_canvas->delete('all') if $ptk_canvas;
    eval <<'EOE' unless $ptk_canvas;
      use Tk;

      $mw = MainWindow->new;
      $ptk_canvas = $mw->Canvas('-height', 400, '-width', '600','-border'=>0, '-relief' => 'raised', '-bg' => 'aliceblue')
	   ->pack('-fill', 'both', '-expand', 1);
      Term::Gnuplot::setcanvas($ptk_canvas);
      $mw->update();
      $mw->fileevent(STDIN, 'readable', sub {<>; $ptk_waited = 1});
EOE
    warn $@ if $@;
  }
  print("Switch to `$name': not OK: $out\n"), return
      unless $out = Term::Gnuplot::change_term($name);
  print "Builtin test for `$name'$comment, press ENTER\n";
  if ($ptk_canvas) {
    $ptk_waited = 0;
    $ptk_canvas->waitVariable(\$ptk_waited);
  } else {
    <>;
  }
  $mw->update() if $ptk;
  if ($name eq 'pm') {
    set_options('"Specially named terminal"');
  } elsif ($name eq 'dumb') {
    set_options(79,32);
  } elsif ($ptk) {
    set_options('tkperl_canvas');
  } else {
    set_options();		#  if $name eq 'gif' - REQUIRED to init things
  }
#  &Term::Gnuplot::init() if !$initialized{$name}++;
  &Term::Gnuplot::term_init();
#  print("Output file $files[0]\n"), plot_outfile_set(shift @files) if @files;

  &Term::Gnuplot::test_term();
  $ptk_canvas->update() if $ptk;
  print "\n$name builtin test OK, Press ENTER\n";
  if ($ptk_canvas) {
    $ptk_waited = 0;
    $ptk_canvas->waitVariable(\$ptk_waited);
  } else {
    <>;
  }


  print "Perl test for `$name'$comment, press ENTER\n";
  if ($ptk_canvas) {
    $ptk_waited = 0;
    $ptk_canvas->waitVariable(\$ptk_waited);
  } else {
    <>;
  }

  use Term::Gnuplot ':ALL';

  print("Output file $files[0]\n"), 
#    plot_outfile_set(shift @files), reset() if @files;
    plot_outfile_set(shift @files) if @files;

  $ptk_canvas->delete('all') if $ptk_canvas;
#  init() unless $initialized{$name}++;
  term_init() unless $initialized{$name}++;
  {
    my($name,$description,$xmax,$ymax,$v_char,$h_char,$v_tic,$h_tic) =
      (&Term::Gnuplot::name,&Term::Gnuplot::description,&Term::Gnuplot::xmax,&Term::Gnuplot::ymax,
       &Term::Gnuplot::v_char,&Term::Gnuplot::h_char,&Term::Gnuplot::v_tic,&Term::Gnuplot::h_tic);
    print <<EOD;
Term data: 
	name '$name',
	description '$description',
	xmax $xmax,
	ymax $ymax,
	v_char $v_char,
	h_char $h_char,
	v_tic $v_tic,
	h_tic $h_tic.
EOD
  }

  my ($xsize,$ysize) = (1,1);
  my $scaling = scale($xsize, $ysize);
  my $xmax = xmax() * ($scaling ? 1 : $xsize);
  my $ymax = ymax() * ($scaling ? 1 : $ysize);
  my $pointsize = 1;			# XXXX We did not set it
  my $key_entry_height = $pointsize * v_tic() * 1.25;

  $key_entry_height = v_char() if $key_entry_height < v_char();
  my $p_width = $pointsize * v_tic();

#  graphics();
  term_start_plot();

  # border linetype 
  linetype(-2);
  move(0,0);
  vector($xmax-1,0);
  vector($xmax-1,$ymax-1);
  vector(0,$ymax-1);
  vector(0,0);
  justify_text(LEFT);
  put_text(h_char()*5, $ymax - v_char()*3,"Terminal Test, Perl");

  # axis linetype 
  linetype(-1);
  move($xmax/2,0);
  vector($xmax/2,$ymax-1);
  move(0,$ymax/2);
  vector($xmax-1,$ymax/2);

  #	/* test width and height of characters */
  linetype(-2);
  move(  $xmax/2-h_char()*10,$ymax/2+v_char()/2);
  vector($xmax/2+h_char()*10,$ymax/2+v_char()/2);
  vector($xmax/2+h_char()*10,$ymax/2-v_char()/2);
  vector($xmax/2-h_char()*10,$ymax/2-v_char()/2);
  vector($xmax/2-h_char()*10,$ymax/2+v_char()/2);
  put_text($xmax/2-h_char()*10,$ymax/2,
		"12345678901234567890");

  # test justification 
  justify_text(LEFT);
  put_text($xmax/2,$ymax/2+v_char()*6,"left justified");
  my $str = "centre+d text";
  if (justify_text(CENTRE)) {
    put_text($xmax/2,
    $ymax/2+v_char()*5,$str);
  } else {
    put_text($xmax/2-length($str)*h_char()/2,
	     $ymax/2+v_char()*5,$str);
  }
  $str = "right justified";
  if (justify_text(RIGHT)) {
    put_text($xmax/2,
	     $ymax/2+v_char()*4,$str);
  } else {
    put_text($xmax/2-length($str)*h_char(),
	     $ymax/2+v_char()*4,$str);
  }

  # test text angle 
  $str = "rotated ce+ntred text";
  if (text_angle(1)) {
    if (justify_text(CENTRE)) {
      put_text(v_char(),
	       $ymax/2,$str);
    } else {
      put_text(v_char(),
	       $ymax/2-length($str)*h_char()/2,$str);
    }
  } else {
    justify_text(LEFT);
    put_text(h_char()*2,$ymax/2-v_char()*2,"Can't rotate text");
  }
  justify_text(LEFT);
  text_angle(0);

  # test tic size 
  move($xmax/2+h_tic()*2,0);
  vector($xmax/2+h_tic()*2,v_tic());
  move($xmax/2,v_tic()*2);
  vector($xmax/2+h_tic(),v_tic()*2);
  put_text($xmax/2-h_char()*10,v_tic()*2+v_char()/2,"test tics");

  # test line and point types 

  pointsize($pointsize);
  my $x = $xmax - h_char()*6 - $p_width;
  my $y = $ymax - v_char();
  my $i;
  for ( $i = -2; $y > $key_entry_height; $i++ ) {
    linetype($i);
    if (justify_text(RIGHT)) {
      put_text($x,$y,$i+1);
    } else {
      put_text($x-length($i+1)*h_char(),$y,$i+1);
    }
    move($x+h_char(),$y);
    vector($x+h_char()*4,$y);
    if ( $i >= -1 ) {
      point($x+h_char()*5 + int($p_width/2),$y,$i);
    }
    $y -= $key_entry_height;
  }

  # test some arrows 
  linetype(0);
  $x = $xmax/4;
  $y = $ymax/4;
  $xl = h_tic()*5;
  $yl = v_tic()*5;
  arrow($x,$y,$x+$xl,$y,1);
  arrow($x,$y,$x+$xl/2,$y+$yl,1);
  arrow($x,$y,$x,$y+$yl,1);
  arrow($x,$y,$x-$xl/2,$y+$yl,0);
  arrow($x,$y,$x-$xl,$y,1);
  arrow($x,$y,$x-$xl,$y-$yl,1);
  arrow($x,$y,$x,$y-$yl,1);
  arrow($x,$y,$x+$xl,$y-$yl,1);

  # and back into text mode 

#  text();
  term_end_plot();
  $ptk_canvas->update() if $ptk;
  # Tk::MainLoop() if $pTk;
  print "\n$name Perl test OK, Press ENTER\n";
  if ($ptk_canvas) {
    $ptk_waited = 0;
    $ptk_canvas->waitVariable(\$ptk_waited);
  } else {
    <>;
  }
  &Term::Gnuplot::reset();
}
