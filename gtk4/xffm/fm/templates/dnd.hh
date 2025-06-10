#ifndef XF_LOCALDND__HH
# define XF_LOCALDND__HH
bool inPathbar = false;
bool inGridView = false;


namespace xf
{
//template <class Type> class View;
template <class Type> class UtilPathbar;
template <class Type> class GridView;
template <class Type>
class Dnd {
  GdkDrag *drag_ = NULL;
  bool dragOn_ = false;
public:

    Dnd(void){
      g_object_set_data(G_OBJECT(xf::Child::mainWidget()), "Dnd", this);
    }
      //bool dragging(void) {return dragging_;}
      //void dragging(bool value) {dragging_ = value;}
    bool dragOn(void) {return dragOn_;}
    void dragOn(bool value) {dragOn_ = value;}
///////////////////////////////////  drop /////////////////////////////////////

    static GtkEventController *createDropController(void *data){
        const char *mimeTypes[]={"text/uri-list", NULL};
        GdkContentFormats *contentFormats = gdk_content_formats_new(mimeTypes, 1);
        GdkDragAction actions =
          (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
        GtkDropTargetAsync *dropTarget = gtk_drop_target_async_new (contentFormats, actions);
        g_signal_connect (dropTarget, "accept", G_CALLBACK (dropAccept), NULL);
        g_signal_connect (dropTarget, "drop", G_CALLBACK (dropDrop), NULL);
        g_signal_connect (dropTarget, "drag-motion", G_CALLBACK (dropMotion), NULL);

        return GTK_EVENT_CONTROLLER(dropTarget);
    }
    
    int getSize(void){
        int size = Settings::getInteger("xfterm", "iconsize", 5);
        if (size < 0) size = 5;
        return size;

    }

    bool inOffset(double x, double y, void *data){
        int size = getSize();
        auto gridView_p = (GridView<Type> *)data;
        if (gridView_p->X() <= 0.1 && gridView_p->Y() <= 0.1) return true;
        graphene_rect_t bounds;
        if (!gtk_widget_compute_bounds (gridView_p->view(), Child::mainWidget(), &bounds)){
          ERROR_("** Error:: viewMotion: should not happen\n");
          return true;
        }
        double mainX = x + bounds.origin.x;
        double mainY = y + bounds.origin.y;        
        double distance = sqrt(pow(gridView_p->X() - mainX,2) + pow(gridView_p->Y() - mainY,2));
        if (distance <= size) return true;
        TRACE("starting drag at point %lf->%lf, %lf->%lf (distance=%lf\n", 
            gridView_p->X(), mainX, gridView_p->Y(), mainY, distance);

        return false;
    }

    // x,y are in Child::mainWidget() frame of reference.
    bool startDrag(GtkEventControllerMotion* self, double x, double y, void *data){
       if (this->dragOn()) {
          TRACE("Dnd::startDrag return true on this->dragOn() == true\n");
          return true;
        }
        auto noStart = inOffset(x, y, data);
        if (noStart) {
          TRACE("Dnd::startDrag return false on noStart == true\n");
          return false;
        }
        this->dragOn(true);

        auto gridView_p = (GridView<Type> *)data;        
        GList *selection_list = gridView_p->getSelectionList();
        if (g_list_length(selection_list) < 1) {
          ERROR_("*** Error:: Dnd::startDrag: no drag, selection list ==0, return false\n");
          return false;
        }
       
        this->dragOn(true);
        TRACE("***Dnd::startDrag setting this->dragOn() = true, last drag_=%p\n", drag_);


        // Last cleanup:
        if (drag_) cleanDrag();
        
        GdkSurface *surface;
        GdkDevice *device;
        GdkDragAction actions;
        GdkContentProvider *content;
        device = gtk_event_controller_get_current_event_device (GTK_EVENT_CONTROLLER(self));
        surface = gtk_native_get_surface (gtk_widget_get_native (GTK_WIDGET (gridView_p->view())));
        char *string = g_strdup("");
        for (GList *l = selection_list; l && l->data; l=l->next){
          auto info = G_FILE_INFO(l->data);
          GFile *file = G_FILE(g_file_info_get_attribute_object (info, "standard::file"));
          auto path = g_file_get_path(file);
          auto selectedPath = g_strconcat("file://", path, "\n", NULL);
          Basic::concat(&string, selectedPath);
          g_free(selectedPath);     
          g_free(path);     
        }

        GBytes *bytes = g_bytes_new(string, strlen(string)+1);
        content = gdk_content_provider_new_for_bytes ("text/uri-list", bytes);
        g_free(string);
        // Watch for allocated  (GBytes *)bytes...
        // The caller of the function takes ownership of the data, and is responsible for freeing it.
        // Free with "g_bytes_unref (GBytes* bytes)"
        //
        // Watch for allocated  (GdkContentProvider *)content...
        // The caller of the function takes ownership of the data, and is responsible for freeing it.
        // Free with "g_object_unref(G_OBJECT(content))" (I suppose since it inherits from GObject).
        //
        
        actions = (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
        drag_ = gdk_drag_begin (surface,
                       device,
                       content,
                       actions,
                       gridView_p->X(),
                       gridView_p->Y());
        TRACE("***Dnd::startDrag drag is now = %p\n", drag_);

        // Keep track of drag_ for memory cleanup.

        // Keep track of bytes and content for memory cleanup,
        // drag_ is a GdkDrag, inherits from GObject.
        g_object_set_data(G_OBJECT(drag_), "bytes", bytes);
        g_object_set_data(G_OBJECT(drag_), "content", content);

        GdkPaintable *paintable;
        int size = getSize();
        if (g_list_length(selection_list) < 2){
          auto info = G_FILE_INFO(selection_list->data);
          //auto *iconPath = Texture<bool>::findIconPath(info);
          paintable = Texture<bool>::getShadedIcon(info, size, size, NULL);   
          //paintable = Texture<bool>::load(info, size); // Loads icon from icontheme.
        } else {
          //auto *iconPath = Texture<bool>::findIconPath("dnd-multiple");
          paintable = Texture<bool>::getShadedIcon("dnd-multiple", size, size, NULL);   
        }
        gtk_drag_icon_set_from_paintable (drag_, paintable,  1, 1);
        // Watch for allocated paintable (GdkPaintable *) paintable.
        // Keep track of paintable for memory cleanup,
        // paintable is a GdkPaintable *, inherits from GObject.
        g_object_set_data(G_OBJECT(drag_), "paintable", paintable);

        return true;
    }

  private:
    void cleanDrag(void){
      if (!this->drag_) {
        return;
      }
      // Cleanups before starting a new drag.
      // Last drag objects remain until program exit (no leaks).
      auto bytes = (GBytes *)g_object_get_data(G_OBJECT(this->drag_), "bytes");
      if (!bytes) {
        TRACE("*** Dnd::cleanDrag() drag_ is already clean.\n");// should not happen.
      } else {
        TRACE("*** Dnd::cleanDrag() cleanup for drag_ %p.\n", this->drag_);
      }
      auto content = GDK_CONTENT_PROVIDER(g_object_get_data(G_OBJECT(this->drag_), "content"));
      auto paintable = GDK_PAINTABLE(g_object_get_data(G_OBJECT(this->drag_), "paintable"));
      if (bytes) g_bytes_unref(bytes);
      if (content) g_object_unref(G_OBJECT(content));
      if (paintable) g_object_unref(G_OBJECT(paintable));
      g_object_set_data(G_OBJECT(drag_), "bytes", NULL);
      g_object_set_data(G_OBJECT(drag_), "content", NULL);
      g_object_set_data(G_OBJECT(drag_), "paintable", NULL);
      g_object_unref(G_OBJECT(this->drag_));
      this->drag_ = NULL;


      return;
    }

  public:
    void dropDone(bool success){
      if (this->drag_) {
        TRACE("Dnd::dropDone() drop success = %s\n", success?"true":"false");
        gdk_drag_drop_done(this->drag_, success);
      } else {
        TRACE("Dnd::dropDone() this->drag_ is NULL, success = %s\n", success?"true":"false");
      }
      //this->drag_ = NULL;
    }

private:
///////////////////////////////////  drag /////////////////////////////////////

// GtkDragSource *dragSource
    
// signal drag-end Emitted on the drag source when a drag is finished.
static void dragEnd ( GtkDragSource* self, GdkDrag* drag, 
    gboolean delete_data, gpointer user_data)
{
  TRACE("dragEnd.\n");
}
// signal drag-cancel Emitted on the drag source when a drag has failed.
static gboolean dragCancel ( GtkDragSource* self, GdkDrag* drag, 
    GdkDragCancelReason* reason, gpointer user_data
){
  TRACE("dragCancel.\n");
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
  TRACE("cancel.\n");
  return true;
}
// signal drop-performed Emitted when the drop operation is performed on an accepting client.
static void drop_performed (
  GdkDrag* self,
  gpointer user_data
){
  TRACE("drop_performed.\n");
}
// signal dnd-finished Emitted when the destination side has finished reading all data.
static void
dnd_finished (
  GdkDrag* self,
  gpointer user_data
){
  TRACE("dnd_finished.\n");
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
  return;
}

// in thread:
static void dropReadCallback(GObject *source_object, GAsyncResult *res, void *arg){
  TRACE("dropReadCallback: do your thing.\n" );
  const char *out_mime_type;
  GError *error_ = NULL;
  GdkDrop *drop = GDK_DROP(source_object);
  GInputStream *input;
  GOutputStream *output;

  input = gdk_drop_read_finish (drop, res, &out_mime_type, &error_);
  if (error_){
      ERROR_( "** Error::dropReadCallback(): %s\n", error_->message);
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
  TRACE("readAction(): bytes (%ld):\ntarget: %s\n%s\n", size, target, (const char *)p);
//  Dialogs::info("wahtever");
  
  dnd((const char *)p, target);
  
  g_free(arg);
  return NULL;

}


  static GtkWidget *getPathbarWidget(double x, double y, GtkBox *pathbar){
    GList *children_list = Basic::getChildren(pathbar);
    GList *children = g_list_first(children_list);
    for (;children && children->data; children=children->next){
      auto widget = GTK_WIDGET(children->data);
      graphene_rect_t bounds;
      if (!gtk_widget_compute_bounds (widget, GTK_WIDGET(pathbar), &bounds)) return NULL;
      bool xOk = false;
      bool yOk = false;
     
      xOk = (x > bounds.origin.x && x < bounds.origin.x + bounds.size.width);
      yOk = (y > bounds.origin.y && y < bounds.origin.y + bounds.size.height); 
      if (xOk && yOk) {
        auto path = (const char *) g_object_get_data(G_OBJECT(widget), "path");
        if (g_file_test(path, G_FILE_TEST_IS_DIR)){
          g_list_free(children_list);
          return widget;
        }
      }
    
    }
    g_list_free(children_list);
    return NULL;
  }

  static bool getPathbarCoordinates(double *x, double *y, GtkBox *pathbar){
    double X = *x;
    double Y = *y;
    
    graphene_rect_t bounds;
    if (!gtk_widget_compute_bounds (GTK_WIDGET(pathbar), Child::mainWidget(), &bounds)) {
      ERROR_("*** Error::getGridCoordinates(): should not happen\n");
      return false;
    }
    double newX = X - bounds.origin.x;
    double newY = Y - bounds.origin.y;
    if (newY < 0 || newY > bounds.size.height){
      TRACE("out of pathbar Y\n");
      if (inPathbar) BasicPathbar<Type>::resetPathbarCSS(pathbar);
      inPathbar = false;
      return false;
    }
    if (newX < 0 || newX > bounds.size.width){
      TRACE("out of pathbar X\n");
      if (inPathbar) BasicPathbar<Type>::resetPathbarCSS(pathbar);
      inPathbar = false;
      return false;
    }
    inPathbar = true;
    // GridView coordinates are displaced by gridview origin
    *x = newX;
    *y = newY;
    TRACE("getPathbarCoordinates: %lf->%lf, %lf->%lf\n", X,*x, Y, *y);
    return true;
  }

  static bool getGridCoordinates(double *x, double *y, void *data){
    double X = *x;
    double Y = *y;
    auto gridview_p = (GridView<Type> *)data;
    graphene_rect_t bounds;
    if (!gtk_widget_compute_bounds (GTK_WIDGET(gridview_p->view()), Child::mainWidget(), &bounds)) {
      ERROR_("*** Error::getGridCoordinates(): should not happen\n");
      return false;
    }
    double newX = X - bounds.origin.x;
    double newY = Y - bounds.origin.y;
    if (newY < 0 || newY >= bounds.size.height){
      TRACE("out of Y\n");
      if(inGridView) resetGridviewCSS(gridview_p);
      inGridView = false;
      return false;
    }
    if (newX < 0 || newX >= bounds.size.width){
      TRACE("out of X\n");
      if(inGridView) resetGridviewCSS(gridview_p);
      inGridView = false;
      return false;
    }
    // GridView coordinates are displaced by gridview origin
    *x = newX;
    *y = newY;
    //TRACE("getGridCoordinates: %lf->%lf, %lf->%lf\n", X,*x, Y, *y);
    return true;
  }
public:
  static void resetGridviewCSS(GridView<Type> *gridview_p){
    //auto lock = Child::tryLockGridView();
    auto listModel = gridview_p->listModel();
    auto n = g_list_model_get_n_items(listModel);
    for (guint i=0; i<n; i++){
      auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
      auto imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "imageBox"));
      gtk_widget_remove_css_class (GTK_WIDGET(imageBox), "dropNegative" );
    }
    //if (lock) Child::unlockGridView();
  }
private:
  static void gridHighlight(double x, double y, GridView<Type> *gridview_p){
    //auto lock = Child::tryLockGridView();
    auto listModel = gridview_p->listModel();
    auto n = g_list_model_get_n_items(listModel);
    char *path = NULL;
    bool xOk = false;
    bool yOk = false;
    TRACE("gridHighlight: x=%lf, y=%lf\n", x, y);
    for (guint i=0; i<n; i++){
      auto info = G_FILE_INFO(g_list_model_get_item(listModel, i)); // GFileInfo
      auto imageBox = GTK_WIDGET(g_object_get_data(G_OBJECT(info), "imageBox"));
      graphene_rect_t bounds;
//      if (gtk_widget_compute_bounds (imageBox, Child::mainWidget(), &bounds)) {
      if (gtk_widget_compute_bounds (imageBox, GTK_WIDGET(gridview_p->view()), &bounds)) {
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
    //if (lock) Child::unlockGridView();

  }

  static void targetHighlight(double x, double y, void *data){
      auto gridview_p = (GridView<Type> *)Child::getGridviewObject();
      GtkBox *pathbar = Child::getPathbar();
      


      if (getGridCoordinates(&x, &y, gridview_p)) {
        BasicPathbar<Type>::resetPathbarCSS(pathbar);
        gridHighlight(x, y, gridview_p);
        return;
      }
      if (getPathbarCoordinates(&x, &y, pathbar)) {
        auto widget = getPathbarWidget(x, y, pathbar);
        // reset all:
        BasicPathbar<Type>::resetPathbarCSS(pathbar);
        if (widget) {
          // Apply mask
          auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
          TRACE("widget at %s\n", path);
          gtk_widget_remove_css_class (widget, "pathbarbox" );
          gtk_widget_add_css_class (widget, "pathbardrop" );
        }
        return;
      }


  }
  static char *getGridDropTarget(double x, double y, GridView<Type> *gridview_p){
    //auto lock = Child::tryLockGridView();
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
          
              //if (lock) Child::unlockGridView();
              return path;
            } else {
              //if (lock) Child::unlockGridView();
              return NULL;
            }
            break;
          }
        } else {
          ERROR_("should not happen: gtk_widget_compute_bounds failed...\n");
        }
      }
      if (!path){
        auto workdir = Child::getWorkdir();
        TRACE("workdir is \"%s\"\n", workdir);
        if (g_file_test(workdir, G_FILE_TEST_IS_DIR)){
          TRACE("*** Drop OK at xok=%d, yok=%d \"%s\"\n", xOk, yOk, workdir);
          
          //if (lock) Child::unlockGridView();
          return g_strdup(workdir);
        } else if (workdir && strcmp(workdir, _("Bookmarks")) == 0){
          DBG("*** Drop into bookmarks: add a bookmark with this drop (FIXME).\n");
        }
      }
    
