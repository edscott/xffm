#ifndef XF_ENTRYFOLDERRESPONSE_HH
# define XF_ENTRYFOLDERRESPONSE_HH
#include "entryresponse.hh"
namespace xf
{
template <class Type> class Response;
template <class Type> class EntryResponse;
template <class Type>
class EntryFolderResponse: public EntryResponse<Type> {
    using gtk_c = Gtk<Type>;
public:

    EntryFolderResponse(GtkWindow *parent, const gchar *windowTitle, const gchar *icon):
        EntryResponse<Type>(parent, windowTitle, icon)
    {
	auto button = gtk_c::dialog_button ("folder-symbolic", NULL);
	auto vbox = gtk_c::vboxNew (FALSE, 6);
	gtk_box_pack_start (this->hbox_, GTK_WIDGET(button), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET(this->entry()));
	gtk_widget_show (GTK_WIDGET(button));
        g_signal_connect (G_OBJECT(button), 
                        "clicked", BUTTON_CALLBACK (folderChooser), 
                        (gpointer) this->entry());
    }
    
    static void
    folderChooser (GtkButton * button, gpointer data) {
        GtkEntry *entry = GTK_ENTRY(data);
        const gchar *text = _("Choose directory");
        EntryFileResponse<Type>::folderChooser(entry, text);
    }

};
}
#endif

