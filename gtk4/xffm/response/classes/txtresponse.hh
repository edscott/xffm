#ifndef TXTRESPONSE_HH
#define TXTRESPONSE_HH


namespace xf {
class TxtDialog {
   const char *title_;
   const char *iconName_;
   const char *label_;
   GtkBox *mainBox_ = NULL;
public:
    const char *title(void){ return _("Informational message");}
    const char *iconName(void){ return EMBLEM_INFO;}
    const char *label(void){ return _("");}

    GtkBox *mainBox(const char *markup){
        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), true);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), true);
        auto label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label), markup);
        gtk_box_append(mainBox_, GTK_WIDGET(label));
        return mainBox_;
    }

    static void *asyncYes(void *data){
      return asyncNo(data);
    }

    static void *asyncNo(void *data){
      // Cancel
      // send interrupt signal to parent
      pid_t parent = getppid();
      kill(parent, SIGINT);
      return NULL;
    }
};
}


#endif