      //if (lock) Child::unlockGridView();
      return NULL;

  }

  static char *getDropTarget(double x, double y, void *data){
      auto gridview_p = (GridView<Type> *)Child::getGridviewObject();
      GtkBox *pathbar = Child::getPathbar();
      TRACE("original x,y = %lf,%lf\n", x, y);
      if (getGridCoordinates(&x, &y, gridview_p)) {
        TRACE("getGridCoordinates x,y = %lf,%lf\n", x, y);
        return getGridDropTarget(x, y, gridview_p);
      }
      resetGridviewCSS(gridview_p);
      if (getPathbarCoordinates(&x, &y, pathbar)) {
        TRACE("Pathbar new x,y = %lf,%lf\n", x, y);
        GtkWidget *widget = getPathbarWidget(x, y, pathbar);
        auto path = (const char *)g_object_get_data(G_OBJECT(widget), "path");
        return g_strdup(path);
      }
      BasicPathbar<Type>::resetPathbarCSS(pathbar);
      return NULL;
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
      auto gridview_p = Child::getGridviewObject();
      //if (!getGridCoordinates(&x, &y, gridview_p)) return false;
      
      auto path = getDropTarget(x, y, gridview_p);
      
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

static GdkDragAction
dropMotion ( GtkDropTarget* self, GdkDrop* drop, gdouble x, gdouble y, gpointer data)
{
  // does not do the trick
  // whatever
    /*  auto gridview_p = (GridView<Type> *)Child::getGridviewObject();
      GtkBox *pathbar = Child::getPathbar();
      
      double pathbarTail=0;
      double gridviewTail=0;
      graphene_rect_t bounds;   
      if (gtk_widget_compute_bounds (GTK_WIDGET(pathbar), Child::mainWidget(), &bounds)) {
        pathbarTail = bounds.origin.y + bounds.size.height;
      }
      if (gtk_widget_compute_bounds (GTK_WIDGET(gridview_p->view()), Child::mainWidget(), &bounds)) {
        gridviewTail = bounds.origin.y + bounds.size.height;
      }
      if (y < pathbarTail || y > gridviewTail){
        if(inGridView) resetGridviewCSS(gridview_p);
        inGridView = false;
        return GDK_ACTION_LINK;
      }*/
  targetHighlight(x, y, data) ;
   
  TRACE("dropMotion %lf,%lf\n", x, y);
  
 
  /* // does not work, no modifierType
  auto eventController = GTK_EVENT_CONTROLLER(self);
  auto event = gtk_event_controller_get_current_event(eventController);
  auto modifierType = gdk_event_get_modifier_state (event);
  //TRACE("dropMotion %lf,%lf, modifierType =  %ld\n", x, y, modifierType);
*/ 
  return GDK_ACTION_COPY;
}
static gboolean dropAccept ( GtkDropTarget* self, GdkDrop* drop, gpointer user_data)
{
  TRACE("dropAccept.\n");
  //if (!inPathbar && !inGridView) return false;
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
      
      auto d = (Dnd<LocalDir > *)g_object_get_data(G_OBJECT(Child::mainWidget()), "Dnd");

      for (auto p=files; p && *p; p++){
        if (g_file_test(*p, G_FILE_TEST_IS_DIR)){
          if (strcmp(*p, target)==0){
            TRACE("Source and target are the same: %s\n", *p);
            auto message = g_strdup_printf(" %s: %s\n", _("Invalid target folder"), target);
            Print::printWarning(Child::getOutput(), message);
            d->dropDone(false);
            goto done;
          }
        }
      }
      if (strcmp(source, target)) {
        auto dialogObject = new DialogButtons<dndResponse>;
        dialogObject->setParent(GTK_WINDOW(Child::mainWidget()));

        dialogObject->setLabelText(markup);
        TRACE("create dialogObject=%p\n", dialogObject); 
        dialogObject->subClass()->setUriList(uriList);
        dialogObject->subClass()->setTarget(target);
        auto count = dialogObject->subClass()->uriCount();
       
        if (count > 1){ // a bit hacky...
          auto buttons = (GtkWidget **)g_object_get_data(G_OBJECT(dialogObject->dialog()), "buttons");
          if (buttons && buttons[2]) {
            gtk_widget_set_visible(GTK_WIDGET(buttons[2]), false);
          } else {
            TRACE("Error:: cannot find button \"%s\" to hide.\n", _("Link"));
          }  
        }
    
       

        g_free(target);

        dialogObject->run();
        TRACE("dialogObject->run(), target=%s, uri=%s\n", target, uriList);
        d->dropDone(true);
        
      } else {
        auto message = g_strdup_printf(" %s %s (%s)\n", _("Drag:"), _("Invalid target folder"), target);
        Print::printWarning(Child::getOutput(), message);
        TRACE("Source and target are the same: %s\n", source);
        d->dropDone(false);
      }
done:
        auto gridview_p = (GridView<Type> *)Child::getGridviewObject();
        resetGridviewCSS(gridview_p);
        inGridView = false;
        
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


