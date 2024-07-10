#!/usr/bin/perl

$_ = `ls coreutils-8.20/*\.po`;
@files = split /\n/;
foreach $file (@files){
    undef %index_hash;
    ($dir,$base) = split /\//, $file, 2;
    print "# $file ---> $base\n";
    open INPUT, "$file" or die "cannot open $file\n";
    $initial_comments = 0;
    open OUTPUT, ">coreatomized/$base" or die "cannot open coreatomized/$base\n";
    print OUTPUT "# Atomized translations for Rodent package.\n";

    $c = "";
    while (<INPUT>){
	if (/msgid/){$initial_comments++}
	if ($initial_comments < 2) {print OUTPUT $_; next}
	if ($_ eq "\n"){next}
#	if (/"\\"/g){print}
#	s/\\\\"/\\\\ "/g;
	s/\\"\\\\"/\\\\ "/g;
	s/\\"/'/g;
#	s/\\\\ "/\\\\"/g;
	
	$c .= $_;
    }
    close INPUT;
    @a = split /msgid /, $c;
    for ($i=0;$i<=$#a; $i++){
	$_ = $a[$i];
#	if ($fuzzy){print "++++fuzzy\n";}
	if (/msgid_plural/) {
#	   print "msgid $_";
	   next;
	}
	if (not /msgstr /) {
#	   print;
	   next;
	}
		

	($b,$d) = split /msgstr /, $_, 2;

#	print "msgid $b";
	($e,$f) = split /\n\n/, $d, 2;
	if ($e =~ m/\n#/){
	    ($ee,$eee)= split /\n#/, $e, 2;
	    $e = $ee;
	}
#	print "msgstr $e\n\n$f";
		$e =~ s/" +/"/g;
		$b =~ s/" +/"/g;
		$e =~ s/"' +/"/g;
		$b =~ s/"' +/"/g;


	if ($b =~ m/"-/ and $b =~ m/\n"/ and $e ne "\"\""){
#	if (not $fuzzy and $b =~ m/"-/ and $b =~ m/\n"/ and $e ne "\"\""){
	    $e .= "\n";
#	    if ($b =~ m/\\"-\\"/g) {
#	        $ee = $b;
#		$ee =~ s/\\"-\\"/'-'/g;
#		print "$b --> $ee\n";
#		$b = $ee;
#	    }
#	    if ($e =~ m/\\"-\\"/g) {
#	        $ee = $e;
#		$ee =~ s/\\"-\\"/'-'/g;
#		print "$e --> $ee\n";
#		$e = $ee;
#	    }

	    @g = split /"-/, $b;
	    @h = split /"-/, $e;
	    
#	    print "g=$#g h=$#h\n";
	    if ($#g == $#h and $#g > 1)
	    {
#	      print OUTPUT "#++++++++++++\n";
#	      print OUTPUT "#msgid $b";
#	      print OUTPUT "#msgstr $e";
#	      print OUTPUT "#------------\n";
	      for ($j=0; $j<=$#g; $j++){
	        $gg = $g[$j];
	        $hh = $h[$j];
#		if ($gg =~ m/^""\n/g) {next}
		if ($gg eq "\"\"") {next}
		if ($gg eq "\"\"\n") {next}
		if ($hh eq "\"\"\n") {next}
		if ($gg =~ m/^"\n""/g) {next}
		if ($gg eq "\"\n") {next}


		if ($hh =~ m/^"\n""/g) {next}
		if ($hh eq "\"\n") {next}

		$gg = "\"-" . $gg;
		$gg =~ s/\n$//g;
		$gg =~ s/^"-""\n//g;
		$gg =~ s/\n"\\n"$//g;
		$gg =~ s/\\n"$/"/g;
		
		if ($gg eq "\"\"") {next}
		
		$hh =~ s/\n$//g;
		$hh =~ s/\n$//g;
		$hh =~ s/\\n"$/"/g;
		$hh = "\"-" . $hh;
		$hh =~ s/^"-""\n//g;

#		Strip trailing ""  (nl.po)
		$hh =~ s/\n\"\"$//g;
		$hh =~ s/\\n"$/"/g;

#$hh =~ s/\n"\\n"$//g;
		if (not $index_hash{"$gg"})
		{
		    print OUTPUT "msgid $gg\n";
		    print OUTPUT "msgstr $hh\n\n";
		    $index_hash{"$gg"} = 1;
		}
	      }
	    }

#	    print "msgstr $e\n\n$f";
	    
	}
	if ($f =~ m/fuzzy/g) {$fuzzy=1;}
	else {undef $fuzzy}
	
    }
    close OUTPUT;
#   exit 1;
}


