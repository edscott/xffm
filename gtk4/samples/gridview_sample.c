/* Example of GtkGridView (GTK4) creation with filter and sort.
 * Compile with:
gcc `pkg-config --cflags gtk4` -o gridview_sample gridview_sample.c `pkg-config --libs gtk4` 
*/
#include <gtk/gtk.h>
static GtkIconTheme *icon_theme;
static void
factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
	GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *label = gtk_label_new( "" );
  gtk_box_append(GTK_BOX(vbox), imageBox);
  gtk_box_append(GTK_BOX(vbox), label);

  g_object_set_data(G_OBJECT(vbox),"label", label);
  g_object_set_data(G_OBJECT(vbox),"imageBox", imageBox);

  GtkListItem *list_item = GTK_LIST_ITEM(object);
	gtk_list_item_set_child(list_item, vbox);
}
// The bind function for the factory 
static void
factoryBind(GtkSignalListItemFactory *self, GObject *object, void *data)
{
  GtkListItem *list_item =GTK_LIST_ITEM(object);
	GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
  GtkWidget *imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(vbox), "imageBox"));
	GtkLabel *label = g_object_get_data(G_OBJECT(vbox), "label");
  GFileInfo *info = G_FILE_INFO(gtk_list_item_get_item(list_item));
  char *name = g_strdup(g_file_info_get_name(info));
  if (name && strlen(name) > 15){
    name[15] = 0;
    name[14] ='~';
  }
  char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
	gtk_label_set_markup( GTK_LABEL(label), markup );
  g_free(name);
  g_free(markup);
  GtkWidget *w = gtk_widget_get_first_child (imageBox);
  if (w) gtk_widget_unparent(w);
  GtkIconPaintable *icon = gtk_icon_theme_lookup_icon (icon_theme,
               "application-x-generic", NULL, 48, 1,    
               GTK_TEXT_DIR_NONE, 
               (GtkIconLookupFlags) 0);
  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(icon));
  gtk_widget_set_size_request(image, 48, 48);
  gtk_box_append(GTK_BOX(imageBox), image);
}
static gboolean
filterFunction(GObject *object, void *data){
  GFileInfo *info = G_FILE_INFO(object);
  return !g_file_info_get_is_hidden(info);
}
static gint
compareFunction(const void *a, const void *b, void *data){
  GFileInfo *infoA = G_FILE_INFO(a);
  GFileInfo *infoB = G_FILE_INFO(b);
  return strcasecmp(g_file_info_get_name(infoA), g_file_info_get_name(infoB));
}
static void
create (GtkWidget *MainWidget){
  GtkScrolledWindow *scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
	gtk_window_set_child( GTK_WINDOW(MainWidget), GTK_WIDGET(scrolledWindow) );
  // Create the initial GtkDirectoryList.
  GFile *gfile = g_file_new_for_path(g_get_home_dir());
  GtkDirectoryList *dList = gtk_directory_list_new("standard::is-hidden,standard::type,standard::file", gfile); 
  // Chain link GtkDirectoryList to a GtkFilterListModel.
  GtkFilter *filter = 
    GTK_FILTER(gtk_custom_filter_new ((GtkCustomFilterFunc)filterFunction, NULL, NULL));
  GtkFilterListModel *filterModel = gtk_filter_list_model_new(G_LIST_MODEL(dList), filter);
  // Chain link GtkFilterListModel to a GtkSortListModel.
  GtkSorter *sorter = 
    GTK_SORTER(gtk_custom_sorter_new((GCompareDataFunc)compareFunction, NULL, NULL));
  GtkSortListModel *sortModel = gtk_sort_list_model_new(G_LIST_MODEL(filterModel), sorter);
  // Chain link GtkSortListModel to a GtkMultiSelection.
  GtkMultiSelection *selection_model = gtk_multi_selection_new(G_LIST_MODEL(sortModel));
  // Create the GtkListItemFactory and connect handlers.
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
	g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), NULL );
	g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), NULL);
	// Create the GtkGridView.
	GtkWidget *view = gtk_grid_view_new( GTK_SELECTION_MODEL( selection_model ), factory );
  gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), TRUE);
  gtk_scrolled_window_set_child(scrolledWindow, GTK_WIDGET(view));
}
int main (int argc, char *argv[])
{
  gtk_init ();
  GdkDisplay *displayGdk = gdk_display_get_default();
  icon_theme = gtk_icon_theme_get_for_display(displayGdk);
  GtkWidget *MainWidget = gtk_window_new ();
	gtk_window_set_default_size( GTK_WINDOW(MainWidget), 400, 400 );
  create(MainWidget);
  gtk_window_present (GTK_WINDOW(MainWidget));
  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);
  return 0;
}
