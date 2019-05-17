/*
 * Copyright 2019 Edscott Wilson Garcia 
 * <edscott@imp.mx>
 * license: GPL v.3
 */

#define STRUCTURE_CC
#define URIFILE "file://"
#define PERL_PARSER "parse11.pl";
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <math.h>
# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
//# define DBG(...)   { (void)0; }
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "*** ERROR> "); fprintf(stderr, __VA_ARGS__);}
# undef WARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}

#include <sys/wait.h>
#include <iostream>
enum {
    TARGET_URI_LIST,
    TARGETS
};
static GtkTargetList *targets=NULL;

static GtkTargetEntry targetTable[] = {
    {(gchar *)"text/uri-list", 0, TARGET_URI_LIST},
};

#define NUM_TARGETS (sizeof(targetTable)/sizeof(GtkTargetEntry))

gchar line[2048];
GList *selection_list=NULL;
//static GtkIconTheme *icon_theme=NULL;
GdkPixbuf *focusPixbuf;

enum {
    ICON,
    ICON2,
    TYPEDEF_NAME,
    PROPERTY_NAME,
    PROPERTY_VALUE,
    PROPERTY_SOURCE,
    TYPETAG_INHERITS,
    REALPATH,
    COLUMNS
};

GMarkupParseContext *mainContext;
GMarkupParseContext *typetagContext;
GMarkupParseContext *propertyContext;
GMarkupParseContext *fileContext;

gchar *sourceFile;
gchar *templates;
gchar *extraIncludes=NULL;
GtkTreeView *treeView;

GtkTreeIter typeTagParent;
GtkTreeIter propertyParent;
GtkTreeIter fileParent;
GtkTreeIter *filesParent;
GtkTreePath *tpathParent;
GtkTreeRowReference *referenceParent;
GtkTreeIter fileChild;
GtkTreeIter *tmpParent=NULL;

FILE *input;

GtkTreeStore *treeStore;
gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
GtkWindow *mainWindow;

GtkRequisition minimumSize_;
GtkRequisition naturalSize_;
GtkRequisition maximumSize_;
        GList *selectionList;

static void exitApp (GtkButton *widget,
           gpointer   user_data){
    gtk_widget_hide(GTK_WIDGET(mainWindow));
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_main_quit();
    _exit(123);
    return;
}

