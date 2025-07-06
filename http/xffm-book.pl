#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-book class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-book $version: Bookmarks plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-book";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191722";
$abstract = "Xffm-book $version is a bookmarks plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The BOOKMARKS plugin allows you to bookmark files and create virtual folder 
without moving anything from the original location on the filesystem. The 
plugin also organizes bookmark files in a bookshelf.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

