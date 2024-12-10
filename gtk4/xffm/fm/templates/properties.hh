#ifndef LOCALPROPERTIES_HH
#define LOCALPROPERTIES_HH

typedef struct {
    GtkWidget *top;
    GtkWidget *user;
    GtkWidget *group;
    struct stat *st;
    gint result;
    gint type;
} dlg;

typedef struct row_t {
    GtkWidget *w[5];
    gboolean flag;
} row_t;

typedef struct entry_t {
    gchar *path;
    gchar *basename;
    gchar *mimetype;
    struct stat st;
} entry_t;



#define X_PAD 8
#define Y_PAD 1
#define TBL_XOPT GTK_FILL
#define TBL_YOPT GTK_SHRINK
/* flags
 */
#define IS_MULTI                1
#define IS_STALE_LINK        2
/* question dialogs: */
#define DLG_OK                        0x11
#define DLG_CLOSE                0x120
#define DLG_ALL                        0x140
#define DLG_SKIP                0x180
#define DLG_RC_CANCEL                0
#define DLG_RC_OK                1
#define DLG_RC_ALL                2
#define DLG_RC_CONTINUE                3
#define DLG_RC_SKIP                4
#define DLG_RC_DESTROY                5
#define DLG_RC_RECURSIVE        6
/* */
#define DLG_OK_CANCEL        (DLG_OK|DLG_CANCEL)
#define DLG_YES_NO        (DLG_YES|DLG_NO)
/* */

namespace xf{

template <class Type> class Preview;
template <class Type>
class Properties {
    GtkWindow *dialog_ = NULL;
    GtkBox *imageBox_ = NULL;
    GtkBox *modeBox_ = NULL;
    GtkButton *apply_ = NULL;
    entry_t *entry_ = NULL;
    GtkEntry *modeEntry_ = NULL;
    GtkLabel *modeLabel_ = NULL;
    GtkLabel *modeInfo_ = NULL;
    bool changed_ = false;
    bool validOctal_ = false;

public:
    GtkLabel *modeInfo(void) {return modeInfo_;}
    GtkLabel *modeLabel(void) {return modeLabel_;}
    GtkEntry *modeEntry(void) {return modeEntry_;}
    entry_t *entry(void){ return entry_;}
    GtkWindow *dialog(void){return dialog_;}
    GtkButton *apply(void) { return apply_;}
    void changed(bool value){changed_ = value;}
    bool changed(void) {return changed_;}
    void validOctal(bool value){validOctal_ = value;}
    bool validOctal(void) {return validOctal_;}
     



    ~Properties(void){
      DBG("Propetructor\n");
        freeEntry(entry_);
    }
    Properties(GFileInfo *info) {
      DBG("Properties constructor\n");

        entry_ = (entry_t *)calloc(1, sizeof(entry_t));
        entry_->path = Basic::getPath(info);
        entry_->basename = g_path_get_basename(entry_->path);
        // Do mime magic on single item, the rest on demand
        //entry_->mimetype = Mime<Type>::mimeMagic(entry_->path);
        if (lstat(entry_->path, &(entry_->st)) < 0){
                DBG("properties.hh::Properties():  %s (%s)\n",
                    entry_->path, strerror(errno));
                freeEntry(entry_);
                throw(1);
        } 
        doDialog();
    }


private:
    static void
    freeEntry(entry_t *entry_){
      g_free(entry_->path);
      g_free(entry_->basename);
      g_free(entry_->mimetype);
      g_free(entry_);
    }
    
//////////////////////////////////////////////////////////////////////////////////

    void 
    doDialog (void) {
      DBG("Properties doDialog\n");
        dialog_ = GTK_WINDOW(gtk_window_new ());


        gtk_window_set_title(dialog_, _("Properties and Attributes"));
        auto mainBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
        gtk_window_set_child(dialog_, GTK_WIDGET(mainBox));
        auto titlePath = GTK_LABEL(gtk_label_new(""));
        Basic::boxPack0(mainBox, GTK_WIDGET(titlePath), TRUE, FALSE, 0);

        auto contentBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        Basic::boxPack0(mainBox, GTK_WIDGET(contentBox), TRUE, FALSE, 0);
        imageBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
        //gtk_widget_set_size_request (GTK_WIDGET(imageBox_), PREVIEW_IMAGE_SIZE, PREVIEW_IMAGE_SIZE);
        auto infoBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));
        Basic::boxPack0(contentBox, GTK_WIDGET(imageBox_), TRUE, FALSE, 0);
        Basic::boxPack0(contentBox, GTK_WIDGET(infoBox), TRUE, FALSE, 10);
        auto buttonBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        Basic::boxPack0(mainBox, GTK_WIDGET(buttonBox), TRUE, FALSE, 0);