static gboolean delete_event (GtkWidget *widget,
           GdkEvent  *event,
           gpointer   user_data){
    gtk_widget_hide(widget);
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_main_quit();
    _exit(123);
    return TRUE;
}

    void setWindowMaxSize(void){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	auto drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	XGetGeometry(display, drawable, &root_return,
		&x_return, &y_return, 
		&w_return, &h_return, 
		&border_return, 
		&d_return);
	GdkGeometry geometry;
	geometry.max_width = w_return - 25;
	geometry.max_height = h_return -25;
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
	gtk_window_set_geometry_hints (GTK_WINDOW(mainWindow), GTK_WIDGET(mainWindow), &geometry, GDK_HINT_MAX_SIZE);
    }
    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        TRACE("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
        gtk_window_set_default_size(mainWindow, 1000, 600);
    }
    
    static GtkTreeViewColumn * 
    mkColumn(void){
        auto column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, FALSE);
        gtk_tree_view_column_set_reorderable(column, TRUE);
        gtk_tree_view_column_set_spacing(column,2);
        return column;
        
    }

    static void
    appendColumnText(GtkTreeView *treeView, const gchar *title, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "text", columnID, 
                        NULL);
        //gtk_tree_view_column_pack_start (column, GtkCellRenderer *cell, FALSE);
        gtk_tree_view_insert_column (treeView,column,-1);
	if (title) gtk_tree_view_column_set_title(column,title);

    }

    static void
    appendColumnPixbuf(GtkTreeView *treeView, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "pixbuf", columnID, 
                        NULL);
        gtk_tree_view_insert_column (treeView,column,-1);

    }
    static gchar *
    getSelectionData(){
        GList *selection_list = selectionList;
        gchar *data = NULL;
        
        for(GList *tmp = selection_list; tmp && tmp->data; tmp = tmp->next) {
            GtkTreePath *tpath = (GtkTreePath *)tmp->data;
            gchar *path;
            GtkTreeIter iter;
            gtk_tree_model_get_iter (GTK_TREE_MODEL(treeStore), &iter, tpath);
            gtk_tree_model_get (GTK_TREE_MODEL(treeStore), &iter, REALPATH, &path, -1);
            if (!data) data = g_strconcat(URIFILE, path, "\n", NULL);
            else {
                gchar *e = g_strconcat(data, URIFILE, path, "\n", NULL);
                g_free(data);
                data = e;
            }
            TRACE("getSelectionData(): append: %s -> \"%s\"\n", path, data);
            g_free(path);
        }
        return data;
    }
    static void
    DragDataSend (GtkWidget * widget,
                       GdkDragContext * context, 
                       GtkSelectionData * selection_data, 
                       guint info, 
                       guint time,
                       gpointer data) {
        TRACE("signal_drag_data_send\n");
        /* prepare data for the receiver */
        if (info != TARGET_URI_LIST) {
            ERROR("signal_drag_data_send: invalid target");
        }
        TRACE( ">>> DND send, TARGET_URI_LIST\n"); 
        gchar *dndData = NULL;
        dndData = getSelectionData();


        if (dndData){
            gtk_selection_data_set (selection_data, 
                gtk_selection_data_get_selection(selection_data),
                8, (const guchar *)dndData, strlen(dndData)+1);
        }
                    
    }

    // sender:
    static void
    DragBegin (GtkWidget * widget, GdkDragContext * context, gpointer data) {
        TRACE("signal_drag_begin\n");
        auto treeModel = GTK_TREE_MODEL(treeStore);
        auto selection = gtk_tree_view_get_selection (treeView);
        if (selectionList){
            g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
            selectionList = NULL;
        }
        selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
        //GdkPixbuf *pixbuf = Pixbuf<Type>::get_pixbuf("edit-copy", -48);
        //gtk_drag_set_icon_pixbuf (context, pixbuf,10,10);
    }
static void
createSourceTargetList (GtkWidget *widget) {
    TRACE("createSourceTargetList..\n");
    gtk_drag_source_set (widget,
                 (GdkModifierType) 0, //GdkModifierType start_button_mask,
                 targetTable,
                 NUM_TARGETS,
                 (GdkDragAction)
                                ((gint)GDK_ACTION_MOVE|
                                 (gint)GDK_ACTION_COPY|
                                 (gint)GDK_ACTION_LINK));
    return;
}
    static GtkTreePath * 
    getTpath(GdkEventButton  *event){
        GtkTreePath *tpath = NULL;
        if (!gtk_tree_view_get_path_at_pos (treeView, event->x, event->y, &tpath, NULL, NULL, NULL)){
           tpath = NULL;
        }
        return tpath;
    }
