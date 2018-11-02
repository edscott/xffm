#ifndef XF_DIALOG
#define XF_DIALOG
GtkWidget *mainWindow;
#include "common/types.h"
#include "signals/dialog.hh"
#include "notebook.hh"
#include "print.hh"
#include <memory>

namespace xf {

template <class Type>
class Dialog :public Notebook<Type> {
    using pixbuf_c = Pixbuf<double>;
    using print_c = Print<double>;
public:
    /*Dialog(void){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
	return;
    }*/
    Dialog(const gchar *title, const gchar *icon){

        Dialog();
	setDialogIcon(icon);
	setDialogTitle(title);
    }
    Dialog(const gchar *path){
	init(path);
    }
    Dialog(void){
	init(NULL);
    }

    void init(const gchar *path){
	dialog_ = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
        mainWindow = GTK_WIDGET(dialog_);
        g_signal_connect (G_OBJECT (dialog_), "delete-event", EVENT_CALLBACK (dialogSignals<Type>::delete_event), NULL);

        gtk_widget_set_has_tooltip (GTK_WIDGET(dialog_), TRUE);
        // FIXME: enable tooltip
        //g_signal_connect (G_OBJECT (dialog_), "query-tooltip", G_CALLBACK (window_tooltip_f), (void *)this);
        g_signal_connect (G_OBJECT (dialog_), "key-press-event", KEY_EVENT_CALLBACK (dialogSignals<Type>::window_keyboard_event), (void *)this);

	gtk_widget_get_preferred_width (GTK_WIDGET(dialog_), &dialogMinW_, &dialogNatW_);
	gtk_widget_get_preferred_height (GTK_WIDGET(dialog_), &dialogMinH_, &dialogNatH_);
	gtk_window_set_type_hint(dialog_, GDK_WINDOW_TYPE_HINT_DIALOG);
	//setWindowMaxSize(dialog_);
	gtk_window_set_position (dialog_, GTK_WIN_POS_MOUSE);
        this->insertNotebook(dialog_);
#ifdef XFFM_CC
        auto dialog = (fmDialog<Type> *) this;
#else
        auto dialog = (Dialog<Type> *) this;
#endif       

        dialog->addPage(path); 
        TRACE("dialog this=%p\n", (void *)this);
        //this->insertPageChild(this->notebook());
        
        
	gint max, current;
        auto vpane = this->vpane();
	g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
	g_object_get(G_OBJECT(vpane), "position", &current, NULL);
	g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(current));
	g_object_set_data(G_OBJECT(vpane), "oldMax", GINT_TO_POINTER(max));
        
	g_signal_connect (G_OBJECT (dialog_), "size-allocate", 
		SIZE_CALLBACK(dialogSignals<Type>::onSizeAllocate), (void *)this);
        setDefaultSize();
        setDefaultFixedFontSize();
        gtk_window_present (dialog_);
        while (gtk_events_pending()) gtk_main_iteration();
	return;
    }

    void setDialogTitle(const gchar *title){
	gtk_window_set_title (dialog_, title);
    }
    void setDialogIcon(const gchar *icon){
	auto pixbuf = pixbuf_c::get_pixbuf(icon, SIZE_ICON);
	gtk_window_set_icon (dialog_, pixbuf);
	g_object_unref(pixbuf);
    }

    void resizeWindow(gint fontSize){
        if (fontSize == 0){
            ERROR("fontSize cannot be zero\n");
            return;
        }
        if (naturalSize_.width == 0 ||
                naturalSize_.height == 0){
            gtk_widget_get_preferred_size (GTK_WIDGET(dialog_),
                               &minimumSize_,
                               &naturalSize_);
        }
        // First try a saved width/height
        gint width = getSettingInteger("window", "width");
        gint height = getSettingInteger("window", "height");
        if (width >= naturalSize_.width && height >= naturalSize_.height){
            gtk_window_resize (GTK_WINDOW(dialog_), width, height);
            return;
        }
        // Now adapt a window size to the selected font
        gint Dw = 8*maximumSize_.width/9 - naturalSize_.width ;
        if (Dw < 0) Dw = 0;
        gint Dh = 12*maximumSize_.height/13 - naturalSize_.height;
        if (Dh < 0) Dh = 0;
        double fraction = (double)(fontSize - 6)/(24 - 6);
        gint w = (fraction * Dw) + naturalSize_.width;
        gint h = (fraction * Dh) + naturalSize_.height;
        WARN("resize window %d: %lf --> %d,%d (min: %d, %d)\n",
               fontSize, fraction, w, h,
               naturalSize_.width,
               naturalSize_.height);
        gtk_window_resize (GTK_WINDOW(dialog_), w, h);
        
    } 

