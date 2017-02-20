#include "tooltip_c.hpp"


tooltip_c::tooltip_c(data_c *data0){
    tooltip_text_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
    tt_window = NULL;
    tooltip_is_mapped = FALSE;
}

tooltip_c::~tooltip_c(void){
    if (tooltip_text_hash) g_hash_table_destroy(tooltip_text_hash);
    tooltip_text_hash = NULL;
}

GtkWidget * 
tooltip_c::get_tt_window(void){return tt_window;}

// Returns a new shadowed pixbuf...
GdkPixbuf *
tooltip_c::shadow_it(const GdkPixbuf *src_pixbuf){
    if (!src_pixbuf) return NULL;
    gint width = gdk_pixbuf_get_width (src_pixbuf);
    gint height = gdk_pixbuf_get_height (src_pixbuf);  

    gint offset = 7;

    GdkPixbuf *shadowed =  gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
	    width + offset, height + offset);
    if (!shadowed) return NULL;

    gdk_pixbuf_fill(shadowed, 0x0);
    gdk_pixbuf_copy_area (src_pixbuf, 0, 0, width, height, shadowed, offset, offset);
    gdk_pixbuf_saturate_and_pixelate (shadowed, shadowed, 0.0, TRUE);
                                   // gfloat saturation, gboolean pixelate);
    gdk_pixbuf_composite (src_pixbuf, shadowed,
                          0, 0,  //dest_x, dest_y,
                          width, height,  //dest_width, dest_height,
			  0, 0,  //offset_x, offset_y,
			  1.0, 1.0, //scale_x, scale_y,
			  GDK_INTERP_NEAREST, 
			  255);   //overall_alpha);    
                
    return shadowed;
}

// sets cool tooltip gradient... 3.20 at least. for 3.16, method in librfm
void 
tooltip_c::set_box_gradient(GtkWidget *wbox){
    
    GtkStyleContext *style_context = gtk_widget_get_style_context (wbox);
    gtk_style_context_add_class(style_context, GTK_STYLE_CLASS_TOOLTIP );
     

    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error=NULL;
    gtk_css_provider_load_from_data (css_provider, 
"\
box * {\
  background-image: -gtk-gradient (linear, left top, right top, from (#aaa), to (#000));\
  color: rgb(255, 255, 255);\
  border-width: 0px;\
  border-radius: 0px;\
  border-color: transparent;\
}\
", 
        -1, &error);
    if (error){
        fprintf(stderr, "gerror: %s\n", error->message);
        g_error_free(error);
    }
    gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER(css_provider),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    return;
}

void 
tooltip_c::tooltip_placement_bug_workaround(GtkWidget *tooltip_window){
#if GTK_MAJOR_VERSION==3
#if GTK_MINOR_VERSION>=8
    // gtk3.8 bug workaround (still in current 3.13):
    static gint last_x = 0;
    static gint last_y = 0;
    
    GdkScreen *screen = gtk_widget_get_screen (tooltip_window);
    gint monitor_num = gdk_screen_get_monitor_at_point (screen,
                                                 last_x,
                                                 last_y);
    GdkRectangle monitor;
    gdk_screen_get_monitor_workarea (screen, monitor_num, &monitor);
    last_x = monitor.width-1;
    last_y = monitor.height-1;
    gtk_window_move (GTK_WINDOW (tooltip_window), 
            last_x,last_y);
#endif
#endif
        return;
}

void
tooltip_c::reset_tooltip(void){
    //NOOP(stderr, "rfm_reset_tooltip\n"); 
    if (tt_window) {
	g_object_set_data(G_OBJECT(tt_window), "tooltip_target", NULL);
	//NOOP(stderr, "rfm_reset_tooltip OK\n"); 
    }
}

void
tooltip_c::set_tooltip_map(gboolean state){ tooltip_is_mapped = state;}

gboolean
tooltip_c::get_tooltip_map(void){ return tooltip_is_mapped;}

static void
tooltip_unmap (GtkWidget *window, gpointer data){
    tooltip_c *tooltip_p =(tooltip_c *)data;
    tooltip_p->set_tooltip_map(FALSE);
}

// This is a gtk placement bug workaround. Should probably fix gtk code and
// submit the patch: this is a long standing bug...
static void
tooltip_map (GtkWidget *window, gpointer data){
    tooltip_c *tooltip_p =(tooltip_c *)data;
    tooltip_p->set_tooltip_map(TRUE);
}



