
#ifndef CREATEINI_H
#define CREATEINI_H
#include <vector>

namespace xf {
template <class Type> 
class iniCreator {
    GtkWindow *creatorWindow_;
    gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
    GtkRequisition minimumSize_;
    GtkRequisition naturalSize_;
    GtkRequisition maximumSize_;
    GtkBox *bottombox_;
    std::vector<GtkBox *> optionBox_;
    gint options_;
    gint maxOptions_;

    GtkLabel *workdirLabel_;
    GtkLabel *executableLabel_;

    GtkEntry *iconEntry_;
    GtkEntry *tooltipEntry_;
    GtkButton *execFileButton_;
    GtkButton *workDirButton_;
    GtkCheckButton *inTerminal;

    gchar *filename_;

public:
    ~iniCreator(void){
        gtk_widget_hide(GTK_WIDGET(creatorWindow_));
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_main_quit();
        //_exit(123);
    }

    iniCreator(gint max=10):options_(0), filename_(NULL)
    {
        maxOptions_ = max;
        
        optionBox_.reserve(max);

        creatorWindow_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
        g_signal_connect (G_OBJECT (creatorWindow_), "delete-event", G_CALLBACK (deleteEvent), this);
        gtk_widget_get_preferred_width (GTK_WIDGET(creatorWindow_), &dialogMinW_, &dialogNatW_);
        gtk_widget_get_preferred_height (GTK_WIDGET(creatorWindow_), &dialogMinH_, &dialogNatH_);
        gtk_window_set_type_hint(creatorWindow_, GDK_WINDOW_TYPE_HINT_DIALOG);
        //setWindowMaxSize(creatorWindow_);
        gtk_window_set_position (creatorWindow_, GTK_WIN_POS_MOUSE);

        auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        gtk_container_add (GTK_CONTAINER(creatorWindow_), GTK_WIDGET(vbox));
        auto headerbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto topbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        auto footerbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto bottombox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(headerbox), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(topbox), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(footerbox), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(bottombox_), TRUE, FALSE, 3);


        auto label1 = GTK_LABEL(gtk_label_new(""));
        auto label2 = GTK_LABEL(gtk_label_new(""));
        auto markup1 = g_strdup_printf("<span size=\"larger\" color=\"blue\">%s</span>", "custom button");
        auto markup2 = g_strdup_printf("<span size=\"larger\" color=\"green\">%s (%s %d)</span>", _("Options"), _("maximum"), maxOptions_);

        gtk_label_set_markup(label1, markup1);
        gtk_label_set_markup(label2, markup2);
        compat<bool>::boxPackStart(headerbox, GTK_WIDGET(label1), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(footerbox, GTK_WIDGET(label2), FALSE, FALSE, 3);

        auto addButton = addIconButton(footerbox, NULL, "list-add");
        gtk_widget_set_tooltip_text(GTK_WIDGET(addButton), _("Add option"));
        g_signal_connect (G_OBJECT (addButton), "clicked", G_CALLBACK (addOption), this);

        auto removeButton = addIconButton(footerbox, NULL, "list-remove");
        gtk_widget_set_tooltip_text(GTK_WIDGET(removeButton), _("Remove option"));
        g_signal_connect (G_OBJECT (removeButton), "clicked", G_CALLBACK (removeOption), this);
        addTopStuff(topbox);

        auto exit = gtk_button_new_with_label(_("Exit"));
        gtk_box_pack_end(headerbox, GTK_WIDGET(exit), FALSE, FALSE, 3);
        g_signal_connect (G_OBJECT (exit), "clicked", G_CALLBACK (exitApp), this);
        
        auto save = gtk_button_new_with_label(_("Save"));
        gtk_box_pack_end(headerbox, GTK_WIDGET(save), FALSE, FALSE, 3);
        g_signal_connect (G_OBJECT (save), "clicked", G_CALLBACK (saveIniFile), this);

        setDefaultSize();
        gtk_window_present (creatorWindow_);
        
        while (gtk_events_pending()) gtk_main_iteration();
        gtk_widget_show_all(GTK_WIDGET(creatorWindow_));

        for (gint i=0; i<maxOptions_; i++){
            optionBox_[i] = addOptionBox(bottombox_, i);
        }

        return ;

    }
    GtkWindow *window(void) {return creatorWindow_;}

private:

