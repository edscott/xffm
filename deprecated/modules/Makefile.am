## Process this file with automake to produce Makefile.in

moduledir = @XFFM_MODULE_DIR@


keybind_DATA = keybind.xsd
keybinddir = $(datadir)/@MODULE_DIR@
EXTRA_DIST= $(keybind_DATA)

CLEANFILES = *~ *.bak core *.core *.o *.lo *.la  
@INTLTOOL_DESKTOP_RULE@

xffmlibs = @DBH_LIBS@ @TUBO_LIBS@ @GMODULE_LIBS@ @GTHREAD_LIBS@ @CAIRO_LIBS@	 @GLIB_LIBS@	 @GTK_LIBS@	 @LIBXML_LIBS@	

common_ldflags = @DBH_LIBS@ @TUBO_LIBS@ @GMODULE_LIBS@ @GTHREAD_LIBS@ @CAIRO_LIBS@	 @GLIB_LIBS@	 @GTK_LIBS@	 @LIBXML_LIBS@	@LIBZIP_LIBS@



module_libs = $(xffmlibs) -avoid-version -module	
AM_CPPFLAGS =  \
	-I../src\
	@DBH_CFLAGS@	\
	@LIBXML_CFLAGS@	\
	@GLIB_CFLAGS@	\
	@CAIRO_CFLAGS@ \
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DLIBDIR=\"$(libdir)\"	\
	-DXFFM_MODULE_DIR=\"@XFFM_MODULE_DIR@\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"


####   modules ... we start with settings #####
module_LTLIBRARIES = 

# settings
module_LTLIBRARIES += libsettings.la
libsettings_la_SOURCES =\
	../src/debug.h \
	settings-module.h\
	settings-module.c\
	settings-module.i

libsettings_la_LDFLAGS = $(module_libs) $(common_ldflags)


# .la files are not required nor wanted for plugins.
install-data-hook:
	rm -f $(DESTDIR)/@XFFM_MODULE_DIR@/*.la