        auto label = GTK_LABEL(gtk_label_new(""));
        auto markup = g_strdup_printf("<span size=\"xx-large\">%s</span><span size=\"x-small\">\nuser/group/others: x->01, w->02, r->04</span>", 
                _("File Mode:"));
        gtk_label_set_markup(label, markup);
        g_free(markup);
        Basic::boxPack0(infoBox, GTK_WIDGET(label), FALSE, FALSE, 0);

        modeBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        g_object_set_data(G_OBJECT(modeBox_), "properties_p", this);
        g_object_set_data(G_OBJECT(modeBox_), "modeList", NULL);
        modeLabel_ = GTK_LABEL(gtk_label_new(""));  //child 1
        modeEntry_ = GTK_ENTRY(gtk_entry_new());  //child 2
        modeInfo_ = GTK_LABEL(gtk_label_new(""));  //child 3
        g_object_set_data(G_OBJECT(modeBox_), "titlePath", titlePath);
        
        Basic::boxPack0(modeBox_, GTK_WIDGET(modeLabel_), FALSE, FALSE, 0);
        Basic::boxPack0(modeBox_, GTK_WIDGET(modeEntry_), FALSE, FALSE, 0);
        Basic::boxPack0(modeBox_, GTK_WIDGET(modeInfo_), FALSE, FALSE, 0);
        Basic::boxPack0(infoBox, GTK_WIDGET(modeBox_), FALSE, FALSE, 0);
        g_object_set_data(G_OBJECT(imageBox_), "modeBox", modeBox_);
        g_object_set_data(G_OBJECT(modeEntry_), "modeBox", modeBox_);
        g_object_set_data(G_OBJECT(modeEntry_), "modeLabel", modeLabel_);
        
        // Add file info stuff
        // set info stuff data according to combo box
        // add signal handler to changed signal for combo box

        // Add process buttons
        //
        apply_ = GTK_BUTTON(gtk_button_new_with_label(_("Apply changes")));
        gtk_widget_set_sensitive(GTK_WIDGET(apply_), FALSE);
        auto cancel = gtk_button_new_with_label(_("Cancel"));
        Basic::boxPack0(buttonBox, GTK_WIDGET(apply_), FALSE, FALSE, 0);
        Basic::boxPack0(buttonBox, GTK_WIDGET(cancel), FALSE, FALSE, 0);
        
        //
        // Add handlers for buttons and destroy/delete
        // XXX g_signal_connect (G_OBJECT (dialog_), "delete-event", G_CALLBACK (deleteEvent), properties_p);

          
        g_signal_connect (G_OBJECT (cancel), "clicked", G_CALLBACK (cancelAction), this);
        g_signal_connect (G_OBJECT (apply_), "clicked", G_CALLBACK (applyAction), this);
        //
        // Create and add image for initial selected file in dialog
        //
        //auto pixbuf = Pixbuf<Type>::getPixbuf("accessories-calculator", -256);
        
        gchar *m2 = Basic::fileInfo(entry_->path);
        if (strstr(m2, "text")) {
            g_free(entry_->mimetype);
            entry_->mimetype = g_strdup("text/plain");
        }

        setUpImage(imageBox_, entry_);
        setUpMode(modeBox_, entry_);

        addKeyController(GTK_WIDGET(modeEntry_));
        // XXX g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (changeCombo), properties_p);
        
        //

