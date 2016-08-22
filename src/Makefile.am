AM_CPPFLAGS = \
	@LIBXML_CFLAGS@	\
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DXFFM_MODULE_DIR=\"@XFFM_MODULE_DIR@\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"

LDADD = @TUBO_LIBS@ @GMODULE_LIBS@ @GTK_LIBS@ @LIBXML_LIBS@ @LIBZIP_LIBS@ -lpthread

FLAGS = -ggdb -O0 
#FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
#LIBS = `pkg-config --libs gtk+-3.0` -lpthread
OBJECTS = xffm_c.o \
	    window_c.o \
	    view_c.o  \
		widgets_c.o   \
		    pathbar_c.o   \
	  signals_c.o \
	  utility_c.o \
	  xfdir_c.o \
	  gtk_c.o \
	    tooltip_c.o \
	    pixbuf_c.o \
		pixbuf_hash_c.o \
		    pixbuf_icons_c.o \
			pixbuf_cairo_c.o \
	  test.o

bin_PROGRAMS = xffm

xffm_SOURCES = \
	  xffm_c.cpp \
	    window_c.cpp \
	    view_c.cpp  \
		widgets_c.cpp   \
		    pathbar_c.cpp   \
	  signals_c.cpp \
	  utility_c.cpp \
	  xfdir_c.cpp \
	  gtk_c.cpp \
	    tooltip_c.cpp \
	    pixbuf_c.cpp \
		pixbuf_hash_c.cpp \
		    pixbuf_icons_c.cpp \
			pixbuf_cairo_c.cpp \
	  main.cpp

xffm_CPPFLAGS = -std=c++11 $(FLAGS) $(AM_CPPFLAGS)
xffm_LDADD = $(LDADD) 

########################################################################

clean-local:
	rm *.o 

