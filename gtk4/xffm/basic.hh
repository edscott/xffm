#ifndef BASIC_HH
#define BASIC_HH
namespace xf {
  class Basic {
private:

    static void *destroy_f(void *window){
      gtk_window_destroy(GTK_WINDOW(window));
      return NULL;
    }

    static void *present_f(void *window){
      gtk_window_present(GTK_WINDOW(window));
      return NULL;
    }
public:

    static void present(GtkWindow *window){
      context_function(present_f, (void *)window);
    }

    static void destroy(GtkWindow *window){
      context_function(destroy_f, (void *)window);
    }

    static gchar *
    esc_string (const gchar * string) {
        gint i, j, k;
        const gchar *charset = "\\\"\' ()|<>";

        for(j = 0, i = 0; i < strlen (string); i++) {
            for(k = 0; k < strlen (charset); k++) {
                if(string[i] == charset[k])
                    j++;
            }
        }
        gchar *estring = (gchar *) calloc (1, strlen (string) + j + 1);
        for(j = 0, i = 0; i < strlen (string); i++, j++) {
            for(k = 0; k < strlen (charset); k++) {
                if(string[i] == charset[k])
                    estring[j++] = '\\';
            }
            estring[j] = string[i];
        }
        TRACE ("ESC:estring=%s\n", estring);
        return estring;
    }
    static gchar *
    get_tilde_dir(const gchar *token){
        struct passwd *pw;
        gchar *tilde_dir = NULL;
        while((pw = getpwent ()) != NULL) {
            gchar *id = g_strdup_printf("~%s/", pw->pw_name);
            if (strncmp(token, id, strlen(id))==0){
                tilde_dir = g_strdup_printf("%s/", pw->pw_dir);
                g_free(id);
                break;
            }
            g_free(id);
        }
        endpwent ();
        return tilde_dir;
    }

    static gint
    length_equal_string(const gchar *a, const gchar *b){
        int length=0;
        int i;
        for (i = 0; i < strlen(a) && i < strlen(b); i++){
            if (strncmp(a,b,i+1)) {
                length=i;
                break;
            } else {
                length=i+1;
            }
        }
         TRACE("%s --- %s differ at length =%d\n", a,b,length);
       return length;
    }

    static void
    concat(gchar **fullString, const gchar* addOn){
        if (!(*fullString)) {
          *fullString = g_strdup(addOn);
        }
        auto newString = g_strconcat(*fullString, addOn, NULL);
        g_free(*fullString);
        *fullString = newString;
    }
    static GtkBox *
    mkEndBox(){ // not a hack, according to gnome people...
      auto endBox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(endBox), false);
      auto spacer = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_widget_set_hexpand(GTK_WIDGET(spacer), true);
      boxPack0(endBox,GTK_WIDGET(spacer), TRUE, TRUE, 0);
      return endBox;
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
    static void
    flushGTK(void){
      while (g_main_context_pending(NULL))
        g_main_context_iteration(NULL, TRUE);
    }
    static gchar *
    utf_string (const gchar * t) {
      if(!t) { return g_strdup (""); }
      if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
      /* so we got a non-UTF-8 */
      /* but is it a valid locale string? */
      gchar *actual_tag;
      actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
      if(actual_tag)
          return actual_tag;
      /* So it is not even a valid locale string... 
       * Let us get valid utf-8 caracters then: */
      const gchar *p;
      actual_tag = g_strdup ("");
      for(p = t; *p; p++) {
          // short circuit end of string:
          gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
          if(g_utf8_validate (p, -1, NULL)) {
              gchar *qq = g_strconcat (actual_tag, p, NULL);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          } else if(r) {
              gchar *qq = g_strconcat (actual_tag, r, NULL);
              g_free (r);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          }
          // convert caracter to utf-8 valid.
          gunichar gu = g_utf8_get_char_validated (p, 2);
          if(gu == (gunichar) - 1) {
              gu = g_utf8_get_char_validated ("?", -1);
          }
          gchar outbuf[8];
          memset (outbuf, 0, 8);
          gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
          if(outbuf_len < 0) {
              ERROR ("utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
          }
          gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
          g_free (actual_tag);
          actual_tag = qq;
      }
      return actual_tag;
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
    
    static GtkButton *mkButton(const char *iconName, const char *markup){
      auto button = gtk_button_new();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      if (iconName){
        auto image = gtk_image_new_from_icon_name(iconName);
        gtk_box_append (box, GTK_WIDGET(image));
      }
      if (markup){
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        gtk_box_append (box, GTK_WIDGET(label));
      }
      gtk_button_set_child(GTK_BUTTON(button), GTK_WIDGET(box));
      return GTK_BUTTON(button);
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
    static GList *getChildren(GtkBox *box){
      GList *list = NULL;
      GtkWidget *w = gtk_widget_get_first_child(GTK_WIDGET(box));
      if (w) {
        list = g_list_append(list, w);
        while ((w=gtk_widget_get_next_sibling(w)) != NULL) list = g_list_append(list, w);
      }
      return list;
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
    static 
    GtkMenuButton *newMenuButton(const gchar *icon, const gchar *tooltipText){
      if (!tooltipText && !icon) {
        fprintf(stderr, "Util::newMenuButton(): programing error.\n");
        exit(1);
      }
      auto button = GTK_MENU_BUTTON(gtk_menu_button_new());
      if (tooltipText) {
        if (!icon) gtk_menu_button_set_label(button, tooltipText);
        else setTooltip(GTK_WIDGET(button), tooltipText);
      }
      if (icon) gtk_menu_button_set_icon_name(button, icon);

      gtk_widget_set_can_focus (GTK_WIDGET(button), FALSE);
      gtk_menu_button_set_has_frame(button, FALSE);
      return button;
    }

private:
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
    static char
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
    
  };

}
#endif
