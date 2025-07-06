#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-applications class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-applications $version: Desktop applications plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-applications";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191718";
$abstract = "Xffm-applications $version is a desktop applications plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The APPLICATIONS plugin allows you to locate, run, modify and copy 
applications for which a .desktop file has been installed in the default 
system location.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

