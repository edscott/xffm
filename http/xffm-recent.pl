#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-recent class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-recent $version: recently/frequently opened-files plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-recent";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191729";
$abstract = "Xffm-recent/Xffm-frequent $version is a recently/frequently opened-files plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The RECENT plugin keeps tabs of which files have been recently opened with 
the xffm filemanager, and allows you access to these files without having to 
search for them.<br>
The FREQUENT plugin keeps tabs of which files are frequently opened with the 
xffm filemanager, and allows you access to the most frequently used files.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

