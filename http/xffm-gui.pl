#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="";
$version = "4.5.0";
$title = "Xffm-gui $version: Xffm GUI library";
$svnpath = "/CURRENT/libraries/xffm-gui";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191725";
$abstract = "Xffm-gui $version is GUI library for the xffm applications with a desktop view, and icon view and a tree view.";
$description ="<br>
<b>Description</b><br><blockquote>
Xffm-gui is the basic GUI library used by some xffm applications, such as the xffm-filemanager.
<br></blockquote>
<b>Development</b><br><blockquote>
Xffm-gui was developed as a part of the xffm-filemanager and has evolved into a separate package. The first release was version 4.5.0 in May 2006.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

