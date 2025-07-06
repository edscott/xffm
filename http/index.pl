#!/usr/bin/perl
#
#
sub document;
use webcommon;

$version="4.5.0";
$svnpath = "/CURRENT";
$screenshots="<li><a href=screenshots/ class=mbold>Screenshots</a></li>";
$title = "Xffm Code Set";
$documentation="<li><a href=\"docs/\">Documentation</a></li>";

$download = "http://sourceforge.net/project/showfiles.php?group_id=70875";
$abstract= "Xffm applications consist of is a graphical means for harnessing the power of command line Un*x and more command line tools. Filemanagement, network access, find tools, and more. The Xffm applications provide more useful commands and a graphical environment to othert useful un*x commands. 
<br>

";
$description="<i>Current version: $version</i><br>
Previously Xffm was only a filemanager. Now Xffm is not a filemanager but a whole set of applications and libraries, (among which is the totally rebuild xffm-filemanager, which is still not your usual run-of-the-mill filemanager). Most applications operate on the wind*ws philosophy, reimplementing commands inherited from the command line. But with time tested and powerful un*x command line options, this is a waste of time and only a source of unnecessary bugs. Thus, the Xffm applications avoid reinventing the wheel and use  interprocess communication with command line applications. Xfsamba is just one of the plugins available with which you can navigate the SMB network. Any one of the programs of the Xffm codeset can be compiled and installed independently (after meeting certain dependency requirements).
<p>
<b>To install the latest release</b>, download <a href=\"http://svn.foo-projects.org/svn/xffm/CURRENT/install.pm\">install.pm</a> and <a href=\"http://svn.foo-projects.org/svn/xffm/CURRENT/install-release\">install-release</a> and execute \"perl install-release\" (this requires wget package). The default instalation prefix is /usr/local. To modify the prefix, specify with \"perl install-release --prefix=/whatever\". 
<p>
Please note that you should <b>uninstall</b> previous versions of xffm or install with the <b>same prefix</b>, otherwise the build will break.
<p>
<b>To install a current snapshot</b>, download <a href=\"http://svn.foo-projects.org/svn/xffm/CURRENT/install.pm\">install.pm</a> and <a href=\"http://svn.foo-projects.org/svn/xffm/CURRENT/install-svn\">install-svn</a> and execute \"perl install-svn\" (this requires subversion package). The default instalation prefix is /usr/local. To modify the prefix, specify with \"perl install-svn --prefix=/whatever\". 
Please note the to install the svn version you will also need gtk-doc and other development tools.
<p>
If you're only interested in a <b>particular application</b> of Xffm, like xfdiff or fgr, just download that tarball from <a href=\"$download\">here</a> and build it.
<p>
Dependencies for xffm-4.5.0:
<ul>
<li>glib-2.6.0</li>
<li>gtk+-2.2.0</li>
<li>libxml-2.4.0</li>
<li>gnome-icon-theme-2.8.0 or xfce4-icon-theme-4.3.0.3</li>
</ul>
<br>
";
$news= <<END;
<blockquote>
<b>Work on xffm-4.6 has begun!</b> 
Meet the new 
<a href=thumbnails/>
xffm development team</a>
<p>
Xffm Code Set includes:
<ul>
<li> 
<b> Command Search Tool: </b>
<a href=fgr.html>Fgr</a>
</li>
<li> 
<b> Encryption tool: </b>
<a href=scramble.html>Scramble</a>
</li>
<li> 
<b> Disk based hashtables: </b>
<a href=http://dbh.sf.net>Libdbh</a>
</li>
<li>
<b> Interprocess communication: </b>
<a href=libtubo.html>Libtubo</a>
</li>
<li>
<b> Filemanagement library: </b>
<a href=libxffm.html>Libxffm</a>
</li>
<li>
<b> GUI library: </b>
<a href=xffm-gui.html>Xffm-gui</a>
</li>
<li> 
<b> Graphic differences: </b>
<a href=xfdiff.html>Xfdiff</a>
</li>
<li> 
<b> Filemanager with three different GUI's: </b>
<a href=xffm-filemanager.html>Xffm-filemanager</a>
<ul>
    <li> Desktop GUI </li>
    <li> Spatial GUI </li>
    <li> Treeview GUI </li>
    <li> File Find GUI </li>
</ul>
</li>
<li> 
<b> Icon customisation utility for Libxffm: </b>
<a href=xffm-icons.html>Xffm-icons</a>
</li>
<li> And the Xffm-plugins: </li>
<ul>
<li> <a href=xffm-applications.html>Xffm-applications</a> (access to other desktop programs)</li>
<li> <a href=xffm-book.html>Xffm-book</a> (filemanager bookmarks)</li>
<li> <a href=xffm-fstab.html>Xffm-fstab</a> (Navigation and mounting from fstab configuration)</li>
<li> <a href=xffm-locate.html>Xffm-locate</a> (graphical output for slocate command)</li>
<li> <a href=xffm-proc.html>xffm-proc</a> (graphical \"top\" program)</li>
<li> <a href=xffm-recent.html>Xffm-recent/Xffm-frequent</a> (recently/frequently accessed files)</li>
<li> <a href=xffm-samba.html>Xffm-samba</a> (samba network browser, previously Xfsamba)</li>
<li> <a href=xffm-trash.html>Xffm-trash</a> (filemanager trash management)</li>
</ul>
</ul>
<br>
</blockquote>
<blockquote>
Foo-Meter (13-03-06):<br>
<ul>
<li>Libtubo  (125)</li>
<li>Xfdiff   (150)</li>
<li>Xfbook   (391)</li>
<li>Xffm     (562)</li>
<li>Xfsamba  (666)</li>
<li> Scramble (986)</li>
</ul>
</blockquote>
    

END
&init($title,$abstract,$description,$download,$documentation,$news,$screenshots,$svnpath);

&document;

