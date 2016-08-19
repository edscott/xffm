noinst_DATA = autogen.sh 
SUBDIRS = po src
DIST_SUBDIRS = po src  

ACLOCAL_AMFLAGS=-I m4

EXTRA_DIST = $(noinst_DATA) $(specfile) configure.ac \
    xffm+.settings.in ChangeLog 

pkgconfigdir = @PKG_CONF_DIR@

##################################################################

distclean-local:
	rm -rf *.cache 
	rm -rf .deps 
	rm -rf .libs 

	
