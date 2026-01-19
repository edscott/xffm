#ifndef XF_UTIL_HH
#define XF_UTIL_HH
#define MAX_LINES_IN_BUFFER 10000    
namespace xf {
  template <class Type> class MainMenu;
  template <class Type>
  class Util {
  public:
      
   static void
  defaultColors(GtkButton *button, void *data){
    auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
    MainMenu<Type>::closePopover(menu);
    const char *x[]={"Bg", "Fg", NULL};
    for (const char **p = x; p && *p; p++){
      auto key = g_strconcat((const char *)data, *p, NULL);
      Settings::removeKey("xfterm", key);
      g_free(key);
    }
    CSS::init();
    
      TRACE("Basic::context_function for reloadAll_f\n");
    Basic::context_function(reloadAll_f, NULL);
    
    return;
  }

    static void
    setColor(GObject* source_object, GAsyncResult* result, gpointer data){
      auto dialog =(GtkColorDialog*)source_object;
      GError *error_=NULL;
      auto item = (const char *)data;

      GdkRGBA *color = gtk_color_dialog_choose_rgba_finish (dialog, result, &error_);

      if (color) {
        TRACE("setColor: r=%f, g=%f, b=%f, a=%f\n",
          color->red, color->green, color->blue, color->alpha);
        short int red = color->red * 255;
        short int green = color->green * 255;
        short int blue = color->blue * 255;
        TRACE("color #%02x%02x%02x for %s\n", red, green, blue, item);
        char buffer[32];
        snprintf(buffer, 32, "#%02x%02x%02x", red, green, blue);
        Settings::setString("xfterm", item, buffer);
        CSS::init();
      TRACE("Basic::context_function for reloadAll_f\n");
        Basic::context_function(reloadAll_f, NULL);
      } else {
        TRACE("No color selected.\n");
      }
      g_free(color);
    }

    
    static void *reloadAll_f(void *data){
        TRACE("util.hh reloadAll_f()...\n");
        auto notebook = GTK_NOTEBOOK(g_object_get_data(G_OBJECT(Child::mainWidget()), "notebook"));
        auto n = gtk_notebook_get_n_pages(notebook);
        for (int i=0; i<n; i++){
          auto child = gtk_notebook_get_nth_page(notebook, i);
          auto path = Child::getWorkdir(child);
          Workdir<Type>::setWorkdir(path, child);
        }
        return NULL;
    }
    
    

  private:
    static void
    clear (GtkButton *button, void *data){
     auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu")); 
     MainMenu<Type>::closePopover(menu);
     auto childWidget =Child::getChild();
      auto output = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(childWidget), "output"));
      Print::clear_text(output);
      return ;
    }

  public:
    static void
    terminalColors(GtkButton *button, void *data){

      auto menu = GTK_POPOVER(g_object_get_data(G_OBJECT(button), "menu"));
      MainMenu<Type>::closePopover(menu);
      
      auto dialog = gtk_color_dialog_new();
      //gtk_widget_set_parent(GTK_WIDGET(dialog), Child::mainWidget());
      gtk_color_dialog_set_modal (dialog, TRUE);
      gtk_color_dialog_set_title (dialog, "Color dialog");
      gtk_color_dialog_choose_rgba (dialog, GTK_WINDOW(Child::mainWidget()), 
          NULL, NULL, Util::setColor, data);
    }

    static void
    popup(GtkButton *self, void *data){
      auto menu = GTK_POPOVER(data);      
      gtk_popover_popup(menu);
    }

    private:
    static gboolean openMenu(GtkGestureClick* self,
              gint n_press,
              gdouble x,
              gdouble y,
              gpointer data){
      auto menu = GTK_POPOVER(data);
      // position is relative to the parent/default widget.
      TRACE("position %lf,%lf\n", x, y);
      gtk_popover_popup(menu);
      return TRUE;
    }

