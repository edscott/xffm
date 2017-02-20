# Edscott Wilson Garcia
AM_CPPFLAGS = -std=c++11 \
	@LIBXML_CFLAGS@	\
	@DBH_CFLAGS@	\
	@TUBO_CFLAGS@	\
	@GTK_CFLAGS@	\
	-DXFFM_MODULE_DIR=\"@XFFM_MODULE_DIR@\"	\
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_ICON_DIR=\""$(icondir)"\"

LDADD = @TUBO_LIBS@ @DBH_LIBS@ @GMODULE_LIBS@ @GTK_LIBS@ @LIBXML_LIBS@ @LIBZIP_LIBS@ -lpthread

FLAGS = -ggdb -O0 
#FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
#LIBS = `pkg-config --libs gtk+-3.0` -lpthread

bin_PROGRAMS = xffm

xffm_SOURCES = \
	  xffm_c.cpp \
	    data_c.cpp \
	    window_c.cpp \
	    view_c.cpp  \
		thread_control_c.cpp   \
		lpterm_c.cpp   \
		    run_c.cpp   \
                        run_button_c.cpp \
                        run_output_c.cpp   \
                            csh_completion_c.cpp   \
                                bash_completion_c.cpp  \
                                    print_c.cpp   \
		widgets_c.cpp   \
		    pathbar_c.cpp   \
	  utility_c.cpp \
	  xfdir_c.cpp \
	    xfdir_root_c.cpp \
	    xfdir_local_c.cpp \
	  gtk_c.cpp \
	    signal_action_c.cpp \
	    tooltip_c.cpp \
	    pixbuf_c.cpp \
		pixbuf_icons_c.cpp \
		    pixbuf_hash_c.cpp \
		    pixbuf_cairo_c.cpp \
	    mime_c.cpp \
		mime_magic_c.cpp \
		lite_c.cpp \
	  main.cpp

xffm_CPPFLAGS = $(FLAGS) $(AM_CPPFLAGS)
xffm_LDADD = $(LDADD) 

########################################################################

clean-local:
	rm *.o 

