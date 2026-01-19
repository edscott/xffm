#!/usr/bin/perl
# execute in po directory.
# options: potfiles, template, fullMerge, merge
#
# install: sudo pacman -S perl-parallel-forkmanager
use strict;
use warnings;
use Fcntl qw ( :flock );
use IO::Socket;
use Parallel::ForkManager;

use File::Compare;
my $srcdir="xffm";
my $workdir=`pwd`;
my $fullPoDir = "/home/GIT/source_po-processed";
my $catalog="xffm4";

my $intltool_update = "/usr/bin/intltool-update";
my $msgmerge = "/usr/bin/msgmerge";

# 1. Generate POTFILES.in
if ($ARGV[0] eq "potfiles") {&potfiles}
# 2. Generate xffm4 pot template file.
if ($ARGV[0] eq "template") { &template}
# 3.1 Merge po template with full po files
if ($ARGV[0] eq "fullmerge") {&fullmerge}
# 3.2 Merge po template with existing po files (empty new translations)
if ($ARGV[0] eq "merge") { &merge}
if ($ARGV[0] eq "fuzzy") { &fuzzy}
exit(1);

sub fuzzy{
    $_ = `ls *.po`;
    my $po;
    my @linguas = split;
    my $count;
    my $msgid;
    my $uso;
    foreach $po (@linguas) {
      $count = 0;
      $msgid=0;
      open IN, "$po" or die "cannot open $po \n";
      while (<IN>){
        if (/fuzzy/) {$count++;}
        if (/^msgid/) {$msgid++;}
      }
      close IN;
      $uso = ($msgid - $count) / $msgid * 100.; 
      printf ("%s fuzzys = %d msgid = %d uso = %.1lf %%\n" ,$po,$count,$msgid,$uso) ;
    }
    exit(1);
}

sub fullmerge{

    my $manager = Parallel::ForkManager->new(20); ###

    $_ = `ls $fullPoDir/*.po`;
    my $po;
    my @linguas = split;
    my $i=0;
    foreach $po (@linguas) {
       $manager->start and next; ###

       $po =~ s/$fullPoDir\///g;
        $i++;
        my $total = $#linguas+1;
        print "Processing $po ($i/$total) ... \n";
        my ($lang, $a) = split /\./,$po,2;
print "$msgmerge $fullPoDir/$po $catalog.pot -o -  --lang=$lang -i -q| grep -v \"#~\" | grep -v \"# \" | grep -v \"#\\.\" > $po.new\n";
        print `$msgmerge $fullPoDir/$po $catalog.pot -o -  --lang=$lang -i -q| grep -v "#~" | grep -v "# " | grep -v "#\\." > $po.new`;
        print "Cleaning $po...\n";
        open INPUT, "$po.new" or die "cannot open $po.new for read";
        my $title = "# Xffm4 translation file: ".`date`.
	    "# Merged from existing open source files\n".
	    "# http://xffm.sf.net\n";
        my $contents = $title;
        while (<INPUT>){$contents .= $_}
        close INPUT;

        open OUTPUT, ">$po" or die "cannot open $po for write";
        while ($contents =~ m/\n\n/g){
          $contents =~ s/\n\n\n/\n\n/g;
        }
        $contents =~ s/# http:\/\/xffm.org\n#\n#, fuzzy/# http:\/\/xffm.sf.net\n#\n#/g;
        $contents =~ s/Rodent Delta/xffm4/g;
        print OUTPUT $contents;
        close OUTPUT;
        print `rm $po.new`;
        
        $manager->finish;        
    }

    $manager->wait_all_children;
    
    exit(1);
}

sub merge {
    my $mergepot = "$intltool_update --gettext-package xffm4 --dist";
    print `$mergepot es_MX`;
    exit(1);
}


sub template {
    my $genpot = "$intltool_update --gettext-package xffm4 --pot";
    print `$genpot`;
    print `cat xffm4.pot`; exit(1);
    exit(1);
}

sub potfiles {
    chdir "..";
    $_ = `find $srcdir -name "*.hh" > po/POTFILES.in`;
    print `cat po/POTFILES.in`; exit(1);
    chdir "po";
    exit(1);
}
