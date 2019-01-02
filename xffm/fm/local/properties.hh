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
    GtkBox *modeBox;
    GtkButton *apply;
    GHashTable *hash;

    Properties(GtkTreeModel *treeModel, GList *selectionList) {
	hash = g_hash_table_new(g_direct_hash, g_direct_equal);
	// copy selection list to private thread safe selection_list... 
	if (!selectionList || g_list_length(selectionList) < 0) {
	    ERROR("propertiesDialog: nothing in selectionList\n");
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
	g_hash_table_destroy(hash);
    }
    
private:
    /* Return a character indicating the type of file described by
       file mode BITS:
       'd' for directories
       'D' for doors
       'b' for block special files
       'c' for character special files
       'n' for network special files
       'm' for multiplexor files
       'M' for an off-line (regular) file
       'l' for symbolic links
       's' for sockets
       'p' for fifos
       'C' for contigous data files
       '-' for regular files
       '?' for any other file type.  */

    static
	char
    ftypelet (mode_t bits) {
#ifdef S_ISBLK
	if(S_ISBLK (bits))
	    return 'b';
#endif
	if(S_ISCHR (bits))
	    return 'c';
	if(S_ISDIR (bits))
	    return 'd';
	if(S_ISREG (bits))
	    return '-';
#ifdef S_ISFIFO
	if(S_ISFIFO (bits))
	    return 'p';
#endif
#ifdef S_ISLNK
	if(S_ISLNK (bits))
	    return 'l';
#endif
#ifdef S_ISSOCK
	if(S_ISSOCK (bits))
	    return 's';
#endif
#ifdef S_ISMPC
	if(S_ISMPC (bits))
	    return 'm';
#endif
#ifdef S_ISNWK
	if(S_ISNWK (bits))
	    return 'n';
#endif
#ifdef S_ISDOOR
	if(S_ISDOOR (bits))
	    return 'D';
#endif
#ifdef S_ISCTG
	if(S_ISCTG (bits))
	    return 'C';
#endif

	/* The following two tests are for Cray DMF (Data Migration
	   Facility), which is a HSM file system.  A migrated file has a
	   `st_dm_mode' that is different from the normal `st_mode', so any
	   tests for migrated files should use the former.  */

#ifdef S_ISOFD
	if(S_ISOFD (bits))
	    /* off line, with data  */
	    return 'M';
#endif
#ifdef S_ISOFL
	/* off line, with no data  */
	if(S_ISOFL (bits))
	    return 'M';
#endif
	return '?';
    }

    static gchar *
    modeString (mode_t mode) {
	gchar *str=(gchar *)calloc(1, 13);
	if (!str) g_error("calloc: %s", strerror(errno));
	str[0] = ftypelet (mode);
	str[1] = mode & S_IRUSR ? 'r' : '-';
	str[2] = mode & S_IWUSR ? 'w' : '-';
	str[3] = mode & S_IXUSR ? 'x' : '-';
	str[4] = mode & S_IRGRP ? 'r' : '-';
	str[5] = mode & S_IWGRP ? 'w' : '-';
	str[6] = mode & S_IXGRP ? 'x' : '-';
	str[7] = mode & S_IROTH ? 'r' : '-';
	str[8] = mode & S_IWOTH ? 'w' : '-';
	str[9] = mode & S_IXOTH ? 'x' : '-';
	if(mode & S_ISUID)
	    str[3] = mode & S_IXUSR ? 's' : 'S';
	if(mode & S_ISGID)
	    str[6] = mode & S_IXGRP ? 's' : 'S';
	if(mode & S_ISVTX)
	    str[9] = mode & S_IXOTH ? 't' : 'T';
	str[10] = 0;
	return (str);
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
	gtk_window_set_title(properties_p->dialog, _("Properties and Attributes"));
	auto mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
	gtk_container_add(GTK_CONTAINER(properties_p->dialog), GTK_WIDGET(mainBox));
	auto titlePath = GTK_LABEL(gtk_label_new(""));
	gtk_box_pack_start(mainBox, GTK_WIDGET(titlePath), TRUE, FALSE, 0);

	auto contentBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
	gtk_box_pack_start(mainBox, GTK_WIDGET(contentBox), TRUE, FALSE, 0);
	properties_p->imageBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
	gtk_widget_set_size_request (GTK_WIDGET(properties_p->imageBox),
		PREVIEW_IMAGE_SIZE, PREVIEW_IMAGE_SIZE);
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
	gtk_combo_box_set_active (GTK_COMBO_BOX(combo), 0);

	auto label = GTK_LABEL(gtk_label_new(""));
	auto markup = g_strdup_printf("<span size=\"xx-large\">%s</span>", 
		_("File Mode:"));
	gtk_label_set_markup(label, markup);
	g_free(markup);
	gtk_box_pack_start(infoBox, GTK_WIDGET(label), FALSE, FALSE, 0);


	properties_p->modeBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
	g_object_set_data(G_OBJECT(properties_p->modeBox), "properties_p", properties_p);
	g_object_set_data(G_OBJECT(properties_p->modeBox), "modeList", NULL);
	auto modeLabel = GTK_LABEL(gtk_label_new(""));  //child 1
	auto modeEntry = GTK_ENTRY(gtk_entry_new());  //child 2
	auto modeInfo = GTK_LABEL(gtk_label_new(""));  //child 3
	g_object_set_data(G_OBJECT(properties_p->modeBox), "titlePath", titlePath);
	
	gtk_box_pack_start(properties_p->modeBox, GTK_WIDGET(modeLabel), FALSE, FALSE, 0);
	gtk_box_pack_start(properties_p->modeBox, GTK_WIDGET(modeEntry), FALSE, FALSE, 0);
	gtk_box_pack_start(properties_p->modeBox, GTK_WIDGET(modeInfo), FALSE, FALSE, 0);
	gtk_box_pack_start(infoBox, GTK_WIDGET(properties_p->modeBox), FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(properties_p->imageBox), "modeBox", properties_p->modeBox);
	g_object_set_data(G_OBJECT(modeEntry), "modeBox", properties_p->modeBox);
	g_object_set_data(G_OBJECT(modeEntry), "modeLabel", modeLabel);
	g_object_set_data(G_OBJECT(modeEntry), "combo", combo);
	g_signal_connect (G_OBJECT (modeEntry), "key-release-event", G_CALLBACK (changeMode), properties_p);

	label = GTK_LABEL(gtk_label_new(""));
	markup = g_strdup_printf("<span size=\"xx-large\">%s</span>", 
		_("File:"));
	gtk_label_set_markup(label, markup);
	g_free(markup);
	gtk_box_pack_start(infoBox, GTK_WIDGET(label), FALSE, FALSE, 0);
	gtk_box_pack_start(infoBox, GTK_WIDGET(combo), FALSE, FALSE, 0);
	
	// Add file info stuff
	// set info stuff data according to combo box
	// add signal handler to changed signal for combo box

	// Add process buttons
	//
	properties_p->apply = Gtk<Type>::dialog_button(NULL, _("Apply changes"));
	gtk_widget_set_sensitive(GTK_WIDGET(properties_p->apply), FALSE);
	auto cancel = Gtk<Type>::dialog_button(NULL, _("Cancel"));
	gtk_box_pack_end(buttonBox, GTK_WIDGET(properties_p->apply), FALSE, FALSE, 0);
	gtk_box_pack_end(buttonBox, GTK_WIDGET(cancel), FALSE, FALSE, 0);
	
	//
	// Add handlers for buttons and destroy/delete
	g_signal_connect (G_OBJECT (properties_p->dialog), "delete-event", G_CALLBACK (deleteEvent), properties_p);
	g_signal_connect (G_OBJECT (cancel), "clicked", G_CALLBACK (cancelAction), properties_p);
	g_signal_connect (G_OBJECT (properties_p->apply), "clicked", G_CALLBACK (applyAction), properties_p);
	//
	// Create and add image for initial selected file in dialog
	//
	//auto pixbuf = Pixbuf<Type>::get_pixbuf("accessories-calculator", -256);
	entry = (entry_t *)properties_p->entryList->data;
	setUpImage(properties_p->imageBox, entry);
	setUpMode(properties_p->modeBox, entry);

	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (changeCombo), properties_p);
	
	//
	gtk_widget_show_all(GTK_WIDGET(properties_p->dialog));
	return NULL;
    }

    static gboolean
    changeMode (GtkEntry *modeEntry, GdkEvent  *event, void *data){
	auto properties_p = (Properties<Type> *)data;
	auto combo = GTK_COMBO_BOX(g_object_get_data(G_OBJECT(modeEntry),"combo"));
	gint index = gtk_combo_box_get_active(combo);
	GList *list = g_list_nth (properties_p->entryList,index);
	// get entry mode
	const gchar *text = gtk_entry_get_text(modeEntry);
	// switch to octal
	auto entry = (entry_t *)list->data;
	mode_t oldMode =  entry->st.st_mode & 0777;
	mode_t mode;
	sscanf(text, "%o", &mode);
	mode &= 0777;
	TRACE("changeMode event old = %o new = %o\n", oldMode, mode);

	// update label
	auto label = GTK_LABEL(g_object_get_data(G_OBJECT(modeEntry), "modeLabel"));
	auto box = GTK_BOX(g_object_get_data(G_OBJECT(modeEntry), "modeBox"));

	// compare with entry_t mode for color
	// if equal, remove from list/hash
	// if different replace in list/hash
	if (mode == oldMode){
	    g_hash_table_remove(properties_p->hash, entry->path);
	} else {
	    g_hash_table_replace(properties_p->hash, entry->path, GINT_TO_POINTER(mode));
	}
	
	setUpModeLabel(box, entry, mode);

    
	return FALSE;
    }

    static void
    setUpModeEntry(GtkBox *box, entry_t *entry){
	auto properties_p = 
	    (Properties<Type> *)g_object_get_data(G_OBJECT(box), 
		    "properties_p");
	GList *list = gtk_container_get_children (GTK_CONTAINER(box));
	if (!list || !list->next) return;
	auto modeEntry = GTK_ENTRY(list->next->data); 
	auto mode = entry->st.st_mode & 0777;
	void *setMode = 
	    g_hash_table_lookup(properties_p->hash, entry->path);
	if (setMode) mode = GPOINTER_TO_INT(setMode);

	auto modeOctal = g_strdup_printf("%0o", mode);
	gtk_entry_set_text(modeEntry, modeOctal);
	g_free(modeOctal);
    }

    static void
    setUpModeLabel(GtkBox *box, entry_t *entry, mode_t newMode){
	auto properties_p = 
	    (Properties<Type> *)g_object_get_data(G_OBJECT(box), "properties_p");
	auto titlePath = GTK_LABEL(g_object_get_data(G_OBJECT(box), "titlePath"));
	auto markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s</span>", entry->path);
	gtk_label_set_markup(titlePath, markup);
	g_free(markup);

	GList *list = gtk_container_get_children (GTK_CONTAINER(box));
	if (!list || !list->next) return;
	auto modeLabel = GTK_LABEL(list->data); 
	// changed in red, unchanged in blue...
	auto mode1 = entry->st.st_mode & 0777000;
	auto mode2 = entry->st.st_mode & 0777;
	auto modeText = modeString(newMode | mode1);
	const gchar *color = (newMode == mode2)?"blue":"red";
	auto modeMarkup = g_strdup_printf("<span size=\"xx-large\" color=\"%s\">%s</span> <span color=\"blue\" size=\"x-large\">(%0o)</span>", 
		color, modeText, mode2);
	gboolean apply = FALSE;
	if (g_hash_table_size(properties_p->hash) > 0) apply = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET(properties_p->apply),g_hash_table_size(properties_p->hash));

	gtk_label_set_markup(modeLabel, modeMarkup);
	g_free(modeMarkup);
	if (setFileInfo(entry->path, GTK_LABEL(list->next->next->data))) 
	    gtk_widget_show(GTK_WIDGET(list->next->next->data));
	else
	    gtk_widget_hide(GTK_WIDGET(list->next->next->data));

	g_list_free(list);

    }

    static gchar *
    trashInfo(const gchar *path, const gchar *item){
	auto basename = g_path_get_basename(path);
	auto keyPath =  g_strconcat(g_get_home_dir(),"/.local/share/Trash/info/", basename, ".trashinfo", NULL);
	auto keyInfo = g_key_file_new();
	auto loaded = 
	    g_key_file_load_from_file(keyInfo, keyPath, (GKeyFileFlags)0, NULL);
	g_free(basename);
	if (!loaded) {
	    ERROR("*** unable to load %s\n", keyPath);
	    g_free(keyPath);
	    return NULL;
	}
	g_free(keyPath);
	GError *error = NULL;
	gchar **p = g_key_file_get_groups (keyInfo, NULL);
	TRACE("Reading from group %s\n", *p);
	//auto value = g_key_file_get_string (keyInfo, "Trash Info", item, &error);
	auto value = g_key_file_get_string (keyInfo, *p, item, &error);
	g_strfreev(p);
	if (error){
	    ERROR("trashInfo(%s): %s\n", item, error->message);
	    g_error_free(error);
	    value = NULL;
        } 
        g_key_file_free(keyInfo);
        return value;
	
    }

    static gboolean 
    setFileInfo(const gchar *path, GtkLabel *label){
	gboolean retval = TRUE;
	auto h = g_get_home_dir();
	//gchar *m = g_strdup("xxx fileinfo");
	gchar *m1 = Util<Type>::statInfo(path);
	gchar *m2 = Util<Type>::fileInfo(path);
	gchar *m = g_strconcat(m1, "\n\n", m2, "\n",NULL);
	g_free(m1);
	g_free(m2);
	gchar *dir = g_path_get_dirname(path);
	if (strncmp(path, h, strlen(h))==0){
	    if (strcmp(dir+strlen(h), "/.local/share/Trash/files")==0){
		auto trashDate = trashInfo(path, "DeletionDate");
		if (strchr(trashDate, 'T'))*strchr(trashDate, 'T')=' ';
		auto trashSource = trashInfo(path, "Path");
		auto mt = g_strdup_printf("<span size=\"large\" color=\"red\">%s\n<span color=\"blue\">%s</span>\n%s\n<span color=\"blue\">%s</span></span>", 
		    _("Successfully moved to trash."), trashDate?trashDate:"?",
		    _("Source:"), trashSource?trashSource:"?");
		g_free(trashDate);
		g_free(trashSource);
		auto *g = g_strconcat(m,"\n", mt, NULL);
		g_free(mt);
		g_free(m);
		m = g;

	    } else TRACE("not trash:\"%s\"\n", dir+strlen(h));

	} else TRACE("not in %s\n", h);

	if (!m) retval = FALSE;
	gtk_label_set_markup(label, m);
	gtk_widget_show(GTK_WIDGET(label));

	g_free(m);
	g_free(dir);
	return retval;
    }

    static void
    setUpMode(GtkBox *box, entry_t *entry){
	auto properties_p = 
	    (Properties<Type> *)g_object_get_data(G_OBJECT(box), 
		    "properties_p");
	mode_t mode = entry->st.st_mode & 0777;
	void *setMode = 
	    g_hash_table_lookup(properties_p->hash, entry->path);
	if (setMode) mode = GPOINTER_TO_INT(setMode);
	
	setUpModeLabel(box, entry, mode);
	setUpModeEntry(box, entry);
    }

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
    changeCombo (GtkComboBox *combo, void *data){
	auto properties_p = (Properties<Type> *)data;
	gint index = gtk_combo_box_get_active(combo);
	GList *list = g_list_nth (properties_p->entryList,index);
	setUpImage(properties_p->imageBox, (entry_t *)list->data);
	setUpMode(properties_p->modeBox, (entry_t *)list->data);

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
	TRACE("cancelAction\n");
	deleteEvent(NULL, NULL, data);
    }

    static void
    setFileMode (gpointer key,
	       gpointer value,
	       gpointer data){
	auto path = (const gchar *)key;
	auto mode = (mode_t)GPOINTER_TO_INT(value);
	if (chmod(path, mode) < 0){
	    ERROR("setFileMode(): chmod(%s) %s\n", path, strerror(errno));
	}
    }    
    
    static void
    applyAction(GtkButton *button, void *data){
	TRACE("applyAction\n");
	auto properties_p = (Properties<Type> *)data;
	g_hash_table_foreach(properties_p->hash, setFileMode, NULL);
	// Apply changes to all changed items
	deleteEvent(NULL, NULL, data);
    }

};
}


#endif
