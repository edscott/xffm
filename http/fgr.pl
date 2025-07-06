#!/usr/bin/perl
#
#
sub init;
sub document;
use webcommon;

$screenshots="";
$version = "4.5.0";
$title = "Fgr Command Find Tool $version";
$documentation="<li><a href=\"docs/fgr/\">Documentation</a></li>";
$svnpath = "/CURRENT/command-line-utilities/fgr";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=162860";
$abstract = "The Fgr program is small and simple application to find files and pipe the result to grep in order to refine the search based on content.";
$description ="<br>
<b>Description</b><br><blockquote>
Fgr is a command find tool which combines the power of \"find\" with the versatility of \"grep\". The Xffm-find GUI uses this simple program for seaching into the contents of files. The ability to use fgr from either a GUI or the command line provides this application with great versatility.
<br></blockquote>
<b>Development</b><br><blockquote>
Fgr first appeared (as glob) in the year 2000 in the xfce3 desktop and is now part of the Xffm distribution. Today fgr can be installed along with Xffm or as a standalone application. This allows for quick and easy installation in any system. The fgr program does not have any dependencies, except for GNU grep 2.x or higher if you want to search into the content of files.<p>
</blockquote>
";
$news= <<END;

END
&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);

&document;

