#ifndef XF_LOCALVIEW__HH
# define XF_LOCALVIEW__HH

#include "fm/model/localmodel.hh"

#include "rm.hh"
#include "icons.hh"
#include "popup.hh"
#include "monitor.hh"


namespace xf
{
template <class Type> class BaseView;
template <class Type> class LocalMonitor;
template <class Type>
class LocalView: 
    public LocalModel<Type>,
    public LocalPopUp<Type>
{
    
public:

    // This mkTreeModel should be static...
    static LocalMonitor<Type> *
    loadModel (BaseView<Type> *baseView, const gchar *path)
    {
	baseView->enableDnD();	
        LocalMonitor<Type> *p = NULL;
        auto iconView = baseView->iconView();
        if (!g_file_test(path, G_FILE_TEST_EXISTS)){
            ERROR("loadModel. %s does not exist\n", path);
            return NULL;
        }
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            gchar *dirname = g_path_get_dirname(path);
            p = loadModel(baseView, dirname);
            g_free(dirname);
            return p;
        }

        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_MULTIPLE);      
        gint items = 
	    LocalModel<Type>::loadModel(baseView->treeModel(),path);
	
        auto fileCount = g_strdup_printf("%0d", items);
        // We do not count "../"
        auto text = g_strdup_printf(_("Files: %s"), fileCount); 
        g_free(fileCount);
        baseView->page()->updateStatusLabel(text);
        g_free(text);
        TRACE("FIXME: Set filecount %d message in status button...\n", items);

        // monitor for less than 500 items...
        if (items <= 500) {
            p = new(LocalMonitor<Type>)(baseView->treeModel(), baseView);
            p->start_monitor(baseView->treeModel(), path);
            localMonitorList = g_list_append(localMonitorList, (void *)p->monitor());
        } 
	return p;
    }


    static void selectables(GtkIconView *iconview){
        GtkTreePath *tpath = gtk_tree_path_new_first ();
	GtkTreeModel *treeModel = gtk_icon_view_get_model (iconview);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) {
            gtk_tree_path_free(tpath);
            return ;
        }
        gboolean retval = LocalModel<Type>::isSelectable(treeModel, &iter);
        if (!retval) {
            gtk_icon_view_unselect_path (iconview,tpath);
        }
        gtk_tree_path_free(tpath);
        return ;
    }


    static void
    runWith(BaseView<Type> *baseView, const GtkTreePath *tpath, const gchar *path){
        DBG("%s is executable file\n", path);
	if (!localItemPopUp) LocalPopUp<Type>::createLocalItemPopUp();

        g_object_set_data(G_OBJECT(localItemPopUp), "baseView", baseView);
        auto lastPath =  g_object_get_data(G_OBJECT(localItemPopUp), "path");
        g_free(lastPath);
        g_object_set_data(G_OBJECT(localItemPopUp), "path", g_strdup(path));

	// get corresponding menuitem
	auto menuItem = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Run Executable..."));
	// openwith dialog.
	LocalPopUp<Type>::runWith(menuItem, localItemPopUp);
    }

    static void
    openWith(BaseView<Type> *baseView, const GtkTreePath *tpath, const gchar *path){
	    TRACE("%s is regular file\n", path);
	    // setup for dialog
	    // if popup menu is not created, then create
	    if (!localItemPopUp) LocalPopUp<Type>::createLocalItemPopUp();

            g_object_set_data(G_OBJECT(localItemPopUp), "baseView", baseView);
            auto lastPath =  g_object_get_data(G_OBJECT(localItemPopUp), "path");
            g_free(lastPath);
            g_object_set_data(G_OBJECT(localItemPopUp), "path", g_strdup(path));

	    // get corresponding menuitem
	    auto menuItem = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Open with"));
	    // openwith dialog.
	    LocalPopUp<Type>::openWith(menuItem, localItemPopUp);
	    // This would open command directly (deprecated mode)
	    // LocalPopUp<Type>::command(menuItem, localItemPopUp);
    }

    static gboolean
    item_activated (BaseView<Type> *baseView, 
	    GtkTreeModel *treeModel, const GtkTreePath *tpath,
	    const gchar *path)
    {
	// regular file test (stat)
	struct stat st;
	stat(path, &st);
	// FIXME: if executable, then dialog to open with null (run) With entry for arguments
	if ((st.st_mode & S_IFMT) == S_IFREG){
	    if (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE)) {
		runWith(baseView, tpath, path);
	    } else {
#if 0
                // XXX This has a problem if the user clicks repeatedly on the
                //     icon, since the command will be issued several times
                //     in a row. 
		auto mimetype = Mime<Type>::mimeType(path);
		gchar *response = Settings<Type>::getSettingString("MimeTypeApplications", mimetype);
		if (response) {
		    gchar *command;
		    // Check whether applicacion is valid.
		    gboolean valid = Mime<Type>::isValidCommand(response);
		    if (!valid){
			gchar *message = g_strdup_printf("\n<span color=\"#990000\"><b>%s</b></span>:\n <b>%s</b>\n", _("Invalid entry"), response); 
			Gtk<Type>::quick_help (GTK_WINDOW(mainWindow), message);
			g_free(message);
			return FALSE;
		    }		 
		    // Is the terminal flag set?
		    if (Mime<Type>::runInTerminal(response)){
			command = Mime<Type>::mkTerminalLine(response, path);
		    } else {
			command = Mime<Type>::mkCommandLine(response, path);
		    }
		    auto page = baseView->page();
		    page->command(command);
		    g_free(response);
		    g_free(command);
		    return FALSE;
		}
#endif
		openWith(baseView, tpath, path);
	    }
	} else{
	    DBG("%s NOT a regular file\n", path);
	}
	
	TRACE("LocalView::item activated: %s\n", path);
	return FALSE;
    }

private:




public:

};
}
#endif

