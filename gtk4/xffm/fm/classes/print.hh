#ifndef PRINT_HH
#define PRINT_HH
#define MAX_LINES_IN_BUFFER 10000    
namespace xf {
  class Print {
  public:
   /////   print  //////
    
    static char *inputText(GtkTextView *input){
        auto buffer = gtk_text_view_get_buffer(input);
        GtkTextIter  start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_strstrip(text);   
        if (text[0] == '$') { // Eliminate any preceeding $
          text[0] = ' ';
          g_strstrip(text);   
        }
        return text;
    }

    static void *
    scroll_to_top(GtkTextView *textview){
        if (!textview) return NULL;
        // make sure all text is written before attempting scroll
        Basic::flushGTK();
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_view_scroll_to_iter (textview,
                              &start,
                              0.0,
                              FALSE,
                              0.0, 0.0);        
        Basic::flushGTK();
        return NULL;
    }

    static void *
    scroll_to_bottom(GtkTextView *textview){
        if (!textview) return NULL;
        // make sure all text is written before attempting scroll
        Basic::flushGTK();
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto mark = gtk_text_buffer_create_mark (buffer, "scrolldown", &end, FALSE);
        gtk_text_view_scroll_to_mark (textview, mark, 0.2,    /*gdouble within_margin, */
                                      TRUE, 1.0, 1.0);
        //gtk_text_view_scroll_mark_onscreen (textview, mark);
        Basic::flushGTK();
        return NULL;
    }

    static void clearText(GtkTextView *textview){
      if (!textview) return;
      void *arg[]={(void *)textview, NULL};
      Basic::context_function(clear_text_buffer_f, arg);
    }
    static void clearText(void){
      clearText(Child::getOutput());
    }

    static void clear_text(GtkTextView *textview){ // deprecated
      clearText(textview);
    }

    static void showText(GtkTextView *textview){
        if (!textview) return;
        auto vpane = GTK_PANED(g_object_get_data(G_OBJECT(textview), "vpane"));
        void *arg[]={(void *)vpane, NULL, NULL, NULL, NULL};
        Basic::context_function(show_text_buffer_f, arg);
    }

    static void print(GtkTextView *textview, gchar *string){
        print(textview, NULL, string);
    }

    static void //  will free string.
    printInfo(GtkTextView *textview, gchar *string){
      if (!string) return;
      showText(textview);
      if (string[strlen(string)-1] != '\n') {
        print(textview, "green/black_bg", g_strconcat(string,"\n",NULL));
        g_free(string);
      } else {
        print(textview, "green/black_bg", string);
      }
//      print(textview, EMBLEM_ABOUT,  "green/black_bg", string);
    }

    static void //  will free string.
    printWarning(GtkTextView *textview, gchar *string){
      showText(textview);
      const char *ret = NULL;
      printIcon(textview, EMBLEM_WARNING, "yellow/black_bg", string);
    }

    static void //  will free string.
    printError(GtkTextView *textview, gchar *string){
      showText(textview);
      const char *ret = NULL;
      if (string[strlen(string)-1] != '\n') ret = "\n"; //hack
      printIcon(textview, EMBLEM_ERROR,"cyan/black_bg", string);
    }

    static void print(GtkTextView *textview, const gchar *tag, gchar *string){
        if (!textview) return;
        void *arg[]={(void *)textview, (void *)tag, (void *)string};
        Basic::context_function(print_f, arg);
        g_free(string);
    }
/*
    static void printIcon(GtkTextView *textview, const gchar *iconname, gchar *string)
    {
        if (!textview) return;
        auto icon = Texture<bool>::load16(iconname);
        void *arg[]={(void *)icon, (void *)textview, NULL, (void *)string};
        Basic::context_function(print_i, arg);
        g_free(string);
    }
*/

    static void print(GtkTextView *textview, 
                              const gchar *iconname, 
                              const gchar *tag, 
                              gchar *string){
      printIcon(textview, iconname, tag, string);
    }
private:

    static void printIcon(GtkTextView *textview, 
                              const gchar *iconname, 
                              const gchar *tag, 
                              gchar *string){
        if (!textview) return;
        auto icon = Texture<bool>::load16(iconname);
        void *arg[]={(void *)icon, (void *)textview, (void *)tag, (void *)string};
        Basic::context_function(print_i, arg);
        g_free(string);
    }
    
    static void *
    print_i(void *data){
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto textview = GTK_TEXT_VIEW(arg[1]);
        auto paintable = GDK_PAINTABLE(arg[0]);
        if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        gtk_text_buffer_insert_paintable (buffer, &end, paintable);
        return print_f(arg+1);
    }

