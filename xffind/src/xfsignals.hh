#ifndef SIGNALS__HH
# define SIGNALS__HH

#include "types.h"
#include "xfprint.hh"
#include "xfrun.hh"
#include "xfutil.hh"

namespace xf
{


template <class Type>
class Signals: public Run<Type>{
    using gtk_c = Gtk<double>;
    using print_c = Print<double>;
    using run_c = Run<double>;
    using util_c = Util<double>;
public:
    static void 
    sensitivize (GtkToggleButton *togglebutton, gpointer data){
	GtkWidget *widget = GTK_WIDGET(data);
	gtk_widget_set_sensitive(widget, gtk_toggle_button_get_active(togglebutton));
    }

    static void 
    sensitivize_radio (GtkToggleButton *togglebutton, gpointer data){
	if (!data) return;
	radio_t *radio_p = (radio_t *)data;
	gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), FALSE);
	GtkToggleButton **tb_p = radio_p->toggle;
	for (; tb_p && *tb_p; tb_p++){
	    if (gtk_toggle_button_get_active(*tb_p)){
		gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), TRUE);
	    }
	}
    }

    static void
    command_up (GtkWidget * button, gpointer data) {
        quick_command(button, data, TRUE);
    }

    static void
    command_down (GtkWidget * button, gpointer data) {
        quick_command(button, data, FALSE);
    }

    static void
    on_buttonHelp (GtkWidget * button, gpointer data) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	const gchar *message = (const gchar *)data;
	std::cerr<<"fixme: signals::on_buttonHelp\n";
	gtk_c::quick_help(dialog_, message);
    }


    static void
    onClearButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
	std::cerr<<"fixme: signals::onClearButton\n";
        print_c::clear_text(diagnostics);
        print_c::hide_text(diagnostics);

    }

    static void
    onEditButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
	std::cerr<<"fixme: signals::onEditButton\n";
        print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        print_c::show_text(diagnostics);        
        run_c::thread_run(diagnostics, "ls -l", FALSE);
    }

    static void
    onCloseButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCloseButton\n";
        GtkWidget *dialog = GTK_WIDGET(data);
        gtk_widget_hide(dialog);
        while (gtk_events_pending()) gtk_main_iteration();
        exit(1);
    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCancelButton\n";
    }

    static void
    onFindButton (GtkWidget * button, gpointer dialog) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog), "diagnostics"));
        updateCompletions(dialog);
        
        print_c::show_text(diagnostics);
	std::cerr<<"fixme: signals::onFindButton\n";
        print_c::print(diagnostics, g_strdup("1.fixme: signals::\n"));
        print_c::print(diagnostics, "tag/green", g_strdup("2.fixme: signals::\n"));
        print_c::print_debug(diagnostics, g_strdup("3.fixme: signals::\n"));
        print_c::print_error(diagnostics, g_strdup("4.fixme: signals::\n"));
        print_c::print_icon(diagnostics, "edit-find",  g_strdup("5.fixme: signals::\n"));
        print_c::print_icon_tag(diagnostics, "edit-find",  "tag/red",g_strdup("fixme: signals::\n"));
    }

    static gint
    on_key_release (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));
        gboolean active = FALSE;
        if (text && strlen(text)) active = TRUE;
        //std::cerr<<"on_key_release: "<< text << " active: " << active << "\n";
        if (data){
            gtk_widget_set_sensitive(GTK_WIDGET(data), active);
        }
        return FALSE;
    }
    static gint
    on_completion (GtkWidget * widget, GdkEventKey * event, gpointer data) {
        GtkEntryCompletion *completion = gtk_entry_get_completion(GTK_ENTRY(widget));
        gtk_entry_completion_complete (completion);
        return FALSE;
    }
    static void
    folderChooser (GtkButton * button, gpointer data) {
        GtkEntry *entry = GTK_ENTRY(data);
        const gchar *text = _("Select folder to search in");
        //const gchar *text = _("Select Files...");
         GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        // GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkDialog *dialog = GTK_DIALOG(gtk_file_chooser_dialog_new (text,
                                                         GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(entry))),
                                                         action,
                                                         _("Cancel"),
                                                         GTK_RESPONSE_CANCEL,
                                                         _("Open"),
                                                         GTK_RESPONSE_ACCEPT,
                                                         NULL));
        gtk_file_chooser_set_action ((GtkFileChooser *) dialog, action);
        gchar *current_folder = g_get_current_dir();
        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, current_folder);

        gint response = gtk_dialog_run(dialog);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_entry_set_text (entry, filename);
            NOOP ("Got %s\n", filename);
            g_free (filename);
        }
        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }


private:
    
    static void
    quick_command (GtkWidget * button, gpointer data, gboolean scrollUp) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(dialog_), "diagnostics"));
	std::cerr<<"fixme: signals::quick_command\n";
        print_c::clear_text(diagnostics);
        //run_c::thread_run(diagnostics, (const gchar *)data);
        run_c::thread_run(diagnostics, (const gchar *)data, scrollUp);
    }

    static void 
    updateCompletions(gpointer dialog){ 
        gchar *history;
        GtkEntry *entry;
        GtkTreeModel *model;

        history = g_build_filename (FILTER_HISTORY);
        entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "filter_entry"));
        model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(entry), "model"));
        util_c::saveHistory(history, model, gtk_entry_get_text(entry));
        g_free(history);

        history = g_build_filename (GREP_HISTORY);
        entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "grep_entry"));
        model = GTK_TREE_MODEL(g_object_get_data(G_OBJECT(entry), "model"));
        util_c::saveHistory(history, model, gtk_entry_get_text(entry));
        g_free(history);
    }


};

}
#endif
