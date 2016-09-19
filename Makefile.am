noinst_DATA = autogen.sh 
SUBDIRS = po src modules share
DIST_SUBDIRS = po src modules share 

ACLOCAL_AMFLAGS=-I m4

EXTRA_DIST = $(noinst_DATA) $(specfile) configure.ac \
    xffm+.settings.in ChangeLog 

pkgconfigdir = @PKG_CONF_DIR@

##################################################################

distclean-local:
	rm -rf `find -name Makefile`
	rm -rf `find -name Makefile.in`
	rm -rf *.cache 
	rm -rf .deps 
	rm -rf .libs 

	
