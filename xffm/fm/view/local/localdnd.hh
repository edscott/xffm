#ifndef XF_LOCALDND__HH
# define XF_LOCALDND__HH
namespace xf
{
template <class Type>
class LocalDnd {

    static GList *
    removeUriFormat(gchar **files) {
        GList *fileList = NULL;
        for (auto f=files; f && *f; f++){
            gchar *file = *f;
            if (strlen(file) > strlen(URIFILE)){
                if (strncmp(file, URIFILE, strlen(URIFILE))==0){
                    file = *f + strlen(URIFILE);
                }
            }
            fileList = g_list_prepend(fileList, g_strdup(file));
        }
        fileList = g_list_reverse(fileList);
        return fileList;
    }

public:
    static gchar *
    sendDndData(BaseView<Type> *baseView){
        return LocalClipBoard<Type>::getSelectionData(baseView, NULL);
    }
                
    static gboolean
    receiveDndData(
            BaseView<Type> *baseView,
            const gchar *target, 
            const GtkSelectionData *selection_data, 
            GdkDragAction action)
    {
        WARN("BaseView::receiveDndData\n");
        if (!selection_data) {
            WARN("!selection_data\n");
            return FALSE;
        }
	const gchar *command;
	const gchar *message;
	switch (action){
	    case GDK_ACTION_MOVE:
		message = _("Moving files");
		command = "mv -b -f";
		break;
	    case GDK_ACTION_COPY:
		message = _("Copying files locally");
		command = "cp -R -b -f";
		break;
	    case GDK_ACTION_LINK:
		message = _("Create Link");
		command = "ln -s -b -f";
		break;

	}

        auto dndData = (const char *)gtk_selection_data_get_data (selection_data);

        gchar **files = g_strsplit(dndData, "\n", -1);
        auto result = execute(message, command, files, target? target: baseView->path());
        if (files) g_strfreev(files);
        return result;
    }

    static gboolean 
    execute(const gchar *message, const gchar *command, gchar **files, const gchar *target){
        if (!files) {
            WARN("!files\n");
            return FALSE;
        }
        if (*files==NULL) {
            WARN("files==NULL\n");
            return FALSE;
        }
            
        gchar *source = g_path_get_dirname(*files);
	if (strncmp(source, URIFILE, strlen(URIFILE))==0){
	    gchar *g = g_strdup(source + strlen(URIFILE));
	    g_free(source);
	    source=g;
	}
        if (!target){
	    ERROR("LocalDnd::execute: target cannot be NULL\n");
            return FALSE;
        }
        WARN("LocalDnd::execute: source=%s target=%s command=%s\n", source, target, command);
        gboolean result = FALSE;
        if (strcmp(source, target) ) result = TRUE;
        else {
	    g_free(source);
            WARN("LocalDnd::execute: source and target are the same\n");
            return FALSE;
        }
	g_free(source);

        GList *fileList = removeUriFormat(files);
	auto dialog = CommandProgressResponse<Type>::dialog(
		message, "system-run", command, fileList, target);

        for (auto l=fileList; l && l->data; l= l->next) g_free(l->data);
        g_list_free(fileList);

        // not needed with GTK_DEST_DEFAULT_DROP
        /*gtk_drag_finish (context, result, 
                (action == GDK_ACTION_MOVE) ? result : FALSE, 
                time);*/


        return result;
    }
    
    


};
}
#endif


