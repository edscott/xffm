/*

ABOUT

An example of how to use GtkColumnView with GtkSignalListItemFactory for each
column, where each list item in each column is a GtkLabel. This is basically
a table with headers.

COMPILE

gcc `pkg-config --cflags gtk4` -o columnview_sample columnview_sample.c `pkg-config --libs gtk4` -lSM -lICE -lX11 -lXext

RUN

./columnview

*/

#include <gtk/gtk.h>
# include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
GtkWidget *MainWidget;

static void i3Dialog(GtkWidget *widget){
  GdkDisplay *displayGdk = gdk_display_get_default();
  if (GDK_IS_X11_DISPLAY (displayGdk)) {
    Display *display = gdk_x11_display_get_xdisplay(displayGdk);
    Atom atom = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE_DIALOG");
    Atom atom0 = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE");

    GtkNative *native = gtk_widget_get_native(widget);
    GdkSurface *surface = gtk_native_get_surface(native);
    Window w = gdk_x11_surface_get_xid (surface);

    XChangeProperty (display, w,
      atom0, XA_ATOM, 
      32, PropModeReplace,
      (guchar *)&atom, 1);
  }
}
static void
//factory2_setup( GtkListItemFactory *factory, GtkListItem *list_item )
factory2_setup(GtkSignalListItemFactory *self, GObject *object, void *data){
	GtkWidget *label = gtk_label_new( "" );
  GtkListItem *list_item = GTK_LIST_ITEM(object);
	gtk_list_item_set_child(list_item, label);
}

/* The bind function for the factory */
static void
//factory2_bind( GtkListItemFactory *factory, GtkListItem *list_item )
factory2_bind(GtkSignalListItemFactory *self, GObject *object, void *data)
{
	GtkWidget *label;
  GtkStringObject *string_object;
	const char *primary_key;

  GtkListItem *list_item =GTK_LIST_ITEM(object);
	label = gtk_list_item_get_child( list_item );

  GFileInfo *info = G_FILE_INFO(gtk_list_item_get_item(list_item));
  char *name = g_strdup(g_file_info_get_name(info));
  if (name && strlen(name) > 15){
    name[15] = 0;
    name[14] ='~';
  }
	gtk_label_set_label( GTK_LABEL( label ), name );
  g_free(name);
}

static void
activate2 (GtkWidget *self, gpointer data)
{
	GtkWidget *view;

	GtkColumnViewColumn* column;

	char *column_names[] = { "Name", "Path", "Type" };

	// Create the application window.
	
	gtk_window_set_title( GTK_WINDOW(MainWidget), "Window" );
	gtk_window_set_default_size( GTK_WINDOW(MainWidget), 400, 400 );
  GtkScrolledWindow *scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
	gtk_window_set_child( GTK_WINDOW(MainWidget), GTK_WIDGET(scrolledWindow) );
  

  GFile *gfile = g_file_new_for_path(g_get_home_dir());
  GtkDirectoryList *dList = gtk_directory_list_new(NULL, gfile); // G_LIST_MODEL
  GtkNoSelection *selection_model = gtk_no_selection_new(G_LIST_MODEL(dList));
        /*while (gtk_directory_list_is_loading(dList)) {
        }
        int num = g_list_model_get_n_items(G_LIST_MODEL(dList));
        fprintf(stderr, "gtk_directory_list_is_loading done: items=%d\n", num);*/
 
  // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
  // bind, setup, teardown and unbind
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

	/* Connect handler to the factory.
	 */
	g_signal_connect( factory, "setup", G_CALLBACK(factory2_setup), NULL );
	g_signal_connect( factory, "bind", G_CALLBACK(factory2_bind), column_names[0] );

	/* Create the column view.
	 */
	view = gtk_column_view_new( GTK_SELECTION_MODEL( selection_model ) );
  gtk_scrolled_window_set_child(scrolledWindow, GTK_WIDGET(view));
	
	/* Create the columns.
	 */
	for ( int i = 0; i < 1; i++ ) {
		column = gtk_column_view_column_new( column_names[i], factory );
		gtk_column_view_append_column( GTK_COLUMN_VIEW( view ), column );
	}

}

     GdkTexture *load(const char *item){
        GError *error_ = NULL;
        if (!item) return NULL;
        if (g_file_test(item, G_FILE_TEST_EXISTS)) {
          // both absolute and relative here
          // FIXME: run in thread
          GdkTexture *texture = gdk_texture_new_from_filename(item, &error_);
          if (error_){
            fprintf(stderr, "Texture::load(): %s\n", error_->message);
            return NULL;
          }
          return texture;
        }
        // From iconname.
      }

