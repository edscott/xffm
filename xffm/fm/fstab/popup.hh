#ifndef XF_FSTABPOPUP__HH
# define XF_FSTABPOPUP__HH
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"
namespace xf
{

template <class Type> class View;
template <class Type> class FstabView;
template <class Type>
class FstabPopUp {
    
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using pixbuf_icons_c = Icons<double>;
    using page_c = Page<Type>;
    static GtkMenu *createPopUp(void){
         menuItem_t item[]={
	    {N_("Open in New Tab"), (void *)LocalPopUp<Type>::newTab, NULL, NULL},
            {N_("NFS Network Volume"), (void *)BasePopUp<Type>::noop, NULL, NULL},
            {N_("SSHFS Remote Synchronization Folder"), (void *)BasePopUp<Type>::noop, NULL, NULL},
            {N_("eCryptfs Volume"), (void *)BasePopUp<Type>::noop, NULL, NULL},
            {N_("CIFS Volume"), (void *)BasePopUp<Type>::noop, NULL, NULL},
            {NULL,NULL,NULL, FALSE}};
	fstabPopUp = BasePopUp<Type>::createPopup(item); 
        BasePopUp<Type>::changeTitle(fstabPopUp, _("Disk Image Mounter"), NULL);

        return fstabPopUp;        
    }  

    static GtkMenu *createItemPopUp(void){
	menuItem_t item[]=
        {
	    {N_("Mount the volume associated with this folder"), (void *)mount, NULL, NULL},
	    {N_("Unmount the volume associated with this folder"), (void *)mount, NULL, NULL},
            //{N_("Add bookmark"), (void *)addBookmark, NULL, NULL},
            //{N_("Remove bookmark"), (void *)removeBookmark, NULL, NULL},
	    
	    //common buttons /(also an iconsize +/- button)
	    //{N_("File Information..."), NULL, NULL, NULL},
	    //{N_("Properties"), NULL, NULL, NULL},
	     {NULL,NULL,NULL,NULL}
        };
	fstabItemPopUp = BasePopUp<Type>::createPopup(item); 
        return fstabItemPopUp;
    }

    static void 
    resetItemPopup(View<Type> *view, const gchar *path) {
        BasePopUp<Type>::clearKeys(fstabItemPopUp);
        GtkTreeIter iter;
	if (!gtk_tree_model_get_iter (view->treeModel(), &iter, 
                    (GtkTreePath *)view->selectionList()->data)) 
        {
	    ERROR("fstabItemPopup: tpath is invalid\n");
	    return;
	}
	gchar *iconName;
	gchar *tooltipText;
	gchar *displayName;
        gchar *mimetype;
        gtk_tree_model_get (view->treeModel(), &iter, 
		MIMETYPE, &mimetype,
		DISPLAY_NAME, &displayName,
		ICON_NAME, &iconName,
                TOOLTIP_TEXT,&tooltipText,
		-1);
	if (!path){
	    ERROR("fstabItemPopup: path is NULL\n");
	    return;
	}

	gchar *g = util_c::fileInfo(path);
	gchar *fileInfo = g_strconcat(g, ": ", (tooltipText)?tooltipText:"", NULL);
	g_free(g);

	g_object_set_data(G_OBJECT(fstabItemPopUp), "iconName", iconName);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "displayName", displayName);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "mimetype", mimetype);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "fileInfo", util_c::fileInfo(path));
	g_object_set_data(G_OBJECT(fstabItemPopUp), "tooltipText", tooltipText);
 	// Set title element
        BasePopUp<Type>::changeTitle(fstabItemPopUp, tooltipText);
    }

    static void
    mount(GtkMenuItem *menuItem, gpointer data)
    {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(data), "view");
        auto baseModel = (BaseModel<Type> *)g_object_get_data(G_OBJECT(data), "baseModel");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");

	TRACE("FstabPopup::mount %s\n", path);
