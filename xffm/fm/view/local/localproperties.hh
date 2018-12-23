#ifndef LOCALPROPERTIES_HH
#define LOCALPROPERTIES_HH
#include "common/preview.hh"

typedef struct {
    GtkWidget *top;
    GtkWidget *user;
    GtkWidget *group;
    struct stat *st;
    gint result;
    gint type;
} dlg;

typedef struct row_t {
    GtkWidget *w[5];
    gboolean flag;
} row_t;

typedef struct entry_t {
    gchar *path;
    gchar *basename;
    gchar *mimetype;
    struct stat st;
} entry_t;



#define box_pack_start(box,w) \
	gtk_box_pack_start(GTK_BOX(box),w,TRUE,FALSE,0)
#define box_pack_end(box,w) \
	gtk_box_pack_end(GTK_BOX(box),w,TRUE,FALSE,0)
#define X_PAD 8
#define Y_PAD 1
#define TBL_XOPT GTK_FILL
#define TBL_YOPT GTK_SHRINK
/* flags
 */
#define IS_MULTI		1
#define IS_STALE_LINK	2
/* question dialogs: */
#define DLG_OK			0x11
#define DLG_CLOSE		0x120
#define DLG_ALL			0x140
#define DLG_SKIP		0x180
#define DLG_RC_CANCEL		0
#define DLG_RC_OK		1
#define DLG_RC_ALL		2
#define DLG_RC_CONTINUE		3
#define DLG_RC_SKIP		4
#define DLG_RC_DESTROY		5
#define DLG_RC_RECURSIVE	6
/* */
#define DLG_OK_CANCEL	(DLG_OK|DLG_CANCEL)
#define DLG_YES_NO	(DLG_YES|DLG_NO)
/* */

namespace xf{

template <class Type> class Preview;

template <class Type>
class Properties {
    GList *entryList;
    GtkWindow *dialog;


public:
    GtkBox *imageBox;
    Properties(GtkTreeModel *treeModel, GList *selectionList) {
	
	// copy selection list to private thread safe selection_list... 
	if (!selectionList || g_list_length(selectionList) < 0) {
	    DBG("propertiesDialog: nothing in selectionList\n");
	    return;
	}
	entryList = NULL;
	for (auto l = selectionList; l && l->data; l = l->next){
	    auto entry = (entry_t *)calloc(1, sizeof(entry_t));
            gchar *path;
	    GtkTreeIter iter;
            auto tpath = (GtkTreePath *)l->data;
            gtk_tree_model_get_iter(treeModel, &iter, tpath);
            gtk_tree_model_get(treeModel, &iter, 
		    PATH, &(entry->path), 
		    MIMETYPE, &(entry->mimetype), 
		    -1);
	    entry->basename = g_path_get_basename(entry->path);
	    if (!entry->mimetype) {
		entry->mimetype = 
		    g_strdup(Mime<Type>::mimeType(entry->path));
	    }
	    if (stat(entry->path, &(entry->st)) < 0){
		ERROR("Cannot stat %s: %s\n", entry->path, strerror(errno));
		removePathList(entryList);
		throw(1);
	    }
            entryList = g_list_prepend(entryList, entry);
	}
	entryList = g_list_reverse(entryList);
	//setup_properties_p(this);
	Util<Type>::context_function(Properties<Type>::doDialog, this);

	return;

    }

    ~Properties(void){
	removePathList(entryList);
    }
    
private:

    static void
    setUpImage(GtkBox *box, entry_t *entry){
	auto pixbuf = 
	    Preview<Type>::previewDefault(entry->path, entry->mimetype, &(entry->st));
	auto image = gtk_image_new_from_pixbuf(pixbuf);
	GList *list = gtk_container_get_children (GTK_CONTAINER(box));
	if (list && list->data){
	    gtk_container_remove(GTK_CONTAINER(box), GTK_WIDGET(list->data));
	    g_list_free(list);
	}
	gtk_container_add(GTK_CONTAINER(box), image);
	gtk_widget_show(GTK_WIDGET(image));
    }

    static void
    removePathList(GList *list){
	for (auto l = list; l && l->data; l = l->next){
	    auto entry = (entry_t *)l->data;
	    g_free(entry->path);
	    g_free(entry->basename);
	    g_free(entry->mimetype);
	    g_free(l->data);
	}
	g_list_free(list);
    }

//////////////////////////////////////////////////////////////////////////////////