GdkTexture *texture;
GdkPaintable *tex2;
GtkSnapshot *snapshot;
static void
factory3_setup(GtkSignalListItemFactory *self, GObject *object, void *data){
  texture = load("/usr/share/icons/Adwaita/scalable/mimetypes/application-certificate.svg");
  snapshot = gtk_snapshot_new();
  gdk_paintable_snapshot(GDK_PAINTABLE(texture), snapshot, 48., 48.);
  graphene_size_t size;
  size.width = size.height = 48.0;
  tex2 = gtk_snapshot_free_to_paintable(snapshot, &size);

	GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *label = gtk_label_new( "" );

  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
  
//  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));

//  GtkWidget *image = gtk_image_new_from_icon_name("text-x-generic");
  //gtk_image_set_icon_size (GTK_IMAGE(image),  GTK_ICON_SIZE_LARGE);

  //gtk_widget_add_css_class(image, "large-icons"); //normal-icons
   
  gtk_box_append(GTK_BOX(vbox), image);
  gtk_widget_set_size_request(image, 96,96);
  gtk_box_append(GTK_BOX(vbox), label);
/*  gtk_widget_set_halign (GTK_WIDGET(vbox),GTK_ALIGN_FILL);
  gtk_widget_set_valign (GTK_WIDGET(image),GTK_ALIGN_FILL);
  gtk_widget_set_halign (GTK_WIDGET(image),GTK_ALIGN_FILL);
  gtk_widget_set_valign (GTK_WIDGET(image),GTK_ALIGN_FILL);
  gtk_widget_set_halign (label,GTK_ALIGN_FILL);
  gtk_widget_set_valign (label,GTK_ALIGN_FILL);
  gtk_widget_set_hexpand(GTK_WIDGET(image), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(image), TRUE);
  gtk_widget_set_hexpand(GTK_WIDGET(label), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(label), TRUE);
  gtk_widget_set_margin_top(GTK_WIDGET(label), 0);
  gtk_widget_set_margin_bottom(GTK_WIDGET(label), 0);*/

  g_object_set_data(G_OBJECT(vbox),"label", label);

  GtkListItem *list_item = GTK_LIST_ITEM(object);
	gtk_list_item_set_child(list_item, vbox);
}

/* The bind function for the factory */
static void
factory3_bind(GtkSignalListItemFactory *self, GObject *object, void *data)
{
  GtkListItem *list_item =GTK_LIST_ITEM(object);
	GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
	GtkLabel *label = g_object_get_data(G_OBJECT(vbox), "label");

  GFileInfo *info = G_FILE_INFO(gtk_list_item_get_item(list_item));
  char *name = g_strdup(g_file_info_get_name(info));
  if (name && strlen(name) > 15){
    name[15] = 0;
    name[14] ='~';
  }
  char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
	gtk_label_set_markup( GTK_LABEL( label ), markup );
  g_free(name);
  g_free(markup);
}


static void
activate3 (GtkWidget *self, gpointer data)
{

	// Create the application window.
	
	gtk_window_set_title( GTK_WINDOW(MainWidget), "Window" );
	gtk_window_set_default_size( GTK_WINDOW(MainWidget), 400, 400 );
  GtkScrolledWindow *scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
	gtk_window_set_child( GTK_WINDOW(MainWidget), GTK_WIDGET(scrolledWindow) );
  

  GFile *gfile = g_file_new_for_path(g_get_home_dir());
  GtkDirectoryList *dList = gtk_directory_list_new(NULL, gfile); // G_LIST_MODEL
  GtkNoSelection *selection_model = gtk_no_selection_new(G_LIST_MODEL(dList));
        /*while (gtk_directory_list_is_loading(dList)) {
        }
        int num = g_list_model_get_n_items(G_LIST_MODEL(dList));
        fprintf(stderr, "gtk_directory_list_is_loading done: items=%d\n", num);*/
 
  // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
  // bind, setup, teardown and unbind
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new();

	/* Connect handler to the factory.
	 */
	g_signal_connect( factory, "setup", G_CALLBACK(factory3_setup), NULL );
	g_signal_connect( factory, "bind", G_CALLBACK(factory3_bind), NULL);

	GtkWidget *view;
	/* Create the view.
	 */
	view = gtk_grid_view_new( GTK_SELECTION_MODEL( selection_model ), factory );
  gtk_scrolled_window_set_child(scrolledWindow, GTK_WIDGET(view));
	

}





