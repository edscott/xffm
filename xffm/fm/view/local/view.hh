#ifndef XF_LOCALVIEW__HH
# define XF_LOCALVIEW__HH


#include "model.hh"
#include "icons.hh"
#include "popup.hh"
#include "monitor.hh"


namespace xf
{
template <class Type> class View;
template <class Type> class LocalMonitor;
template <class Type>
class LocalView: 
    public LocalModel<Type>,
    public LocalPopUp<Type>
{
    
public:

    // This mkTreeModel should be static...
    static LocalMonitor<Type> *
    loadModel (View<Type> *view, const gchar *path)
    {
        TRACE("*** local/view-hh: loadModel()\n");
	view->enableDnD();	
        LocalMonitor<Type> *p = NULL;
        auto iconView = view->iconView();
        if (!g_file_test(path, G_FILE_TEST_EXISTS)){
            ERROR("local/view.hh::loadModel. %s does not exist\n", path);
            return NULL;
        }
        if (!g_file_test(path, G_FILE_TEST_IS_DIR)){
            ERROR("local/view.hh::loadModel. %s is not a directory\n", path);
            return NULL;
        }

        gtk_icon_view_set_selection_mode (iconView,GTK_SELECTION_MULTIPLE);      
        gint items = 
	    LocalModel<Type>::loadModel(view,path);

        // monitor for less than 500 items...
        //if (items <= 500) 
        {
            p = new(LocalMonitor<Type>)(view->treeModel(), view);
            p->start_monitor(view, path);
            view->setMonitorObject(p);
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

    static void selectables(GtkTreeView *treeView){
        
        GtkTreePath *tpath = gtk_tree_path_new_first ();
	GtkTreeModel *treeModel = gtk_tree_view_get_model (treeView);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (treeModel, &iter, tpath)) {
            gtk_tree_path_free(tpath);
            return ;
        }
        gboolean retval = LocalModel<Type>::isSelectable(treeModel, &iter);
        if (!retval) {
	    auto selection = gtk_tree_view_get_selection (treeView);
	    gtk_tree_selection_unselect_path (selection, tpath);
        }
        gtk_tree_path_free(tpath);
        return ;
    }


    static void
    runWith(View<Type> *view, const GtkTreePath *tpath, const gchar *path){
        TRACE("%s is executable file\n", path);
	if (!localItemPopUp) LocalPopUp<Type>::createLocalItemPopUp();

        g_object_set_data(G_OBJECT(localItemPopUp), "view", view);
        auto lastPath =  g_object_get_data(G_OBJECT(localItemPopUp), "path");
        g_free(lastPath);
        g_object_set_data(G_OBJECT(localItemPopUp), "path", g_strdup(path));

	// get corresponding menuitem
	auto menuItem = GTK_MENU_ITEM(g_object_get_data(G_OBJECT(localItemPopUp), "Run Executable..."));
	// openwith dialog.
	LocalPopUp<Type>::runWith(menuItem, localItemPopUp);
    }

    static void
    openWith(View<Type> *view, const GtkTreePath *tpath, const gchar *path){
	    TRACE("%s is regular file\n", path);
	    // setup for dialog
	    // if popup menu is not created, then create
	    if (!localItemPopUp) LocalPopUp<Type>::createLocalItemPopUp();

            g_object_set_data(G_OBJECT(localItemPopUp), "view", view);
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
    item_activated (View<Type> *view, 
	    GtkTreeModel *treeModel, const GtkTreePath *tpath,
	    const gchar *path)
    {
	// regular file test (stat)
	struct stat st;
	errno=0;
	if (stat(path, &st)<0){
	    TRACE("local/view.hh::item_activated(): stat %s (%s)\n",
		path, strerror(errno));
	    errno=0;
	    return FALSE;
	}
	// FIXME: if executable, then dialog to open with null (run) With entry for arguments
	if ((st.st_mode & S_IFMT) == S_IFREG){
	    if (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE)) {
		runWith(view, tpath, path);
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
		    auto page = view->page();
		    page->command(command);
		    g_free(response);
		    g_free(command);
		    return FALSE;
		}
#endif
		openWith(view, tpath, path);
	    }
	} else{
	    ERROR("local/view.hh::%s NOT a regular file\n", path);
	}
	
	TRACE("LocalView::item activated: %s\n", path);
	return FALSE;
    }

private:




public:

};
}
#endif