    static void *
    doDialog (void *data) {
	auto properties_p = (Properties<Type> *)data;
	properties_p->dialog = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_type_hint(properties_p->dialog, GDK_WINDOW_TYPE_HINT_DIALOG);
	auto mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
	gtk_container_add(GTK_CONTAINER(properties_p->dialog), GTK_WIDGET(mainBox));
	auto title = GTK_LABEL(gtk_label_new(""));
	auto markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s</span>",
		_("Properties management"));
	gtk_label_set_markup(title, markup);
	g_free(markup);
	gtk_box_pack_start(mainBox, GTK_WIDGET(title), TRUE, FALSE, 0);

	auto contentBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
	gtk_box_pack_start(mainBox, GTK_WIDGET(contentBox), TRUE, FALSE, 0);
	properties_p->imageBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
	auto infoBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
	gtk_box_pack_start(contentBox, GTK_WIDGET(properties_p->imageBox), TRUE, FALSE, 0);
	gtk_box_pack_start(contentBox, GTK_WIDGET(infoBox), TRUE, FALSE, 0);
	auto buttonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
	gtk_box_pack_start(mainBox, GTK_WIDGET(buttonBox), TRUE, FALSE, 0);

	
	auto combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	entry_t *entry;
	
	for (auto l=properties_p->entryList; l && l->data; l = l->next){
	    entry = (entry_t *)l->data;
	    gtk_combo_box_text_append (combo, NULL, entry->basename);
	}
	gtk_box_pack_start(infoBox, GTK_WIDGET(combo), TRUE, FALSE, 0);
	gtk_combo_box_set_active (GTK_COMBO_BOX(combo), 0);

	// Add file info stuff
	// set info stuff data according to combo box
	// add signal handler to changed signal for combo box

	// Add process buttons
	//
	auto apply = Gtk<Type>::dialog_button(NULL, _("Apply changes"));
	auto cancel = Gtk<Type>::dialog_button(NULL, _("Cancel"));
	gtk_box_pack_end(buttonBox, GTK_WIDGET(apply), FALSE, FALSE, 0);
	gtk_box_pack_end(buttonBox, GTK_WIDGET(cancel), FALSE, FALSE, 0);
	
	//
	// Add handlers for buttons and destroy/delete
	g_signal_connect (G_OBJECT (properties_p->dialog), "delete-event", G_CALLBACK (deleteEvent), properties_p);
	g_signal_connect (G_OBJECT (cancel), "clicked", G_CALLBACK (cancelAction), properties_p);
	g_signal_connect (G_OBJECT (apply), "clicked", G_CALLBACK (applyAction), properties_p);
	//
	// Create and add image for initial selected file in dialog
	//
	//auto pixbuf = Pixbuf<Type>::get_pixbuf("accessories-calculator", -256);
	entry = (entry_t *)properties_p->entryList->data;
	setUpImage(properties_p->imageBox, entry);

	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (changeCombo), properties_p);
	
	//
	gtk_widget_show_all(GTK_WIDGET(properties_p->dialog));
	return NULL;
    }

    static void
    changeCombo (GtkComboBox *combo, void *data){
	auto properties_p = (Properties<Type> *)data;
	gint index = gtk_combo_box_get_active(combo);
	GList *list = g_list_nth (properties_p->entryList,index);
	setUpImage(properties_p->imageBox, (entry_t *)list->data);

    }

    static gboolean 
    deleteEvent(GtkWidget *dialog, GdkEvent *event, gpointer data){
	auto properties_p = (Properties<Type> *)data;
	gtk_widget_hide(GTK_WIDGET(properties_p->dialog));
	gtk_widget_destroy(GTK_WIDGET(properties_p->dialog));
	delete(properties_p);
	return TRUE;
    }

    static void
    cancelAction(GtkButton *button, void *data){
	DBG("cancelAction\n");
	deleteEvent(NULL, NULL, data);
    }

    static void
    applyAction(GtkButton *button, void *data){
	DBG("applyAction\n");
	// Apply changes to all changed items
	deleteEvent(NULL, NULL, data);
    }

};
}


#endif
