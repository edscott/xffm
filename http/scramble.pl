#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="";
$version = "4.5.0";
$title = "Scramble Command Tool $version";
$svnpath = "/CURRENT/command-line-utilities/scramble";
$documentation="<li><a href=\"docs/scramble/\">Documentation</a></li>";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=163452";
$abstract = "The Scramble program is small and simple application to encrypt files.";
$description ="<br>
<b>Description</b><br><blockquote>
Scramble is the command find tool used by the Xffm-filemanager to encrypt and descrypt files upon user request. The application is also used to schred files before deleting. 
<br></blockquote>
<b>Development</b><br><blockquote>
The scramble program does not have any dependencies<p>
If you want a GUI front-end for scrambling and unscrambling, you may use the xffm-filemanager. Otherwise you can use the scramble application from the command line.
</blockquote>
";
$news= <<END;
END
&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);
&document;