gboolean dragOn_ = FALSE;
gint buttonPressX=-1;
gint buttonPressY=-1;
    static gboolean
    buttonPress (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        TRACE("buttonpress\n");
        buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;

        GtkTreePath *tpath = NULL;
        if (event->button == 1) {
            gint mode = 0;
        TRACE("buttonpress 1\n");
            tpath = getTpath(event);
            if (tpath == NULL){ 
                return FALSE;
            } else {
                if (selectionList){
                    g_list_free_full (selectionList, (GDestroyNotify) gtk_tree_path_free);
                    selectionList = NULL;
                } else TRACE("no selection list\n");
                TRACE("button press %d mode %d\n", event->button, mode);
		buttonPressX = event->x;
		buttonPressY = event->y;
		gtk_tree_path_free(tpath);
            }

            return FALSE;
        }
        return FALSE;

    }

    static gboolean
    motionNotifyEvent (GtkWidget *widget,
                   GdkEvent  *ev,
                   gpointer   data)
    {
        auto e = (GdkEventMotion *)ev;
        auto event = (GdkEventButton  *)ev;

        if (buttonPressX >= 0 && buttonPressY >= 0){
	    TRACE("buttonPressX >= 0 && buttonPressY >= 0\n");
	    if (sqrt(pow(e->x - buttonPressX,2) + pow(e->y - buttonPressY, 2)) > 10){
                auto selection = gtk_tree_view_get_selection (treeView);
                auto treeModel = GTK_TREE_MODEL(treeStore);
                // free selection list
                if (!selectionList) selectionList = gtk_tree_selection_get_selected_rows (selection, &treeModel);
                if (selectionList==NULL) {
                    return FALSE;
                }

                auto tpath = (GtkTreePath *)selectionList->data;
                GtkTreeIter iter;
                gtk_tree_model_get_iter (GTK_TREE_MODEL(treeStore), &iter, tpath);
                gchar *realpath;
                gtk_tree_model_get (GTK_TREE_MODEL(treeStore), &iter, REALPATH, &realpath, -1);
                TRACE("realpath=%s\n", realpath);
                if (realpath){
                    g_free(realpath);
                    // start DnD (multiple selection)
                    TRACE("dragOn_ = TRUE\n");
                    dragOn_ = TRUE;

                    if (!targets) targets= gtk_target_list_new (targetTable,TARGETS);

                    //context =
                        gtk_drag_begin_with_coordinates (GTK_WIDGET(mainWindow),
                                 targets,
                                 (GdkDragAction)(((gint)GDK_ACTION_MOVE)|
                       ((gint)GDK_ACTION_COPY)|
                       ((gint)GDK_ACTION_LINK)), //GdkDragAction actions,
                                 1, //gint button,
                                 (GdkEvent *)event, //GdkEvent *event,
                                 event->x, event->y);
                }
                             
		buttonPressX = buttonPressY = -1;
                //g_object_ref(G_OBJECT(context)); 
	    }
        }



    
        return FALSE;
    }
    static gboolean
    buttonRelease (GtkWidget *widget,
                   GdkEventButton  *event,
                   gpointer   data)
    {
        TRACE("buttonRelease\n");
        //GdkEventButton *event_button = (GdkEventButton *)event;
        if (!dragOn_){
	    GtkTreePath *tpath;

	    //Cancel DnD prequel.
	    buttonPressX = buttonPressY = -1;
	    dragOn_ = FALSE;

	    /*if (isTreeView){
		auto selection = 
		    gtk_tree_view_get_selection (view->treeView());
		gtk_tree_view_get_path_at_pos (view->treeView(), 
				   event->x, event->y, &tpath, NULL,  NULL, NULL);
		if (tpath) {
		    // unselect everything
		    gtk_tree_selection_unselect_all (selection);
		    // reselect item to activate
		    gtk_tree_selection_select_path (selection, tpath);
		}
	    } 
	    if (tpath) {
		TRACE("Here we do a call to activate item.\n");
		for (auto popup=popUpArray; popup && *popup; popup++){
		    g_object_set_data(G_OBJECT(*popup), "baseModel", (void *)view);
		    g_object_set_data(G_OBJECT(*popup), "view", (void *)view);
		}
		BaseSignals<Type>::activate(tpath, data);
		gtk_tree_path_free(tpath);
	    }*/
	    return TRUE;
        }

	buttonPressX = buttonPressY = -1;
        dragOn_ = FALSE;
	
        return FALSE;
    }


