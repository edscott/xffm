#ifndef PATHBAR_HH
#define PATHBAR_HH

namespace xf {
template <class Type> class Page;

template <class Type> 
class Pathbar
{
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
    using print_c = Print<double>;

    gchar *path_;
    
public:
    Pathbar(void) {
	path_ = NULL;
	pathbar_ = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        setStyle();

	auto eback = gtk_event_box_new();
	g_object_set_data(G_OBJECT(eback), "name", g_strdup("RFM_GOTO"));
	g_object_set_data(G_OBJECT(eback), "path", g_strdup("xffm:goto"));

	auto backPixbuf = Pixbuf<Type>::get_pixbuf("go-previous", -24);
        auto backimage = gtk_image_new_from_pixbuf(backPixbuf);
	gtk_container_add (GTK_CONTAINER(eback), GTK_WIDGET(backimage));

	gtk_box_pack_start (GTK_BOX (pathbar_), GTK_WIDGET(eback), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(eback));
	g_signal_connect (G_OBJECT(eback) , "button-press-event", EVENT_CALLBACK (go_back), (void *)this);
	


	auto eb = gtk_event_box_new();
	g_object_set_data(G_OBJECT(eb), "name", g_strdup("RFM_GOTO"));
	g_object_set_data(G_OBJECT(eb), "path", g_strdup("xffm:goto"));

	auto pixbuf = Pixbuf<Type>::get_pixbuf("go-jump", -24);
        auto image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add (GTK_CONTAINER(eb), GTK_WIDGET(image));

	gtk_box_pack_start (GTK_BOX (pathbar_), GTK_WIDGET(eb), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(eb));
	g_signal_connect (G_OBJECT(eb) , "button-press-event", EVENT_CALLBACK (go_jump), (void *)this);
	

        // xffm:root button:
        auto pb_button = pathbarLabelButton(".");

        
	gtk_box_pack_start (GTK_BOX (pathbar_), GTK_WIDGET(pb_button), FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
	g_object_set_data(G_OBJECT(pb_button), "path", g_strdup("xffm:root"));
    
	g_signal_connect (G_OBJECT(pb_button) , "button-press-event", EVENT_CALLBACK (pathbar_go), (void *)this);
	g_signal_connect (G_OBJECT(pb_button) , "enter-notify-event", EVENT_CALLBACK (pathbar_white), (void *)this);
	g_signal_connect (G_OBJECT(pb_button) , "leave-notify-event", EVENT_CALLBACK (pathbar_blue), (void *)this);
	TRACE("showing pathbar pb_button\n" );
        
	gtk_widget_show(GTK_WIDGET(pb_button));

    }

    GtkWidget *
    pathbar(void){ return pathbar_;}

    const gchar *path(void){ return path_;}

    void 
    update_pathbar(const gchar *path){
	TRACE( "update pathbar to %s\n", path);
	g_free(path_);
	path_ = g_strdup(path);
	void *arg[]={(void *)this, (void *)(path?g_strdup(path):NULL)};
	util_c::context_function(update_pathbar_f, arg);
    }

private:
    GtkWidget *pathbar_;

    GtkEventBox *
    pathbarLabelButton (const char *text) {
	auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_EVENT_BOX(gtk_event_box_new());
	if (text) {
	    auto v = util_c::utf_string(text);
	    auto g = g_markup_escape_text(v, -1);
	    g_free(v);
	    auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
	    g_free(g);
	    gtk_label_set_markup(label, markup);
	    g_free(markup);
	} else {
            gtk_label_set_markup(label, "");
        }
        gtk_container_add(GTK_CONTAINER(eventBox), GTK_WIDGET(label));
	g_object_set_data(G_OBJECT(eventBox), "label", label);
	g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
	return eventBox;
    }

    const gchar *getClickPath(GtkWidget *eventBox){
	GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar_));
	GList *children = children_list;
	for (;children && children->data; children=children->next){
	    if (eventBox == children->data){
		const gchar *path = (gchar *)g_object_get_data(G_OBJECT(eventBox), "path");
                if (!path) {
                    TRACE("path is null at pathbar.hh::pathbar_ok\n");
		    break;
                }
		TRACE("getClickPath(): %s\n", path);
		return path;
	    } 
	}
	return "xffm:root";
    }

    View<Type> *pathbarView(void){
        auto page = (Page<Type> *)this;
        return (View<Type> *)
	    g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
    }

