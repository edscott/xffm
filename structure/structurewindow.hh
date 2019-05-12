#ifndef STRUCTUREWINDOW_HH
#define STRUCTUREWINDOW_HH
namespace xf {
template <class Type>
    Type structure;
class StructureWindow{

GtkTreeView *treeView;
gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
GtkRequisition minimumSize_;
GtkRequisition naturalSize_;
GtkRequisition maximumSize_;
// FIXME: which one, selectionList or selection_list?
GList *selectionList;
GList *selection_list=NULL;


GdkPixbuf *focusPixbuf;

GtkTreeIter *filesParent;
GtkTreePath *tpathParent;
GtkTreeIter fileChild;
GtkTreeIter *tmpParent=NULL;


public:
    GtkWindow *mainWindow;
    StructureWindow(Type object){
	structure = object;
    	createWindow();
    }

private:
    
    static GtkWindow *
    createWindow(void){
	mainWindow = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	g_signal_connect (G_OBJECT (mainWindow), "delete-event", G_CALLBACK (Signals<Type>::delete_event), NULL);
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


};



}
#endif