static GtkWindow *
createWindow(void){
    mainWindow = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    g_signal_connect (G_OBJECT (mainWindow), "delete-event", G_CALLBACK (delete_event), NULL);
    gtk_widget_get_preferred_width (GTK_WIDGET(mainWindow), &dialogMinW_, &dialogNatW_);
    gtk_widget_get_preferred_height (GTK_WIDGET(mainWindow), &dialogMinH_, &dialogNatH_);
    gtk_window_set_type_hint(mainWindow, GDK_WINDOW_TYPE_HINT_DIALOG);
    //setWindowMaxSize(mainWindow);
    gtk_window_set_position (mainWindow, GTK_WIN_POS_MOUSE);

    auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
    gtk_container_add (GTK_CONTAINER(mainWindow), GTK_WIDGET(vbox));
    auto topbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
    auto bottombox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
    gtk_box_pack_start(vbox, GTK_WIDGET(topbox), FALSE, FALSE, 3);
    gtk_box_pack_start(vbox, GTK_WIDGET(bottombox), TRUE, TRUE, 3);
    auto label1 = GTK_LABEL(gtk_label_new(""));
    auto label2 = GTK_LABEL(gtk_label_new(""));
    auto markup1 = g_strdup_printf("<span size=\"larger\" color=\"blue\">Structure: %s    </span>", sourceFile, templates);
    auto markup2 = g_strdup_printf("<span size=\"small\" color=\"red\">Templates: %s%s</span>", templates, extraIncludes?extraIncludes:"");
    gtk_label_set_markup(label1, markup1);
    gtk_label_set_markup(label2, markup2);
    gtk_box_pack_start(topbox, GTK_WIDGET(label1), FALSE, FALSE, 3);
    gtk_box_pack_start(topbox, GTK_WIDGET(label2), FALSE, FALSE, 3);
    auto exit = gtk_button_new_with_label("Exit");
    gtk_box_pack_end(topbox, GTK_WIDGET(exit), FALSE, FALSE, 3);
    g_signal_connect (G_OBJECT (exit), "clicked", G_CALLBACK (exitApp), NULL);

    treeView = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(treeStore)));
    //gtk_tree_view_set_show_expanders(treeView, TRUE);
    gtk_tree_view_set_headers_visible(treeView, TRUE);
        appendColumnPixbuf(treeView, ICON);
        appendColumnPixbuf(treeView, ICON2);
        appendColumnText(treeView, "TypeTag/property/value", TYPEDEF_NAME);

//        appendColumnText(treeView, "Property",PROPERTY_NAME );
//        appendColumnText(treeView, "Value", PROPERTY_VALUE);
        appendColumnText(treeView, "Source", PROPERTY_SOURCE);
        appendColumnText(treeView, "inherits", TYPETAG_INHERITS);

    auto scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
    gtk_scrolled_window_set_policy(scrolledWindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(bottombox, GTK_WIDGET(scrolledWindow), TRUE, TRUE, 3);

    gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(treeView));

        // source widget
        g_signal_connect (G_OBJECT (mainWindow), 
             "drag-begin", G_CALLBACK(DragBegin),
             mainWindow);
        
        g_signal_connect (G_OBJECT (mainWindow), 
             "drag-data-get", G_CALLBACK(DragDataSend), 
             mainWindow);
         g_signal_connect (mainWindow, "button-release-event",
             G_CALLBACK(buttonRelease), 
             mainWindow);
         g_signal_connect (treeView, "button-press-event",
             G_CALLBACK(buttonPress), 
             mainWindow);
        
        // source widget
        g_signal_connect (treeView, "motion-notify-event", 
            G_CALLBACK (motionNotifyEvent), 
            mainWindow);

    setDefaultSize();
    gtk_window_present (mainWindow);
    while (gtk_events_pending()) gtk_main_iteration();

    createSourceTargetList(GTK_WIDGET(mainWindow));

    return mainWindow;

}

static void
startProperty (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    auto parent = (GtkTreeIter *)data; 

    TRACE ("start -> %s\n",element_name); 
    if(strcmp (element_name, "property")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"property\")!= 0 (%s)\n", element_name);
        return;
    }
    // Simple property into allProperties
    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    gboolean validValueIter = FALSE;
    for (; name && *name; name++, value++){
        if (strcmp(*name, "name")==0){
            TRACE("Property %s=%s\n", *name, *value); 
        } else {
            TRACE( "         %s=%s\n", *name, *value); 
        }
        GtkTreeIter nameiter;
        GtkTreeIter valueiter;
        GtkTreeIter sourceiter;
        if (strcmp(*name, "name")==0) {
            if (data) {
                gtk_tree_store_append(treeStore, &nameiter, (GtkTreeIter *)data);
            } else {
                gtk_tree_store_append(treeStore, &nameiter, tmpParent);
            }
            auto g = g_strdup_printf("   %s", *value);
            gtk_tree_store_set(treeStore, &nameiter,
               TYPEDEF_NAME, g, -1);
            g_free(g);
        }
        if (strcmp(*name, "value")==0) {
            gtk_tree_store_append(treeStore, &valueiter, &nameiter);
            validValueIter = TRUE;
            auto g = g_strdup_printf("       %s", *value);
            gtk_tree_store_set(treeStore, &valueiter,
               TYPEDEF_NAME, g, -1);
            g_free(g);
        }
        if (validValueIter && strcmp(*name, "source")==0) {
            auto g = g_strdup_printf("%s", *value);
            gtk_tree_store_set(treeStore, &valueiter,
               PROPERTY_SOURCE, g, -1);
            g_free(g);
        }
        if (strcmp(*name, "realpath")==0) {
            gtk_tree_store_set(treeStore, &nameiter, REALPATH, *value, -1);
            gtk_tree_store_set(treeStore, &valueiter, REALPATH, *value, -1);
        }

    }
    return;
}
static GdkPixbuf *
getEmblem(gchar which){
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();	
    GError *error = NULL;
    const gchar *iconName;
    switch (which) {
        case '1':
            iconName = "emblem-important";
            break;
//        default :
  //          iconName = "emblem-default";
        case '2':
            iconName = "emblem-default";
            break;
        case '3':
            iconName = "emblem-default-symbolic";
            break;
        case '4':
            iconName = "emblem-ok-symbolic";
            break;
        default:
            iconName = "emblem-new";
    }
    TRACE("focus iconName=%s\n", iconName);
    auto pixbuf = gtk_icon_theme_load_icon (icon_theme,
                  iconName,
                  16, 
                  GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
                  &error);
    if (error) {
        ERROR("icons.hh:get_theme_pixbuf: %s\n", error->message);
        g_error_free(error);
        //return 1;
    }
    g_object_ref(pixbuf);
    return pixbuf;
}

