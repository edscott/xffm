#!/usr/bin/perl
$file = $ARGV[0];
`cp $file $file.bak`;
open INPUT, "$file.bak" or die "cannot open $file.bak";
open OUPUT, ">$file.new" or die "cannot open >$file.new";
$fuzzyZap = 0;
while (<INPUT>){
    if (/^#~ /){ $fuzzyZap = 1;}
    if (not /^#~ /){
	if (not $fuzzyZap) {
	    print OUPUT;
	}
    }
}
close INPUT;

