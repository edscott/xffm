#ifndef XF_LOCALDND__HH
# define XF_LOCALDND__HH
namespace xf
{
template <class Type> class ClipBoard;
template <class Type> class View;
template <class Type>
class Dnd {

public:
    static gchar *
    sendDndData(View<Type> *view){
        return ClipBoard<Type>::getSelectionData(view, NULL);
    }
                
    static gboolean
    receiveDndData(
            View<Type> *view,
            const gchar *target, 
            const GtkSelectionData *selection_data, 
            GdkDragAction action)
    {
        TRACE("View::receiveDndData\n");
        if (!selection_data) {
            ERROR("!selection_data\n");
            return FALSE;
        }
	const gchar *command;
	const gchar *message;
        const gchar *icon;
        gint mode;
	switch (action){
	    case GDK_ACTION_DEFAULT: 
	    case GDK_ACTION_MOVE:
		message = _("Moving files");
		command = "mv -b -f";
                mode = MODE_MOVE;
                icon = "edit-copy/NE/edit-cut/2.0/220";
		break;
	    case GDK_ACTION_COPY:
		message = _("Copying files locally");
		command = "cp -R -b -f";
                mode = MODE_COPY;
                icon = "edit-copy";
		break;
	    case GDK_ACTION_LINK:
		message = _("Create Link");
		command = "ln -s -b -f";
                mode = MODE_LINK;
                icon = "edit-copy/NE/emblem-symbolic-link/2.0/220";
		break;
	    case GDK_ACTION_PRIVATE:
	    case GDK_ACTION_ASK:
		ERROR("Not supported GDK_ACTION_PRIVATE || GDK_ACTION_ASK\n");
		return FALSE;

	}

        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);

        gchar **files = g_strsplit(dndData, "\n", -1);
        auto more = (files[1] != NULL && strstr(files[1], "file://"))?
                g_strdup_printf("[+ %s]", _("more")):
                g_strdup("");
        TRACE("%s %s %s ---> %s\n", message, files[0], more, target? target: view->path());

	    
        Print<Type>::print(view->page()->output(), "green", 
                    g_strdup_printf("%s %s %s ---> %s\n", 
                    message, files[0], more, target? target: view->path())
                );

        g_free(more);
        auto result = Gio<Type>::executeURL(files, target? target: view->path(), mode);
        if (files) g_strfreev(files);
        return result;
    }

};
}
#endif


