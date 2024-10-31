/* Example of GtkGridView (GTK4) creation with filter and sort.
 * Compile for X11 with:
 * gcc -ggdb `pkg-config --cflags gtk4` dnd_sample.c -o dnd_sample `pkg-config --libs gtk4` -lSM -lICE -lX11 -lXext
 * otherwise:
 * gcc -ggdb `pkg-config --cflags gtk4` dnd_sample.c -o dnd_sample `pkg-config --libs gtk4` 
*/

// Child sample of gridview_sample.c
// 
#define DIRECTORY "/home/tmp"
//#define DIRECTORY g_get_home_dir()

#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
# include <gdk/x11/gdkx.h>
# include <X11/Xlib.h>
# include <X11/Xatom.h>
#endif

GtkWidget *view;
static GtkEventController *dropController; // bug workaround, does not work.
static GtkIconTheme *icon_theme;
///  drag ////
//
static GList *getSelectionList(GtkSelectionModel *selection_model){
  GList *selectionList = NULL;
  guint position;
  GtkBitsetIter iter;
  GtkBitset *bitset = gtk_selection_model_get_selection (GTK_SELECTION_MODEL(selection_model));
  if (gtk_bitset_iter_init_first (&iter, bitset, &position)){
    GListModel *list = G_LIST_MODEL(selection_model);        
    do {
      GFileInfo *info = G_FILE_INFO(g_list_model_get_item (list, position));
      selectionList = g_list_append(selectionList, info);
    } while (gtk_bitset_iter_next (&iter,&position));
  }
  return selectionList;
}

static void
image_drag_begin (GtkDragSource *source,
                    GdkDrag       *drag,
                    GtkWidget     *widget)
{
  GdkPaintable *paintable;
  paintable = gtk_widget_paintable_new (widget);
  gtk_drag_source_set_icon (source, paintable, 0, 0);
  g_object_unref (paintable);
  fprintf(stderr,"image_drag_begin.\n");
}
GdkContentProvider*
image_drag_prepare (
  GtkDragSource* self,
  gdouble x,
  gdouble y,
  gpointer data
){
  GdkContentProvider *dndcontent;
  GtkSelectionModel *selection_model = g_object_get_data(G_OBJECT(data), "selection_model");
  GList *selection_list = getSelectionList(selection_model);
  char *string = g_strdup("");
  if (g_list_length(selection_list) < 2){
    GFileInfo *info = g_object_get_data(G_OBJECT(data), "info");
    GFile *file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
    char *path = g_file_get_path(file);
    char *g = g_strconcat(string, "file://", path, "\n", NULL);
    g_free(string);
    string = g;
    g_free(path);
  } else {
    for (GList *l = selection_list; l && l->data; l=l->next){
      GFileInfo *info = l->data;
      GFile *file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
      char *path = g_file_get_path(file);
      char *g = g_strconcat(string, "file://", path, "\n", NULL);
      g_free(string);
      string = g;
      g_free(path);     
    }

  }

//  dndcontent = gdk_content_provider_new_typed (G_TYPE_STRING, string);
  GBytes *bytes = g_bytes_new(string, strlen(string)+1);
  dndcontent = gdk_content_provider_new_for_bytes ("text/uri-list", bytes);

  g_free(string);
//  GtkDragSource *source = gtk_drag_source_new ();
//  gtk_drag_source_set_content (source, dndcontent);
  
  fprintf(stderr,"image_drag_prepare.\n");
  return dndcontent;
}


//
//
static void
dragBegin (GtkDragSource *source,
               GdkDrag *drag,
               void *data)
{
  fprintf(stderr,"dragBegin.\n");
  // Set the widget as the drag icon
 /* GdkPaintable *paintable = Texture<bool>::load("emblem-redball", 48);
  gtk_drag_source_set_icon (source, paintable, 0, 0);
  g_object_unref (paintable);*/
}
GdkContentProvider*
dragPrepare (
  GtkDragSource* self,
  gdouble x,
  gdouble y,
  gpointer user_data
){
  fprintf(stderr,"dragPrepare.\n");
}

void
dragEnd (
  GtkDragSource* self,
  GdkDrag* drag,
  gboolean delete_data,
  gpointer user_data
){
  fprintf(stderr,"dragEnd.\n");
}
gboolean
dragCancel (
  GtkDragSource* self,
  GdkDrag* drag,
  GdkDragCancelReason* reason,
  gpointer user_data
){
  fprintf(stderr,"dragCancel.\n");
}
//// drop  ////
gboolean
dropAccept (
  GtkDropTarget* self,
  GdkDrop* drop,
  gpointer user_data
)
{
  fprintf(stderr,"dropAccept.\n");
  //return false; //drop not accepted on enter
  return true; //drop accepted on enter
}