//        if (FstabView<Type>::isInFstab(path) || FstabView<Type>::isMounted(path)) {
        if (FstabView<Type>::isMounted(path)) {
	    TRACE("mount: %s\n", path);
            FstabView<Type>::mountPath(view, path, NULL);
            return;            
        }

        gchar *shortLabel = FstabView<Type>::e2Label(path);
        gchar *mountTarget = NULL;

        auto label = g_strdup_printf("LABEL=%s", shortLabel);
        g_free(shortLabel);
        if (FstabView<Type>::isInFstab(label)) {
	    mountTarget = FstabView<Type>::mountTarget(label);
        }
        g_free(label);
        if (!mountTarget && FstabView<Type>::isInFstab(path)) {
	    mountTarget = FstabView<Type>::mountTarget(path);
        }

       // File chooser
        auto entryResponse = new(EntryFolderResponse<Type>)(GTK_WINDOW(mainWindow), _("Mount Device"), NULL);

        
        auto basename = g_path_get_basename(path);
	auto displayPath = util_c::valid_utf_pathstring(path);
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", path);  
	g_free(displayPath);
	
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setEntryLabel(_("Mount point:"));
        // get last used arguments...
        gchar *dirname = NULL;
	if (Settings<Type>::keyFileHasGroupKey("MountPoints", basename)){
	    dirname = Settings<Type>::getSettingString("MountPoints", basename);
	} 
	if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
	    g_free(dirname);
	    dirname = g_strdup_printf("/tmp/%s", basename);
	}
        if (mountTarget) {
            entryResponse->setEntryDefault(mountTarget);
        } else {
            entryResponse->setEntryDefault(dirname);
        }
        g_free(dirname);
        g_free(mountTarget);
        
        auto response = entryResponse->runResponse();
        delete entryResponse;
	TRACE("response=%s\n", response);
	if (response){
	    g_strstrip(response);
	    Settings<Type>::setSettingString("MountPoints", basename, response);
	    if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
		// Try to create...
		// FIXME: use sudo if configured and fails
		if (g_mkdir_with_parents(response, 0770) <0){
		    gchar *m = g_strdup_printf("\nmkdir %s: %s\n", response, strerror(errno));
		    Gtk<Type>::quickHelp (GTK_WINDOW(mainWindow), m, "dialog-error");
		    g_free(response);
		    return;
		}
	    } 
	    FstabView<Type>::mountPath(view, path, response);
	    g_free(response);
	}
	g_free(basename);
    }
    
public:
    /*
    static GtkMenu *popUp(View<Type> *view){
        if (!fstabPopUp) fstabPopUp = createLocalPopUp();   
        Popup();
        return fstabPopUp;
    }

    static void
    resetLocalPopup(void) {
        // Path is set on buttonpress signal...
        auto path = (const gchar *)g_object_get_data(G_OBJECT(fstabPopUp), "path");
        if (!path){
	    ERROR("resetLocalPopup: path is NULL\n");
	    return;
	}
	const gchar *mimetype = Mime<Type>::mimeType(path);
	const gchar *keys[] = {"displayName",  "mimetype", NULL};
	const gchar **q;
	for (q=keys; q && *q; q++){
	    auto cleanup = (gchar *)g_object_get_data(G_OBJECT(fstabPopUp), *q);
	    g_free(cleanup);
	}
        gchar *name = util_c::valid_utf_pathstring(path);
	g_object_set_data(G_OBJECT(fstabPopUp), "displayNam", name);
	g_object_set_data(G_OBJECT(fstabPopUp), "mimetype", g_strdup(mimetype));
	// Set title element
	BasePopUp<Type>::changeTitle(fstabPopUp,(gchar *)"folder-remote", _("Mount Helper"), NULL, NULL, "xffm:fstab");
    }
*/
    static GtkMenu *popUp(void){
        if (!fstabPopUp) fstabPopUp = createPopUp();   
        return fstabPopUp;
    }
    static GtkMenu *popUpItem(void){
        if (!fstabItemPopUp) fstabItemPopUp = createItemPopUp();   
        return fstabItemPopUp;
    }

    static void
    resetPopup(void){
        auto path = g_object_get_data(G_OBJECT(fstabPopUp), "path");
        if (!path) g_object_set_data(G_OBJECT(fstabPopUp), "path", (void *)g_strdup("xffm:fstab"));
        //auto w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabPopUp), "View as list"));
        //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), isTreeView);
    }

    static void
    resetMenuItems(void) {
        auto view = (View<Type> *)g_object_get_data(G_OBJECT(fstabItemPopUp), "view");
        auto path = (const gchar *)g_object_get_data(G_OBJECT(fstabItemPopUp), "path");
        resetItemPopup(view, path);

        // Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(fstabItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
            
	auto v2 = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "title"));
        gtk_widget_show(v2);
        // mount options
        GtkWidget *w;
        if (FstabView<Type>::isMounted(path)){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, TRUE);
            gtk_widget_show(w);
        } else if (strcmp(path,"/dev/disk")) {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, FALSE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Mount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, TRUE);
            gtk_widget_show(w);
        }
    }
    


};
}
#endif