GtkWidget *
tooltip_c::get_tt_window(const GdkPixbuf *pixbuf, const gchar *markup, const gchar *label_text){
    if (!tt_window) {
        tt_window = gtk_window_new(GTK_WINDOW_POPUP);
        //NOOP(stderr, "New tooltip window now...\n");
        g_signal_connect (G_OBJECT (tt_window), "map", G_CALLBACK (tooltip_map), (void *)this);
        g_signal_connect (G_OBJECT (tt_window), "unmap", G_CALLBACK (tooltip_unmap), (void *)this);
        gtk_window_set_type_hint (GTK_WINDOW (tt_window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
        gtk_widget_set_app_paintable (tt_window, TRUE);
        gtk_window_set_resizable (GTK_WINDOW (tt_window), FALSE);
        gtk_widget_set_name (tt_window, "gtk-tooltip");  
    } else {
        GtkWidget *old_content = 
            gtk_bin_get_child(GTK_BIN(tt_window));
        gtk_container_remove(GTK_CONTAINER(tt_window), old_content);
    }

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_show(vbox);

    GtkWidget *top_frame = gtk_frame_new(NULL);
    gtk_widget_show(top_frame);
    gtk_container_add (GTK_CONTAINER (tt_window), top_frame);
    gtk_container_add (GTK_CONTAINER (top_frame), vbox);

    GtkWidget *wbox  = gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (vbox), wbox);
    gtk_widget_show(wbox);
   
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_show(frame);
    if (label_text) {
	GtkWidget *label = gtk_label_new("");
	gtk_widget_show(label);
	gchar *utf_text =  utf_string (label_text);
	gchar *label_markup;
        label_markup = 
            g_strdup_printf("<span color=\"yellow\" font_family=\"monospace\" weight=\"bold\"> %s </span>",utf_text); 
	gtk_label_set_markup(GTK_LABEL(label), label_markup);
	g_free(utf_text);
	g_free(label_markup);
	gtk_frame_set_label_widget(GTK_FRAME(frame), label);
    }
    gtk_container_add (GTK_CONTAINER (wbox), frame);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_show(box);
    gtk_container_add (GTK_CONTAINER (frame), box);

    GtkWidget *tip_image = NULL;
    if (pixbuf){
	if (label_text) {
	    GdkPixbuf *shadowed = shadow_it(pixbuf);
	    if (shadowed) {
		tip_image = gtk_image_new_from_pixbuf ((GdkPixbuf *)pixbuf);
		g_object_unref(shadowed);
	    } else {
		tip_image = gtk_image_new_from_pixbuf ((GdkPixbuf *)pixbuf);
	    }
	} else {
	    tip_image = gtk_image_new_from_pixbuf ((GdkPixbuf *)pixbuf);
	}
	gtk_box_pack_start(GTK_BOX(box),tip_image, FALSE, FALSE,0);
	gtk_widget_show(tip_image);
    }
    if (markup) {
	GtkWidget *label = gtk_label_new("");	
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gchar *small = g_strdup_printf("<span  color=\"white\" size=\"smaller\"> %s </span>", markup);
        gtk_label_set_markup(GTK_LABEL(label), small);
        g_free(small);

	gtk_box_pack_start(GTK_BOX(box),label,TRUE,TRUE,0);
	gtk_widget_show(label);
    }
                                                      
    gtk_widget_show(box);
    g_object_set_data(G_OBJECT(tt_window), "box", box); 
    g_object_set_data(G_OBJECT(tt_window), "image", tip_image); 
    g_object_set_data(G_OBJECT(tt_window), "pixbuf", (void *)pixbuf); 
    gint width = 0;
    gint height = 0;
    if (pixbuf) {
        width = gdk_pixbuf_get_width(pixbuf);
        height = gdk_pixbuf_get_height(pixbuf);
    }
    g_object_set_data(G_OBJECT(tt_window), "width", GINT_TO_POINTER(width)); 
    g_object_set_data(G_OBJECT(tt_window), "height", GINT_TO_POINTER(height)); 

    //gtk_widget_set_tt_window (widget, GTK_WINDOW(tt_window));
    gtk_widget_realize(tt_window);
    set_box_gradient(wbox);
    tooltip_placement_bug_workaround(tt_window);
    return tt_window;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static gboolean
widget_tooltip_function(
        GtkWidget * widget, 
        gint x, 
        gint y, 
        gboolean keyboard_mode, 
        GtkTooltip * tooltip,
        gpointer user_data
) {
    tooltip_c *tooltip_p = (tooltip_c *)user_data;
    GtkWidget *tt_window = tooltip_p->get_tt_window();
    gchar *tooltip_text = (gchar *)g_object_get_data(G_OBJECT(widget), "tooltip_text");
    if (tt_window) {
	GtkWidget *tooltip_target = (GtkWidget *)
	    g_object_get_data(G_OBJECT(tt_window), "tooltip_target");

        //fprintf(stderr, "tooltip target,widget %p,%p\"\n", tooltip_target, widget); 

        if (tooltip_target == widget) return TRUE;
    } else {
        fprintf(stderr, "New tooltip window\"%s\"\n", tooltip_text); 
    }

    GdkPixbuf *tooltip_pixbuf = (GdkPixbuf *)g_object_get_data(G_OBJECT(widget), "tooltip_pixbuf");
    gchar *label_text = NULL;

    if (tooltip_text){
	if (strchr(tooltip_text, '\n')) {
	    label_text = g_strdup(tooltip_text);
	    *(strchr(label_text, '\n')) = 0;
	    tooltip_text = strchr(tooltip_text, '\n') + 1;
	}
    }
    tt_window = tooltip_p->get_tt_window(tooltip_pixbuf, tooltip_text, label_text);

    gtk_widget_set_tooltip_window (widget, GTK_WINDOW(tt_window));

    g_object_set_data(G_OBJECT(tt_window), "tooltip_target", widget);

    g_free(label_text);
    return TRUE;
}



static void
destroy_widget(GtkWidget *button, void *data){
    tooltip_c *tooltip_p = (tooltip_c *)data;
    GdkPixbuf *tooltip_pixbuf = (GdkPixbuf *)
        g_object_get_data(G_OBJECT(button), "tooltip_pixbuf");
    if (!tooltip_p->get_tooltip_text_hash()) DBG("destroy_widget: hash is null!\n");
    gchar *tooltip_text =
        (gchar *)g_hash_table_lookup(tooltip_p->get_tooltip_text_hash(), button);
    if (tooltip_text) {
        // The free is done by removing item from hash table:
        g_hash_table_remove(tooltip_p->get_tooltip_text_hash(), button);
    }

    if (tooltip_pixbuf) g_object_unref(tooltip_pixbuf);
    g_object_set_data(G_OBJECT(button), "tooltip_text", NULL);
    g_object_set_data(G_OBJECT(button), "tooltip_pixbuf", NULL);
}



static void *
custom_tooltip_f(void * data){
    void **arg=(void **)data;
    GtkWidget *widget = (GtkWidget *)arg[0];
    GdkPixbuf *pixbuf = (GdkPixbuf *)arg[1];
    const gchar *text = (const gchar *)arg[2];
    void *object = arg[3];
    tooltip_c *tooltip_p = (tooltip_c *)object;

    fprintf(stderr, "custom_tooltip_f for %s\n", text);

    gchar *t = g_strdup(text);
    g_object_set_data(G_OBJECT(widget), "tooltip_text", t);
    if (!tooltip_p->get_tooltip_text_hash()) DBG("custom_tooltip_f: hash is null!\n");
    g_hash_table_replace(tooltip_p->get_tooltip_text_hash(), widget, t);
    g_object_set_data(G_OBJECT(widget), "tooltip_pixbuf", pixbuf);

    gtk_widget_set_has_tooltip(widget, TRUE);
    g_signal_connect (G_OBJECT (widget), "destroy",
	    G_CALLBACK (destroy_widget), object);
    g_signal_connect (G_OBJECT (widget), "query-tooltip",
	    G_CALLBACK (widget_tooltip_function), object);
    return NULL;
}

void tooltip_c::custom_tooltip(GtkWidget *widget, GdkPixbuf *pixbuf, const gchar *text){
    void *arg[]={widget, pixbuf, (void *)text, (void *)this};
    context_function(custom_tooltip_f, arg);
}

GHashTable *
tooltip_c::get_tooltip_text_hash(void){ return tooltip_text_hash;}
