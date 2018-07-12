## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects
#INCLUDES =

CLEANFILES = *~ *.bak core *.core *.o *.lo *.la 	
AM_CXXFLAGS = -ggdb -O0 -std=c++11
AM_CPPFLAGS =  -I./ \
		-I$(top_srcdir) \
	        -DLIBDIR=\"@LIBDIR@\" \
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"

LDADD = $(TCMALLOC)  @TUBO_LIBS@ @GTK_LIBS@ -lpthread -lX11
GUI_CFLAGS = @GTK_CFLAGS@						\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" 				\
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"		

bin_PROGRAMS = xffind


xffind_SOURCES =      \
	$(top_srcdir)/debug.h \
	xffind.cpp	
	

xffind_CPPFLAGS = $(AM_CPPFLAGS) 
xffind_LDADD = $(LDADD) 

##################################################################
svnclean: distclean
distclean-local:
	rm -rf *.cache 
	rm -rf .deps 
	rm -rf .libs 

