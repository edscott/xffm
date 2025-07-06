#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-icons.html class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-icons $version";
$svnpath = "/CURRENT/graphic-utilities/xffm-icons";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=84284";
$abstract = "Xffm-icons $version is  an icon customisation utility for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
Xffm-icons allows you to change icons associated to mimetypes shown in the xffm-filemanager, regardless of the icon naming scheme of the icon-theme to which the icons belong. You can also create your own icon themes for the filemanager by selecting your own icons.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

