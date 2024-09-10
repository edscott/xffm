#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {

  class UtilBasic: 
    public Child,
    public Clipboard
  {
    public:
      static bool
      alwaysTerminal(const char *command){
        char *key = g_strdup(command);
        if (strchr(key, ' ')) *strrchr(key, ' ') = 0;
        const char *always[] = {"vi", "vim", "nano", "emacs", NULL};
        for (auto p=always; p && *p; p++){
          if (strcmp(key, *p) == 0) {
            g_free(key);
            return true;
          }
        }
        g_free(key);
        return false;
      }
      
    static GtkBox *
    mkEndBox(){ // not a hack, according to gnome people...
       auto endBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(endBox), false);
        auto spacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(spacer), true);
        UtilBasic::boxPack0(endBox,GTK_WIDGET(spacer), TRUE, TRUE, 0);
        return endBox;
    }
    static
    GtkTextView *createInput(void){
        GtkTextView *input = GTK_TEXT_VIEW(gtk_text_view_new ());
        
        gtk_text_view_set_pixels_above_lines (input, 5);
        gtk_text_view_set_pixels_below_lines (input, 5);
        gtk_text_view_set_monospace (input, TRUE);
        gtk_text_view_set_editable (input, TRUE);
        gtk_text_view_set_cursor_visible (input, TRUE);
        gtk_text_view_place_cursor_onscreen(input);
        gtk_text_view_set_wrap_mode (input, GTK_WRAP_CHAR);
        gtk_widget_set_can_focus(GTK_WIDGET(input), TRUE);
        
        return input;
    }
    static void
    flushGTK(void){
      while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, TRUE);
    }
    static void
    setTooltip(GtkWidget *w, const gchar *text){
      //auto t =g_strconcat("<span color=\"yellow\" bgcolor=\"blue\"><i>", text, "</i></span>", NULL);
      auto t =g_strconcat("<span color=\"yellow\"><i>", text, "</i></span>", NULL);
      gtk_widget_set_tooltip_markup (w,t);
      //gtk_widget_add_css_class (w, "tooltip" );
      g_free(t);
      return;
    }
    static 
    GtkButton *newButton(const gchar *icon, const gchar *tooltipText){
      auto button = GTK_BUTTON(gtk_button_new_from_icon_name(icon));
      setTooltip(GTK_WIDGET(button), tooltipText);

      gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
      gtk_button_set_has_frame(button, FALSE);
      return button;
    }
    static void setAsDialog(GtkWidget *widget, const char *Xname, const char *Xclass){
        bool OK = false;
 #ifdef GDK_WINDOWING_X11
        GdkDisplay *displayGdk = gdk_display_get_default();
        if (GDK_IS_X11_DISPLAY (displayGdk)) {
          OK = true;
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
#ifdef GDK_WINDOWING_WAYLAND
//#warning "Compiling for Wayland (unstable)"
        OK = true;
#endif        
#ifdef GDK_WINDOWING_WIN32
#warning "Compiling for Windows (unstable)"
        OK = true;
#endif
        if (!OK) {
          g_error ("Unsupported GDK backend");
          exit(1);
        }
   }
  static 
  void boxPack0(  
      GtkBox* box,
      GtkWidget* child,
      gboolean expand,
      gboolean fill,
      guint padding)
  {
    GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(box));
    gtk_box_append(box, child);
    gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_START);
    // other options: GTK_ALIGN_START, GTK_ALIGN_END, GTK_ALIGN_BASELINE
    if (orientation == GTK_ORIENTATION_HORIZONTAL){
      gtk_widget_set_hexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_start(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_end(GTK_WIDGET(child), padding);
    } else if (orientation == GTK_ORIENTATION_VERTICAL){
      gtk_widget_set_vexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_top(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_bottom(GTK_WIDGET(child), padding);
    } else {
      fprintf(stderr, "boxPack0(): programming error. Exit(2)\n");
      exit(2);
    }
  }

    static GList *getChildren(GtkBox *box){
      GList *list = NULL;
      GtkWidget *w = gtk_widget_get_first_child(GTK_WIDGET(box));
      if (w) {
        list = g_list_append(list, w);
        while ((w=gtk_widget_get_next_sibling(w)) != NULL) list = g_list_append(list, w);
      }
      return list;
    }


  static void *context_function(void * (*function)(gpointer), void * function_data){
      pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
      auto result=GINT_TO_POINTER(-1);
      void *arg[] = {
          (void *)function,
          (void *)function_data,
          (void *)&mutex,
          (void *)&signal,
          (void *)&result
      };
      gboolean owner = g_main_context_is_owner(g_main_context_default());
      if (owner){
          context_function_f(arg);
      } else {
          g_main_context_invoke(NULL, CONTEXT_CALLBACK(context_function_f), arg);
          pthread_mutex_lock(&mutex);
          if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
          pthread_mutex_unlock(&mutex);
      }
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&signal);
      return result;
  }
  static gboolean
  context_function_f(gpointer data){
      void **arg = (void **)data;
      void * (*function)(gpointer) = (void* (*)(void*))arg[0];
      gpointer function_data = arg[1];
      pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
      pthread_cond_t *signal = (pthread_cond_t *)arg[3];
      auto result_p = (void **)arg[4];
      void *result = (*function)(function_data);
      pthread_mutex_lock(mutex);
      *result_p = result;
      pthread_cond_signal(signal);
      pthread_mutex_unlock(mutex);
      return FALSE;
  }
    private:


 
    public:
    static GtkBox *
    pathbarLabelButton (const char *text) {
        auto label = GTK_LABEL(gtk_label_new(""));
        auto eventBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        if (text) {
            auto v = UtilBasic::utf_string(text);
            auto g = g_markup_escape_text(v, -1);
            g_free(v);
            auto markup = g_strdup_printf("   <span size=\"small\">  %s  </span>   ", g);
            g_free(g);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        } else {
            gtk_label_set_markup(label, "");
        }
        UtilBasic::boxPack0 (eventBox, GTK_WIDGET(label), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(eventBox), "label", label);
        g_object_set_data(G_OBJECT(eventBox), "name", text?g_strdup(text):g_strdup("RFM_ROOT"));
        return eventBox;
    }
 
    static void 
    setMenuTitle(GtkPopover *menu, const char *title){
     if (title) {      
        auto titleBox = GTK_BOX(g_object_get_data(G_OBJECT(menu), "titleBox"));
        auto titleLabel = GTK_LABEL(g_object_get_data(G_OBJECT(menu), "titleLabel"));
        auto markup = g_strdup_printf("<span color=\"blue\"><b>%s</b></span>", title);
        gtk_label_set_markup(titleLabel, markup);
        g_free(markup);
      }

    }
    static GtkPopover *mkMenu(const char **text, GHashTable **mHash, const gchar *title){
      auto vbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
      auto titleBox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append (vbox, GTK_WIDGET(titleBox));

      auto titleLabel = GTK_LABEL(gtk_label_new(""));

      gtk_box_append (titleBox, GTK_WIDGET(titleLabel));
      
      

      GtkPopover *menu = GTK_POPOVER(gtk_popover_new ());
      g_object_set_data(G_OBJECT(menu), "titleBox", titleBox);
      g_object_set_data(G_OBJECT(menu), "titleLabel", titleLabel);
      g_object_set_data(G_OBJECT(menu), "vbox", vbox);
      setMenuTitle(menu, title);

      gtk_popover_set_autohide(GTK_POPOVER(menu), TRUE);
      gtk_popover_set_has_arrow(GTK_POPOVER(menu), FALSE);
      gtk_widget_add_css_class (GTK_WIDGET(menu), "inquire" );


      for (const char **p=text; p && *p; p++){
        auto hbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        auto label = GTK_LABEL(gtk_label_new(""));
        gtk_label_set_markup(label, *p);
        auto icon = (const char *) g_hash_table_lookup(mHash[0], *p);
        //DBG("icon is %s\n",icon);
        if (icon){
          auto image = gtk_image_new_from_icon_name(icon);
          boxPack0(hbox, GTK_WIDGET(image),  FALSE, FALSE, 0);
        }
        boxPack0(hbox, GTK_WIDGET(label),  FALSE, FALSE, 5);

        if (GPOINTER_TO_INT(g_hash_table_lookup(mHash[2], *p)) == -1) {
          boxPack0(vbox, GTK_WIDGET(hbox),  FALSE, FALSE, 0);
          g_object_set_data(G_OBJECT(menu), *p, hbox);
          gtk_widget_set_visible(GTK_WIDGET(hbox), TRUE);
          continue;
        } else {
          GtkButton *button = GTK_BUTTON(gtk_button_new());
          g_object_set_data(G_OBJECT(button), "menu", menu);
          gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(hbox));
          boxPack0(vbox, GTK_WIDGET(button),  FALSE, FALSE, 0);
          g_object_set_data(G_OBJECT(menu), *p, button);
          gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
          gtk_widget_set_visible(GTK_WIDGET(button), TRUE);
        
          auto callback = g_hash_table_lookup(mHash[1], *p);
          TRACE("mkMenu() callback=%p, icon=%s\n", callback, icon);
          if (callback) {
            auto data = g_hash_table_lookup(mHash[2], *p);
            g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK(callback), data);
          }
          g_object_set_data(G_OBJECT(button), "menu", menu);
          g_object_set_data(G_OBJECT(menu), *p, button);
          continue;
        }
      }
          

      gtk_popover_set_child (menu, GTK_WIDGET(vbox));
      return menu;
    }
    static
        char
    ftypelet (mode_t bits) {
#ifdef S_ISBLK
        if(S_ISBLK (bits))
            return 'b';
#endif
        if(S_ISCHR (bits))
            return 'c';
        if(S_ISDIR (bits))
            return 'd';
        if(S_ISREG (bits))
            return '-';
#ifdef S_ISFIFO
        if(S_ISFIFO (bits))
            return 'p';
#endif
#ifdef S_ISLNK
        if(S_ISLNK (bits))
            return 'l';
#endif
#ifdef S_ISSOCK
        if(S_ISSOCK (bits))
            return 's';
#endif
#ifdef S_ISMPC
        if(S_ISMPC (bits))
            return 'm';
#endif
#ifdef S_ISNWK
        if(S_ISNWK (bits))
            return 'n';
#endif
#ifdef S_ISDOOR
        if(S_ISDOOR (bits))
            return 'D';
#endif
#ifdef S_ISCTG
        if(S_ISCTG (bits))
            return 'C';
#endif

        /* The following two tests are for Cray DMF (Data Migration
           Facility), which is a HSM file system.  A migrated file has a
           `st_dm_mode' that is different from the normal `st_mode', so any
           tests for migrated files should use the former.  */

#ifdef S_ISOFD
        if(S_ISOFD (bits))
            /* off line, with data  */
            return 'M';
#endif
#ifdef S_ISOFL
        /* off line, with no data  */
        if(S_ISOFL (bits))
            return 'M';
#endif
        return '?';
    }

    static gchar *
    modeString (mode_t mode) {
        gchar *str=(gchar *)calloc(1, 15);
        if (!str) g_error("calloc: %s", strerror(errno));
        str[0] = ftypelet (mode);
        str[1] = mode & S_IRUSR ? 'r' : '-';
        str[2] = mode & S_IWUSR ? 'w' : '-';
        str[3] = mode & S_IXUSR ? 'x' : '-';

        str[4] = mode & S_IRGRP ? 'r' : '-';
        str[5] = mode & S_IWGRP ? 'w' : '-';
        str[6] = mode & S_IXGRP ? 'x' : '-';

        str[7] = mode & S_IROTH ? 'r' : '-';
        str[8] = mode & S_IWOTH ? 'w' : '-';
        str[9] = mode & S_IXOTH ? 'x' : '-';
        if(mode & S_ISUID)
            str[3] = mode & S_IXUSR ? 's' : 'S';
        if(mode & S_ISGID)
            str[6] = mode & S_IXGRP ? 's' : 'S';
        if(mode & S_ISVTX)
            str[9] = mode & S_IXOTH ? 't' : 'T';
        str[10] = 0;
        return (str);
    }

    static gchar *statInfo(const struct stat *st){
        if (!st){
            DBG("util::statinfo: st is null\n");
            return NULL;
        }
        auto mode = modeString(st->st_mode);
        struct passwd *pw = getpwuid (st->st_uid);
        struct group *gr = getgrgid(st->st_gid);
        auto links = st->st_nlink;

        auto user = pw ? g_strdup(pw->pw_name) : g_strdup_printf("%d", st->st_uid);
        auto group = gr ? g_strdup(gr->gr_name) : g_strdup_printf("%d", st->st_gid);
        auto date = dateString(st->st_mtime);
        auto size = sizeString(st->st_size);

        auto info = g_strdup_printf("%s %3luL %s %s %s:%s ", 
            mode, (unsigned long)links, size, date, user, group);
        //auto info = g_strdup_printf("%s %s:%3lu %s %s:%s ", mode, _("Links"), (unsigned long)links, date, user, group);
        g_free(size);
        g_free(date);
        g_free(user);
        g_free(group);
        g_free(mode);
        return info;
    }
    
    static gchar *
    sizeString (size_t size) {
        if (size > 1024 * 1024 * 1024){
            return g_strdup_printf(" %3ld GB", size/(1024 * 1024 * 1024));
        }
        if (size > 1024 * 1024){
            return g_strdup_printf(" %3ld MB", size/(1024 * 1024));
        }
        if (size > 1024){
            return g_strdup_printf(" %3ld KB", size/(1024));
        }
        return g_strdup_printf(" %3ld B ", size);
    }
    static gchar *
    dateString (time_t the_time) {
        //pthread_mutex_lock(&dateStringMutex);

#ifdef HAVE_LOCALTIME_R
            struct tm t_r;
#endif
            struct tm *t;

#ifdef HAVE_LOCALTIME_R
            t = localtime_r (&the_time, &t_r);
#else
            t = localtime (&the_time);
#endif
            gchar *date_string=
                g_strdup_printf (" %04d/%02d/%02d  %02d:%02d", t->tm_year + 1900,
                     t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
       // pthread_mutex_unlock(&dateStringMutex);

        return date_string;
    }
    static gchar *fileInfo(const gchar *path){
        gchar *file = g_find_program_in_path("file");
        if (!file) return g_strdup("\"file\" command not in path!");
        gchar *result = NULL; 
        gchar *command = g_strdup_printf("%s \'%s\'", file, path);
        result = pipeCommand(command);
        g_free(command);
        g_free(file);

        if (result){
            gchar *retval;
            if (strchr(result, ':')) retval=g_strdup(strchr(result, ':')+1);
            else retval = g_strdup(result);
            g_free(result);
            gchar *p=retval;
            for(;p && *p; p++) {
                if (*p == '<') *p='[';
                else if (*p == '>') *p=']';
            }
            lineBreaker(retval, 40);
            return retval;
        }
        return NULL;
    }
#define PAGE_LINE 256

    static gchar *pipeCommand(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar line[PAGE_LINE];
            line[PAGE_LINE - 1] = 0;
            if (!fgets (line, PAGE_LINE - 1, pipe)){
                  DBG("fgets(%s): %s\n", command, "no characters read.");
            } else {
              if (strchr(line, '\n'))*(strchr(line, '\n'))=0;
            }
            pclose (pipe);
            return g_strdup(line);
        } 
        return NULL;
    }

    static gchar *pipeCommandFull(const gchar *command){
        FILE *pipe = popen (command, "r");
        if(pipe) {
            gchar *result=g_strdup("");
            gchar line[PAGE_LINE];
            while (fgets (line, PAGE_LINE - 1, pipe) && !feof(pipe)){
                line[PAGE_LINE - 1] = 0;
                auto g = g_strconcat(result, line, NULL);
                g_free(result);
                result = g;
            }
            pclose (pipe);
            return result;
        } 
        return NULL;
    }

    static void 
    lineBreaker(gchar *inputLine, gint lineLength){
        if (strlen(inputLine) > lineLength){
            gchar *remainder;
            gchar *p = inputLine+lineLength;
            do{
                if (*p ==' ' || *p =='_') {
                    *p = '\n';
                    remainder = p+1;
                    break;
                }
                p++;
            } while (*p);
            if (*p != 0) lineBreaker(remainder, lineLength);
        }
    }
    
    static gboolean backupType(const gchar *file){
        if (!file) return FALSE;
        // GNU backup type:
         if(file[strlen (file) - 1] == '~' || 
                 file[strlen (file) - 1] == '%'|| 
                 file[strlen (file) - 1] == '#') return TRUE;
        // MIME backup type:
        const gchar *e = strrchr(file, '.');
        if (e){
            if (strcmp(e,".old")==0) return TRUE;
            else if (strcmp(e,".bak")==0) return TRUE;
            else if (strcmp(e,".sik")==0) return TRUE;
        }
        return FALSE;
    }

   
  };
}
#endif