static void
startTypeTag (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    TRACE ("start -> %s\n",element_name); 
    if(strcmp (element_name, "typetag")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"typetag\")!= 0 (%s)\n", element_name);
        return;
    }

    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    auto parent = (GtkTreeIter *)data; 
    static GtkTreeIter iter;

    for (; name && *name; name++, value++){
        if (strcmp(*name, "name")==0){
            gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, TYPEDEF_NAME, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        } 
        if (strcmp(*name, "inherits")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, TYPETAG_INHERITS, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        }
        if (strcmp(*name, "source")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, PROPERTY_SOURCE, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        }
        if (strcmp(*name, "focus")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, ICON2, getEmblem((*value)[0]), -1);
            TRACE( "focus %s=%s\n", element_name, *value); 
        }
        if (strcmp(*name, "realpath")==0) {
            gtk_tree_store_set(treeStore, &iter, REALPATH, *value, -1);
        }
   }
    if (tmpParent) gtk_tree_iter_free(tmpParent);
    tmpParent = gtk_tree_iter_copy(&iter);
    return;
}
int recurseCount = 0;
static void
startFiles(GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    TRACE ("start -> %s\n",element_name); 
            
    if(strcmp (element_name, "files")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"files\")!= 0 (%s)\n", element_name);
        return;
    }
    recurseCount++;

    
    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    
    TRACE ("%d: %s %s\n",recurseCount, element_name, *value); 
    for (; name && *name; name++, value++){
        GtkTreeIter parent;
        GtkTreeIter sourceiter;
        if (strcmp(*name, "name")==0) {
            //gtk_tree_store_append(treeStore, &fileChild, filesParent);
            GtkTreePath *tpath = gtk_tree_row_reference_get_path(referenceParent);
            gtk_tree_model_get_iter(GTK_TREE_MODEL(treeStore), &parent, tpath);
            gtk_tree_store_append(treeStore, &fileChild, &parent);
            gtk_tree_path_free(tpath);
            tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore), &fileChild);
            gtk_tree_row_reference_free(referenceParent);
            referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
            auto g = g_strdup("");
            for (auto i=0;i<recurseCount; i++){
                auto gg = g_strconcat(g, "   ", NULL);
                g_free(g);
                g=gg;
            }
            auto k = g_strdup_printf("%s%s", g, *value);
            gtk_tree_store_set(treeStore, &fileChild,
               TYPEDEF_NAME, k, -1);
               //PROPERTY_NAME, *value, -1);
            g_free(g);
            g_free(k);
        }
        if (strcmp(*name, "realpath")==0) {
            gtk_tree_store_set(treeStore, &fileChild, PROPERTY_SOURCE, *value, -1);
            gtk_tree_store_set(treeStore, &fileChild, REALPATH, *value, -1);
        }
        
    }

    return;
}

