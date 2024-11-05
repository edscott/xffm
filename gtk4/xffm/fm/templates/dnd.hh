#ifndef XF_LOCALDND__HH
# define XF_LOCALDND__HH

namespace xf
{
//template <class Type> class View;
template <class Type> class GridView;
template <class DirectoryClass>
class Dnd {
public:
///////////////////////////////////  drop /////////////////////////////////////

static GtkEventController *createDropController(void *data){
    const char *mimeTypes[]={"text/uri-list", NULL};
    GdkContentFormats *contentFormats = gdk_content_formats_new(mimeTypes, 1);
    GdkDragAction actions =
      (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
    GtkDropTargetAsync *dropTarget = gtk_drop_target_async_new (contentFormats, actions);
    g_signal_connect (dropTarget, "accept", G_CALLBACK (dropAccept), NULL);
    g_signal_connect (dropTarget, "drop", G_CALLBACK (dropDrop), data);
    g_signal_connect (dropTarget, "drag-motion", G_CALLBACK (dropMotion), NULL);

    g_signal_connect (dropTarget, "drag-enter", G_CALLBACK (dropEnter), NULL);
    /*g_signal_connect (dropTarget, "drag-leave", G_CALLBACK (dropLeave), NULL);*/
    return GTK_EVENT_CONTROLLER(dropTarget);
}


///////////////////////////////////  drag /////////////////////////////////////
    // signal drag-begin Emitted on the drag source when a drag is started.
    static void
    image_drag_begin (GtkDragSource *source, GdkDrag *drag, GtkWidget *widget)
    {
      GdkPaintable *paintable;
      paintable = gtk_widget_paintable_new (widget);
      gtk_drag_source_set_icon (source, paintable, 0, 0);
      g_object_unref (paintable);
      GdkDragAction actions =
        (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
      gtk_drag_source_set_actions(source, actions);
      
      fprintf(stderr,"image_drag_begin.\n");
    }
    // signal prepare Emitted when a drag is about to be initiated.
    static GdkContentProvider* 
    image_drag_prepare ( GtkDragSource* self, gdouble x, gdouble y, gpointer data)
    {
      GdkContentProvider *dndcontent;
      auto gridView_p = (GridView<DirectoryClass> *)data;

      //GtkSelectionModel *selection_model = gridView_p->selectionModel();
      GList *selection_list = gridView_p->getSelectionList();
      if (g_list_length(selection_list) < 1) {
        DBG("*** no drag, selection list ==0\n");
        return NULL;
      }
      char *string = g_strdup("");
      for (GList *l = selection_list; l && l->data; l=l->next){
        auto info = G_FILE_INFO(l->data);
        GFile *file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
        char *path = g_file_get_path(file);
        char *g = g_strconcat(string, "file://", path, "\n", NULL);
        g_free(string);
        string = g;
        g_free(path);     
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
private:
///////////////////////////////////  drag /////////////////////////////////////

// GtkDragSource *dragSource
    
// signal drag-end Emitted on the drag source when a drag is finished.
static void dragEnd ( GtkDragSource* self, GdkDrag* drag, 
    gboolean delete_data, gpointer user_data)
{
  fprintf(stderr,"dragEnd.\n");
}
// signal drag-cancel Emitted on the drag source when a drag has failed.
static gboolean dragCancel ( GtkDragSource* self, GdkDrag* drag, 
    GdkDragCancelReason* reason, gpointer user_data
){
  fprintf(stderr,"dragCancel.\n");
  return true;
}


// GdkDrag *drag
// signal cancel Emitted when the drag operation is cancelled.
static gboolean cancel (
  GdkDrag* self,
  GdkDragCancelReason* reason,
  gpointer user_data
)
{
  fprintf(stderr,"cancel.\n");
  return true;
}
// signal drop-performed Emitted when the drop operation is performed on an accepting client.
static void drop_performed (
  GdkDrag* self,
  gpointer user_data
){
  fprintf(stderr,"drop_performed.\n");
}
// signal dnd-finished Emitted when the destination side has finished reading all data.
static void
dnd_finished (
  GdkDrag* self,
  gpointer user_data
){
  fprintf(stderr,"dnd_finished.\n");
}

///////////////////////////////////  drop /////////////////////////////////////

// In main context:
static void dropReadDoneCallback(GObject *source_object, GAsyncResult *res, void *arg){
  auto drop = GDK_DROP(g_object_get_data (source_object, "drop"));
  GOutputStream *stream = G_OUTPUT_STREAM (source_object);
  GdkDragAction action = GDK_ACTION_COPY; // FIXME
  GBytes *bytes;

  if (g_output_stream_splice_finish (stream, res, NULL) >= 0){
      bytes = g_memory_output_stream_steal_as_bytes (G_MEMORY_OUTPUT_STREAM (stream));
  }

  gdk_drop_finish (drop, action);
  g_object_unref (drop);

  void **argData = (void **)arg;
  void *(*f)(void *) = (void* (*)(void*))argData[0];
  argData[2] = (void *)bytes;
  f(arg);
  /*
  pthread_t thread;
  pthread_create(&thread, NULL, f, arg);
  pthread_detach(thread);
   */
  return;
}

// in thread:
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
      gdk_drop_finish (drop, (GdkDragAction)0);
      g_error_free(error_);
      return;
  }
  if (input == NULL) {
      gdk_drop_finish (drop, (GdkDragAction)0);
      return;
  }

  output = g_memory_output_stream_new_resizable ();
  g_object_set_data (G_OBJECT (output), "drop", drop);
  g_object_ref (drop);
  int flags = (int)G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE | (int)G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET;
  g_output_stream_splice_async (output,
                                input,
                                (GOutputStreamSpliceFlags)flags,
                                G_PRIORITY_DEFAULT,
                                NULL,
                                dropReadDoneCallback,
                                arg);
  g_object_unref (output);
  g_object_unref (input);
  return;
}

static void *readDrop(GdkDrop *drop, void *(*f)(void *), void *data, char *target){
  auto arg = (void **)calloc(4, sizeof(void*));
  arg[0] = (void *)f;
  arg[1] = data;
  arg[3] = target;
  const char *mimeTypes[]={"text/uri-list", NULL};
  gdk_drop_read_async (drop, mimeTypes, G_PRIORITY_DEFAULT, // int io_priority,
  NULL, dropReadCallback, (void *)arg);
  return NULL;
}



static void *readAction(void *arg){
  void **argData = (void **)arg;
  void *data = argData[1];
  GBytes *bytes = (GBytes *)argData[2];
  char *target = (char *) argData[3];

  gsize size;
  const void *p = g_bytes_get_data(bytes, &size);
  fprintf(stderr, "readAction(): bytes (%d):\ntarget: %s\n%s\n", size, target, (const char *)p);
//  Dialogs::info("wahtever");
  
  Dialogs::dnd((const char *)p);
  
  g_free(arg);
  return NULL;

}

 // signals ///////
 

    static gboolean dropDrop ( GtkDropTarget* self, GdkDrop* drop,  
        gdouble x, gdouble y, gpointer data)
    {
      GdkDragAction action = gdk_drop_get_actions(drop);
      if (action == (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE| GDK_ACTION_LINK)){
        action = GDK_ACTION_MOVE;
      }
      DBG("action = %d (%d,%d,%d)\n", action, GDK_ACTION_COPY, GDK_ACTION_MOVE, GDK_ACTION_LINK);

      fprintf(stderr,"*** dropDrop %lf,%lf .\n", x, y);
 
      
      auto gridview_p = (GridView<DirectoryClass> *)data;
      
      auto listModel = gridview_p->listModel();
      auto n = g_list_model_get_n_items(listModel);
      char *path = NULL;
      bool xOk = false;
      bool yOk = false;
      for (guint i=0; i<n; i++){
        auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
        auto imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "imageBox"));
        graphene_rect_t bounds;
        if (gtk_widget_compute_bounds (imageBox, GTK_WIDGET(gridview_p->view()), &bounds)) {
          xOk = (x > bounds.origin.x && x < bounds.origin.x + bounds.size.width);
          yOk = (y > bounds.origin.y && y < bounds.origin.y + bounds.size.height); 
          if (xOk && yOk) {
            path = Basic::getPath(info);
            if (g_file_test(path, G_FILE_TEST_IS_DIR)){
              DBG("*** Drop OK at xok=%d, yok=%d \"%s\"\n", xOk, yOk, path);
            }
            break;
          }
        } else {
          DBG("gtk_widget_compute_bounds failed...\n");
        }
      }
      if (!path){
        auto workdir = Child::getWorkdir();
        DBG("workdir is \"%s\"\n", workdir);
        if (g_file_test(workdir, G_FILE_TEST_IS_DIR)){
          DBG("*** Drop OK at xok=%d, yok=%d \"%s\"\n", xOk, yOk, workdir);
        } else if (workdir && strcmp(workdir, "Gtk:bookmarks") == 0){
          DBG("*** Drop into bookmarks.\n");
        }

      }

        
            

      // XXX target is synchronous.
      //     now we have to set it up for asyncronous read. Or rather, get target and
      //     send to asynchronous...
      readDrop(drop, readAction, data, g_strdup(path)); // this is asynchronous...
      g_free(path);
      
      return true; //drop accepted
      //return false; //drop not accepted
    }
  
static   GdkDragAction
dropEnter (
  GtkDropTarget* self,
  GdkDrop* drop,
  gdouble x,
  gdouble y,
  gpointer user_data
)
{
  fprintf(stderr,"dropEnter %lf,%lf.\n", x, y);

  return GDK_ACTION_LINK;
  return GDK_ACTION_COPY;
}
static void
dropLeave (
  GtkDropTarget* self,
  GdkDrop* drop,
  gpointer user_data
)
{
  fprintf(stderr,"dropLeave.\n");
}
static GdkDragAction
dropMotion ( GtkDropTarget* self, GdkDrop* drop, gdouble x, gdouble y, gpointer data)
{
  /* // does not work, no modifierType
  auto eventController = GTK_EVENT_CONTROLLER(self);
  auto event = gtk_event_controller_get_current_event(eventController);
  auto modifierType = gdk_event_get_modifier_state (event);
  //DBG("dropMotion %lf,%lf, modifierType =  %ld\n", x, y, modifierType);
 
  GdkDragAction actions =
      (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
 
  if (modifierType & ((GDK_SHIFT_MASK & GDK_MODIFIER_MASK) | (GDK_CONTROL_MASK& GDK_MODIFIER_MASK))) {
    gdk_drop_status (drop, actions, GDK_ACTION_LINK);
    return GDK_ACTION_LINK;
  }
  if (modifierType & (GDK_SHIFT_MASK & GDK_MODIFIER_MASK)) {
    DBG("dropMotion modifierType = GDK_SHIFT_MASK \n");
    gdk_drop_status (drop, actions, GDK_ACTION_MOVE);
    return GDK_ACTION_MOVE;
  }
  if (modifierType & (GDK_CONTROL_MASK & GDK_MODIFIER_MASK)) {
    gdk_drop_status (drop, actions, GDK_ACTION_COPY);
    return GDK_ACTION_COPY;
  }
  gdk_drop_status (drop, actions, GDK_ACTION_MOVE);*/
  return GDK_ACTION_COPY;
}
static gboolean dropAccept ( GtkDropTarget* self, GdkDrop* drop, gpointer user_data)
{
  fprintf(stderr,"dropAccept.\n");
/*  auto eventController = GTK_EVENT_CONTROLLER(self);
  auto event = gtk_event_controller_get_current_event(eventController);
  auto modifierType = gdk_event_get_modifier_state (event);
  GdkDragAction actions =
      (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);

  if (modifierType & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
    gdk_drop_status (drop, actions, GDK_ACTION_LINK);
  }
  else if (modifierType & GDK_SHIFT_MASK) {
    gdk_drop_status (drop, actions, GDK_ACTION_MOVE);
  }
  else if (modifierType & GDK_CONTROL_MASK) {
    gdk_drop_status (drop, actions, GDK_ACTION_COPY);
  }
  else if (modifierType == GDK_NO_MODIFIER_MASK) {
    gdk_drop_status (drop, actions, GDK_ACTION_MOVE);
  }
  gdk_drop_status (drop, actions, GDK_ACTION_MOVE);*/
  
  //return false; //drop not accepted on enter
  return true; //drop accepted on enter
}

};
}
#endif


