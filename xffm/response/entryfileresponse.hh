#ifndef XF_ENTRYFILERESPONSE_HH
#define XF_ENTRYFILERESPONSE_HH
namespace xf {

template <class Type>
class EntryFileResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
public:
    static void
    folderChooser(GtkEntry *entry, const gchar *text) {
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
        auto entryValue = gtk_entry_get_text(entry);
        gchar *current_folder;
        if (entryValue && g_file_test(entryValue, G_FILE_TEST_IS_DIR)) current_folder = g_strdup(entryValue);
        else current_folder = g_get_current_dir();
        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, current_folder);
        g_free(current_folder);
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        gint response = gtk_dialog_run(dialog);
        gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_entry_set_text (entry, filename);
            TRACE("Got %s\n", filename);
            g_free (filename);
        }
        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }
};
}

#endif

