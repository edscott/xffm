#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-fstab class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-samba $version: Samba network navigator plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-samba";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191719";
$abstract = "Xffm-samba $version is a Samba network navigator plugin for the Xffm-filemanager. Previously known as Xfsamba.";
$description ="<br>
<b>Description</b><br><blockquote>
The SMB network plugin queries the SAMBA network for master browsers and 
allows you see and browse other samba servers, in combination with other SMB 
plugins.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