  static void *
  clear_text_buffer_f(void *data){
      if (!data) return GINT_TO_POINTER(-1);
      auto arg=(void **)data;
      auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (arg[0]));
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_delete (buffer, &start, &end);
      return NULL;
  }
  static void * 
  print_f(void *data){
      if (!data) return GINT_TO_POINTER(-1);
      void **arg = (void **)data;
      auto textview = GTK_TEXT_VIEW(arg[0]);
      if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
      auto tag = (const gchar *)arg[1];
      auto string = (const gchar *)arg[2];

      auto buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
      if (trim_output(buffer)){
          DBG("trim_output():: textview is at line limit.\n");
      }

      auto tags = resolve_tags(buffer, tag);
      if(string && strlen (string)) {
          insert_string (buffer, string, tags);
      }
      g_free(tags);
      scroll_to_bottom(textview);
      return NULL;
  }


  static gboolean trim_output(GtkTextBuffer *buffer) {
  //  This is just to prevent a memory overrun (intentional or unintentional).
      auto line_count = gtk_text_buffer_get_line_count (buffer);
      auto max_lines_in_buffer = MAX_LINES_IN_BUFFER;
      if (line_count > max_lines_in_buffer) {
          gtk_text_buffer_set_text(buffer, "", -1);
          return TRUE;
      }
      return FALSE;
  }
  static void insert_string (GtkTextBuffer * buffer, const gchar * s, GtkTextTag **tags) {
      if(!s) return;
      GtkTextIter start, end, real_end;
      auto line = gtk_text_buffer_get_line_count (buffer);
      gchar *a = NULL;
     

      gchar esc[]={0x1B, '[', 0};
      gboolean head_section = strncmp(s, esc, strlen(esc));
      const char *esc_location = strstr (s, esc);
      if(esc_location) {      // escape sequence: <ESC>[

          // do a split
              TRACE( "0.splitting %s\n", s);
          gchar **split = g_strsplit(s, esc, -1);
          insert_string (buffer, split[0], NULL);
          gchar *fullTag=NULL;
          gchar **pp=split+1;
          GtkTextTag **textviewTags = NULL;
          // single
          for (;pp && *pp; pp++){
              gchar *code = *pp;

              if (*code == 'K'){
                   // erase from cursor to eol
                   // insert string with whatever tag you have.
                  insert_string (buffer, code+1, textviewTags);
                  continue;
              }
              /* if (strncmp(code, "0K", 2)==0 ||strncmp(code, "1K", 2)==0 || strncmp(code, "2K", 2)==0){
                  // erase from cursor to eol, from bol to cursor, erase line
                  Basic::concat(&fullString, code+2);
                  continue;
              }*/
               // split on m
              if (*code == 'm'){
                  g_free(fullTag); fullTag = NULL;
                  g_free(textviewTags); textviewTags = NULL;
                  insert_string (buffer, code+1, NULL);
                  continue;
              }
              gchar **ss = g_strsplit(code, "m", 2);
              gchar **codes;
              // Check for multiple tags
              if (strchr(code, ';')){
                  codes = g_strsplit(ss[0], ";", -1);
              } else {
                  codes = (gchar **)calloc(2, sizeof(gchar*));
                  codes[0] = g_strdup(ss[0]);
              }
              TRACE( "1.splitting %s --> %s, %s: codes[0]=%s\n", *pp, ss[0], ss[1],codes[0]);
              // construct xffm tag
              gchar **t;
              gchar *thisTag=NULL;
              for (t=codes; t && *t; t++){
                  const gchar *ansiTag = get_ansi_tag(*t);
                  if (!ansiTag){
                      if (strcmp(*t, "0")) {
                          g_free(fullTag); fullTag = NULL;
                          g_free(textviewTags); textviewTags = NULL;
                          insert_string (buffer, ss[1], NULL);
                          g_strfreev(ss);
                          goto endloop;
                      } else {
                          TRACE("no ansiTag for \"%s\"\n", *t);
                      }
                      continue;
                  } else {
                      TRACE("ansiTag=%s\n", ansiTag);
                       Basic::concat(&fullTag, "/");
                       Basic::concat(&fullTag, ansiTag);
                      TRACE("fullTag= %s\n", fullTag); 
                  }
              }
              // Insert string
              g_free(textviewTags);
              if (fullTag) textviewTags = resolve_tags(buffer, fullTag);
              else textviewTags = NULL;
              insert_string (buffer, ss[1], textviewTags);
              g_strfreev(ss);
endloop:;
          }
          g_free(fullTag);
          g_free(textviewTags);
          g_strfreev(split);
          // done
          return;
      }

      auto mark = gtk_text_buffer_get_mark (buffer, "rfm-ow");
      gchar *cr = (gchar *)strchr (s, 0x0D);
      if(cr && cr[1] == 0x0A) *cr = ' ';       //CR-LF
      if(strchr (s, 0x0D)) {      //CR
          auto aa = g_strdup (s);
          *strchr (aa, 0x0D) = 0;
          insert_string (buffer, aa, tags);
          g_free (aa);
          const char *caa = strchr (s, 0x0D) + 1;
          if(mark) {
              gtk_text_buffer_get_iter_at_line (buffer, &start, line);
              gtk_text_buffer_move_mark (buffer, mark, &start);
          }
          insert_string (buffer, caa, tags);
          g_free (a);
          // we're done
          return;
      }

      gchar *q = Basic::utf_string (s);
      /// this should be mutex protected since this function is being called
      //  from threads all over the place.
      static pthread_mutex_t insert_mutex =  PTHREAD_MUTEX_INITIALIZER;

      pthread_mutex_lock(&insert_mutex);
      if(mark == NULL) {
          gtk_text_buffer_get_end_iter (buffer, &end);
          gtk_text_buffer_create_mark (buffer, "rfm-ow", &end, FALSE);
      } else {
          gtk_text_buffer_get_iter_at_mark (buffer, &end, mark);
      }

      gtk_text_buffer_get_iter_at_line (buffer, &start, line);
      gtk_text_buffer_get_end_iter (buffer, &real_end);

      // overwrite section
      auto pretext = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
      auto posttext = gtk_text_buffer_get_text (buffer, &end, &real_end, TRUE);
      long prechars = g_utf8_strlen (pretext, -1);
      long postchars = g_utf8_strlen (posttext, -1);
      long qchars = g_utf8_strlen (q, -1);
      g_free (pretext);
      g_free (posttext);
      if(qchars < postchars) {
          gtk_text_buffer_get_iter_at_line_offset (buffer, &real_end, line, prechars + qchars);
      }
      /////
      gtk_text_buffer_delete (buffer, &end, &real_end);
      if(tags) {
          gint tag_count = 0;
          GtkTextTag **tt = tags;
          for (;tt && *tt; tt++)tag_count++;
          switch (tag_count) { // This is hacky
              case 1:
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1, 
                          tags[0], NULL);
                  break;
              case 2:
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
                          tags[0],tags[1], NULL);
                  break;
              default: // Max. 3 tags...
                  gtk_text_buffer_insert_with_tags (buffer, &end, q, -1,
                          tags[0],tags[1],tags[2], NULL);
                  break;
          }
                  
      } else {
          TRACE( "gtk_text_buffer_insert %s\n", q);
          gtk_text_buffer_insert (buffer, &end, q, -1);
      }
      pthread_mutex_unlock(&insert_mutex);
      g_free (q);
      g_free (a);
      return;
  }
 public:
 private:

