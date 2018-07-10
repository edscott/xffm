#!/usr/bin/perl
use File::Basename;
# Fast and clean. Once "make update-po" has run successfully, use this script
# to cleanout all the commented translations which are not used in Rodent
# applications. This will also remove the translators string that appears 
# in the "about" dialog, so mk_translators.pl should follow this one, always.
#
# Use this script before doing a commit, always.

#
$_=`ls *.po`;
    @files=split;
    foreach $file (@files){
	print "cleaning $file...\n";
	if (not $file =~ m/\.po$/g) {next}
	$buffer="";
	open INPUT, "$file" or die "cannot open ./$file for input\n";
	while (<INPUT>){
	    if (/Project-Id-Version: Rodent Beta/){
		s/Rodent Beta/Rodent Gamma/g;
	    }
	    $buffer .= $_;
	}
	close INPUT;
	@lines=split /\n/, $buffer;
	open OUTPUT, ">$file" or die "cannot open $file for output\n";
	undef $fuzzy_line;
	foreach $_ (@lines){
	    if ($_ eq "\n" and $fuzzy_line) {
		undef $fuzzy_line;
		next;
	    }
	    if ($lastline eq "" and $_ eq "") {next}
	    if (/^#, fuzzy/){
		$fuzzy_line=1;
		next;
	    }

	    if (not /^#~ /) {
		if ($fuzzy_line){
		    print OUTPUT "#, fuzzy\n";
		    undef $fuzzy_line;
		}
		print OUTPUT "$_\n";
		$lastline=$_;
	    }
	}
	close OUTPUT;
    }



