
#ifndef CREATEINI_H
#define CREATEINI_H
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "../xffm/types.h"
#include "../xffm/gtk/gtk.hh"

# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
//# define DBG(...)   { (void)0; }
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "*** ERROR> "); fprintf(stderr, __VA_ARGS__);}
# undef WARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}
#include "../xffm/gtk/gtk.hh"

namespace xf {
template <class Type> 
class iniCreator {
    GtkWindow *mainWindow_;
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
    GtkRequisition minimumSize_;
    GtkRequisition naturalSize_;
    GtkRequisition maximumSize_;
    GtkBox *bottombox_;

public:
    ~iniCreator(void){
        gtk_widget_hide(GTK_WIDGET(mainWindow_));
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        _exit(123);
    }

    iniCreator(void)
    {
        mainWindow_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
        g_signal_connect (G_OBJECT (mainWindow_), "delete-event", G_CALLBACK (deleteEvent), this);
        gtk_widget_get_preferred_width (GTK_WIDGET(mainWindow_), &dialogMinW_, &dialogNatW_);
        gtk_widget_get_preferred_height (GTK_WIDGET(mainWindow_), &dialogMinH_, &dialogNatH_);
        gtk_window_set_type_hint(mainWindow_, GDK_WINDOW_TYPE_HINT_DIALOG);
        //setWindowMaxSize(mainWindow_);
        gtk_window_set_position (mainWindow_, GTK_WIN_POS_MOUSE);

        auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        gtk_container_add (GTK_CONTAINER(mainWindow_), GTK_WIDGET(vbox));
        auto headerbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto topbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        auto footerbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto bottombox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        gtk_box_pack_start(vbox, GTK_WIDGET(headerbox), FALSE, FALSE, 3);
        gtk_box_pack_start(vbox, GTK_WIDGET(topbox), FALSE, FALSE, 3);
        gtk_box_pack_start(vbox, GTK_WIDGET(footerbox), FALSE, FALSE, 3);
        gtk_box_pack_start(vbox, GTK_WIDGET(bottombox_), TRUE, FALSE, 3);


        auto label1 = GTK_LABEL(gtk_label_new(""));
        auto label2 = GTK_LABEL(gtk_label_new(""));
        auto markup1 = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s</span>", "custom button");
        auto markup2 = g_strdup_printf("<span size=\"small\" color=\"red\">%s</span>", "options");

        gtk_label_set_markup(label1, markup1);
        gtk_label_set_markup(label2, markup2);
        gtk_box_pack_start(headerbox, GTK_WIDGET(label1), FALSE, FALSE, 3);
        gtk_box_pack_start(footerbox, GTK_WIDGET(label2), FALSE, FALSE, 3);

        auto addButton = addIconButton(footerbox, NULL, "list-add");
        gtk_widget_set_tooltip_text(GTK_WIDGET(addButton), _("Add option"));
        g_signal_connect (G_OBJECT (addButton), "clicked", G_CALLBACK (addOption), this);

        addTopStuff(topbox);
        //addOptionBox(bottombox_);
        // test:
        
        auto exit = gtk_button_new_with_label("Exit");
        gtk_box_pack_end(headerbox, GTK_WIDGET(exit), FALSE, FALSE, 3);
        g_signal_connect (G_OBJECT (exit), "clicked", G_CALLBACK (exitApp), this);

        setDefaultSize();
        gtk_window_present (mainWindow_);
        while (gtk_events_pending()) gtk_main_iteration();

        //run();
        return ;

    }
    GtkWindow *window(void) {return mainWindow_;}

    void run(void){
        gtk_widget_show_all(GTK_WIDGET(mainWindow_));
        gtk_main();
    }

private:
    GtkEntry *iconEntry_;
    GtkEntry *tooltipEntry_;
    GtkButton *execFileButton_;
    GtkButton *workDirButton_;
    GtkCheckButton *inTerminal;

    GtkBox *labelBox(GtkBox *vbox, const gchar *text){
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto label = GTK_LABEL(gtk_label_new(text));
        g_object_set_data(G_OBJECT(hbox), "label", (void *)label);
        gtk_box_pack_start(hbox, GTK_WIDGET(label), FALSE, FALSE, 3);
        gtk_box_pack_start(vbox, GTK_WIDGET(hbox), FALSE, FALSE, 3);
        return hbox;
    }


    GtkButton *getButton(GtkBox *hbox, const gchar *iconId){
        auto button = GTK_BUTTON(gtk_button_new());
	Gtk<Type>::set_bin_contents(GTK_BIN(button), iconId,
                NULL, SIZE_BUTTON);
        return button;
    }

    GtkCheckButton *addCheckButton(GtkBox *vbox, const gchar *text){
        auto hbox = labelBox(vbox, text);
        auto button = GTK_CHECK_BUTTON(gtk_check_button_new());
        gtk_box_pack_start(hbox, GTK_WIDGET(button), FALSE, FALSE, 3);
        return button;
        
    }

