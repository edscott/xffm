#ifndef XF_FSTABPOPUP__HH
# define XF_FSTABPOPUP__HH
#include "response/comboresponse.hh"
#include "response/commandresponse.hh"
#include "fm/view/fstab/fstab.hh"
namespace xf
{

static GtkMenu *fstabPopUp=NULL;
static GtkMenu *fstabItemPopUp=NULL;
template <class Type> class BaseView;
template <class Type> class Fstab;
template <class Type>
class FstabPopUp {
    
    using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using util_c = Util<double>;
    using pixbuf_icons_c = Icons<double>;
    using page_c = Page<Type>;
    static GtkMenu *createLocalPopUp(void){
         menuItem_t item[]={
            {N_("Open in New Tab"), NULL, NULL, NULL},
            {N_("Open in New Window"), NULL, NULL, NULL},
            
             // main menu items
            //{N_("Home"), NULL, (void *) menu},
            //{N_("Open terminal"), NULL, (void *) menu},
            //{N_("About"), NULL, (void *) menu},
            //
            //common buttons /(also an iconsize +/- button)
            //{N_("Paste"), NULL, (void *) menu},
            //{N_("Sort data in ascending order"), NULL, (void *) menu},
            //{N_("Sort data in descending order"), NULL, (void *) menu},
            //{N_("Sort case insensitive"), NULL, (void *) menu},
            
            //{N_("Select All"), NULL, (void *) menu},
            //{N_("Invert Selection"), NULL, (void *) menu},
            //{N_("Unselect"), NULL, (void *) menu},
            //{N_("Select Items Matching..."), NULL, (void *) menu},
            //{N_("Unselect Items Matching..."), NULL, (void *) menu},
            //{N_("Sort by name"), NULL, (void *) menu},
            //{N_("Default sort order"), NULL, (void *) menu},
            //{N_("Sort by date"), NULL, (void *) menu},
            //{N_("Sort by size"), NULL, (void *) menu},
            //{N_("View as list"), NULL, (void *) menu},
            {NULL,NULL,NULL, FALSE}};
	fstabPopUp = BasePopUp<Type>::createPopup(item); 
        return fstabPopUp;        
    }  

    static GtkMenu *createLocalItemPopUp(void){
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
    resetLocalItemPopup(BaseView<Type> *baseView, const GtkTreePath *tpath) {
        BasePopUp<Type>::clearKeys(fstabItemPopUp);
        GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}
	gtk_tree_path_free(ttpath);
	gchar *partuuid;
	gchar *iconName;
	gchar *tooltipText;
	gchar *path;
	gchar *displayName;
        gtk_tree_model_get (baseView->treeModel(), &iter, 
                PARTUUID, &partuuid,
		//MIMETYPE, &partuuid,
		DISK_LABEL, &displayName,
		ICON_NAME, &iconName,
		PATH, &path,
                TOOLTIP_TEXT,&tooltipText,
		-1);
	if (!path){
	    ERROR("fstabItemPopup: path is NULL\n");
	    return;
	}

	gchar *g = util_c::fileInfo(path);
	gchar *fileInfo = g_strconcat(g, ": ", (tooltipText)?tooltipText:"", NULL);
	g_free(g);

        gchar *mimetype = g_strdup_printf("partuuid: %s", partuuid);

	g_object_set_data(G_OBJECT(fstabItemPopUp), "iconName", iconName);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "displayName", displayName);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "path", g_strdup(path));
	g_object_set_data(G_OBJECT(fstabItemPopUp), "mimetype", partuuid);
	g_object_set_data(G_OBJECT(fstabItemPopUp), "fileInfo", util_c::fileInfo(path));
	g_object_set_data(G_OBJECT(fstabItemPopUp), "tooltipText", tooltipText);
 	// Set title element
        BasePopUp<Type>::changeTitle(fstabItemPopUp);
    }

    static void
    resetMenuItems(BaseView<Type> *baseView, const GtkTreePath *tpath) {
        resetLocalItemPopup(baseView, tpath);
        GtkTreeIter iter;
	GtkTreePath *ttpath = gtk_tree_path_copy(tpath);
	if (!gtk_tree_model_get_iter (baseView->treeModel(), &iter, ttpath)) {
	    gtk_tree_path_free(ttpath);
	    return;
	}

	gtk_tree_path_free(ttpath);

	gchar *path;
        path = (gchar *)g_object_get_data(G_OBJECT(fstabItemPopUp), "path");
        g_free(path);
	gtk_tree_model_get (baseView->treeModel(), &iter, 
		PATH, &path,
	    -1);
        g_object_set_data(G_OBJECT(fstabItemPopUp), "path", path);
	struct stat st;
        // Hide all...
        GList *children = gtk_container_get_children (GTK_CONTAINER(fstabItemPopUp));
        for (GList *child = children; child && child->data; child=child->next){
            gtk_widget_hide(GTK_WIDGET(child->data));
        }
            
	auto v2 = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "title"));
        gtk_widget_show(v2);
        // mount options
        GtkWidget *w;
        if (Fstab<Type>::isMounted(path)){
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, TRUE);
            gtk_widget_show(w);
        } else {
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Unmount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, FALSE);
            w = GTK_WIDGET(g_object_get_data(G_OBJECT(fstabItemPopUp), "Mount the volume associated with this folder"));
            gtk_widget_set_sensitive(w, TRUE);
            gtk_widget_show(w);
        }
    }

    static void
    mount(GtkMenuItem *menuItem, gpointer data)
    {
        auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(data), "baseView");
	auto path = (const gchar *)g_object_get_data(G_OBJECT(data), "path");

	WARN("FstabPopup::mount %s\n", path);
        if (Fstab<Type>::isInFstab(path) || Fstab<Type>::isMounted(path)) {
	    WARN("mount: %s\n", path);
            Fstab<Type>::mountPath(baseView, path, NULL);
            return;            
        }
	auto dname = (const gchar *)g_object_get_data(G_OBJECT(data), "displayName");
        auto label = g_strdup_printf("LABEL=%s", dname);
        if (Fstab<Type>::isInFstab(label)) {
	    gchar *mountTarget = Fstab<Type>::mountTarget(label);
	    WARN("mount: %s --> %s\n", label, mountTarget);
            if (mountTarget){
		Fstab<Type>::mountPath(baseView, mountTarget, NULL);
	    }
            g_free(label);
            g_free(mountTarget);
            return;            
        }
        g_free(label);


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
        entryResponse->setEntryDefault(dirname);
        g_free(dirname);
        
        auto response = entryResponse->runResponse();
        delete entryResponse;
	WARN("response=%s\n", response);
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
	    Fstab<Type>::mountPath(baseView, path, response);
	    g_free(response);
	}
	g_free(basename);
    }
    
public:
    /*
    static GtkMenu *popUp(BaseView<Type> *baseView){
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
    static GtkMenu *popUp(BaseView<Type> *baseView, const GtkTreePath *tpath){
        if (!fstabItemPopUp) fstabItemPopUp = createLocalItemPopUp();   
	resetMenuItems(baseView, tpath);
        return fstabItemPopUp;
    }

    


};
}
#endif