static void *readAction(void *arg){
  void **argData = (void **)arg;
  void *data = argData[1];
  GBytes *bytes = argData[2];

  gsize size;
  const void *p = g_bytes_get_data(bytes, &size);
  fprintf(stderr, "showDrop(): bytes (%d):\n%s\n", size, (const char *)p);
  g_free(arg);
}

gboolean dropDrop ( GtkDropTarget* self, GdkDrop* drop,  gdouble x, gdouble y, gpointer data);

static GtkEventController *createDropController(void){
    const char *mimeTypes[]={"text/uri-list", NULL};
    GdkContentFormats *contentFormats = gdk_content_formats_new(mimeTypes, 1);
    GdkDragAction actions =
      (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
    GtkDropTargetAsync *dropTarget = gtk_drop_target_async_new (contentFormats, actions);
    g_signal_connect (dropTarget, "accept", G_CALLBACK (dropAccept), NULL);
    g_signal_connect (dropTarget, "drop", G_CALLBACK (dropDrop), NULL);
   /* g_signal_connect (dropTarget, "drag-enter", G_CALLBACK (dropEnter), NULL);
    g_signal_connect (dropTarget, "drag-leave", G_CALLBACK (dropLeave), NULL);
    g_signal_connect (dropTarget, "drag-motion", G_CALLBACK (dropMotion), NULL);*/
    return GTK_EVENT_CONTROLLER(dropTarget);
}

static void dropReadDoneCallback(GObject *source_object, GAsyncResult *res, void *arg){
  GdkDrop *drop = g_object_get_data (source_object, "drop");
  GOutputStream *stream = G_OUTPUT_STREAM (source_object);
  GdkDragAction action = GDK_ACTION_COPY; // FIXME
  GBytes *bytes;

  if (g_output_stream_splice_finish (stream, res, NULL) >= 0){
      bytes = g_memory_output_stream_steal_as_bytes (G_MEMORY_OUTPUT_STREAM (stream));
  }

  gdk_drop_finish (drop, action);
  g_object_unref (drop);
  // replace controller: does not eliminate bug https://gitlab.gnome.org/GNOME/gtk/-/issues/3755
  //gtk_widget_remove_controller (view, dropController);
  //dropController = createDropController();
  //gtk_widget_add_controller (GTK_WIDGET (view), GTK_EVENT_CONTROLLER (dropController));
  
 // const void *p = g_bytes_get_data(bytes, &size);
 // fprintf(stderr, "dropReadDoneCallback(): bytes (%d):\n%s\n", size, (const char *)p);

  void **argData = (void **)arg;
  void *(*f)(void *) = argData[0];
  argData[2] = (void *)bytes;
  f(arg);
  /*
  pthread_t thread;
  pthread_create(&thread, NULL, f, arg);
  pthread_detach(thread);
   */
  return;
}


static void dropReadCallback(GObject *source_object, GAsyncResult *res, void *arg){
  fprintf(stderr,"dropReadCallback: do your thing.\n" );
  const char *out_mime_type;
  GError *error_ = NULL;
  GdkDrop *drop = GDK_DROP(source_object);
  GInputStream *input;
  GOutputStream *output;

  input = gdk_drop_read_finish (drop, res, &out_mime_type, &error_);
  if (error_){
      fprintf(stderr, "** Error::dropReadCallback(): %s\n", error_->message);
      gdk_drop_finish (drop, 0);
      g_error_free(error_);
      return;
  }
  if (input == NULL) {
      gdk_drop_finish (drop, 0);
      return;
  }

  output = g_memory_output_stream_new_resizable ();
  g_object_set_data (G_OBJECT (output), "drop", drop);
  g_object_ref (drop);

  g_output_stream_splice_async (output,
                                input,
                                G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE | G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
                                G_PRIORITY_DEFAULT,
                                NULL,
                                dropReadDoneCallback,
                                arg);
  g_object_unref (output);
  g_object_unref (input);
  return;
}
static void *readDrop(GdkDrop *drop, void *(*f)(void *), void *data){
  void **arg = calloc(3, sizeof(void*));
  arg[0] = (void *)f;
  arg[1] = data;
  const char *mimeTypes[]={"text/uri-list", NULL};
  gdk_drop_read_async (drop, mimeTypes, G_PRIORITY_DEFAULT, // int io_priority,
  NULL, dropReadCallback, (void *)arg);
  return NULL;
}


gboolean
dropDrop (
  GtkDropTarget* self,
  //const GValue* value,  
  GdkDrop* drop,  
  gdouble x,
  gdouble y,
  gpointer data
)
{
  fprintf(stderr,"dropDrop %lf,%lf .\n", x, y);
  readDrop(drop, readAction, data);
  return true; //drop accepted
  //return false; //drop not accepted
}
GdkDragAction
dropEnter (
  GtkDropTarget* self,
  GdkDrop* drop,
  gdouble x,
  gdouble y,
  gpointer user_data
)
{
  fprintf(stderr,"dropEnter %lf,%lf.\n", x, y);
  return GDK_ACTION_COPY;
}
void
dropLeave (
  GtkDropTarget* self,
  GdkDrop* drop,
  gpointer user_data
)
{
  fprintf(stderr,"dropLeave.\n");
}
GdkDragAction
dropMotion (
  GtkDropTarget* self,
  GdkDrop* drop,
  gdouble x,
  gdouble y,
  gpointer user_data
)
{
  //fprintf(stderr,"dropMotion %lf,%lf.\n", x, y);
  return GDK_ACTION_COPY;
}


static void setAsDialog(GtkWidget *widget, const char *Xname, const char *Xclass){
#ifdef GDK_WINDOWING_X11
  gtk_widget_realize(widget);
  GdkDisplay *displayGdk = gdk_display_get_default();
  if (GDK_IS_X11_DISPLAY (displayGdk)) {
    Display *display = gdk_x11_display_get_xdisplay(displayGdk);
    XClassHint *wm_class = (XClassHint *)calloc(1, sizeof(XClassHint));
    wm_class->res_name = g_strdup(Xname);
    wm_class->res_class = g_strdup(Xclass);

    GtkNative *native = gtk_widget_get_native(widget);
    GdkSurface *surface = gtk_native_get_surface(native);
    Window w = gdk_x11_surface_get_xid (surface);
    XSetClassHint(display, w, wm_class);

    Atom atom = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE_DIALOG");
    Atom atom0 = gdk_x11_get_xatom_by_name_for_display (displayGdk, "_NET_WM_WINDOW_TYPE");
    XChangeProperty (display, w,
      atom0, XA_ATOM, 
      32, PropModeReplace,
      (guchar *)&atom, 1);
  }
#endif
}

static gboolean showIt(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
  GtkPopover *popover = GTK_POPOVER(data);       
  gtk_popover_popup(popover);
  return TRUE;
}

static void addGesture(GtkWidget *widget, GtkPopover *popover){
  GtkGestureSingle *gesture = GTK_GESTURE_SINGLE(gtk_gesture_click_new());
  gtk_gesture_single_set_button(gesture,3);
  g_signal_connect (G_OBJECT(gesture) , "pressed", G_CALLBACK (showIt), (void *)popover);
  gtk_widget_add_controller(GTK_WIDGET(widget), GTK_EVENT_CONTROLLER(gesture));
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(gesture), 
      GTK_PHASE_CAPTURE);
}

