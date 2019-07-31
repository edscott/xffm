#!/usr/bin/perl
$files = `ls *.po`;
@files = split /\n/, $files;
foreach $file (@files){
    print "file = $file\n";
    print `perl clean.pl $file`;
    print `mv -v $file.new $file`;
}