    void
    saveSettings(void){
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        if (!loaded) {
            gchar *text = g_strdup_printf(_("Creating a new file (%s)"), file);
            DBG("%s", text);
            g_free(text);
        }
        GtkAllocation allocation;
        gtk_widget_get_allocation(GTK_WIDGET(dialog_), &allocation);
        g_key_file_set_integer (key_file, "window", "width", allocation.width);
        g_key_file_set_integer (key_file, "window", "height", allocation.height);
        
        auto page = this->currentPageObject();
        g_key_file_set_integer (key_file, "xfterm", "fontSize", page->fontSize());
        

        write_keyfile(key_file, file);
        g_free(file);
        g_key_file_free(key_file);
   }

    static void
    saveSettings(const gchar *group, const gchar *item, gint value){
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        if (!loaded) {
            gchar *text = g_strdup_printf(_("Creating a new file (%s)"), file);
            DBG("%s", text);
            g_free(text);
        }
        g_key_file_set_integer (key_file, group, item, value);
        write_keyfile(key_file, file);
        g_free(file);
        g_key_file_free(key_file);
   }
    
   static void
   setSettingString(const gchar *group, const gchar *item, const gchar *value){
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        g_free(file);
        g_key_file_set_string (key_file, group, item, value);
        write_keyfile(key_file, file);
        g_free(file);
        g_key_file_free(key_file);
   }
    
   static gchar *
   getSettingString(const gchar *group, const gchar *item){
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        g_free(file);
        gchar *value=NULL;
        if (loaded) {
            GError *error = NULL;
            value = g_key_file_get_string (key_file, group, item, &error);
            if (error){
                TRACE("%s\n", error->message);
                g_error_free(error);
                value = NULL;
            }
        } 
        g_key_file_free(key_file);
        return value;
   }
   static gint 
   getSettingInteger(const gchar *group, const gchar *item){
        GKeyFile *key_file = g_key_file_new();
        gchar *file = g_build_filename(g_get_user_config_dir(),"xffm+","settings.ini", NULL);
        gboolean loaded = g_key_file_load_from_file(key_file, file,
               (GKeyFileFlags) (G_KEY_FILE_KEEP_COMMENTS |  G_KEY_FILE_KEEP_TRANSLATIONS),
                NULL);
        g_free(file);
        gint value=-1;
        if (loaded) {
            GError *error = NULL;
            value = g_key_file_get_integer (key_file, group, item, &error);
            if (error){
                TRACE("%s\n", error->message);
                g_error_free(error);
                value = -1;
            }
        } 
        g_key_file_free(key_file);
        return value;
   }
protected:
    GtkWindow *dialog(){
	return dialog_;
    }


    void setDefaultFixedFontSize(void){
        auto page = this->currentPageObject();
	gint size = page->fontSize();
//        page->setSizeScale(size);
        print_c::set_font_size(GTK_WIDGET(page->output()), size);
        print_c::set_font_size(GTK_WIDGET(page->input()), size);
        resizeWindow(size);
    }

private:
        GtkRequisition minimumSize_;
        GtkRequisition naturalSize_;
        GtkRequisition chosenSize_;
        GtkRequisition maximumSize_;
    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(dialog_),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        WARN("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
    }

    void setDefaultSize(gint w, gint h){
        chosenSize_.width = w;
        chosenSize_.height = h;
    }
    void setWindowMaxSize(void){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	auto drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	XGetGeometry(display, drawable, &root_return,
		&x_return, &y_return, 
		&w_return, &h_return, 
		&border_return, 
		&d_return);
	GdkGeometry geometry;
	geometry.max_width = w_return - 25;
	geometry.max_height = h_return -25;
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
	gtk_window_set_geometry_hints (GTK_WINDOW(dialog_), GTK_WIDGET(dialog_), &geometry, GDK_HINT_MAX_SIZE);
    }

    static void
    write_keyfile(GKeyFile *key_file, const gchar *file){
        TRACE( "group_options_write_keyfile: %s\n", file);
        // Write out key_file:
        gsize file_length;
        gchar *file_string = g_key_file_to_data (key_file, &file_length, NULL);
        gchar *config_directory = g_path_get_dirname(file);
        if (!g_file_test(config_directory, G_FILE_TEST_IS_DIR)){
            TRACE( "creating directory %s\n", config_directory);
            g_mkdir_with_parents(config_directory, 0700);
        }
        g_free(config_directory);
        gint fd = creat(file, O_WRONLY | S_IRWXU);
        if (fd >= 0){
            if (write(fd, file_string, file_length) < 0){
                ERROR("write_keyfile(): cannot write to %s: %s\n", file, strerror(errno));
            }
            close(fd);
        } else {
            ERROR("write_keyfile(): cannot open %s for write: %s\n", file, strerror(errno));
        }
    }

private:
    GtkWindow *dialog_;
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
};
}

#endif
