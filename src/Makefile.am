# Edscott Wilson Garcia
AM_CPPFLAGS =  \
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
AM_CXXFLAGS = -ggdb -O0 -std=c++11
#FLAGS = -ggdb -O0 -std=c++11 `pkg-config --cflags gtk+-3.0`
#LIBS = `pkg-config --libs gtk+-3.0` -lpthread

bin_PROGRAMS = xffm

xffm_SOURCES = \
	  combobox_c.cpp \
	  data_c.cpp \
	  main.cpp \
	  gnu_utils_c.cpp \
	  utility_c.cpp \
          run_button_c.cpp \
	  pathbar_c.cpp   \
	  gtk_c.cpp \
	    tooltip_c.cpp \
	    pixbuf_c.cpp \
		pixbuf_icons_c.cpp \
		    pixbuf_hash_c.cpp \
		    pixbuf_cairo_c.cpp \
	    mime_c.cpp \
		mime_magic_c.cpp \
		lite_c.cpp \
	  xffm_c.cpp \
	  window_c.cpp \
	  view_c.cpp  \
	    widgets_c.cpp   \
	    thread_control_c.cpp   \
	  lpterm_c.cpp   \
	    run_c.cpp   \
              run_output_c.cpp   \
                csh_completion_c.cpp   \
                  bash_completion_c.cpp  \
		    base_completion_c.cpp \
                    print_c.cpp   \
	  xfdir_root_c.cpp \
	    xfdir_c.cpp \
	      menu_c.cpp \
	  xfdir_local_c.cpp \
	    local_monitor_c.cpp \
	    local_cnp_c.cpp \
	    local_dnd_c.cpp

xffm_CPPFLAGS = $(FLAGS) $(AM_CPPFLAGS)
xffm_LDADD = $(LDADD) 

########################################################################

clean-local:
	rm *.o 