      DBG("Properties gtk_window_present %p\n", dialog_);
      gtk_widget_realize(GTK_WIDGET(dialog_));
      Basic::setAsDialog(GTK_WIDGET(dialog_), "xfproperties", "Xfproperties");
#if 10
       // gtk_widget_set_parent(GTK_WIDGET(dialog_), MainWidget);
        gtk_window_set_destroy_with_parent(dialog_, true);
#endif
      gtk_window_present(dialog_);
        //Dialogs<Type>::placeDialog(GTK_WINDOW(dialog_));
        return ;
    }

    void
    validOctal(const char *text){
      if (!text || strlen(text)!=3) {
        validOctal_ = false;
        return ;
      }
      for (int i=0; i<3; i++){
        if (text[i] < '0' || text[i] > '7') {
          validOctal_ = false;
          return ;
        }
      }
      validOctal_ = true;
      return ;
    }

    static gboolean
    changeMode (GtkEventControllerKey* self,
          guint keyval,
          guint keycode,
          GdkModifierType state,
          gpointer data){
      
        auto properties_p = (Properties *)data;
        // get entry mode
        auto entryBuffer = gtk_entry_get_buffer(properties_p->modeEntry());
        const gchar *text = gtk_entry_buffer_get_text(entryBuffer);
        // switch to octal
        auto entry = properties_p->entry();
        mode_t oldMode =  entry->st.st_mode & 0777;
        mode_t mode;
        unsigned int modeInput;
        sscanf(text, "%o", &modeInput);
        properties_p->validOctal(text);
        mode = modeInput & 0777;
        DBG("changeMode (\"%s\") event old = %o new = %o\n",text, oldMode, mode);

        // update label
        auto modeEntry = properties_p->modeEntry();
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(modeEntry), "modeLabel"));
        auto box = GTK_BOX(g_object_get_data(G_OBJECT(modeEntry), "modeBox"));

        // compare with entry_t mode for color
        // if equal, remove from list/hash
        // if different replace in list/hash
     /*   if (mode == oldMode){
            g_hash_table_remove(properties_p->hash, entry->path);
        } else {
            g_hash_table_replace(properties_p->hash, entry->path, GINT_TO_POINTER(mode));
        }     */  
        setUpModeLabel(box, entry, mode);

    
        return TRUE;
    }

    static void
    setUpModeEntry(GtkBox *box, entry_t *entry){
        auto properties_p = 
            (Properties *)g_object_get_data(G_OBJECT(box), 
                    "properties_p");

        auto modeEntry = properties_p->modeEntry(); 
        auto mode = entry->st.st_mode & 0777;

        auto modeOctal = g_strdup_printf("%0o", mode);
        auto entryBuffer = gtk_entry_get_buffer(modeEntry);
        gtk_entry_buffer_set_text(entryBuffer, modeOctal, -1);
        g_free(modeOctal);
    }

    static void
    setUpModeLabel(GtkBox *box, entry_t *entry, mode_t newMode){
        auto properties_p = 
            (Properties *)g_object_get_data(G_OBJECT(box), "properties_p");
        auto titlePath = GTK_LABEL(g_object_get_data(G_OBJECT(box), "titlePath"));
        auto markup = g_strdup_printf("<span color=\"blue\" size=\"larger\">%s</span>", entry->path);
        gtk_label_set_markup(titlePath, markup);
        g_free(markup);

        auto modeLabel = properties_p->modeLabel(); 
        // changed in red, unchanged in blue...
        auto mode1 = entry->st.st_mode & 0777000;
        auto mode2 = entry->st.st_mode & 0777;
        auto modeText = Basic::modeString(newMode | mode1);
        properties_p->changed(newMode != mode2);
        auto changed = properties_p->changed();
        const gchar *color = changed?"red":"blue";
        auto apply = GTK_WIDGET(properties_p->apply());
        gtk_widget_set_sensitive(apply, changed && properties_p->validOctal());

        auto modeMarkup = g_strdup_printf("<span size=\"xx-large\" color=\"%s\">%s</span> <span color=\"blue\" size=\"x-large\">(%0o)</span>", 
                color, modeText, mode2);
        gtk_label_set_markup(modeLabel, modeMarkup);
        g_free(modeMarkup);
        setFileInfo(entry, properties_p->modeInfo());
        return;

    }

    static gchar *
    trashInfo(const gchar *path, const gchar *item){
        auto basename = g_path_get_basename(path);
        auto keyPath =  g_strconcat(g_get_home_dir(),"/.local/share/Trash/info/", basename, ".trashinfo", NULL);
        auto keyInfo = g_key_file_new();
        auto loaded = 
            g_key_file_load_from_file(keyInfo, keyPath, (GKeyFileFlags)0, NULL);
        g_free(basename);
        if (!loaded) {
            ERROR("fm/view/local/properties.hh:: *** unable to load %s\n", keyPath);
            g_free(keyPath);
            return NULL;
        }
        g_free(keyPath);
        GError *error = NULL;
        gchar **p = g_key_file_get_groups (keyInfo, NULL);
        TRACE("Reading from group %s\n", *p);
        //auto value = g_key_file_get_string (keyInfo, "Trash Info", item, &error);
        auto value = g_key_file_get_string (keyInfo, *p, item, &error);
        g_strfreev(p);
        if (error){
            ERROR("fm/view/local/properties.hh:: trashInfo(%s): %s\n", item, error->message);
            g_error_free(error);
            value = NULL;
        } 
        g_key_file_free(keyInfo);
        return value;
        
    }

    static void 
    setFileInfo(entry_t *entry, GtkLabel *label){
        TRACE("setFileInfo... %s\n",entry->path);
        auto h = g_get_home_dir();
        struct stat st;
        if (stat(entry->path, &st)){
          if (lstat(entry->path, &st)){
            DBG("lstat(%s) failed!\n", entry->path);
          }
        }
        if (!entry->mimetype) entry->mimetype = MimeMagic::mimeMagic(entry->path);
        gchar *m1 = Basic::statInfo(&st);
        gchar *m2 = Basic::fileInfo(entry->path);
        auto size = Basic::sizeString(st.st_size);
        auto date = Basic::dateString(st.st_mtime);
        auto encoding = MimeMagic::encoding(entry->path);
        auto mimetype = entry->mimetype;
        auto encodingString = encoding?g_strconcat(_("Encoding"),": ", encoding, "\n", NULL):g_strdup("");
        auto mimetypeString = mimetype?g_strconcat(_("Mimetype"),": ", entry->mimetype, "\n", NULL):g_strdup("");
        g_free(encoding);

        gchar *m = g_strconcat(m1, "\n",
                _("Size"), ": ", size, "\n", 
                _("Date"), ": ", date, "\n", 
                encodingString,
                mimetypeString,
                "\n", m2, "\n",
                NULL);
        g_free(mimetypeString);
        g_free(encodingString);
        g_free(m1);
        g_free(m2);
        gchar *dir = g_path_get_dirname(entry->path);
        if (strncmp(entry->path, h, strlen(h))==0){
            if (strcmp(dir+strlen(h), "/.local/share/Trash/files")==0){
                auto trashDate = trashInfo(entry->path, "DeletionDate");
                auto trashSource = trashInfo(entry->path, "Path");
                if (trashDate && trashSource){
                if (strchr(trashDate, 'T'))*strchr(trashDate, 'T')=' ';
                    auto mt = g_strdup_printf("<span size=\"large\" color=\"red\">%s\n<span color=\"blue\">%s</span>\n%s\n<span color=\"blue\">%s</span></span>", 
                        _("Successfully moved to trash."), trashDate?trashDate:"?",
                        _("Source:"), trashSource?trashSource:"?");
                    auto *g = g_strconcat(m,"\n", mt, NULL);
                    g_free(mt);
                    g_free(m);
                    m = g;
                }
                g_free(trashDate);
                g_free(trashSource);

            } else TRACE("not trash:\"%s\"\n", dir+strlen(h));

        } else TRACE("not in %s\n", h);

        gtk_label_set_markup(label, m);
        gtk_widget_set_visible(GTK_WIDGET(label), true);

        g_free(m);
        g_free(dir);
        return ;
    }

    static void
    setUpMode(GtkBox *box, entry_t *entry){
        auto properties_p = 
            (Properties *)g_object_get_data(G_OBJECT(box), 
                    "properties_p");
        mode_t mode = entry->st.st_mode & 0777;
        setUpModeLabel(box, entry, mode);
        setUpModeEntry(box, entry);
    }

    void
    setUpImage(GtkBox *box, entry_t *entry){
        TRACE("setUpImage... mimetype = %s\n", entry->mimetype);
        if (!entry->mimetype) entry->mimetype = MimeMagic::mimeMagic(entry->path);
    

        auto paintable = Preview<Type>::loadPath(entry->path);
        if (paintable == NULL) {
          gtk_widget_set_visible(GTK_WIDGET(box), false);
        } else {
          auto image = gtk_image_new_from_paintable(paintable);
          gtk_widget_set_size_request(GTK_WIDGET(image), 320, 320);
          gtk_box_append(box, image);
        }
    }

    static void
    cancelAction(GtkButton *button, void *data){
        auto properties_p = (Properties *)data;
        gtk_window_destroy(properties_p->dialog());
        delete properties_p;
        TRACE("cancelAction\n");
    }

