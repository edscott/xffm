#ifndef XF_CHOOSERRESPONSE_HH
# define XF_CHOOSERRESPONSE_HH
#include "entryresponse.hh"
#include "comboresponse.hh"
namespace xf
{

template <class Type> class Fm;
template <class Type>
class ChooserResponse {

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

        auto wd = Fm<Type>::getCurrentDirectory(entry);
        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, wd);


        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), FALSE);
        gint response = gtk_dialog_run(dialog);
        //gtk_widget_set_sensitive(GTK_WIDGET(mainWindow), TRUE);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_entry_set_text (entry, filename);
            TRACE("Got %s\n", filename);
            g_free (filename);
        } else TRACE("response was not GTK_RESPONSE_ACCEPT\n");

        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }

public:
  
    static void
    folderChooser(GtkButton *button, void *data) {
        auto entry = GTK_ENTRY(data);
        chooser(entry, _("Choose directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    }
    static void
    fileChooser(GtkButton *button, void *data) {
        auto entry = GTK_ENTRY(data);
        chooser(entry, _("Choose file"), GTK_FILE_CHOOSER_ACTION_OPEN);
    }
    
    static void
    localfolderChooser(GtkButton *button, void *data) {
        auto entryResponse = (EntryResponse<Type> *)data;
        entryResponse->unsetTimeout();
        chooser(GTK_ENTRY(entryResponse->entry()), _("Choose directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
        entryResponse->resetTimeout();
    }
    static void
    localfileChooser(GtkButton *button, void *data) {
        auto entryResponse = (EntryResponse<Type> *)data;
        entryResponse->unsetTimeout();
        chooser(GTK_ENTRY(entryResponse->entry()), _("Choose file"), GTK_FILE_CHOOSER_ACTION_OPEN);
        entryResponse->resetTimeout();
    }
};

template <class Type>
class EntryChooser: public EntryResponse<Type> {

protected:
    GtkButton *chooserButton_;
public:
    GtkButton *chooserButton(void) {return chooserButton_;}

    EntryChooser(GtkWindow *parent, const gchar *windowTitle, const gchar *icon, gboolean fileSelector=FALSE):
        EntryResponse<Type>(parent, windowTitle, icon)
    {
        chooserButton_ = Gtk<Type>::dialog_button (fileSelector?"document-new-symbolic":"folder-symbolic", NULL);
        gtk_box_pack_start (this->hbox_, GTK_WIDGET(chooserButton_), FALSE, FALSE, 0);
        gtk_widget_show (GTK_WIDGET(this->entry()));
        gtk_widget_show (GTK_WIDGET(chooserButton_));
    }
};

template <class Type>
class EntryFileResponse: public EntryChooser<Type> {
public:
    EntryFileResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryChooser<Type>(parent, windowTitle, icon, TRUE)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::localfileChooser), 
                        (gpointer) this);
    }
};

template <class Type>
class EntryFolderResponse: public EntryChooser<Type> {
public:
    EntryFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryChooser<Type>(parent, windowTitle, icon)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::localfolderChooser), 
                        (gpointer) this);
    }
};

template <class Type>
class ComboChooser: public ComboResponse<Type> {

protected:
    GtkButton *chooserButton_;
public:
    ComboChooser(GtkWindow *parent, const gchar *windowTitle, const gchar *icon, gboolean fileSelector=FALSE):
        ComboResponse<Type>(parent, windowTitle, icon)
    {
        chooserButton_ = Gtk<Type>::dialog_button (fileSelector?"document-new-symbolic":"folder-symbolic", NULL);
        gtk_box_pack_start (this->hbox_, GTK_WIDGET(chooserButton_), FALSE, FALSE, 0);
        gtk_widget_show (GTK_WIDGET(chooserButton_));
    }
};

template <class Type>
class ComboFileResponse: public ComboChooser<Type> {
public:

    ComboFileResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        ComboChooser<Type>(parent, windowTitle, icon, TRUE)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::localfileChooser), 
                        (gpointer) this->comboEntry());
    }
};
template <class Type>
class ComboFolderResponse: public ComboChooser<Type> {
public:

    ComboFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        ComboChooser<Type>(parent, windowTitle, icon, FALSE)
    {
        g_signal_connect (G_OBJECT(this->chooserButton_), 
                        "clicked", BUTTON_CALLBACK (ChooserResponse<Type>::localfolderChooser), 
                        (gpointer) this);
    }
};
}
#endif

