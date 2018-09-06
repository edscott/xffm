#ifndef PATHBAR_HH
#define PATHBAR_HH
#include "common/gtk.hh"
#include "common/util.hh"
namespace xf {

template <class Type> 
class Pathbar
{
    using gtk_c = Gtk<double>;
    using util_c = Util<double>;
private:
        //void *window_p;
	GtkWidget *pathbar_;
        //void *view_p;
    
public:
    Pathbar(void) {
	pathbar_ = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_set_data(G_OBJECT(pathbar_), "callback", (void *)pathbar_go);
	GtkButton *pb_button = pathbar_button( NULL, ".");

	gtk_box_pack_start (GTK_BOX (pathbar_), GTK_WIDGET(pb_button), FALSE, FALSE, 0);
	g_object_set_data(G_OBJECT(pb_button), "name", g_strdup("RFM_ROOT"));
	g_signal_connect (G_OBJECT(pb_button) , "clicked", BUTTON_CALLBACK (pathbar_go), (void *)this);
	gtk_widget_show(GTK_WIDGET(pb_button));
    }

    GtkWidget *
    get_pathbar(void){ return pathbar_;}


    GtkButton *
    pathbar_button (const char *icon_id, const char *text) {
	GtkButton  *pb_button = GTK_BUTTON( gtk_button_new ());

	g_object_set (G_OBJECT(pb_button), 
		"can-focus", FALSE, 
		"relief", GTK_RELIEF_NONE, 
		NULL);
	g_object_set_data(G_OBJECT(pb_button), "name", text?g_strdup(text):NULL); 
	gchar *markup = NULL;
	if (text) {
	    gchar *v = util_c::utf_string(text);
	    gchar *g = g_markup_escape_text(v, -1);
	    g_free(v);
	    markup = g_strdup_printf("<span size=\"x-small\">%s</span>", g);
	    g_free(g);
	}
	gtk_c::set_bin_contents(GTK_WIDGET(pb_button), icon_id, markup, 12);
	g_free(markup);
	return pb_button;
    }


    void 
    update_pathbar(const gchar *path){
	TRACE( "update pathbar to %s\n", path);
	void *arg[]={(void *)this, (void *)(path?g_strdup(path):NULL)};
	util_c::context_function(update_pathbar_f, arg);
    }

    void 
    pathbar_ok(GtkButton * button){
	GList *children_list = gtk_container_get_children(GTK_CONTAINER(pathbar_));
	GList *children = children_list;
	for (;children && children->data; children=children->next){
	    if (button == children->data){
		const gchar *path = (gchar *)g_object_get_data(G_OBJECT(button), "path");
		if (!path) path = "xffm:root";
		DBG("FIXME: send cd instruction: %s\n", path);
		/*
		view_c *view_p = (view_c *)g_object_get_data(G_OBJECT(pathbar_), "view_p");
		if (!view_p) g_error("view_p data not set for g_object pathbar!\n");
		DBG("pathbar_ok: path=%s\n", path);
		view_p->reload(path);*/
	    } 
	}
    }

    void 
    toggle_pathbar(const gchar *path){
	GList *children_list = 
	    gtk_container_get_children(GTK_CONTAINER(pathbar_));
	GList *children;
	    
	GtkRequisition minimum;
	GtkAllocation allocation;
	//gtk_widget_get_allocation(pathbar_, &allocation);
	gtk_widget_get_allocation(gtk_widget_get_toplevel(pathbar_), &allocation);
	DBG("pathbar width=%d\n", allocation.width);
	gint width = allocation.width;

	// First we hide all buttons, except "RFM_ROOT"
	children = g_list_last(children_list);
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
	gtk_widget_show(GTK_WIDGET(active->data));

	gtk_widget_get_preferred_size(GTK_WIDGET(active->data), &minimum, NULL);
	    DBG("#### width, minimum.width %d %d\n",width,  minimum.width);
	width -= minimum.width;
     
	// Work backwards from active button we show buttons that will fit.
	children = active->prev;
	for (;children && children->data; children=children->prev){
	    gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) continue;
	    gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
	    DBG("#### width, allocaltion.width %d %d\n",width,  allocation.width);
	    width -= allocation.width;
	    if (width < 0) break;
	    gtk_widget_show(GTK_WIDGET(children->data));
	}

	// Now we work forwards, showing buttons that fit.
	children = active->next;
	for (;children && children->data; children=children->next){
	   gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) continue;
	    gtk_widget_get_allocation(GTK_WIDGET(children->data), &allocation);
	    width -= allocation.width;
	    if (width < 0) break;
	    gtk_widget_show(GTK_WIDGET(children->data));
	}