    GtkBox *labelBox(GtkBox *vbox, const gchar *text){
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto label = GTK_LABEL(gtk_label_new(text));
        g_object_set_data(G_OBJECT(hbox), "label", (void *)label);
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(label), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(vbox, GTK_WIDGET(hbox), FALSE, FALSE, 3);
        return hbox;
    }


    GtkButton *getButton(GtkBox *hbox, const gchar *iconId){
        auto button = GTK_BUTTON(gtk_button_new());
        Gtk<Type>::set_bin_contents(GTK_BIN(button), iconId,
                NULL, SIZE_BUTTON);
        return button;
    }

    GtkCheckButton *addCheckButton(GtkBox *vbox, const gchar *text){
        auto hbox = labelBox(vbox, text);
        auto button = GTK_CHECK_BUTTON(gtk_check_button_new());
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(button), FALSE, FALSE, 3);
        return button;
        
    }

    GtkButton *addIconButton(GtkBox *vbox, const gchar *text, const gchar *iconId)
    {
        auto hbox = labelBox(vbox, text);
        auto label = GTK_LABEL(g_object_get_data(G_OBJECT(hbox), "label"));
        if (text) {
            auto markup = g_strdup_printf("<span color=\"red\">%s</span>", text);
            gtk_label_set_markup(label, markup);
            g_free(markup);
        }
        auto button = getButton(hbox, iconId);
        if (text) gtk_widget_set_tooltip_text(GTK_WIDGET(button), text); 
        g_object_set_data(G_OBJECT(button), "label", (void *)label);
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(button), FALSE, FALSE, 3);
        return button;
        
    }

    GtkEntry *addEntryInput(GtkBox *vbox, const gchar *text){
        auto hbox = labelBox(vbox, text);
        auto entry = GTK_ENTRY(gtk_entry_new());
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(entry), FALSE, FALSE, 3);
        return entry;
        
    }
    void addTopStuff(GtkBox *vbox){
        iconEntry_ = addEntryInput(vbox, _("icon ID"));
        tooltipEntry_ = addEntryInput(vbox, _("tooltip"));
        workDirButton_ = addIconButton(vbox, _("Select work directory"),
                "folder-new");
        workdirLabel_ = GTK_LABEL(g_object_get_data(G_OBJECT(workDirButton_), "label"));
        execFileButton_ = addIconButton(vbox, _("Select executable"), 
                "text-x-script");
        executableLabel_ = GTK_LABEL(g_object_get_data(G_OBJECT(execFileButton_), "label"));
        inTerminal = addCheckButton(vbox, _("Run in terminal") );

        g_signal_connect (G_OBJECT(workDirButton_), 
                        "clicked", 
                        G_CALLBACK (folderChooser), 
                        (gpointer) this);
        g_signal_connect (G_OBJECT(execFileButton_), 
                        "clicked", 
                        G_CALLBACK (fileChooser), 
                        (gpointer) this);
        gtk_widget_show_all(GTK_WIDGET(vbox));
    }

    void setWindowMaxSize(void){
        gint x_return, y_return;
        guint w_return, h_return, d_return, border_return;
        Window root_return;
        auto drawable = gdk_x11_get_default_root_xwindow ();
        //Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
        auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
        XGetGeometry(display, drawable, &root_return,
                &x_return, &y_return, 
                &w_return, &h_return, 
                &border_return, 
                &d_return);
        GdkGeometry geometry;
        geometry.max_width = w_return - 25;
        geometry.max_height = h_return -25;
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
        gtk_window_set_geometry_hints (GTK_WINDOW(creatorWindow_), 
                GTK_WIDGET(creatorWindow_), &geometry, GDK_HINT_MAX_SIZE);
    }

    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(creatorWindow_),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        TRACE("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
        gtk_window_set_default_size(creatorWindow_, 300, 400);
    }

    static void exitApp (GtkButton *widget,
               gpointer   data){
        deleteEvent(NULL, NULL, data);
        return;
    }

    static gboolean deleteEvent (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
        auto object = (iniCreator<Type> *)data;
        delete (object);
        return TRUE;
    }

    void
    chooser(GtkLabel *label, const gchar *text, GtkFileChooserAction action) {
        // GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
        GtkDialog *dialog = GTK_DIALOG(gtk_file_chooser_dialog_new (text,
                                                         GTK_WINDOW (gtk_widget_get_toplevel(GTK_WIDGET(label))),
                                                         action,
                                                         _("Cancel"),
                                                         GTK_RESPONSE_CANCEL,
                                                         _("Open"),
                                                         GTK_RESPONSE_ACCEPT,
                                                         NULL));
        gtk_file_chooser_set_action ((GtkFileChooser *) dialog, action);

        auto wd = g_get_current_dir();

        gtk_file_chooser_set_current_folder ((GtkFileChooser *) dialog, wd);

        g_free(wd);

        gtk_widget_set_sensitive(GTK_WIDGET(creatorWindow_), FALSE);
        gint response = gtk_dialog_run(dialog);
        gtk_widget_set_sensitive(GTK_WIDGET(creatorWindow_), TRUE);

        if(response == GTK_RESPONSE_ACCEPT) {
            gchar *path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            gtk_label_set_markup(label, path);
            g_free(g_object_get_data(G_OBJECT(label), "path"));
            g_object_set_data(G_OBJECT(label), "path", path);
            
            TRACE("Got %s\n", path);
        } else TRACE("response was not GTK_RESPONSE_ACCEPT\n");

        gtk_widget_hide (GTK_WIDGET(dialog));
        gtk_widget_destroy (GTK_WIDGET(dialog));

    }

    GtkLabel *workdirLabel(void){return this->workdirLabel_;}
    static void
    fileChooser(GtkButton *button, void *data) {
        auto object =(iniCreator<Type> *)data;
        auto label = object->executableLabel();
        object->chooser(label, _("Choose file"), GTK_FILE_CHOOSER_ACTION_OPEN);

    }

    GtkLabel *executableLabel(void){return this->executableLabel_;}
    static void
    folderChooser(GtkButton *button, void *data) {
        auto object =(iniCreator<Type> *)data;
        auto label = object->workdirLabel();
        object->chooser(label, _("Choose directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    }

    static void
    removeOption(GtkButton *button, void *data) {
        auto object = (iniCreator<Type> *) data;
        object->hideOption();      
    }

    void
    hideOption(void) {
        if (this->options_ <= 0) return;
        this->downOption();
        gtk_widget_set_sensitive(GTK_WIDGET(this->optionBox_[options_]), FALSE);
        gtk_widget_hide(GTK_WIDGET(this->optionBox_[options_]));
        TRACE("option_=%d\n",this->options_); 
    }

    static void
    addOption(GtkButton *button, void *data) {
        auto object = (iniCreator<Type> *) data;
        object->showOption();      
    }

    void
    showOption(void) {
        if (this->options_ >= maxOptions_) return;
        gtk_widget_show_all(GTK_WIDGET(this->optionBox_[options_]));
        gtk_widget_set_sensitive(GTK_WIDGET(this->optionBox_[options_]), TRUE);
        this->upOption();
        TRACE("option_=%d\n",this->options_); 
    }

    void upOption(void){
        this->options_++;
    }

    void downOption(void){
        this->options_--;
    }

    void save(const gchar *path){
        auto iconId = gtk_entry_get_text(iconEntry_);
        auto tooltip = gtk_entry_get_text(iconEntry_);
        auto workdir = (const gchar *)g_object_get_data(G_OBJECT(workdirLabel_), "path");
        auto executable = (const gchar *)g_object_get_data(G_OBJECT(executableLabel_), "path");
        gboolean terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inTerminal));
        //FILE *out = stderr;
        FILE *out = fopen(this->filename_, "w");
        fprintf(out, "[custombutton]\n");
        if (strlen(iconId)) {
            fprintf(out, "icon=%s\n", iconId);
        }
        if (strlen(tooltip)) {
            fprintf(out, "tooltip=%s\n", tooltip);
        }
        if (executable){
            fprintf(out, "exec=%s\n", executable);
        }
        if (workdir){
            fprintf(out, "workdir=%s\n", workdir);
        }
        fprintf(out, "terminal=%d\n\n", terminal?1:0);
        fprintf(out, "[options]\n");



        for (gint i=0; i<maxOptions_; i++){
            if (!gtk_widget_get_sensitive(GTK_WIDGET(optionBox_[i]))) continue;
            auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(optionBox_[i]), "entry"));
            auto option = gtk_entry_get_text(entry);
            if (strlen(option)) {
                // get which radio is active...
                auto list = (GSList *)g_object_get_data(G_OBJECT(optionBox_[i]), "group");
                for (;list && list->data; list=list->next){
                    auto check = (GTK_TOGGLE_BUTTON(list->data));
                    if (gtk_toggle_button_get_active(check)){
                        auto type = (const gchar *)g_object_get_data(G_OBJECT(check), "type");
                        fprintf(out, "option= --%s=%s\n", option, type);
                        break;
                    }
                }
            }
        }
        fclose(out);

    }
    
    void saver(void){
        auto dialog = gtk_file_chooser_dialog_new (_("Save File"),
                                              this->creatorWindow_,
                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                              _("Cancel"),
                                              GTK_RESPONSE_CANCEL,
                                              _("Save"),
                                              GTK_RESPONSE_ACCEPT,
                                              NULL);
        auto chooser = GTK_FILE_CHOOSER (dialog);

        gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

        if (this->filename_ == NULL)
          gtk_file_chooser_set_current_name (chooser,
                                             _("customButton.ini"));
        else
          gtk_file_chooser_set_filename (chooser,
                                         this->filename_);

        auto res = gtk_dialog_run (GTK_DIALOG (dialog));
        if (res == GTK_RESPONSE_ACCEPT)
          {
            auto filename = gtk_file_chooser_get_filename (chooser);
            this->save(filename);
            g_free(this->filename_);
            this->filename_ = filename;
          }

        gtk_widget_destroy (dialog);
    }


    static void
    saveIniFile(GtkButton *button, void *data) {
        auto object = (iniCreator<Type> *) data;

        object->saver();      
    }