    void 
    pathbar_ok(GtkWidget *eventBox){
        TRACE("pathbar_ok\n");
        static gboolean pathbarBusy = FALSE;
        if (pathbarBusy) return;
        pathbarBusy = TRUE;
	const gchar *path = getClickPath(eventBox); 
        auto page = (Page<Type> *)this;
	pathbarView()->loadModel(path);     
        pathbarBusy = FALSE;
    }

/*    void 
    pathbar_ok(GtkWidget *eventBox){
        TRACE("pathbar_ok\n");
        static gboolean pathbarBusy = FALSE;
        if (pathbarBusy) return;
        pathbarBusy = TRUE;
        
	GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar_));
	GList *children = children_list;
        auto page = (Page<Type> *)this;
	for (;children && children->data; children=children->next){
	    if (eventBox == children->data){
		const gchar *path = (gchar *)g_object_get_data(G_OBJECT(eventBox), "path");
                if (!path){
		    path="xffm:root";
                    TRACE("path is null at pathbar.hh::pathbar_ok\n");
                }
                auto view = (View<Type> *)
                    g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
                view->loadModel(path);

	    } 
	}
        pathbarBusy = FALSE;
    }*/

    static void         
    showWhatFits(GtkWidget *pathbar, const gchar *path, GList *children_list){
	GtkRequisition minimum;
	GtkAllocation allocation;
	gtk_widget_get_allocation(gtk_widget_get_toplevel(pathbar), &allocation);
	TRACE("pathbar width=%d\n", allocation.width);
	gint width = allocation.width;
	// First we hide all buttons, except "RFM_ROOT"
	GList *children = g_list_last(children_list);
	for (;children && children->data; children=children->prev){
	    gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) {
		gtk_widget_get_preferred_size(GTK_WIDGET(children->data), 
			&minimum, NULL);
		width -= minimum.width;
		continue;
	    }
	    gtk_widget_hide(GTK_WIDGET(children->data));
	}

	// Find first item to place in pathbar.
	// This item *must* be equal to path, if path is in buttons.

	children = g_list_last(children_list);
	GList *active = children;
	// If path is not in the buttons, then the first to map
	// will be the last path visited.
	if (path) for (;children && children->data; children=children->prev){
	    auto pb_path = (const gchar *)
		g_object_get_data(G_OBJECT(children->data), "path");
	    if (!pb_path) continue;
	    if (strcmp(path, pb_path)==0) {
		active = children;
		break;
	    }
	}
 	// Show active button
	gtk_widget_show_all(GTK_WIDGET(active->data));

	gtk_widget_get_preferred_size(GTK_WIDGET(active->data), &minimum, NULL);
	    TRACE("#### width, minimum.width %d %d\n",width,  minimum.width);
	width -= minimum.width;
     
	// Work backwards from active button we show buttons that will fit.
	children = active->prev;
	for (;children && children->data; children=children->prev){
	    gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) continue;
	    gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
	    TRACE("#### width, allocaltion.width %d %d\n",width,  allocation.width);
	    width -= allocation.width;
	    if (width < 0) break;
	    gtk_widget_show_all(GTK_WIDGET(children->data));
	}

	// Now we work forwards, showing buttons that fit.
	children = active->next;
	for (;children && children->data; children=children->next){
	   gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) continue;
	    gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
	    width -= allocation.width;
	    if (width < 0) break;
	    gtk_widget_show_all(GTK_WIDGET(children->data));
	}
    }

    void 
    toggle_pathbar(const gchar *path){
        // Hiding stuff which does not fit does not work until
        // window has been shown. This is not yet the case on
        // initial startup, so we skip that on first pass.
        TRACE("*** toggle_pathbar\n");
	GList *children_list = 
	    gtk_container_get_children(GTK_CONTAINER(pathbar_));

        if (gtk_widget_is_visible(GTK_WIDGET(mainWindow))) showWhatFits(pathbar_, path, children_list);
        else gtk_widget_show_all(GTK_WIDGET(pathbar_));


	// Finally, we differentiate active button.
	GList *children = g_list_first(children_list);
	for (;children && children->data; children=children->next){

	    setPathButtonText(GTK_WIDGET(children->data), path, "blue", NULL);

	}
	g_list_free(children_list);
    }

