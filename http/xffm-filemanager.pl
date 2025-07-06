#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-filemanager $version: The Xffm-filemanager";
$svnpath = "/CURRENT/graphic-utilities/xffm-filemanager";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191728";
$abstract = "Xffm-filemanager $version is the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The Xffm-filemanager has three different GUI's:
<ul>
<li>Xffm-iconview: a spatial iconview filemanager</li>
<li>Xffm-deskview: a desktop view for ~/Desktop folder (or other)</li>
<li>Xffm-treeview: a treeview filemanager</li>
</ul>
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

