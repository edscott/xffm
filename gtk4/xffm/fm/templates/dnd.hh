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

static GtkEventController *createDropControllerPathbar(void *data){
    const char *mimeTypes[]={"text/uri-list", NULL};
    GdkContentFormats *contentFormats = gdk_content_formats_new(mimeTypes, 1);
    GdkDragAction actions =
      (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
    GtkDropTargetAsync *dropTarget = gtk_drop_target_async_new (contentFormats, actions);
    g_signal_connect (dropTarget, "accept", G_CALLBACK (dropAccept), NULL);
    g_signal_connect (dropTarget, "drop", G_CALLBACK (dropDropPathbar), data);
    g_signal_connect (dropTarget, "drag-motion", G_CALLBACK (dropMotionPathbar), NULL);

    g_signal_connect (dropTarget, "drag-enter", G_CALLBACK (dropEnter), NULL);
    /*g_signal_connect (dropTarget, "drag-leave", G_CALLBACK (dropLeave), NULL);*/
    return GTK_EVENT_CONTROLLER(dropTarget);
}


///////////////////////////////////  drag /////////////////////////////////////
    // signal drag-begin Emitted on the drag source when a drag is started.
    static void
    image_drag_begin (GtkDragSource *source, GdkDrag *drag, GtkWidget *widget)
    {
      auto gridView_p = (GridView<DirectoryClass> *)Child::getGridviewObject();
      GList *selection_list = gridView_p->getSelectionList();
      GdkPaintable *paintable;
      if (g_list_length(selection_list) > 1) {
        paintable = Texture<bool>::load("dnd-multiple", 48);
      } else {
        // get from item selected.
        //paintable = gtk_widget_paintable_new (widget);
        auto info = G_FILE_INFO(selection_list->data);
        paintable = Texture<bool>::load(info); // Loads icon from icontheme.
      }
      
      
      gtk_drag_source_set_icon (source, paintable, 0, 0);
      g_object_unref (paintable);
      GdkDragAction actions =
        (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
      gtk_drag_source_set_actions(source, actions);
      
      DBG("image_drag_begin.\n");
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

      //FIXME get widget at x,y. If widget not in selection list, cancel dnd.
      // return NULL;
/*
      bool item = false;
      for (auto l=selection_list; l && l->data; l=l->next){
        if (l->data == itemInfo){
          item = true;
          break;
        }
      }
      if (!item) return NULL;
*/
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
        
        DBG("image_drag_prepare.\n");
        return dndcontent;
}
private:
///////////////////////////////////  drag /////////////////////////////////////

// GtkDragSource *dragSource
    
// signal drag-end Emitted on the drag source when a drag is finished.
static void dragEnd ( GtkDragSource* self, GdkDrag* drag, 
    gboolean delete_data, gpointer user_data)
{
  DBG("dragEnd.\n");
}
// signal drag-cancel Emitted on the drag source when a drag has failed.
static gboolean dragCancel ( GtkDragSource* self, GdkDrag* drag, 
    GdkDragCancelReason* reason, gpointer user_data
){
  DBG("dragCancel.\n");
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
  DBG("cancel.\n");
  return true;
}
// signal drop-performed Emitted when the drop operation is performed on an accepting client.
static void drop_performed (
  GdkDrag* self,
  gpointer user_data
){
  DBG("drop_performed.\n");
}
// signal dnd-finished Emitted when the destination side has finished reading all data.
static void
dnd_finished (
  GdkDrag* self,
  gpointer user_data
){
  DBG("dnd_finished.\n");
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
  DBG("dropReadCallback: do your thing.\n" );
  const char *out_mime_type;
  GError *error_ = NULL;
  GdkDrop *drop = GDK_DROP(source_object);
  GInputStream *input;
  GOutputStream *output;

  input = gdk_drop_read_finish (drop, res, &out_mime_type, &error_);
  if (error_){
      DBG( "** Error::dropReadCallback(): %s\n", error_->message);
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
  DBG("readAction(): bytes (%ld):\ntarget: %s\n%s\n", size, target, (const char *)p);
//  Dialogs::info("wahtever");
  
  dnd((const char *)p, target);
  
  g_free(arg);
  return NULL;

}

 // signals ///////
  static void setDropPathbar(double x, double y, void *data){
    DBG("FIXME: dnd.hh: setDropPathbar()\n");
    // here we need to have sent:
    //   1. widget that owns the drop event
    //   2. all widgets in pathbar
    //
    //   So we set to green widget and unset all others...
    //   something like that, or rather use enter and 
    //   exit signals since each widget has its own 
    //   drop controller...
    /*
      auto gridview_p = (GridView<DirectoryClass> *)Child::getGridviewObject();
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
          // FIXME:
          //       seems like xorigin and yorigin is cero, so only .. is ok
          xOk = (x > bounds.origin.x && x < bounds.origin.x + bounds.size.width);
          yOk = (y > bounds.origin.y && y < bounds.origin.y + bounds.size.height); 
          path = Basic::getPath(info);
          if (g_file_test(path, G_FILE_TEST_IS_DIR)){
            if (xOk && yOk) {
              gtk_widget_add_css_class (GTK_WIDGET(imageBox), "dropNegative" );
            } else {
              gtk_widget_remove_css_class (GTK_WIDGET(imageBox), "dropNegative" );
            }
          }
          g_free(path);
        }
      }
      */
  }
  static void setDropTarget(double x, double y, void *data){
      auto gridview_p = (GridView<DirectoryClass> *)Child::getGridviewObject();
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
          // FIXME:
          //       seems like xorigin and yorigin is cero, so only .. is ok
          xOk = (x > bounds.origin.x && x < bounds.origin.x + bounds.size.width);
          yOk = (y > bounds.origin.y && y < bounds.origin.y + bounds.size.height); 
          path = Basic::getPath(info);
          if (g_file_test(path, G_FILE_TEST_IS_DIR)){
            if (xOk && yOk) {
              gtk_widget_add_css_class (GTK_WIDGET(imageBox), "dropNegative" );
            } else {
              gtk_widget_remove_css_class (GTK_WIDGET(imageBox), "dropNegative" );
            }
          }
          g_free(path);
        }
      }
  }

  static char *getDropTarget(double x, double y, void *data){
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
              TRACE("*** Drop OK at xok=%d, yok=%d \"%s\"\n", xOk, yOk, path);
              return path;
            } else {
              return NULL;
            }
            break;
          }
        } else {
          DBG("should not happen: gtk_widget_compute_bounds failed...\n");
        }
      }
      if (!path){
        auto workdir = Child::getWorkdir();
        DBG("workdir is \"%s\"\n", workdir);
        if (g_file_test(workdir, G_FILE_TEST_IS_DIR)){
          DBG("*** Drop OK at xok=%d, yok=%d \"%s\"\n", xOk, yOk, workdir);
          return g_strdup(workdir);
        } else if (workdir && strcmp(workdir, "Gtk:bookmarks") == 0){
          DBG("*** Drop into bookmarks: add a bookmark with this drop (FIXME).\n");
        }
      }
      return NULL;
  }
    static gboolean dropDropPathbar ( GtkDropTarget* self, GdkDrop* drop,  
        gdouble x, gdouble y, gpointer data)
    {
      GdkDragAction action = gdk_drop_get_actions(drop);
      if (action == (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE| GDK_ACTION_LINK)){
        action = GDK_ACTION_MOVE;
      }
      TRACE("action = %d (%d,%d,%d)\n", action, GDK_ACTION_COPY, GDK_ACTION_MOVE, GDK_ACTION_LINK);

      TRACE("*** dropDropPathbar %lf,%lf .\n", x, y);
      auto path = (const char *)g_object_get_data(G_OBJECT(self), "path");
      
      if (!path) {
        gdk_drop_finish(drop, GDK_ACTION_COPY);
        return false; //drop not accepted
      }
            

      // XXX target is synchronous.
      //     now we have to set it up for asyncronous read. Or rather, get target and
      //     send to asynchronous...
      readDrop(drop, readAction, data, g_strdup(path)); // this is asynchronous...
      //g_free(path);
      
      return true; //drop accepted
    }

    static gboolean dropDrop ( GtkDropTarget* self, GdkDrop* drop,  
        gdouble x, gdouble y, gpointer data)
    {
      GdkDragAction action = gdk_drop_get_actions(drop);
      if (action == (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE| GDK_ACTION_LINK)){
        action = GDK_ACTION_MOVE;
      }
      TRACE("action = %d (%d,%d,%d)\n", action, GDK_ACTION_COPY, GDK_ACTION_MOVE, GDK_ACTION_LINK);

      TRACE("*** dropDrop %lf,%lf .\n", x, y);
      auto path = getDropTarget(x, y, data);
      
      if (!path) {
        gdk_drop_finish(drop, GDK_ACTION_COPY);
        return false; //drop not accepted
      }
            

      // XXX target is synchronous.
      //     now we have to set it up for asyncronous read. Or rather, get target and
      //     send to asynchronous...
      readDrop(drop, readAction, data, g_strdup(path)); // this is asynchronous...
      g_free(path);
      
      return true; //drop accepted
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
  DBG("dropEnter %lf,%lf.\n", x, y);

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
  DBG("dropLeave.\n");
}
static GdkDragAction
dropMotionPathbar( GtkDropTarget* self, GdkDrop* drop, gdouble x, gdouble y, gpointer data)
{
  setDropPathbar(x, y, data) ;
  return GDK_ACTION_COPY;
}
static GdkDragAction
dropMotion ( GtkDropTarget* self, GdkDrop* drop, gdouble x, gdouble y, gpointer data)
{

  setDropTarget(x, y, data) ;
   
  //DBG("dropMotion %lf,%lf\n", x, y);
 
  /* // does not work, no modifierType
  auto eventController = GTK_EVENT_CONTROLLER(self);
  auto event = gtk_event_controller_get_current_event(eventController);
  auto modifierType = gdk_event_get_modifier_state (event);
  //DBG("dropMotion %lf,%lf, modifierType =  %ld\n", x, y, modifierType);
*/ 
  return GDK_ACTION_COPY;
}
static gboolean dropAccept ( GtkDropTarget* self, GdkDrop* drop, gpointer user_data)
{
  DBG("dropAccept.\n");
  
  //return false; //drop not accepted on enter
  return true; //drop accepted on enter
}

