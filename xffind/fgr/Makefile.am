## Process this file with automake to produce Makefile.in

SUBDIRS = src po man
DIST_SUBDIRS = src po man
ACLOCAL_AMFLAGS=-I m4
CLEANFILES = *~ *.bak core *.core *.o *.lo *.la

EXTRA_DIST=fgr.settings \
        install-sh config.guess config.sub missing compile depcomp ltmain.sh

##################################################################

distclean-local:
	rm -rf *.cache 
	rm -rf .deps 
	rm -rf .libs 
rpm: 
	make dist
	mv $(PACKAGE)-$(VERSION).tar.gz ~/rpmbuild/SOURCES/
	rpmbuild -ba $(PACKAGE).spec

	
