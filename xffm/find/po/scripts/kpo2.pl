#!/usr/bin/perl
use File::Basename;
# This script creates the contrib dir from svn and git sources, copying only 
# the po files and creating a uniform structure for the po files.
#
# This is not run too often, just when adding new kde files as svn is 
# updated and gnome git files are updated.
#
# Easier just run po.pl, which creates the extended po files which should 
# then be tamed by make update-po and scripts/clean_po.pl
#
# Should be updated when KDE completes move to GIT version control.
#
$xffmdir="/home/SVN/xffm/CURRENT/rodent";
$kdedir="/home/SVN/kde/kde/l10n-kde4";
$gnomedir="/home/GIT/gnome";
$contribdir= "/tmp/source_po";

#@xffmpackages=(
#	"src/fgr",
#);

#$dry_run=1;

mkdir "$contribdir";
mkdir "$contribdir/core";
#mkdir "$contribdir/xffm";
mkdir "$contribdir/kde";
mkdir "$contribdir/gnome";

# GNU core stuff
print
`rsync -av /home/SVN/xffm/BRANCHES/contrib/coreutils-8.7/po/ $contribdir/core/coreutils-8.7/`;
print
`rsync -av /home/SVN/xffm/BRANCHES/contrib/diffutils-2.8.7/po/ $contribdir/core/diffutils-8.7/`;
#exit;
# copy previous fgr translations... 
#foreach $package (@xffmpackages) {
#    $tgt=$package;
#    $tgt =~ s/\//-/g;
#    mkdir "$contribdir/xffm/$tgt";
#    $c="cp -v $xffmdir/$package/po/*.po $contribdir/xffm/$tgt/";
#    print	"$c\n";
#    if (not $dry_run){print `$c`; }
#}
	  
# copy gnome contributions...
$_=`ls $gnomedir`;
@gnomepackages=split;
foreach $package (@gnomepackages) {
    if (not -e "$gnomedir/$package/po") {
#	print "$gnomedir/$package/po not found\n";
	next;
    }
    $tgt=basename($package);
    mkdir "$contribdir/gnome/$tgt";
    $c="cp -v $gnomedir/$package/po/*.po $contribdir/gnome/$tgt/";
    print	"$c\n";
    if (not $dry_run){print `$c`; }
}

$_=`ls $kdedir`;
@dirs=split;
foreach $dir (@dirs){
    print "$kdedir/$dir  ***\n";
    $lang=basename($dir);
    if ($lang =~ m/x-test/g){
	next;
    }
    $_=`ls $kdedir/$dir/messages`;
    @kdepackages=split;

    foreach $package (@kdepackages) {
	$_ = `ls $kdedir/$dir/messages/$package/*.po`;
	@po_files = split;
	foreach $po_file (@po_files) {
	  $t=basename($po_file);
	  ($tgt,$dump) = split /\.po/, $t, 2;
	  mkdir "$contribdir/kde/$tgt";
	  $c="cp -v $kdedir/$dir/messages/$package/$t $contribdir/kde/$tgt/$lang.po";
 
	  print "$c\n";
	  if (not $dry_run)
	  {
	    print `$c`; 
	  }
	}
    }
}

exit;
 

