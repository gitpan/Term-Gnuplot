use Term::Gnuplot;
use integer;			# To get the same results as standard one

list_terms();
&test_term("dumb");
if ($^O eq 'os2') {
  &test_term("pm");
} 
if ($ENV{DISPLAY}) {
  &test_term("x11");
}
while (1) {
  $|=1;
  list_terms();
  print "Input terminal name or ENTER to finish: ";
  $in = <>;
  chomp $in;
  last unless $in;
  &test_term($in);
}

sub test_term {
  my $name = shift;
  print("Switch to `$name': not OK: $out\n"), return
      unless ($out=Term::Gnuplot::change_term($name))>=0;
  print "Builtin test for `$name', press ENTER\n";
  <>;
  &Term::Gnuplot::init() if !$initialized{$name}++;
  &Term::Gnuplot::test_term();
  print "\n$name builtin test OK, Press ENTER\n";
  <>;

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
  
  print "Perl test for `$name', press ENTER\n";
  <>;
  use Term::Gnuplot ':ALL';

  init() unless $initialized{$name}++;
  my ($xsize,$ysize) = (1,1);
  my $scaling = scale($xsize, $ysize);
  my $xmax = xmax() * ($scaling ? 1 : $xsize);
  my $ymax = ymax() * ($scaling ? 1 : $ysize);

  graphics();

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
  put_text($xmax/2+h_tic()*2,v_tic()*2+v_char()/2,"test tics");

  # test line and point types 
  my $x = $xmax - h_char()*4 - h_tic()*4;
  my $y = $ymax - v_char();
  my $i;
  for ( $i = -2; $y > v_char(); $i++ ) {
    linetype($i);
    if (justify_text(RIGHT)) {
      put_text($x,$y,$i+1);
    } else {
      put_text($x-length($i+1)*h_char(),$y,$i+1);
    }
    move($x+h_char(),$y);
    vector($x+h_char()*4,$y);
    if ( $i >= -1 ) {
      point($x+h_char()*4+h_tic()*2,$y,$i);
    }
    $y -= v_char();
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
    
  text();
  print "\n$name Perl test OK, Press ENTER\n";
  <>;  
  &Term::Gnuplot::reset();
}
