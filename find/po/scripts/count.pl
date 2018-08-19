#!/usr/bin/perl
open INPUT,$ARGV[0] or die "cannot open $ARGV[0] (ARGV[0]) for input";
loop:
$lang=<INPUT>;
if ($lang){
    chop $lang;
    $_=<INPUT>;
    $_=<INPUT>;
    ($count,$dump) = split /mensajes/, $_, 2;
    $percent=100*$count/340 + 0.5;
    $ipercent=int $percent;

#    print "$lang $ipercent% ($count/340)\n";
    $lang =~ s/://g;
    print "$lang $ipercent $count/340\n";
    goto loop;
}
close INPUT;

