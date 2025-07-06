#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="";
$version = "4.5.0";
$title = "Libxffm $version: Xffm filemanagement library";
$svnpath = "/CURRENT/libraries/libxffm";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191724";
$abstract = "Libxffm $version is filemanagement library for the xffm applications.";
$description ="<br>
<b>Description</b><br><blockquote>
Libxffm is the basic filemanagement library used by some xffm applications, such as the xffm-filemanager.
<br></blockquote>
<b>Development</b><br><blockquote>
Libxffm was developed as a part of the xffm-filemanager and has evolved into a separate package. The first release was version 4.5.0 in May 2006.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

