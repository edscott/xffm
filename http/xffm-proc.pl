#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-proc class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-proc $version: process monitor plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-proc";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191730";
$abstract = "Xffm-proc $version is a process monitor plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The PROCESSES plugin allows you to locate and control processed running on 
your system.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

