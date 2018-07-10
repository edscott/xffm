## Process this file with automake to produce Makefile.in
CLEANFILES = *~ *.bak core *.core *.o *.lo *.la fgr

bin_PROGRAMS = fgr

fgr_CFLAGS = 	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" 


fgr_SOURCES =  	\
	fgr.c  \
	fgr-globber.c \
	fgr-globber.h	\
	fgr-intl.h

fgr_LDADD = $(LDADD) $(INTLLIBS)

