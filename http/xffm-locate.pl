#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-locate class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-locate $version: locate plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-locate";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191721";
$abstract = "Xffm-locate $version is a locate plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The SLOCATE plugin allows you to perform \"slocate\" queries and to put the 
results into the xffm GUI for further manipulation and navigation.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

