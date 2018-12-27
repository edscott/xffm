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
        WARN("View::receiveDndData\n");
        if (!selection_data) {
            WARN("!selection_data\n");
            return FALSE;
        }
	const gchar *command;
	const gchar *message;
        const gchar *icon;
        gint mode;
	switch (action){
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

	}

        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);

        gchar **files = g_strsplit(dndData, "\n", -1);
        auto result = Gio<Type>::execute(files, target? target: view->path(), mode);
        if (files) g_strfreev(files);
        return result;
    }

};
}
#endif


