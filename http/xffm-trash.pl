#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-trash class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-trash $version: wastebasket/trashcan plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-trash";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191727";
$abstract = "Xffm-trash $version is a wastebasket/trashcan plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The TRASH plugin allows you to keep wastebaskets for trash in the 
directories where the trash is generated, allowing for easy recovery. The 
plugin also keeps tabs of all generated wastebaskets and allows management 
from a central location.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