public:

    GtkBox *bottombox(void){return bottombox_;}

    GtkBox *addOptionBox(GtkBox *box, gint i){
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        auto label = GTK_LABEL(gtk_label_new(""));
        auto markup = g_strdup_printf("<span color=\"green\"> %d </span>", i);
        gtk_label_set_markup(label, markup);
        g_free(markup);

        compat<bool>::boxPackStart(box, GTK_WIDGET(hbox), FALSE, FALSE, 3);
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(label), FALSE, FALSE, 3);
        auto entry = addEntryInput(hbox, _("Option"));
        g_object_set_data(G_OBJECT(hbox), "entry", entry);

        auto text = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    NULL, _("text")));
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(text), FALSE, FALSE, 3);
        g_object_set_data(G_OBJECT(text), "type", (void *)"text");
        


        auto file = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(text), _("file")));
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(file), FALSE, FALSE, 3);
        g_object_set_data(G_OBJECT(file), "type", (void *)"file");

        auto folder = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(text), _("folder")));
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(folder), FALSE, FALSE, 3);
        g_object_set_data(G_OBJECT(folder), "type", (void *)"folder");

        auto on = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(text), _("on")));
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(on), FALSE, FALSE, 3);
        g_object_set_data(G_OBJECT(on), "type", (void *)"on");

        auto off = GTK_RADIO_BUTTON(gtk_radio_button_new_with_label(
                    gtk_radio_button_get_group(text), _("off")));
        compat<bool>::boxPackStart(hbox, GTK_WIDGET(off), FALSE, FALSE, 3);
        g_object_set_data(G_OBJECT(off), "type", (void *)"off");

        GSList *group = gtk_radio_button_get_group(text);
        g_object_set_data(G_OBJECT(hbox), "group", group);
        gtk_widget_set_sensitive(GTK_WIDGET(hbox), FALSE);
        return hbox;
    }

};
} // end namespace xf
#if 0
int main(int argc, gchar **argv){
        
    gtk_init (&argc, &argv);
    gint max = 10;
    if (argv[1] != NULL){
        max = atoi(argv[1]);
        if (max < 0 || max > 50) {
            DBG("max out of range %d\n", max);
        }
    }

    
    auto creator = new (xf::iniCreator<double>)(max);
    gtk_main();

    return 0;

}
#endif

#endif
