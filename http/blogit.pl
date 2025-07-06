#!/usr/bin/perl

open INPUT, "blog.xml" or die "cannot open blog.xml";
$blog="";
while (<INPUT>){
    $blog.=$_;
}
close INPUT;
`cp blog.xml blog.txt.bak`;
$date=`LC_ALL=C date +"%A %B %d, %Y"`;
chop $date;

print "title:";
$title=<>;
chop $title;
print "entry: (end with EOF)\n";
$entry="";
while ($_=<>){
    if (/^\n/) {$entry .= "<br>\n"; next}
    if (/^EOF/g){goto done}
    $entry .= "$_";
}
done:


$newblog="
<blockquote>
<date>$date</date>
<title>$title</title>
$entry
</blockquote>

$blog";

open OUTPUT, ">blog.xml" or die "cannot open blog.xml for write\n";
print OUTPUT $newblog;
close OUTPUT;