/* We could play the same game with the setup function, to produce a custom
 * widget type for the items in each column, but for now we'll just use a
 * GtkLabel for all items in the view.
 */
static void
factory_setup( GtkListItemFactory *factory, GtkListItem *list_item )
{
	GtkWidget *label = gtk_label_new( "" );
	gtk_list_item_set_child( list_item, label );
}

/* The bind function for the primary key factory, corresponding to the first
 * column.
 */
static void
primary_key_factory_bind( GtkListItemFactory *factory, GtkListItem *list_item )
{
	GtkWidget *label;
GtkStringObject *string_object;
	const char *primary_key;

	label = gtk_list_item_get_child( list_item );
	string_object = GTK_STRING_OBJECT( gtk_list_item_get_item( list_item ) );
	primary_key = gtk_string_object_get_string( string_object );

	gtk_label_set_label( GTK_LABEL( label ), primary_key );
}

/* The bind function for the other factories, corresponding to columns after
 * the primary key column.
 */
static void
value_factory_bind( GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data )
{
	GtkWidget *label;
	GtkStringObject *string_object;
	const char *primary_key, *value, *column_name = (const char *) user_data;

	label = gtk_list_item_get_child( list_item );
	string_object = GTK_STRING_OBJECT( gtk_list_item_get_item( list_item ) );
	primary_key = gtk_string_object_get_string( string_object );

	// In a real application, do some complicated lookup here to get value.
	// For this example, just concatenate the primary key and the column
	// name that was passed as user_data.
	value = g_strdup_printf( "%s-%s", column_name, primary_key );

	gtk_label_set_label( GTK_LABEL( label ), value );
}

/* Callback that starts the application.
 */
static void
activate (GtkWidget *self, gpointer data)
{
	GtkWidget *view;

	GtkColumnViewColumn* column;

	char *column_names[] = { "PrimaryKey", "Apple", "Banana" };

	/* Create the application window.
	 */
	gtk_window_set_title( GTK_WINDOW(MainWidget), "Window" );
	gtk_window_set_default_size( GTK_WINDOW(MainWidget), 200, 200 );

 	/* Define the list items of the first column. In practice, these will be the
	 * unique string keys identifying a row, and showing the primary key column
	 * will be optional ( simply skip it when adding columns ).
	 */
	char* primary_keys[] = { "0", "1", "2", NULL };

	GtkStringList *list_model = gtk_string_list_new( (const char* const*) primary_keys );

	/* Create simple selection model, which does not have any selection
	 * logic. Curiously, you can still see the mouseover highlight effect
	 * and onclick  highlight effect even for GtkNoSelection, perhaps for
	 * accessibility reasons.
	 */
	GtkNoSelection *selection_model = gtk_no_selection_new( G_LIST_MODEL( list_model ) );

	/* Initialize the array of GtkListItemFactory - one for each column.
	 */

  // GtkListItemFactory implements GtkSignalListItemFactory, which can be connected to
  // bind, setup, teardown and unbind
	GtkListItemFactory *factories[3];
	for ( int i = 0; i < 3; i++ )
		factories[i] = gtk_signal_list_item_factory_new();

	/* Connect handlers to the primary key factory.
	 */
	g_signal_connect( factories[0], "setup", G_CALLBACK( factory_setup ), NULL );
	g_signal_connect( factories[0], "bind", G_CALLBACK( primary_key_factory_bind ), column_names[0] );

	/* Connect handlers to the other factories.
	 */
	for ( int i = 1; i < 3; i++ ) {
		g_signal_connect( factories[i], "setup", G_CALLBACK( factory_setup ), NULL );
		g_signal_connect( factories[i], "bind", G_CALLBACK( value_factory_bind ), column_names[i] );
	}

	/* Create the column view.
	 */
	view = gtk_column_view_new( GTK_SELECTION_MODEL( selection_model ) );
	gtk_window_set_child( GTK_WINDOW(MainWidget), view );
	
	/* Create the columns.
	 */
	for ( int i = 0; i < 3; i++ ) {
		column = gtk_column_view_column_new( column_names[i], factories[i] );
		gtk_column_view_append_column( GTK_COLUMN_VIEW( view ), column );
	}

}

int main (int argc, char *argv[])
{
  gtk_init ();
    
  MainWidget = gtk_window_new ();
  activate3(MainWidget, NULL);
  gtk_widget_realize(MainWidget);
  i3Dialog(MainWidget);
  gtk_window_present (GTK_WINDOW(MainWidget));

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