/*    static void
    setFileMode (gpointer key,
               gpointer value,
               gpointer data){
        auto path = (const gchar *)key;
        auto mode = (mode_t)GPOINTER_TO_INT(value);
        if (chmod(path, mode) < 0){
            ERROR("fm/view/local/properties.hh::setFileMode(): chmod(%s) %s\n", path, strerror(errno));
        }
    }   */ 
    
    static void
    applyAction(GtkButton *button, void *data){
        TRACE("applyAction\n");
        auto properties_p = (Properties *)data;
        auto modeEntry = properties_p->modeEntry(); 
        auto entryBuffer = gtk_entry_get_buffer(modeEntry);
        auto text = gtk_entry_buffer_get_text(entryBuffer);
        errno=0;
        auto mode = strtol(text, NULL, 8);
        if (errno) {
          DBG("Error:: applyAction: %s\n", strerror(errno));
        } else {
          auto path = properties_p->entry()->path;
          if (chmod(path, mode) < 0){
              ERROR("fm/view/local/properties.hh::setFileMode(): chmod(%s) %s\n", path, strerror(errno));
          }
        }

        //g_hash_table_foreach(properties_p->hash, setFileMode, NULL);
        // Apply changes to all changed items
        gtk_window_destroy(properties_p->dialog());
        delete properties_p;
    }

    void addKeyController(GtkWidget  *widget){
        auto keyController = gtk_event_controller_key_new();
        gtk_event_controller_set_propagation_phase(keyController, GTK_PHASE_CAPTURE);
        gtk_widget_add_controller(GTK_WIDGET(widget), keyController);
        g_signal_connect (G_OBJECT (keyController), "key-released", 
            G_CALLBACK (changeMode), (void *)this);
    }

};
}


#endif
