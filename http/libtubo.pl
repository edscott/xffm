#!/usr/bin/perl
#
#
sub document;
use webcommon;

$screenshots="";
$version = "4.5.0";
$title = "Libtubo Interprocess Communication $version";
$svnpath = "/CURRENT/libraries/libtubo";
$documentation="<li><a href=\"docs/libtubo/\">Documentation</a></li>";
$download = "http://sourceforge.net/project/showfiles.php?group_id=70875&package_id=162718";
$abstract = "The Libtubo library $version is small and simple function set to enable a process to run any other process in the background and communicate via the stdout, stderr and stdin file descriptors.";
$description ="<br>
<b>Description</b><br><blockquote>
The functionality of libtubo is similar to the glib function  <font color=#666666>g_spawn_async_with_pipes()</font> except that all pipe setup and monitoring is taken care of. The calling function only has to provide the functions with which to process the input/output of the remote process.<br></blockquote>
<b>Development</b><br><blockquote>
The library first appeared in the year 2000 in the xfce3 desktop as part of the <a href=http://xfsamba.sf.net>xfsamba</a> application, an application to provide a graphical user interface for the smbclient and nmblookup programs of the <a href=http://www.samba.org>Samba Suite</a>. Later on, libtubo was integrated into the xfce3 find tool, <b>xfglob</b> in conjuction with the command line program <b>fgr</b>.<br>
During development of xfce4, as the functionality of xfglob and xfsamba was merged with xftree to produce the xffm filemanager, <b>libtubo</b> became the means by which the filemanager handles almost all user requests, such as cp/mv/ln/mount and others. <br>
And now, as the xffm filemanager becomes a set of individual Opensource Codeset Packages, as distinct functionalities are separated into independent packages. <b>Libtubo</b> can be installed and used without the need for installing any other of the Xffm Opensource Codeset applications. API Reference Manual can be examined <a href=$documentation>here</a>. </blockquote>
";
$news= <<END;
END
&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);

&document;

