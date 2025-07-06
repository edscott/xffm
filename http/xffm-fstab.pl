#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="<li><a href=screenshots/xffm-fstab class=mbold>Screenshots</a></li>";
$version = "4.5.0";
$title = "Xffm-fstab $version: Mount/fstab plugin for the Xffm-filemanager";
$svnpath = "/CURRENT/filemanager-plugins/xffm-fstab";
$documentation="";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=191720";
$abstract = "Xffm-fstab $version is a mount/fstab plugin for the Xffm-filemanager.";
$description ="<br>
<b>Description</b><br><blockquote>
The FSTAB plugin keeps track of the mount points listed in /etc/fstab and 
allows you to easily mount or unmount these volumes with the popup menu, 
while navigating through the filesystem.
</blockquote>
";
$news= <<END;
END

&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

