#ifndef XF_COMMANDRESPONSE_HH
#define XF_COMMANDRESPONSE_HH
#include "types.h"
#include "common/gtk.hh"
#include "common/pixbuf.hh"
namespace xf {

template <class Type>
class CommandResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    
    static GtkWindow * 
    dialog(const gchar *message, const gchar *icon, gint pid)
    {
        if (pid == 0) return NULL;
         GtkWindow *dialog = NULL;

         // Create the widgets
         dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
         gtk_window_set_title(dialog, _("Running…"));
         gtk_window_set_transient_for (dialog,GTK_WINDOW(mainWindow));

         auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
         gtk_container_add (GTK_CONTAINER (dialog), GTK_WIDGET(vbox));
         auto label = GTK_LABEL(gtk_label_new (""));
	 auto markup = 
	    g_strdup_printf("   <span color=\"blue\" size=\"larger\"><b>%s</b></span>   ", message);           
         gtk_label_set_markup(label, markup);
         g_free(markup);
         
         // Add the label, and show everything we have added
         if (icon){
            auto pixbuf = Pixbuf<Type>::get_pixbuf(icon, -96);
            if (pixbuf) {
                auto image = gtk_image_new_from_pixbuf(pixbuf);
                if (image) {
                    gtk_box_pack_start(vbox, image, FALSE, FALSE,0);
                    gtk_widget_show (image);
                }
            }
         }
         //auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
         //gtk_container_add (GTK_CONTAINER (vbox), GTK_WIDGET(hbox));
         gtk_box_pack_start(vbox, GTK_WIDGET(label), FALSE, FALSE,0);
         auto progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
         auto text = g_strdup_printf("%s (pid: %d)", 
                 _("Waiting for operation to finish..."),
                 Tubo<Type>::getChild (pid));
         gtk_progress_bar_set_text (progress, text);
         g_free(text);
         gtk_progress_bar_set_show_text (progress, TRUE);
         gtk_box_pack_start(vbox, GTK_WIDGET(progress), FALSE, FALSE,0);
         gtk_progress_bar_pulse(progress);


         gtk_widget_show_all (GTK_WIDGET(dialog));
	 gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
         auto arg = (void **)calloc(3, sizeof (void *));
         arg[0]=(void *)progress;
         arg[1]=(void *)dialog;
         arg[2]=GINT_TO_POINTER(pid);
         g_timeout_add(250, pulse_f, (void *)arg);
	 //gtk_dialog_run(dialog);
         
         //gtk_widget_destroy(GTK_WIDGET(dialog));
         return dialog;
    }

    static gboolean pulse_f(void *data) {
        auto arg = (void **)data;
        auto progress = GTK_PROGRESS_BAR(arg[0]);
        gtk_progress_bar_pulse(progress);
        // pid active?
        gint pid = GPOINTER_TO_INT(arg[2]);
        gchar *c = g_strdup_printf("ps -p %d", pid); 
        gchar *s = g_strdup_printf("%d", pid);
        gboolean alive = FALSE;
        FILE *pipe = popen(c, "r");
        if (pipe) {
            gchar buffer[256];
            while (fgets(buffer, 255, pipe) && !feof(pipe)){
                buffer[255]=0;
                if (strstr(buffer, s)){
                    // alive
                    alive = TRUE;
                    break;
                }
            }
            pclose(pipe);
        }
        g_free(c);
        g_free(s);
        if (alive) return TRUE;
        auto dialog = GTK_WIDGET(arg[1]);
        gtk_widget_hide(GTK_WIDGET(dialog));
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return FALSE;
    }

    /*static void
    fork_finished_function (void *data) {
        g_timeout_add(5, done_f, data);
    }*/
    


};
}

#endif

