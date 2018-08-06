noinst_DATA = autogen.sh install-sh config.guess config.sub missing compile depcomp ltmain.sh 

SUBDIRS = src 
#SUBDIRS = po man icons
DIST_SUBDIRS = src 
#DIST_SUBDIRS = po man icons

@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = fgr+.appdata.xml

desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
desktop_in_files = fgr+.desktop.in

CLEANFILES = *~ *.bak core *.core *.desktop *.desktop.in	
ACLOCAL_AMFLAGS=-I m4
specfile = @PACKAGE@.spec
#noinst_DATA += build WISHLIST


#dist_data_DATA = autogen.sh build
dirplug=$(top_srcdir)/plugins
dirfuse=$(dirplug)/filesystem_user_space
dirsmb=$(dirplug)/samba

EXTRA_DIST = $(noinst_DATA) \
    xffind+.appdata.xml.in \
    xffind+.appdata.xml.in \
    xffind+.desktop.in.in \
    configure.ac \
    @PACKAGE@.settings.in ChangeLog

##################################################################
distclean-local:
	rm -rf *.cache 
	rm -rf .deps 
	rm -rf .libs 

	