typedef struct lpterm_colors_t {
    const gchar *id;
    guint16 red;
    guint16 green;
    guint16 blue;
} lpterm_colors_t;

  static
  GtkTextTag *
  resolve_tag (GtkTextBuffer * buffer, const gchar * id) {
      if (!id || !strlen(id)) return NULL;
      lpterm_colors_t lpterm_colors_v[] = {
// xterm colors
          // specific
          {"command", 0x5858, 0x3434, 0xcfcf},
          {"stderr", 0xcccc, 0, 0},
          {"command_id", 0x0000, 0x0000, 0xffff},
          // plain 
          {"gray", 0x8888, 0x8888, 0x8888},
          {"brown", 0x6666, 0x6666, 0x0},
          {"darkcyan", 0x0, 0x6666, 0x6666},
          // normal
          {"black", 0x0000, 0x0000, 0x0000},
          {"red", 0xcdcd, 0x0, 0x0},
          {"green", 0x0, 0xcdcd, 0x0},
          {"orange", 0xffff, 0xbebe, 0x6f6f},//#ffbe6f
          {"yellow", 0xcdcd, 0xcdcd, 0x0},
          {"blue", 0x0, 0x0, 0xeeee},
          {"magenta", 0xcdcd, 0x0, 0xcdcd},
          {"cyan", 0x0, 0xcdcd, 0xcdcd},
          {"white", 0xe5e5, 0xe5e5, 0xe5e5}, //gray
          // bright
          {"Black", 0x7f7f, 0x7f7f, 0x7f7f},
          {"Red", 0xffff, 0x0, 0x0},
          {"Green", 0x0, 0xffff, 0x0},
          {"Yellow", 0xffff, 0xffff, 0x0},
          {"Blue", 0x0, 0x0, 0xffff},
          {"Magenta", 0xffff, 0x0, 0xffff},
          {"Cyan", 0x0, 0xffff, 0xffff},
          {"White", 0xffff, 0xffff, 0xffff}, 

          {NULL, 0, 0, 0}
      }; 
      TRACE("Print::resolve_tag(%s)\n", id);
      if (!id) return NULL;
      GtkTextTag *tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), id);

      if (!tag) { // New tag, foreground.
        for(auto p=lpterm_colors_v; p && p->id; p++) { // Initialize foreground color.
          // Foreground.
          if (strcmp(p->id, id) == 0){
            auto rgba = (GdkRGBA *)calloc(1, sizeof(GdkRGBA));
            rgba->red = (float) p->red / 0xffff;
            rgba->green = (float) p->green / 0xffff;
            rgba->blue = (float) p->blue / 0xffff;
            rgba->alpha = 1.0; 
            // Foreground color.
            tag = gtk_text_buffer_create_tag (buffer, p->id, "foreground-rgba", rgba, NULL);
            TRACE("*** gtk_text_tag_table_lookup(%s) -> %p\n",p->id, tag);
            break;
          }
        }
      }
      if (!tag) { // New tag, background.
        char *bg = g_strdup(id);
        if (strstr(bg, "_bg")) *strstr(bg, "_bg") = 0;
        for(auto p=lpterm_colors_v; p && p->id; p++) { // Initialize background color.
          TRACE("id=\"%s\", bg=\"%s\" p->id=\"%s\"\n", id, bg, p->id);
          if (strcmp(p->id, bg) == 0){
            TRACE("bg gotcha: %s\n", bg);
            auto rgba = (GdkRGBA *)calloc(1, sizeof(GdkRGBA));
            rgba->red = (float) p->red / 0xffff;
            rgba->green = (float) p->green / 0xffff;
            rgba->blue = (float) p->blue / 0xffff;
            rgba->alpha = 1.0; 
            // Background color.
            tag = gtk_text_buffer_create_tag (buffer, id, "background-rgba", rgba, NULL);
            TRACE("*** gtk_text_tag_table_lookup(%s) -> %p\n",p->id, tag);
            break;
          }
        }        
        g_free(bg); 
      }
      // No predefined tag? Specific CSS color. (Currently only foreground).
      if (!tag && id[0] == '#'){
        // create a new tag for color id.
        TRACE("Print::resolve_tag(%s): *** creating new tag.\n", id);
        auto color = (GdkRGBA *)calloc(1, sizeof(GdkRGBA));
        if (!color){
            ERROR("Print::resolve_tag: calloc: %s\n", strerror(errno));
            exit(1);
        }
        if (gdk_rgba_parse (color, id)) {
            TRACE("***tag %s is %lf,%lf,%lf\n", id, color->red,color->green,color->blue); 
            TRACE("***tag %s is 0x%0x,0x%0x,0x%0x\n", id, c->red,c->green,c->blue); 
            tag = gtk_text_buffer_create_tag(buffer, id, "foreground-rgba", color, NULL);
        } 
      }

      if (!tag) {
        DBG("***Error:: resolve_tag(): No GtkTextTag for %s\n", id);
      }
      return tag;
  }

  static GtkTextTag **
  resolve_tags(GtkTextBuffer * buffer, const gchar *tag){
    // 
      if (!tag) return NULL;
      gchar **userTags;
      if (strchr(tag, '/')){
          // multiple tags
          userTags = g_strsplit(tag, "/", -1);
      } else {
          userTags = (gchar **)calloc(2, sizeof(GtkTextTag *));
          userTags[0] = g_strdup(tag);
      }
      gchar **t;
      gint tag_count = 0;
      for (t = userTags;t && *t; t++){
          tag_count++;
      }
      auto tags = (GtkTextTag **)calloc(tag_count+1, sizeof(GtkTextTag *));
      int i=0;
      for (t=userTags; t && *t;t++){
          tags[i] = resolve_tag (buffer, *t);
          if (tags[i] == NULL) {
              TRACE("*** could not resolve tag: \"%s\"\n", *t);
          } else i++;
      }
      g_strfreev(userTags);
      return tags;
  }
 public:
 private:
  static const gchar *
  get_ansi_tag(const gchar *code){
      static sequence_t sequence_v[] = {
              {"black", "30"},
              {"black_bg", "40"},
              {"red", "31"},
              {"red_bg", "41"},
              {"green", "32"},
              {"green_bg", "42"},
              {"yellow", "33"},
              {"yellow_bg", "43"},
              {"blue", "34"},
              {"blue_bg", "44"},
              {"magenta", "35"},
              {"magenta_bg", "45"},
              {"cyan", "36"},
              {"cyan_bg", "46"},
              {"white", "37"},
              {"white_bg", "47"},
            
              {"Black_bg", "100"},
              {"Black", "90"},
              {"Red_bg", "101"},
              {"Red", "91"},
              {"Green_bg", "102"},
              {"Green", "92"},
              {"Yellow_bg", "103"},
              {"Yellow", "93"},
              {"Blue_bg", "104"},
              {"Blue", "94"},
              {"Magenta_bg", "105"},
              {"Magenta", "95"},
              {"Cyan_bg", "106"},
              {"Cyan", "96"},
              {"White_bg", "107"},
              {"White", "97"},

              {"bold", "1"},
              {"bold", "01"},
              {"italic", "4"},
              {"italic", "04"},
              {"blink", "5"},
              {"blink", "05"},
              {NULL, ""},
              {NULL, "0"},
              {NULL, "00"},
              {NULL, "22"},
              {NULL, "24"},
              {NULL, NULL}            // this marks the end of sequences.
      };
      sequence_t *p;
      for(p = sequence_v; p && p->sequence; p++) {
          if(strcmp (code, p->sequence) == 0) {
              return p->id;
          }
      }
      return NULL;

  }
    static void *
    show_text_buffer_f (void *data) {
        if (!data) return GINT_TO_POINTER(-1);
        auto arg=(void **)data;
        auto vpane = GTK_PANED(arg[0]);
        auto fullview =arg[1]; 
        auto small = arg[2];
        auto err = arg[3];
        if(!vpane) {
            ERROR("show_text_buffer_f(): vpane is NULL\n");
            return NULL;
        }
        TRACE("show_text_buffer_f:: err=%p\n", err);
  /*      gint min, max;
        g_object_get(G_OBJECT(vpane), "min-position", &min, NULL);
        g_object_get(G_OBJECT(vpane), "max-position", &max, NULL);
        if (fullview) {
            TRACE("show_text_buffer_f()::fullview:setting vpane position to %d\n", min);
            gtk_paned_set_position (vpane, min);
            g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(min));
            while (gtk_events_pending()) gtk_main_iteration();
            TRACE("vpane position set to =%d\n", gtk_paned_get_position(vpane));
            return NULL;
        }*/

        graphene_rect_t grect;
        if (!gtk_widget_compute_bounds (GTK_WIDGET(vpane), Child::mainWidget(), &grect)){
          fprintf(stderr, "show_text_buffer_f:: should not happen.\n");
        }

        //GtkAllocation allocation;
        //gtk_widget_get_allocation(GTK_WIDGET(vpane), &allocation);
        float vheight = grect.size.height;
        gint height ;
        if (small) height = 8*vheight/10;
        else height = 2*vheight/3;
        TRACE("vheight = %d, position = %d\n", vheight, gtk_paned_get_position(vpane));
        if (gtk_paned_get_position(vpane) > height) {
            TRACE("show_text_buffer_f()::setting vpane position to %d\n", height);
            gtk_paned_set_position (vpane, height);
            //g_object_set_data(G_OBJECT(vpane), "oldCurrent", GINT_TO_POINTER(height));
        } else TRACE("not setting vpane position to %d\n", height);
        

        return NULL;
    }
 public:
    static gchar *
    get_current_text (GtkTextView *textview) {
        // get current text
        GtkTextIter start, end;
        auto buffer = gtk_text_view_get_buffer (textview);

        gtk_text_buffer_get_bounds (buffer, &start, &end);
        auto t = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
        g_strchug(t);
        return t;
    }
    static gboolean isValidTextView(void *textView){
        gboolean retval = FALSE;
        void *p = g_list_find(textviewList, textView);
        if (p) retval = TRUE;
        return retval;
    }

    static void *reference_textview(GtkTextView *textView){
        TRACE("reference_run_button(%p)\n", (void *)textView);
        textviewList = g_list_prepend(textviewList, (void *)textView);
        return NULL;
    }

    static void
    unreference_textview(GtkTextView *textView){
        TRACE("unreference_run_button(%p)\n", (void *)textView);
        void *p = g_list_find(textviewList, (void *)textView);
        if (p){
            textviewList = g_list_remove(textviewList, (void *)textView);
        }
    }

  };
}
#endif