static void
mainStart (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    TRACE ("start -> %s\n",element_name); 

    if(strcmp (element_name, "structure")==0 ){
        const gchar **name = attribute_names;
        const gchar **value = attribute_values;
        for (; name && *name; name++, value++){
            if (strcmp(*name, "source")==0)sourceFile = g_strdup(*value);
            if (strcmp(*name, "templates")==0)templates = g_strdup(*value);
            if (strcmp(*name, "include")==0)
                extraIncludes = g_strdup_printf("\nInclude: %s",*value);
        }
        return;
    }
    if(strcmp (element_name, "property")==0 ){
        // Simple one line elements:
        startProperty (propertyContext,
               element_name,
               attribute_names, 
	       attribute_values, 
	       &propertyParent, 
	       error);
        return;
    }
    if(strcmp (element_name, "typetag")==0 ){
        startTypeTag (typetagContext,
               element_name,
               attribute_names, 
	       attribute_values, 
	       &typeTagParent, 
	       error);
        // Next lines:
        while(!feof (input)
                && 
                fgets (line, 2048, input) 
                &&
                !strstr(line,"</typetag>"))
        {
            // Simple one line elements:
            line[2048] = 0;
            TRACE("start->\n%s<-end\n", line);
            g_markup_parse_context_parse (propertyContext, line, strlen(line), error);
        }
        return;
    }
    return;
}