static void factoryTeardown(GtkSignalListItemFactory *self, GObject *object, void *data){
  //fprintf(stderr, "factoryTeardown...\n"); 
  GtkListItem *list_item = GTK_LIST_ITEM(object);
	GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
	gtk_list_item_set_child(list_item, NULL);
  gtk_widget_unparent(GTK_WIDGET(vbox));
}
static void
factorySetup(GtkSignalListItemFactory *self, GObject *object, void *data){
  GtkListItem *list_item = GTK_LIST_ITEM(object);
  // Here we still do not have the list populated with GFileInfos.
  GFileInfo *info = G_FILE_INFO(gtk_list_item_get_item(list_item));
  //fprintf(stderr, "factorySetup..object=%p data=%p, info=%p\n",object, data);
	GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *imageBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *label = gtk_label_new( "" );
  gtk_box_append(GTK_BOX(vbox), imageBox);
  gtk_box_append(GTK_BOX(vbox), label);

  
  GtkWidget *content = gtk_label_new("");
  GtkPopover *popover = GTK_POPOVER(gtk_popover_new ());
  gtk_popover_set_default_widget(popover, imageBox);
  gtk_widget_set_parent(GTK_WIDGET(popover), imageBox);
  gtk_popover_set_position(popover, GTK_POS_RIGHT);
  gtk_popover_set_autohide(popover, TRUE); 
  gtk_popover_set_has_arrow(popover, FALSE);
  gtk_popover_set_child (popover, content);
  addGesture(imageBox, popover);

  g_object_set_data(G_OBJECT(vbox),"label", label);
  g_object_set_data(G_OBJECT(vbox),"imageBox", imageBox);
  g_object_set_data(G_OBJECT(vbox),"content", content);

	gtk_list_item_set_child(list_item, vbox);
}
// object is list_item
// item_get_item is GFileInfo
// item_get child is GtkWidget
static void
factoryUnBind(GtkSignalListItemFactory *self, GObject *object, void *data){
  GtkListItem *list_item =GTK_LIST_ITEM(object);
	GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
  void *image = g_object_get_data(G_OBJECT(vbox), "image");  
  gtk_widget_unparent(GTK_WIDGET(image));
}
// The bind function for the factory 
static void
factoryBind(GtkSignalListItemFactory *self, GObject *object, void *data)
{
  GtkSelectionModel *selection_model = data;
  GtkListItem *list_item =GTK_LIST_ITEM(object);
  // By now, we have the list populated with GFileInfos and we set up the associated widgets.
	GtkBox *vbox = GTK_BOX(gtk_list_item_get_child( list_item ));
  GtkWidget *imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(vbox), "imageBox"));
	GtkLabel *label = g_object_get_data(G_OBJECT(vbox), "label");
  GFileInfo *info = G_FILE_INFO(gtk_list_item_get_item(list_item));
	GtkLabel *content = g_object_get_data(G_OBJECT(vbox), "content");
  gtk_label_set_markup(content, g_file_info_get_name(info));
  
  char *name = g_strdup(g_file_info_get_name(info));
  //fprintf(stderr, "factoryBind...itemchild=%p, object=%p data=%p info=%p name=%s\n", vbox, object, data, info, name);
  if (name && strlen(name) > 15){
    name[15] = 0;
    name[14] ='~';
  }
  char *markup = g_strconcat("<span size=\"small\">", name, "</span>", NULL);
	gtk_label_set_markup( GTK_LABEL(label), markup );
  g_free(name);
  g_free(markup);
  GtkIconPaintable *icon = gtk_icon_theme_lookup_icon (icon_theme,
               "application-x-generic", NULL, 48, 1,    
               GTK_TEXT_DIR_NONE, 
               (GtkIconLookupFlags) 0);
  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(icon));
  g_object_set_data(G_OBJECT(image), "info", info);
  g_object_set_data(G_OBJECT(image), "selection_model", selection_model);
  g_object_unref(icon);
  gtk_widget_set_size_request(image, 48, 48);
  gtk_box_append(GTK_BOX(imageBox), image);
  g_object_set_data(G_OBJECT(vbox), "image", image);

  GtkDragSource *source = gtk_drag_source_new ();
  g_signal_connect (source, "prepare", G_CALLBACK (image_drag_prepare), image);
  
  g_signal_connect (source, "drag-begin", G_CALLBACK (image_drag_begin), image);
  gtk_widget_add_controller (image, GTK_EVENT_CONTROLLER (source));

