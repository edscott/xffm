#ifndef FINDSIGNALS_HH
#define FINDSIGNALS_HH

namespace xf
{
        ///////////////////   signals  /////////////////////////
template <class Type> class DnDBox;
template <class Type> class FindResponse;
template <class Type>
class FindSignals {
  private:
    static const gchar *
    get_time_type(GtkBox *mainBox){
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(mainBox), "radio1"))){
            return "-M";
        }
        if (gtk_toggle_button_get_active ((GtkToggleButton *) g_object_get_data(G_OBJECT(mainBox), "radio2"))){
            return "-C";
        }
        return "-A";
    }

    static gboolean
    on_find_clicked_action (void *data) {
      auto object = (FindResponse<Type> *)data;
        TRACE("on_find_clicked_action\n");
        GtkTextView *textview = object->textview();
        // Get the search path.
        GtkEntry *entry = GTK_ENTRY(g_object_get_data(G_OBJECT(object->mainBox()), "path_entry"));
        auto buffer = gtk_entry_get_buffer(entry);
        char *path = g_strdup(gtk_entry_buffer_get_text(buffer));
        /* tilde expansion */
        if(path[0] == '~'){
            char *t = g_strconcat(g_get_home_dir(),"/", path+1, NULL);
            g_free(path);
            path = t;
        }
        /* environment variables */
        else if(path[0] == '$') {
            const gchar *p = getenv (path + 1);
            if(p){
                g_free(path);
                path = g_strdup(p);
            }
        }
        if(!path || strlen (path) == 0 || !g_file_test (path, G_FILE_TEST_EXISTS)) {
            char *message = g_strconcat(strerror (ENOENT), ": ", path, "\n", NULL);
            Print::printError(textview, message);
            g_free(path);
            return FALSE;
        }

        fgrData_t *Data = (fgrData_t *)calloc(1,sizeof(fgrData_t));

        Data->mainBox = object->mainBox();
        object->Data(Data);
        Data->object = (void *)object;

        Data->done = FALSE; // (This is redundant with calloc, here just for clarity).

        /* get the parameters set by the user... *****/
        get_arguments(path, Data);

    /*    fprintf(stderr, "DBG> arguments: \'");
        for (auto p=Data->argument; p && *p; p++){
          fprintf(stderr, "%s ", *p);
        }
        fprintf(stderr, "\n");*/

        int flags = TUBO_EXIT_TEXT|TUBO_VALID_ANSI|TUBO_CONTROLLER_PID;

        // When the dialog is destroyed on action, then output is going to a
        // widgets_p away from dialog window, so we let the process run on and on.
        char *command = g_strdup("");
        char **p=Data->argument;
        int k=0;
        for (;p && *p; p++, k++){
            if (k==0) {
              auto base = g_path_get_basename(*p);
              Basic::concat(&command, base);
              g_free(base);
            } else Basic::concat(&command, *p);
            Basic::concat(&command, " ");
        }
        //g_hash_table_replace(controllerHash, GINT_TO_POINTER(Data->pid), command);
        Print::print(textview, EMBLEM_FIND, "green", g_strconcat( _("Searching..."), "\n", NULL));
        Print::print(textview, EMBLEM_GREEN_BALL, "blue",
            g_strdup_printf("%s\n", command));
        g_free(command);
        Print::showText(textview);

        Data->pid = Run<Type>::thread_run(Data, (const gchar **)Data->argument, stdout_f, stderr_f, forkCleanup);

        return FALSE;
    }

    static void getSizeOptions(fgrData_t *Data, int *i){
      GtkBox *mainBox = Data->mainBox;
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "size_greater")))) 
      {
           int size_greater = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_greater_spin")));            
           Data->argument[(*i)++] = g_strdup("-s");
           Data->argument[(*i)++] = g_strdup_printf("+%d", size_greater);
      }
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "size_smaller")))) 
      {
           gint size_smaller = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "size_smaller_spin")));            
           Data->argument[(*i)++] = g_strdup("-s");
           Data->argument[(*i)++] = g_strdup_printf("-%d", size_smaller);
      }
    }

    static void getTimeOptions(fgrData_t *Data, int *i){
      GtkBox *mainBox = Data->mainBox;
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "last_months")))) 
      {
          Data->argument[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
          gint last_months = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_months_spin")));            
          Data->argument[(*i)++] = g_strdup("-m");
          Data->argument[(*i)++] = g_strdup_printf("%d", last_months);
      }
      else if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                  g_object_get_data(G_OBJECT(mainBox), "last_days"))))
      {
          Data->argument[(*i)++] = (gchar *)get_time_type(mainBox);
          gint last_days = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_days_spin")));            
          Data->argument[(*i)++] = g_strdup("-d");
          Data->argument[(*i)++] = g_strdup_printf("%d", last_days);
      }
      else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "last_hours"))))
      {
          Data->argument[(*i)++] = (gchar *)get_time_type(mainBox);
          gint last_hours = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_hours_spin")));            
          Data->argument[(*i)++] = g_strdup("-h");
          Data->argument[(*i)++] = g_strdup_printf("%d", last_hours);
      }
      else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "last_minutes"))))
      {
          Data->argument[(*i)++] = g_strdup((gchar *)get_time_type(mainBox));
          gint last_minutes = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "last_minutes_spin")));            
          Data->argument[(*i)++] = g_strdup("-k");
          Data->argument[(*i)++] = g_strdup_printf("%d", last_minutes);
      }
    }

    static void getFileOptions(fgrData_t *Data, int *i){
        GtkBox *mainBox = Data->mainBox;
      // SUID/EXE
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("Executable"))))
          ||
          gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), _("SUID"))))){
            Data->argument[(*i)++] = g_strdup("-p");
            if (gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("Executable"))))) {
              Data->argument[(*i)++] = g_strdup("exe");
            } else {
              Data->argument[(*i)++] = g_strdup("suid");
            }
            return;
      }
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("Octal Permissions")))))
      {
          Data->argument[(*i)++] = g_strdup("-o");
          auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "permissions_entry"));
          auto buffer = gtk_entry_get_buffer(entry);
          const char *c = gtk_entry_buffer_get_text (buffer);
          // FIXME: check for valid octal number <= 0777
          if (c && strlen(c)) Data->argument[(*i)++] = g_strdup(c);
          else Data->argument[(*i)++] = g_strdup("0666");
          return;
      } 
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "uid"))))
      {
          auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
          auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
          auto selected = gtk_string_object_get_string(item);
        
          if(selected && strlen(selected)) {
              Data->argument[(*i)++] = g_strdup("-u");
              struct passwd *pw = getpwnam (selected);
              if(pw) {
                  Data->argument[(*i)++] = g_strdup_printf("%d", pw->pw_uid);
              } else {
                  Data->argument[(*i)++] = g_strdup_printf("%d",atoi(selected));
              }

          }
          return;
      }
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "gid"))))
      {
          auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "uid_combo")); 
          auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
          auto selected = gtk_string_object_get_string(item);
        
          if(selected && strlen(selected)) {
              Data->argument[(*i)++] = g_strdup("-g");
              struct group *gr = getgrnam (selected);
              if(gr) {
                  Data->argument[(*i)++] = g_strdup_printf("%d", gr->gr_gid);
              } else {
                  Data->argument[(*i)++] = g_strdup_printf("%d",atoi(selected));
              }

          }
          return;
      }

      

    }

    static void getGrepOptions(fgrData_t *Data, int *i){
        GtkBox *mainBox = Data->mainBox;
      if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), "case_sensitive"))))
      {
          Data->argument[(*i)++] = g_strdup("-i");
      } 
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                  g_object_get_data(G_OBJECT(mainBox), "line_count"))))
      {
          Data->argument[(*i)++] = g_strdup("-c");
      } 

      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "grep_entry"));
      auto buffer = gtk_entry_get_buffer(entry);
      const char *token = gtk_entry_buffer_get_text(buffer);  

      if(token && strlen(token)) {
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                      g_object_get_data(G_OBJECT(mainBox), "ext_regexp"))))
          {
              Data->argument[(*i)++] = g_strdup("-E");
          } else {
              Data->argument[(*i)++] = g_strdup("-e");
          }
          Data->argument[(*i)++] = g_strdup(token);

          /* options for grep: ***** */
          if(!gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "look_in_binaries"))))
          {
              Data->argument[(*i)++] = g_strdup("-I");
          }
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "match_words"))))
          {
              Data->argument[(*i)++] = g_strdup("-w");
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "match_lines"))))
          {
              Data->argument[(*i)++] = g_strdup("-x");
          }
          else if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                      g_object_get_data(G_OBJECT(mainBox), "match_no_match"))))
          {
              Data->argument[(*i)++] = g_strdup("-L");
          }
      }
    }

    static void
    get_arguments(gchar *path, fgrData_t *Data){
      int i=0;
      GtkBox *mainBox = Data->mainBox;

      /* limit */
      Data->resultLimit = gtk_spin_button_get_value_as_int (
              GTK_SPIN_BUTTON (g_object_get_data(G_OBJECT(mainBox), "upper_limit_spin")));
      Data->resultLimitCounter = 0; // fgr ends with "fgr search complete"
      Data->argument = (char **)calloc(MAX_COMMAND_ARGS, sizeof(gchar *));
      if (!Data->argument){
          std::cerr<<"calloc error at get_arguments()\n";
          exit(1);
      }
      
      /* the rest */
      Data->argument[i++] = g_strdup(xffindProgram);
      Data->argument[i++] = g_strdup("--fgr");
      Data->argument[i++] = g_strdup("-v"); // (verbose output) 
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("Recursive")))))
      {
          if(gtk_check_button_get_active (GTK_CHECK_BUTTON(
                      g_object_get_data(G_OBJECT(mainBox), _("Find hidden files and directories"))))) 
          {
              Data->argument[i++] = g_strdup("-r");
          } else {
              Data->argument[i++] = g_strdup("-R");
          }
      } 

      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( //?? check this XXX
                  g_object_get_data(G_OBJECT(mainBox), _("Stay on single filesystem")))))
      {
          Data->argument[i++] = g_strdup("-a");
      } 
      
      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("File size"))))) {
        getSizeOptions(Data, &i);
      }


      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("Specific Time"))))){
        getTimeOptions(Data, &i);
      }

      if(gtk_check_button_get_active (GTK_CHECK_BUTTON( 
                  g_object_get_data(G_OBJECT(mainBox), _("File properties"))))){
        getFileOptions(Data, &i);
      }


      //  grep options
      getGrepOptions(Data, &i);

      /* select list */

      auto dd = GTK_DROP_DOWN(g_object_get_data(G_OBJECT(mainBox), "file_type_om")); 
      auto item = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(dd));
      auto ftype = gtk_string_object_get_string(item);
      auto _ftypes = Ffunctions::ftypes();
      auto _ft = Ffunctions::ft();
      for(int j = 0; _ftypes[j] != NULL; j++) {
          if(ftype && strcmp (ftype, _(_ftypes[j])) == 0) {
              Data->argument[i++] = g_strdup("-t");
              Data->argument[i++] = g_strdup(_ft[j]);
              break;
          }
      }

      auto filterEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(mainBox), "filter_entry"));
      auto filterBuffer = gtk_entry_get_buffer(filterEntry);
      const char *filter = gtk_entry_buffer_get_text(filterBuffer);
      Data->argument[i++] = g_strdup("-f");
      if(filter && strlen (filter)) {
          Data->argument[i++] = g_strdup(filter);
      } else {
          Data->argument[i++] = g_strdup("*");
          Data->argument[i++] = g_strdup("-D");
      }

      /* last Data->argument is the path */
      Data->argument[i++] = path;
      Data->argument[i] = (char *)0;
    }

  protected:
  
    static void
    onFindButton (GtkWidget * button, void *data) {
      auto object = (FindResponse<Type> *)data;
      TRACE("onFindButton...\n");
        GtkTextView *textview = object->textview();
        
        on_find_clicked_action (data);
        gtk_widget_set_visible(GTK_WIDGET(object->cancelButton()), true);
        gtk_widget_set_visible(GTK_WIDGET(object->findButton()), false);
        object->saveHistories();
    }

    static void
    onClearButton (GtkWidget * button, gpointer data) {
      TRACE("onClearButton...\n");
      auto object = (FindResponse<Type> *)data;

        auto textview = object->textview();
        TRACE("fixme: signals::onClearButton\n");
        Print::clear_text(textview);
        auto vpane =object->vpane();
        gtk_paned_set_position(vpane, 2500);
    }

    static void
    freeFgrData(void *data){
      if (!data) return;
        fgrData_t *Data = (fgrData_t *)data;
        gchar **a=Data->argument;
        for (;*a; a++) g_free(*a);
        for (auto l=Data->findList; l && l->data; l=l->next) g_free(l->data);
        g_slist_free(Data->findList);
        g_free(Data->argument);
        g_free(Data);
        return ;
    }

    static void *processEnd(void *data){
       fgrData_t *Data = (fgrData_t *)data;
       TRACE("Cleanup\n");
       auto object = (FindResponse<Type> *)Data->object;
       auto mainBox = Data->mainBox;
       gtk_widget_set_visible(GTK_WIDGET(object->cancelButton()), false);
       gtk_widget_set_visible(GTK_WIDGET(object->findButton()), true);
       Basic::flushGTK();
       if (Data->findList) {
         GSList *dndList = NULL;
         for (auto l=Data->findList; l && l->data; l=l->next){
           auto string = (const char *)l->data;
           dndList = g_slist_append(dndList, g_strdup(string));
         }
         
         int k = 0;
         for (;Data->argument[k];k++);
         k--;
         auto textview = object->textview();
         auto dndBox = DnDBox<Type>::openDnDBox(Data->argument[k], dndList, textview);
       } else {
         DBG("No Data->findList\n");
       }    

       GtkWidget *edit_button = GTK_WIDGET(g_object_get_data(G_OBJECT(mainBox), "edit_button"));
       if (g_slist_length(Data->findList)){
            const gchar *editor = Basic::getEditor();
            if (!editor || strlen(editor)==0){
                GtkTextView *textview = object->textview();
                Print::print(textview, EMBLEM_WARNING, 
                        g_strdup_printf("%s\n", _("No editor for current action.")));
                return NULL;
            }
       } 
       return NULL;
    }

    static gboolean
    Cleanup (void *data) {
       fgrData_t *Data = (fgrData_t *)data;
       if (Data->done) {
           freeFgrData(Data);
           return FALSE;
       }
       g_free(Data);
       return TRUE;
    }


    static void
    forkCleanup (void *data) {
       TRACE("forkCleanup\n");
        fgrData_t *Data = (fgrData_t *)data;
        g_timeout_add_seconds(1, Cleanup, (void *)Data);
        
    }

    static void
    stderr_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;

        auto object = (FindResponse<Type> *)g_object_get_data(G_OBJECT( Data->mainBox), "object");
    
        GtkTextView *textview = object->textview();
        if (!gtk_widget_is_visible(GTK_WIDGET(textview))) return;

        char *line;
        line = (char *)stream;


        if(line[0] != '\n') {
            Print::print(textview, EMBLEM_WARNING, "red", g_strdup(line));
        }

        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
       return;
    }


    static void
    stdout_f (void *data, void *stream, int childFD) {
        fgrData_t *Data = (fgrData_t *)data;
        char *line;
        line = (char *)stream;
        TRACE("FORK stdout: %s\n", line);

        if(line[0] == '\n') return;
        auto object = (FindResponse<Type> *)g_object_get_data(G_OBJECT( Data->mainBox), "object");
    
        GtkTextView *textview = object->textview();

        if (Data->resultLimit > 0 && Data->resultLimit==Data->resultLimitCounter) {
            gchar *g=g_strdup_printf("%s. %s %d", _("Results"), _("Upper limit:"), Data->resultLimit);
            Print::print(textview, EMBLEM_WARNING, "green", g_strconcat(g, "\n", NULL));
            Print::print(textview, "blue",  g_strconcat(_("Counting files..."), "\n", NULL));
            g_free(g);
        }

        if(strncmp (line, "Tubo-id exit:", strlen ("Tubo-id exit:")) == 0) {
          if(strchr (line, '\n')) *strchr (line, '\n') = 0;
          gchar *plural_text = 
              g_strdup_printf (
                      ngettext ("Found %d match", "Found %d matches", 
                          Data->resultLimitCounter), Data->resultLimitCounter);
          gchar *message = g_strdup_printf(_("%s Finished : %s"), xffindProgram, plural_text);
          gchar *g = g_strdup_printf("%c[31m%s\n",27, message);
          Print::print(textview, EMBLEM_RED_BALL, "Red", g);

          Data->done = TRUE;
          Basic::context_function(processEnd,data);

          g_free(plural_text);
          g_free(message);
        } else {
          if (!gtk_widget_is_visible(GTK_WIDGET(textview))) return;
          gchar *file = g_strdup(line);
          if (strchr(file, '\n')) *strchr(file, '\n') = 0;
          if (g_file_test(file, G_FILE_TEST_EXISTS)) {
              Data->resultLimitCounter++; 
              if (Data->resultLimit ==0 ||
                  (Data->resultLimit > 0 && Data->resultLimit > Data->resultLimitCounter) ) {
                  Print::print(textview, g_strdup_printf("%s\n", file));
                  TRACE("--> %s\n",file);
                  TRACE("resultLimitCounter:%d %s\n", Data->resultLimitCounter, line);
                  Data->findList = g_slist_prepend(Data->findList, file);
              }
          } else {
             TRACE("ignoring: \"%s\"\n", file);
             g_free(file);
          }
        }         
        // With this, this thread will not do a DOS attack
        // on the gtk event loop.
        static gint count = 1;
        if (count % 20 == 0){
            usleep(10000);
        } 
        
        return;
    }
    
    static void
    onCancelButton(GtkWidget * button, void *data){
      auto object = (FindResponse<Type> *)data;
        GtkTextView *textview = object->textview();
        auto dialog = object->dialog();
        auto Data = object->Data();
        TRACE("cancel_all\n");
        Print::print(textview, EMBLEM_DELETE, "blue", 
                g_strdup_printf("%s: pid %d\n",_("Cancel"), Data->pid));
        kill(Data->pid, SIGUSR2);
    }

  };
}
#endif
