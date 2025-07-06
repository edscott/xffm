#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xfdiff class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xfdiff $version: Patch Manager and Difference Viewer";
$svnpath = "/CURRENT/graphic-utilities/xfdiff";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=165444";
$abstract = "Xfdiff $version is graphic interface to the GNU diff and patch commands.";
$description ="<br>
<b>Description</b><br><blockquote>
Xfdiff $version is graphic interface to the GNU diff and patch commands. With this utility, you can view differences side by side for files or directories. You can also view differences that applying a patch file would imply, without applying the patch. You can also apply patches to the hard disc or create patch files for differences between files or directories. All-in-all, a handy utility for lazy chaps who don't want to type the <b>diff</b> command.
<br></blockquote>
<b>Development</b><br><blockquote>
Xfdiff first appeared around year 2000 in the xfce3 desktop as an included application. The application was later ported to GTK-2 and included within the xffm tarball, which uses the application as a filemanagement tool. During the 4.3 release series, xfdiff has returned to the status of independent application to allow installation for uses without xffm filemanager or the xfce programs.<br>
Although not really necessary, documentation will soon be available. </blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

