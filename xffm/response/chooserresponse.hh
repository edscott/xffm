#ifndef XF_CHOOSERRESPONSE_HH
# define XF_CHOOSERRESPONSE_HH
#include "entryresponse.hh"
#include "comboresponse.hh"
namespace xf
{

template <class Type>
class ChooserResponse {
    using pixbuf_c = Pixbuf<double>;
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;

    static void
    chooser(GtkEntry *entry, const gchar *text, GtkFileChooserAction action) {
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
        auto workdir = (const gchar *)g_object_get_data(G_OBJECT(entry), "workdir");
        if (entryValue && g_file_test(entryValue, G_FILE_TEST_IS_DIR)) {
            current_folder = g_strdup(entryValue);
        } else if (workdir && g_file_test(workdir, G_FILE_TEST_IS_DIR)) {
            current_folder = g_strdup(workdir);
        } else current_folder = g_get_current_dir();
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

public:
  
/*    static void
    folderChooser(GtkEntry *entry, const gchar *text) {
         chooser(entry, text, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    }*/

    static void
    folderChooser(GtkButton *button, void *data) {
         chooser(GTK_ENTRY(data), _("Choose directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    }
    static void
    fileChooser(GtkButton *button, void *data) {
         chooser(GTK_ENTRY(data), _("Choose file"), GTK_FILE_CHOOSER_ACTION_OPEN);
    }
};

template <class Type>
class EntryChooser: public EntryResponse<Type> {

protected:
    GtkButton *chooserButton_;
public:
    EntryChooser(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryResponse<Type>(parent, windowTitle, icon)
    {
	chooserButton_ = Gtk<Type>::dialog_button ("folder-symbolic", NULL);
	gtk_box_pack_start (this->hbox_, GTK_WIDGET(chooserButton_), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(this->entry()));
	gtk_widget_show (GTK_WIDGET(chooserButton_));
    }
};

template <class Type>
class EntryFileResponse: public EntryChooser<Type> {
public:
    EntryFileResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryChooser<Type>(parent, windowTitle, icon)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::fileChooser), 
                        (gpointer) this->entry());
    }
};

template <class Type>
class EntryFolderResponse: public EntryChooser<Type> {
public:
    EntryFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryChooser<Type>(parent, windowTitle, icon)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::folderChooser), 
                        (gpointer) this->entry());
    }
};

template <class Type>
class ComboChooser: public ComboResponse<Type> {

protected:
    GtkButton *chooserButton_;
public:
    ComboChooser(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        ComboResponse<Type>(parent, windowTitle, icon)
    {
	chooserButton_ = Gtk<Type>::dialog_button ("folder-symbolic", NULL);
	gtk_box_pack_start (this->hbox_, GTK_WIDGET(chooserButton_), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(chooserButton_));
    }
};

template <class Type>
class ComboFileResponse: public ComboChooser<Type> {
public:

    ComboFileResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        ComboChooser<Type>(parent, windowTitle, icon)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::fileChooser), 
                        (gpointer) this->comboEntry());
    }
};
template <class Type>
class ComboFolderResponse: public ComboChooser<Type> {
public:

    ComboFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        ComboChooser<Type>(parent, windowTitle, icon)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::folderChooser), 
                        (gpointer) this->entry());
    }
};
}
#endif