GMarkupParser mainParser = {
    mainStart,
    NULL, // mainEnd,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser typeTagParser = {
    startTypeTag,
    NULL,   //endTypeTag,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser propertyParser = {
    startProperty,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser fileParser = {
    startFiles,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};


static void
parseXML (const gchar * file) {
    GError *error = NULL;

    TRACE("glib_parser(icon-module): parsing %s\n", file);

    mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
    typetagContext = g_markup_parse_context_new (&typeTagParser, (GMarkupParseFlags)0, &typeTagParent, NULL);
    propertyContext = g_markup_parse_context_new (&propertyParser, (GMarkupParseFlags)0, NULL, NULL);
    fileContext = g_markup_parse_context_new (&fileParser, (GMarkupParseFlags)0, GINT_TO_POINTER(1), NULL);

    input = fopen (file, "r");
    if(!input) {
        TRACE ("cannot open %s\n", file);
        return;
    }
    while(!feof (input) && fgets (line, 2048, input)) {
        line[2048] = 0;
        if (strstr(line, "<files name=") || strstr(line, "</files")){
            if (strstr(line, "<files name=")){
                g_markup_parse_context_parse (fileContext, line, strlen(line), &error);
            } else {
                auto tpath = gtk_tree_row_reference_get_path(referenceParent);
                gtk_tree_path_up(tpath);
                gtk_tree_row_reference_free(referenceParent);
                referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
                recurseCount--;
           }
                    
        } else {
            g_markup_parse_context_parse (mainContext, line, strlen(line), &error);
        }
    }
    fclose (input);

    g_markup_parse_context_free (typetagContext);
    g_markup_parse_context_free (propertyContext);
    g_markup_parse_context_free (fileContext);
    g_markup_parse_context_free (mainContext);
}
namespace xf {
template <class Type>
class Structure {
    gchar *xmlFile_;
    gchar *parserPath_;
public:
    ~Structure(void){
        g_free(xmlFile_);
        g_free(parserPath_);
    }
    Structure(gchar **argv){
        xmlFile_ = g_strdup("structure.xml");
	if (argv[1] == NULL){
	    if (!g_file_test("structure.xml", G_FILE_TEST_IS_REGULAR)){
		std::cerr<<"Cannot find file \"structure.xml\" to open\n";
		throw 5;
	    }
	    return;
	}
	if (!checkArguments(argv)){
	    std::cerr<<"Usage: "<<argv[0]<<" <target file> \n    --templates=<systemwide templates> \n    --include=<override includes> \n    [--problemTypeTag=<DuMuX problem TypeTag>]\n";
	    throw 6;
	}
	createStructureXML(argv);
    }

    const gchar *xmlFile(void){return xmlFile_;}
private:

    gboolean checkDir(gchar **argv, const gchar *a){
	// required directory
	for (auto p=argv+1; p && *p; p++){
	    if (strstr(*p, a)){
		auto g = g_strsplit(*p, "=",2);
		if (g_file_test(g[1], G_FILE_TEST_IS_DIR)){
		    return TRUE;
		} else std::cerr<<g[1]<<" is not a directory\n";
	    }
	}
	std::cerr<<"Missing argument: "<<a<<"\n";
	return FALSE;
    }

    gboolean checkArguments(gchar **argv){
	// source file
	gboolean OK=FALSE;
	for (auto p=argv+1; p && *p; p++){
	    if (g_file_test(*p, G_FILE_TEST_IS_REGULAR)){
		OK=TRUE;
		break;
	    }
	}
	if (!OK) return FALSE;
	// required arguments
	if (!checkDir(argv, "--templates=")) return FALSE;
	if (!checkDir(argv, "--include=")) return FALSE;
	return TRUE;
    }

    void createStructureXML(gchar **argv){
        auto parser=PERL_PARSER;
        parserPath_ = g_find_program_in_path(parser);
        if (!parserPath_){
            std::cerr<<"Cannot find "<<parser<<" in path\n";
            throw 1;
        }
        // Get source file, template directory and include directory options
        auto sourceFile = getSourceFile(argv);
        int status;
        auto pid = fork();
        if (pid) wait(&status);
        else {
            gchar *a[10];
            int i=0;
            a[i++] = (gchar *)"/usr/bin/perl";
            a[i++] = (gchar *)parserPath_;
            a[i++] = (gchar *)sourceFile;
            for (auto p=argv+1; p && *p && i<9; p++){
                if (strstr(*p, "--"))a[i++] = (gchar *)*p;
            }
            a[i] = NULL;
            for (auto j=0; j<i; j++)fprintf(stderr,"%s ", a[j]); fprintf(stderr,"\n");
            execvp("/usr/bin/perl", a);
        }
        fprintf(stderr, "OK\n");
   }

   const gchar *getSourceFile(gchar **argv){
        const gchar *retval=NULL;
        for (auto p=argv+1; p && *p; p++){
            if (strstr(*p, "--"))continue;
            if (!g_file_test(*p, G_FILE_TEST_IS_REGULAR)){
                std::cerr<< *p << "is not a regular file\n";
                throw 2;
            }
            retval = *p;
        }
        if (!retval) throw 3;
        return retval;
    }

};
}

int
main (int argc, char *argv[]) {
    // If no argument is specified, then the program will try to read
    // structure.xml from the current directory
    // 
    xf::Structure<double>  *structure;
    try {
        structure = new(xf::Structure<double>)(argv);
    } catch (int e){
        exit(1);
    }
 
    treeStore = gtk_tree_store_new(COLUMNS, 
	    GDK_TYPE_PIXBUF, // icon in treeView display
	    GDK_TYPE_PIXBUF, // icon in treeView display
	    G_TYPE_STRING,   // typedef name or all-properties 
	    G_TYPE_STRING,   // property name
	    G_TYPE_STRING,   // property value
	    G_TYPE_STRING,   // property source
	    G_TYPE_STRING,   // property typetag
	    G_TYPE_STRING);   // realpath (for DND)
    gtk_tree_store_append(treeStore, &fileParent, NULL);
    gtk_tree_store_set(treeStore, &fileParent, TYPEDEF_NAME, "Files", -1);
    auto tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore), &fileParent);
    referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
    gtk_tree_path_free(tpath);
    //filesParent = gtk_tree_iter_copy(&fileParent);
    gtk_tree_store_append(treeStore, &propertyParent, NULL);
    gtk_tree_store_set(treeStore, &propertyParent, TYPEDEF_NAME, "Properties", -1);
    gtk_tree_store_append(treeStore, &typeTagParent, NULL);
    gtk_tree_store_set(treeStore, &typeTagParent, TYPEDEF_NAME, "TypeTags", -1);
    gtk_init(&argc, &argv);

//    parseXML(argv[1]?argv[1]:"structure.xml");
    parseXML(structure->xmlFile());
	
    
    createWindow();
    gtk_widget_show_all(GTK_WIDGET(mainWindow));
    gtk_main();
    return 0;
}