/*
          GdkDragAction actions =
          (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK);

        GtkDragSource *dragSource = gtk_drag_source_new ();
        g_signal_connect (dragSource, "prepare", G_CALLBACK (dragPrepare), NULL);
        g_signal_connect (dragSource, "drag-begin", G_CALLBACK (dragBegin), NULL);
        g_signal_connect (dragSource, "drag-end", G_CALLBACK (dragEnd), NULL);
        g_signal_connect (dragSource, "drag-cancel", G_CALLBACK (dragCancel), NULL);
        gtk_drag_source_set_actions(dragSource, actions); 
        gtk_widget_add_controller (GTK_WIDGET (image), GTK_EVENT_CONTROLLER (dragSource));
*/
  
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
  GFile *gfile = g_file_new_for_path(DIRECTORY);
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
	g_signal_connect( factory, "setup", G_CALLBACK(factorySetup), selection_model );
	g_signal_connect( factory, "bind", G_CALLBACK(factoryBind), selection_model);
	g_signal_connect( factory, "unbind", G_CALLBACK(factoryUnBind), selection_model);
  g_signal_connect( factory, "teardown", G_CALLBACK(factoryTeardown), selection_model);
	// Create the GtkGridView.
	view = gtk_grid_view_new( GTK_SELECTION_MODEL( selection_model ), factory );
  gtk_grid_view_set_enable_rubberband(GTK_GRID_VIEW(view), true);
  gtk_scrolled_window_set_child(scrolledWindow, GTK_WIDGET(view));
  
  dropController = createDropController();
  gtk_widget_add_controller (GTK_WIDGET (view), GTK_EVENT_CONTROLLER (dropController));
}
int main (int argc, char *argv[])
{
  gtk_init ();

  GdkDisplay *displayGdk = gdk_display_get_default();
  icon_theme = gtk_icon_theme_get_for_display(displayGdk);
  GtkWidget *MainWidget = gtk_window_new ();
	gtk_window_set_default_size( GTK_WINDOW(MainWidget), 400, 400 );
  create(MainWidget);
  setAsDialog(MainWidget, "test", "Test");  
  gtk_window_present (GTK_WINDOW(MainWidget));
  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);
  return 0;
}