private:

    static void dnd(const char *uriList, char *target){
      char *source = NULL;
      auto files = getUriFiles(uriList, &source);
      auto block = getFilesBlock(files);
      auto markup = g_strconcat("<span color=\"red\">",_("Target"), ": </span>", target, "\n",
         "<span color=\"green\">", _("Source"),  ": </span>", source, "\n",
         "<span color=\"black\">", block, "\n</span>", NULL);
      if (strcmp(source, target)) {
        auto dialogObject = new DialogButtons<dndResponse>;
        dialogObject->setParent(GTK_WINDOW(MainWidget));

        dialogObject->setLabelText(markup);
        DBG("create dialogObject=%p\n", dialogObject); 
        dialogObject->subClass()->setUriList(uriList);
        dialogObject->subClass()->setTarget(target);
        auto count = dialogObject->subClass()->uriCount();
       
        if (count > 1){ // a bit hacky...
          auto buttons = (GtkWidget **)g_object_get_data(G_OBJECT(dialogObject->dialog()), _("buttons"));
          if (buttons && buttons[2]) {
            gtk_widget_set_visible(GTK_WIDGET(buttons[2]), false);
          } else {
            DBG("Error:: cannot find button \"%s\" to hide.\n", _("Link"));
          }  
        }
    
       

        g_free(target);

        dialogObject->run();
        
      } else {
        DBG("Source and target are the same: %s\n", source);
      }
      g_free(markup);
      g_free(block);
      g_strfreev(files);


    }

    static char **getUriFiles(const char *uriList, char **path_p){
      auto files = g_strsplit(uriList, "\n", -1);
      for (auto p=files ; p && *p; p++){
        if (strncmp(*p, "file://", strlen("file://")) == 0){
          for (int i=0; i<strlen("file://"); i++) (*p)[i] = ' ';
          g_strstrip(*p);
          if (*path_p == NULL) *path_p = g_path_get_dirname(*p);
        }
        if (strlen(*p) == 0) *p = NULL;
      }
      return files;
    }

    
    static char *getFilesBlock(char **files){
      auto block = g_strdup("");
      int count = 0;
      int total = 0;
      for (auto p=files ; p && *p; p++)total++;
      for (auto p=files ; p && *p; p++, count++){
        if (count >= 5) {
          auto h = g_strdup_printf("%d %s", total-count, _("files"));
          auto hh = g_strdup_printf(_("+ %s more"), h);
          Basic::concat(&block, hh);
          g_free(h);
          g_free(hh);
          break;
        }
        auto g = g_path_get_basename(*p);
        Basic::concat(&block, _("file"));
        Basic::concat(&block, _(": "));
        Basic::concat(&block, g);
        Basic::concat(&block, "\n");
        g_free(g);
      }
      return block;
    }


};
}
#endif