    GtkButton *addIconButton(GtkBox *vbox, const gchar *text, const gchar *iconId)
    {
        auto hbox = labelBox(vbox, text);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(hbox), "label"));
        if (text) {
            auto markup = g_strdup_printf("<span color=\"red\">%s</span>", text);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        }
        auto button = getButton(hbox, iconId);
        if (text) gtk_widget_set_tooltip_text(GTK_WIDGET(button), text); 
        g_object_set_data(G_OBJECT(button), "label", (void *)label);
        gtk_box_pack_start(hbox, GTK_WIDGET(button), FALSE, FALSE, 3);
        return button;
        
    }

    GtkEntry *addEntryInput(GtkBox *vbox, const gchar *text){
        auto hbox = labelBox(vbox, text);
        auto entry = GTK_ENTRY(gtk_entry_new());
        gtk_box_pack_start(hbox, GTK_WIDGET(entry), FALSE, FALSE, 3);
        return entry;
        
    }
    void addTopStuff(GtkBox *vbox){
        iconEntry_ = addEntryInput(vbox, _("icon ID"));
        tooltipEntry_ = addEntryInput(vbox, _("tooltip"));
        workDirButton_ = addIconButton(vbox, _("Select work directory"),
                "folder-new");
        execFileButton_ = addIconButton(vbox, _("Select executable"), 
                "text-x-script");
        inTerminal = addCheckButton(vbox, _("Run in terminal") );

        g_signal_connect (G_OBJECT(workDirButton_), 
                        "clicked", 
                        G_CALLBACK (folderChooser), 
                        (gpointer) this);
        g_signal_connect (G_OBJECT(execFileButton_), 
                        "clicked", 
                        G_CALLBACK (fileChooser), 
                        (gpointer) this);
        gtk_widget_show_all(GTK_WIDGET(vbox));
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
	gtk_window_set_geometry_hints (GTK_WINDOW(mainWindow_), 
                GTK_WIDGET(mainWindow_), &geometry, GDK_HINT_MAX_SIZE);
    }

    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow_),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        TRACE("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
        gtk_window_set_default_size(mainWindow_, 800, 600);
    }

    static void exitApp (GtkButton *widget,
               gpointer   data){
        deleteEvent(NULL, NULL, data);
        return;
    }

    static gboolean deleteEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
        auto object = (iniCreator<Type> *)data;
        delete (object);
        return TRUE;
    }
    
    void
    chooser(GtkLabel *label, const gchar *text, GtkFileChooserAction action) {
        // GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkDialog *dialog = GTK_DIALOG(gtk_file_chooser_dialog_new (text,
                                                         GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(label))),
                                                         action,
                                                         _("Cancel"),
                                                         GTK_RESPONSE_CANCEL,
                                                         _("Open"),
                                                         GTK_RESPONSE_ACCEPT,
                                                         NULL));
        gtk_file_chooser_set_action ((GtkFileChooser *) dialog, action);

	auto wd = g_get_current_dir();

        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, wd);

        g_free(wd);

        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow_), FALSE);
        gint response = gtk_dialog_run(dialog);
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow_), TRUE);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_label_set_markup(label, path);
            g_free(g_object_get_data(G_OBJECT(label), "path"));
            g_object_set_data(G_OBJECT(label), "path", path);
            
            TRACE("Got %s\n", path);
        } else TRACE("response was not GTK_RESPONSE_ACCEPT\n");

        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }

    static void
    fileChooser(GtkButton *button, void *data) {
        auto object =(iniCreator<Type> *)data;
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "label"));
        object->chooser(label, _("Choose file"), GTK_FILE_CHOOSER_ACTION_OPEN);

    }

    static void
    folderChooser(GtkButton *button, void *data) {
        auto object =(iniCreator<Type> *)data;
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(button), "label"));
        object->chooser(label, _("Choose directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    }

    static void
    addOption(GtkButton *button, void *data) {
        auto object = (iniCreator<Type> *) data;
        object->addOptionBox(object->bottombox());      
    }
public:

    GtkBox *bottombox(void){return bottombox_;}

    GtkBox *addOptionBox(GtkBox *box){
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        gtk_box_pack_start(box, GTK_WIDGET(hbox), FALSE, FALSE, 3);
        auto entry = addEntryInput(hbox, _("Option"));
        auto radio1 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    NULL, _("text")));
        gtk_box_pack_start(hbox, GTK_WIDGET(radio1), FALSE, FALSE, 3);

        auto radio2 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(radio1), _("file")));
        gtk_box_pack_start(hbox, GTK_WIDGET(radio2), FALSE, FALSE, 3);
        auto radio3 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(radio1), _("folder")));
        gtk_box_pack_start(hbox, GTK_WIDGET(radio3), FALSE, FALSE, 3);
        auto radio4 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(radio1), _("on")));
        gtk_box_pack_start(hbox, GTK_WIDGET(radio4), FALSE, FALSE, 3);
        auto radio5 = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(radio1), _("off")));
        gtk_box_pack_start(hbox, GTK_WIDGET(radio5), FALSE, FALSE, 3);
        gtk_widget_show_all(GTK_WIDGET(hbox));
    }

};
} // end namespace xf

int main(int argc, gchar **argv){
	
    gtk_init (&argc, &argv);
    
    auto creator = new (xf::iniCreator<double>);
    gtk_widget_show_all(GTK_WIDGET(creator->window()));
    gtk_main();

    return 0;

}
#endif

