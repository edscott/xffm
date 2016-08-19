## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

modules = $(top_srcdir)/modules
moduledir = @RFM_MODULE_DIR@
includedir=$(top_srcdir)/include

dirprimary=$(top_srcdir)/primary
dirmod=$(top_srcdir)/modules

keybind_DATA = $(modules)/keybind.xsd
keybinddir = $(datadir)/@MODULE_DIR@
EXTRA_DIST= $(keybind_DATA)

CLEANFILES = *~ *.bak core *.core *.o *.lo *.la  
@INTLTOOL_DESKTOP_RULE@

rfmlibs = @DBH_LIBS@ @TUBO_LIBS@ @GMODULE_LIBS@ @GTHREAD_LIBS@ @CAIRO_LIBS@	 @GLIB_LIBS@	 @GTK_LIBS@	 @LIBXML_LIBS@	

common_ldflags = @DBH_LIBS@ @TUBO_LIBS@ @GMODULE_LIBS@ @GTHREAD_LIBS@ @CAIRO_LIBS@	 @GLIB_LIBS@	 @GTK_LIBS@	 @LIBXML_LIBS@	@LIBZIP_LIBS@



module_libs = $(rfmlibs) -avoid-version -module	
AM_CPPFLAGS =  \
        -I$(top_srcdir) \
        -I$(includedir) \
	-I../../primary\
	@DBH_CFLAGS@	\
	@LIBXML_CFLAGS@	\
	@GLIB_CFLAGS@	\
	@CAIRO_CFLAGS@ \
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DLIBDIR=\"$(libdir)\"	\
	-DRFM_MODULE_DIR=\"@RFM_MODULE_DIR@\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"


####   modules ... #####
module_LTLIBRARIES = 

# callbacks (popup menu and keybindings)
module_LTLIBRARIES += libcallbacks.la
libcallbacks_la_SOURCES =\
	$(top_srcdir)/debug.h \
	$(includedir)/rfm_modules.h\
	$(top_srcdir)/primary/primary-internal.h\
	../../modules/callbacks.c\
	../../modules/dialogs.i\
	../../modules/touch.i\
	../../modules/ls.i\
	../../modules/rm.i\
	../../modules/cp.i\
	../../modules/rename_entry.i\
	../../modules/callbacks.i

libcallbacks_la_LDFLAGS = $(module_libs) $(common_ldflags)

# run
module_LTLIBRARIES += librun.la
librun_la_SOURCES =\
	$(top_srcdir)/debug.h \
	../../modules/run.c\
	../../modules/run.i

librun_la_LDFLAGS = $(module_libs) $(common_ldflags)

# settings
module_LTLIBRARIES += libsettings.la
libsettings_la_SOURCES =\
	$(top_srcdir)/debug.h \
	../../modules/settings-module.c	$(includedir)/settings-module.h\
	../../modules/settings-module.i

libsettings_la_LDFLAGS = $(module_libs) $(common_ldflags)

# FIXME: (in progress) modify code in libxmltree to make it nonspecific and
#        use it for all xml files. This can then be used in reservoir
#        simulator (fork into separate package... maybe).
module_LTLIBRARIES += libxmltree.la
libxmltree_la_SOURCES =\
	$(top_srcdir)/debug.h \
	../../modules/tagfile.h \
	../../modules/tagfile.i \
	../../modules/tagfile.c \
	../../include/xmltree.h \
	../../modules/xmltree.i \
	../../modules/xsdtree.i \
	../../modules/xmltree.c	

libxmltree_la_LDFLAGS = $(module_libs) $(common_ldflags)


if WANT_BCRYPT_P
# bcrypt
module_LTLIBRARIES += libbcrypt.la
libbcrypt_la_SOURCES =\
	$(top_srcdir)/debug.h \
	../../modules/bcrypt.c

libbcrypt_la_LDFLAGS = $(module_libs) $(common_ldflags)
endif

if WANT_COMPLETION_P
# completion
module_LTLIBRARIES += libcompletion.la
libcompletion_la_SOURCES =\
	$(top_srcdir)/debug.h \
	../../modules/completion.c\
	../../modules/completion.i

libcompletion_la_LDFLAGS = $(module_libs) $(common_ldflags)
endif

if WANT_COMBOBOX_P
# combobox
module_LTLIBRARIES += libcombobox.la
libcombobox_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/combobox-module.c 	$(includedir)/combobox-module.h\
	../../modules/combobox-module.i

libcombobox_la_LDFLAGS = $(module_libs)  $(common_ldflags)
endif

if WANT_PROPERTIES_P
# properties
module_LTLIBRARIES += libproperties.la
libproperties_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/properties-module.c	$(includedir)/properties-module.h\
	../../modules/properties-module.i

libproperties_la_LDFLAGS = $(module_libs) $(common_ldflags)
endif

if WANT_MIME_P
# mimetypes (by extension or file magic)
module_LTLIBRARIES += libmime.la
libmime_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/mime-module.c $(includedir)/mime-module.h\
    	../../modules/mime-mouse_magic.i\
	../../modules/mime-module.i
 
libmime_la_LDFLAGS = $(module_libs)  $(common_ldflags)
endif

if WANT_MMAGIC_P
# mimemagic (previews, depends on mimetypes)
module_LTLIBRARIES += libmimemagic.la
libmimemagic_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/mimemagic.c $(includedir)/mime-module.h
 
libmimemagic_la_LDFLAGS = $(module_libs)  $(common_ldflags)
endif

if WANT_ZMAGIC_P
# mimezip (zip previews, depends on mimetypes and mimemagic)
module_LTLIBRARIES += libmimezip.la
libmimezip_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/mime-zip.c

libmimezip_la_CFLAGS =  @LIBZIP_CFLAGS@ 
libmimezip_la_LDFLAGS = $(module_libs) $(common_ldflags)
endif

if WANT_ICON_P
# iconthemes
module_LTLIBRARIES += libicons.la
libicons_la_SOURCES = \
	$(top_srcdir)/debug.h \
	../../modules/icon-module.c 	$(includedir)/icon-module.h \
	../../modules/icon-module-theme.i	../../modules/icon-module-mime.i \
	../../modules/icon-module-hash.i
 
libicons_la_LDFLAGS = $(module_libs) $(common_ldflags) 
endif

# .la files are not required nor wanted for plugins.
install-data-hook:
	rm -f $(DESTDIR)/@RFM_MODULE_DIR@/*.la
