## Process this file with automake to produce Makefile.in
CLEANFILES = *~ *.bak core *.core *.o *.lo *.la fgr

AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = fgr

fgr_CFLAGS = 	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" 


fgr_SOURCES =  	\
	../src/fgr.c  \
	../src/fgr-globber.c \
	../src/fgr-globber.h	\
	../src/fgr-intl.h

fgr_LDADD = $(LDADD) $(INTLLIBS)

