#ifndef TARRESPONSE_HH
#define TARRESPONSE_HH

namespace xf {
template <class Type>
class tarResponse {
   using subClass_t = tarResponse;
   using dialog_t = DialogComplex<subClass_t>;
   GtkBox *mainBox_;
   GtkWindow *dialog_ = NULL;
   char *tar_ = NULL;
   char *zip_ = NULL;
   const char *formats[5]={N_("GZip"),N_("BZip2"),N_("XZ"),N_("Zip"),NULL};
   public:
   ~tarResponse(void){
     g_free(tar_);
     g_free(zip_);
   }
   
    const char *title(void){ return _("Compressed file...");}
    const char *label(void){return _("Create a compressed archive with the selected objects");}
    void dialog(GtkWindow *value){ dialog_ = value; }
    GtkWindow *dialog(void){return dialog_;}

     static void *asyncYes(void *data){
      DBG("%s", "Tar hello world\n");
      return NULL;
    }

    static void *asyncNo(void *data){
      DBG("%s", "Tar goodbye world\n");
      return NULL;
    }
 
    GtkBox *mainBox(const char *folder, const char *path) {
      tar_ = g_find_program_in_path("tar");
      zip_ = g_find_program_in_path("zip");

        mainBox_ = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
        gtk_widget_set_size_request(GTK_WIDGET(mainBox_), 550, 400);
        gtk_widget_set_vexpand(GTK_WIDGET(mainBox_), false);
        gtk_widget_set_hexpand(GTK_WIDGET(mainBox_), false);
        auto hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_vexpand(GTK_WIDGET(hbox), false);
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), true);
        gtk_box_append(mainBox_, GTK_WIDGET(hbox));
        auto label0 = gtk_label_new(label());
        gtk_box_append(hbox, GTK_WIDGET(label0));
        return mainBox_;
    }

};
}
#endif