	// Finally, we differentiate active button.
	children = g_list_first(children_list);
	for (;children && children->data; children=children->next){
	    gchar *name = (gchar *)g_object_get_data(G_OBJECT(children->data), "name");
	    if (strcmp(name, "RFM_ROOT")==0) continue;
	    if (!path) {
		// no path means none is differentiated.
		gchar *v = util_c::utf_string(name);
		gchar *g = g_markup_escape_text(v, -1);
		g_free(v);
		gchar *markup = g_strdup_printf("<span size=\"x-small\" color=\"blue\" bgcolor=\"#dcdad5\">%s</span>", g);
		gtk_c::set_bin_markup(GTK_WIDGET(children->data), markup);
		g_free(g);
		g_free(markup);
		continue;
	    } 
	    const gchar *pb_path = 
		(const gchar *)g_object_get_data(G_OBJECT(children->data), "path");
	    if (!pb_path){
		g_warning("rfm_update_pathbar(): pb_path is null\n");
		continue;
	    }
	    if (!strlen(pb_path)) pb_path=G_DIR_SEPARATOR_S;//?
	    if (strcmp(pb_path, path)==0) {
		gchar *v = util_c::utf_string(name);
		gchar *g = g_markup_escape_text(v, -1);
		g_free(v);
		gchar *markup = g_strdup_printf("<span size=\"x-small\" color=\"red\"bgcolor=\"#dcdad5\">%s</span>", g);
		gtk_c::set_bin_markup(GTK_WIDGET(children->data), markup);
		g_free(g);
		g_free(markup);
	    }
	    else {
		gchar *v = util_c::utf_string(name);
		gchar *g = g_markup_escape_text(v, -1);
		g_free(v);
		gchar *markup = g_strdup_printf("<span size=\"x-small\" color=\"blue\"bgcolor=\"#dcdad5\">%s</span>", g);
		gtk_c::set_bin_markup(GTK_WIDGET(children->data), markup);
		g_free(g);
		g_free(markup);
	    }
	}
	g_list_free(children_list);
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////

    static void
    pathbar_go(GtkButton * button, gpointer data){
	Pathbar *pathbar_p = (Pathbar *)data;
	pathbar_p->pathbar_ok(button);

    }

    static void *
    update_pathbar_f(void *data){
	void **arg = (void **)data;
	Pathbar *pathbar_p = (Pathbar *)arg[0];
	gchar *path = (gchar *)arg[1];
	GtkWidget *pathbar = pathbar_p->get_pathbar();
	DBG( "update_pathbar_f:: %s\n", path);

	if (!pathbar) return NULL;
	if (!path){
	    TRACE("##### toggle_pathbar(pathbar, NULL)\n");
	    pathbar_p->toggle_pathbar(NULL);
	    return NULL;
	}

	// Trim pathbar.
	gchar **paths;
	if (strcmp(path, G_DIR_SEPARATOR_S)==0){
	    paths = (gchar **)malloc(2*sizeof(gchar *));
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
	    if (strcmp(name, "RFM_ROOT")==0 || strcmp(name, "<")==0) continue;
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
		g_free(name);
		gtk_container_remove(GTK_CONTAINER(pathbar), GTK_WIDGET(tail->data));
	    }
	    break;
	}
	g_list_free(children_list);

	// Add new tail
	gpointer callback = (gpointer)g_object_get_data(G_OBJECT(pathbar), "callback");
	if (strcmp(path, "RFM_MODULE")) for (;paths[i]; i++){
	    GtkButton *pb_button = pathbar_p->pathbar_button(NULL, 
		    strlen(paths[i])?paths[i]:G_DIR_SEPARATOR_S);
	    gtk_container_add(GTK_CONTAINER(pathbar), GTK_WIDGET(pb_button));

	    gchar *g = (pb_path!=NULL)?
		g_strdup_printf("%s%s%s",pb_path, 
			strcmp(pb_path,G_DIR_SEPARATOR_S)? 
			G_DIR_SEPARATOR_S:"", paths[i]):
		g_strdup(paths[i]);
	    g_free(pb_path);
	    pb_path = g;
	    DBG( "+++***** setting pbpath --> %s\n", pb_path);
	    g_object_set_data(G_OBJECT(pb_button), "path", g_strdup(pb_path));
	    g_signal_connect (G_OBJECT(pb_button) , "clicked", G_CALLBACK (callback), (void *)pathbar_p);
	    
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