#define MAX_NAME_LENGTH 13
    static gboolean
    chopBeginning (gchar * b) {
        gint len = strlen (b);

        if(len <= MAX_NAME_LENGTH) {
            return FALSE;
        }
        gint newStart = len - MAX_NAME_LENGTH;
        memmove(b, b+newStart, MAX_NAME_LENGTH+1);
        return TRUE;
    }



    static gchar *
    compact_line(const gchar *line){
        //1. Remove leading and trailing whitespace
        //2. Compact intermediate whitespace

        gchar *newline= g_strdup(line); 
        g_strstrip(newline);
        gchar *p = newline;
        for(;p && *p; p++){
            if (*p ==' ') g_strchug(p+1);
        }
        return newline;
    }


    public:


    static bool
    cd (const gchar **v, GtkWidget *child) {   
        if (v[1] == NULL){
          return Workdir<Type>::setWorkdir(g_get_home_dir());
        }
        // tilde and $HOME
        if (strncmp(v[1], "~", strlen("~")) == 0){
          const char *part2 = v[1] + strlen("~");
          char *dir = g_strconcat(g_get_home_dir(), part2, NULL);
          auto retval = Workdir<Type>::setWorkdir(dir);
          g_free(dir);
          return retval;
        }
        if (strncmp(v[1], "$HOME", strlen("$HOME")) == 0){
          const char *part2 = v[1] + strlen("$HOME");
          char *dir = g_strconcat(g_get_home_dir(),  part2, NULL);
          auto retval = Workdir<Type>::setWorkdir(dir);
          g_free(dir);
          return retval;
        }

        // must allow relative paths too.
        if (!g_path_is_absolute(v[1])){
          //const gchar *wd = Child::getWorkdir(child);
          //bool isRoot = (strcmp(wd, G_DIR_SEPARATOR_S)==0);
          char *dir =  g_strconcat(Child::getWorkdir(child), G_DIR_SEPARATOR_S, v[1], NULL);
          gchar *rpath = realpath(dir, NULL);
          g_free(dir);
          if (!rpath) return false;

          if(!g_file_test (rpath, G_FILE_TEST_IS_DIR)) {
            g_free(rpath);
            return false; 
          }
          auto retval = Workdir<Type>::setWorkdir(rpath);
          g_free(rpath);
          return retval;
        }
        // absolute path

        gchar *rpath = realpath(v[1], NULL);
        if (!rpath) return false;
        

        auto retval = Workdir<Type>::setWorkdir(rpath);
        g_free(rpath);

        return retval;
    }

    static 
    GtkTextView *newTextView(void){
        auto output = GTK_TEXT_VIEW(gtk_text_view_new ());
        gtk_text_view_set_monospace (output, TRUE);
        gtk_widget_set_can_focus(GTK_WIDGET(output), FALSE);
        gtk_text_view_set_wrap_mode (output, GTK_WRAP_WORD);
        gtk_text_view_set_cursor_visible (output, FALSE);

        gtk_widget_add_css_class (GTK_WIDGET(output), "output" );
        gtk_widget_add_css_class (GTK_WIDGET(output), "outputview" );
        UtilBasic::setFontCss(GTK_WIDGET(output));

        return output;
    }

    private:
    public:
private:

    static const gchar *fixTerminalEditor(const gchar *e){
        // Terminal based editors...
        for (auto p=Basic::getTerminalEditors(); p && *p; p++){
            if (strncmp(e, *p,strlen(*p))==0){
                auto terminalCmd = Basic::getTerminalCmd();
                // A terminal based editor.
                static gchar *f = g_strdup_printf("%s %s", terminalCmd, e); 
                setenv("EDITOR", f, 1);
                return f;
            }
        }
        return e;
    }

    static const gchar *fixGvim(const gchar *e){
        // Do not fork gvim, so that git commit works...
        if (e && strcmp(e, "gvim")==0) return "gvim -f";
        return e;
    }

    static void setEditor(void){
        static gboolean done = FALSE;
        if (done) return;

        // Environment variable EDITOR was defined previously.
        const gchar *e = getenv("EDITOR");
        if (e && strlen(e)==0) e = NULL;

        else if (e) { // Predefined value.
            e = fixGvim(e);
            e = fixTerminalEditor(e);
            setenv("EDITOR", e, 1);
            done = TRUE;
            return;
        }

        // Environment variable EDITOR was not defined.
        // Look for one.
        auto editors = Basic::getEditors();
        for (auto p=editors; p && *p; p++){
            auto s = g_strdup(*p);
            if (strchr(s, ' ')) *strchr(s, ' ') = 0;
            auto t = g_find_program_in_path (s);
            g_free(s);
            if (t) {
                e=*p;
                g_free(t);
                break;  
            }  
        }

        if (!e){
            INFO("No suitable EDITOR found, defaulting to gvim. Please install or define EDITOR environment variable.\n");
            e="vi";
        } else {
            INFO("Found EDITOR %s\n", e);

        }
        e = fixGvim(e);
        e = fixTerminalEditor(e);
        setenv("EDITOR", e, 1);
        done = TRUE;
        return;
    }

    
public:

  };
}
#endif
