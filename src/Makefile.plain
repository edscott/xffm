AM_CPPFLAGS = \
	@LIBXML_CFLAGS@	\
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DXFFM_MODULE_DIR=\"@XFFM_MODULE_DIR@\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"

common_ldflags = @TUBO_LIBS@ @GMODULE_LIBS@ @GTK_LIBS@ @LIBXML_LIBS@ @LIBZIP_LIBS@

FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0` -lpthread
OBJECTS = xffm_c.o \
	    window_c.o \
	    view_c.o  \
		widgets_c.o   \
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

all: test
#.SUFFIXES: .cpp .hpp .o
SUFFIXES=.cpp .hpp .o

#GNU make extension:
#%.o: %.cpp %.hpp Makefile.am
.cpp.o: Makefile.am
	c++ -c $(FLAGS) $(CPPFLAGS) -o $@ $< 
test.o: test.cpp Makefile.am
	c++ -c $(FLAGS) $(CPPFLAGS) -o test.o test.cpp
	
test: $(OBJECTS) Makefile.am
	c++ $(LIBS) $(OBJECTS) -o test
	
clean-local:
	rm *.o test