private:
    static void 
    setPathButtonText(GtkWidget *eventBox, const gchar *path, const gchar *color, const gchar *bgcolor){
	//const gchar *fontSize = "size=\"small\"";
	const gchar *fontSize = "";
	gchar *name = (gchar *)g_object_get_data(G_OBJECT(eventBox), "name");
	if (strcmp(name, "RFM_ROOT")==0) {
	    // no path means none is differentiated.
	    gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, color, bgcolor?bgcolor:"#dcdad5", ".");
	    auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
	    gtk_label_set_markup(label, markup);
	    g_free(markup);
	    return;
	} 
	if (strcmp(name, "RFM_GOTO")==0) {
	    return;
	} 
	const gchar *pb_path = 
	    (const gchar *)g_object_get_data(G_OBJECT(eventBox), "path");
	if (!pb_path){
	    g_warning("rfm_update_pathbar(): pb_path is null\n");
	    return;
	}
	if (!strlen(pb_path)) pb_path=G_DIR_SEPARATOR_S;//?
	if (strcmp(pb_path, path)==0) {
	    gchar *v = util_c::utf_string(name);
	    gchar *g = g_markup_escape_text(v, -1);
	    g_free(v);
	    gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, bgcolor?"white":"red", bgcolor?bgcolor:"#dcdad5", g);
	    auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
	    gtk_label_set_markup(label, markup);

	    g_free(g);
	    g_free(markup);
	}
	else {
	    gchar *v = util_c::utf_string(name);
	    gchar *g = g_markup_escape_text(v, -1);
	    g_free(v);
	    gchar *markup = g_strdup_printf("<span %s color=\"%s\" bgcolor=\"%s\">  %s  </span>", fontSize, color, bgcolor?bgcolor:"#dcdad5", g);
	    auto label = GTK_LABEL(g_object_get_data(G_OBJECT(eventBox), "label"));
	    gtk_label_set_markup(label, markup);

	    g_free(g);
	    g_free(markup);
	}
	return;
    }

    void setStyle(void){
        GError *error=NULL;
	GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET(pathbar_));
	gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_BUTTON );
	GtkCssProvider *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data (css_provider, 
    "\
    box * {\
      background-color: #dcdad5;\
      border-width: 0px;\
      border-radius: 0px;\
      border-color: transparent;\
    }\
    ", 
	    -1, &error);
        if (error){
            ERROR("fm/dialog/notebook/page/pathbar.hh::setStyle(): %s\n", error->message);
            g_error_free(error);
        } else {
	    gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
				    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////


    static gboolean
    pathbar_white (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
	Pathbar *pathbar_p = (Pathbar *)data;
	setPathButtonText(eventBox, pathbar_p->path(), "white", "#acaaa5");
        return FALSE;
    }

    static gboolean
    pathbar_blue (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
	Pathbar *pathbar_p = (Pathbar *)data;
	setPathButtonText(eventBox, pathbar_p->path(), "blue", NULL);
        return FALSE;

    }

    static gboolean
    pathbar_go (GtkWidget *eventBox,
               GdkEventButton  *event,
               gpointer   data) {
	Pathbar *pathbar_p = (Pathbar *)data;
	
        if (event->button == 1) {
	    pathbar_p->pathbar_ok(eventBox);
	}

        if (event->button == 3) {
	    auto view = pathbar_p->pathbarView();
	    const gchar *path = pathbar_p->getClickPath(eventBox);
	    TRACE("***clickpath=%s\n", path);
	    GtkMenu *menu = NULL;
	    if (g_file_test(path, G_FILE_TEST_IS_DIR)){ 
		menu = LocalPopUp<Type>::popUp();
		Popup<Type>::setWidgetData(menu, "path", path);
		g_object_set_data(G_OBJECT(menu),"view", NULL);
		BaseSignals<Type>::configureViewMenu(LOCALVIEW_TYPE);
	    } else {
		// do xffm:root menu
		RootPopUp<Type>::resetPopup();
		menu = RootPopUp<Type>::popUp();
	    }
	    if (menu) {
		gtk_menu_popup_at_pointer (menu, (const GdkEvent *)event);
	    }  	
	}

        return FALSE;
    }
    
    static gboolean
    go_back (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
	Pathbar *pathbar_p = (Pathbar *)data;
	auto page = (Page<Type> *)pathbar_p;
	auto view = (View<Type> *)
		g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
	view->goBack();
        return FALSE;
    }

    static gboolean
    go_jump (GtkWidget *eventBox,
               GdkEvent  *event,
               gpointer   data) {
	Pathbar *pathbar_p = (Pathbar *)data;
        // File chooser
        auto entryResponse = new(EntryResponse<Type>)(GTK_WINDOW(mainWindow), _("Go to"), "go-jump");
	auto markup = 
	    g_strdup_printf("<span color=\"blue\" size=\"larger\"><b>%s</b></span>", _("Go to"));  
	
        entryResponse->setResponseLabel(markup);
        g_free(markup);

        entryResponse->setEntryLabel(_("Specify Output Directory..."));
        // get last used arguments...
        gchar *dirname = NULL;
	if (Settings<Type>::keyFileHasGroupKey("GoTo", "Default")){
	    dirname = Settings<Type>::getSettingString("GoTo", "Default");
	} 
	if (!dirname || !g_file_test(dirname, G_FILE_TEST_IS_DIR) ) {
	    g_free(dirname);
	    dirname = g_strdup("");
	}
        entryResponse->setEntryDefault(dirname);
        g_free(dirname);
	auto page = (Page<Type> *)pathbar_p;
	const gchar *wd = page->workDir();
	if (!wd) wd = g_get_home_dir();
        entryResponse->setEntryBashCompletion(wd);
        entryResponse->setInLineCompletion(TRUE);
        
        auto response = entryResponse->runResponse();
        TRACE("response=%s\n", response);
	
        if (!response) return FALSE;
        if (strlen(response) > 1 && response[strlen(response)-1] == G_DIR_SEPARATOR){
            response[strlen(response)-1] = 0;
        }
	if (!g_file_test(response, G_FILE_TEST_IS_DIR)){
	    gchar *message = g_strdup_printf("\n  %s:  \n  %s  \n", response, _("Not a directory"));
	    Dialogs<Type>::quickHelp(GTK_WINDOW(mainWindow), message, "dialog-error");
	    g_free(message);
	} else {
	    auto view = (View<Type> *)
		g_object_get_data(G_OBJECT(page->topScrolledWindow()), "view");
	    view->loadModel(response);
	}
	g_free(response);
        return FALSE;

    }

    static void *
    update_pathbar_f(void *data){
	void **arg = (void **)data;
	Pathbar *pathbar_p = (Pathbar *)arg[0];
	gchar *path = (gchar *)arg[1];
	GtkWidget *pathbar = pathbar_p->pathbar();
	TRACE( "update_pathbar_f:: %s\n", path);

	if (!pathbar) return NULL;
	if (!path){
	    TRACE("##### toggle_pathbar(pathbar, NULL)\n");
	    pathbar_p->toggle_pathbar(NULL);
	    return NULL;
	}

	// Trim pathbar.
	gchar **paths;
	if (strcmp(path, G_DIR_SEPARATOR_S)==0){
	    paths = (gchar **)calloc(2, sizeof(gchar *));
	    if (!paths){
		g_warning("update_pathbar(): cannot malloc\n");
		return NULL;
	    }
	    paths[1]=NULL;
	} else {
	    paths = g_strsplit(path, G_DIR_SEPARATOR_S, -1);
	    g_free(paths[0]);
	}
	paths[0]= g_strdup(G_DIR_SEPARATOR_S);

	GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar));
	GList *children = children_list;
	gint i=0;
	gchar *pb_path = NULL;
	for (;children && children->data; children=children->next){
	    gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0 || strcmp(name, "RFM_GOTO")==0) continue;
	    //gchar *p = g_strdup_printf("%s%c", paths[i], G_DIR_SEPARATOR);
	    TRACE( "(%d) comparing %s <--> %s\n", i, name, paths[i]);
	    if (paths[i] && strcmp(name, paths[i]) == 0){
		g_free(pb_path);
		const gchar *p = (const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
		pb_path = g_strdup(p);
		i++; 
		continue;
	    }
	    // Eliminate tail (only if tail will differ)
	    if (paths[i] == NULL) break;
	    TRACE( "Zapping tail: \"%s\"\n", paths[i]);
	    GList *tail = children;
	    for (;tail && tail->data; tail = tail->next){
		gchar *name  = (gchar *)g_object_get_data(G_OBJECT(tail->data), "name");
		TRACE( "Zapping tail item: \"%s\"\n", name);
		g_free(name);
		gtk_container_remove(GTK_CONTAINER(pathbar), GTK_WIDGET(tail->data));
	    }
	    break;
	}
	g_list_free(children_list);

	// Add new tail
	for (;paths[i]; i++){
	    auto pb_button = 
                pathbar_p->pathbarLabelButton(strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);
	    gtk_container_add(GTK_CONTAINER(pathbar), GTK_WIDGET(pb_button));

	    gchar *g = (pb_path!=NULL)?
		g_strdup_printf("%s%s%s",pb_path, 
			strcmp(pb_path,G_DIR_SEPARATOR_S)? 
			G_DIR_SEPARATOR_S:"", paths[i]):
		g_strdup(paths[i]);
	    g_free(pb_path);
	    pb_path = g;
	    TRACE( "+++***** setting pbpath --> %s\n", pb_path);
	    g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(pb_path));
	    g_signal_connect (G_OBJECT(pb_button) , "button-press-event", EVENT_CALLBACK (pathbar_go), (void *)pathbar_p);
	    g_signal_connect (G_OBJECT(pb_button) , "enter-notify-event", EVENT_CALLBACK (pathbar_white), (void *)pathbar_p);
	    g_signal_connect (G_OBJECT(pb_button) , "leave-notify-event", EVENT_CALLBACK (pathbar_blue), (void *)pathbar_p);


	    gtk_widget_show(GTK_WIDGET(pb_button));
	}
	g_free(pb_path);
	g_strfreev(paths);
	
	// show what fits
	pathbar_p->toggle_pathbar(path);
	g_free(path);
	return NULL;
    }

};
}
#endif
