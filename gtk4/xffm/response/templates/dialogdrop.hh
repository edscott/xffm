#ifndef DIALOGDROP_HH
# define DIALOGDROP_HH
namespace xf
{

  template <class dialogClass>
  class DialogDrop : public DialogBasic<dialogClass>{
    GtkLabel *actionLabel_;
    GtkProgressBar *progress_;
    public:
    GtkLabel *actionLabel(void){ return actionLabel_;}
    GtkProgressBar *progress(void){ return progress_;}
    DialogDrop(void){
      actionLabel_ = GTK_LABEL(gtk_label_new(""));
      gtk_box_append(this->actionArea(), GTK_WIDGET(actionLabel_));
      progress_ = GTK_PROGRESS_BAR(gtk_progress_bar_new());
      gtk_progress_bar_set_show_text(progress_, true);
      auto dialog = this->dialog();
      auto contentArea = this->contentArea();
      //auto buttons = this->subClass()->getButtons();
      auto box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
      gtk_box_append(contentArea, GTK_WIDGET(box));
      gtk_box_append(box, GTK_WIDGET(progress_));
      gtk_widget_set_hexpand(GTK_WIDGET(box), true);
      gtk_widget_set_halign (GTK_WIDGET(progress_),GTK_ALIGN_END);
      // enough space, seems to resize on each label grow change...
      gtk_widget_set_size_request(GTK_WIDGET(dialog), 200, 75);

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);

    }

    void setProgress (int k, int total, const char *path, const char *file, int bytes, int totalBytes){
        this->lockResponse();
        if (this->cancelled()){
          this->unlockResponse();
          return;
        }
        this->unlockResponse();
        bool copy = this->subClass()->copy();

        auto markup1 = g_strconcat("<span color=\"red\">",copy?_("Copying"):_("Moving"), 
            " ---> ", path,
            "</span>\n", file, NULL);
        auto markup2 = g_strdup_printf("<span color=\"blue\">%s %.1lf/%.1lf MB</span>", _("Completed"),
            (double)bytes/1024/1024, (double)totalBytes/1024/1024);
        void *arg[] = {
          (void *)markup1,
          (void *)markup2,
          GINT_TO_POINTER(k),
          GINT_TO_POINTER(total),
          (void *)this,
         NULL};
        Basic::context_function(setLabel_f, arg);
        g_free(markup1);
        g_free(markup2);
    }

    
    private:
    static void *setLabel_f(void *data){
      void **arg = (void **)data;
      auto markup1 = (const char *)arg[0];
      auto markup2 = (const char *)arg[1];
      auto k = GPOINTER_TO_INT(arg[2]);
      auto total = GPOINTER_TO_INT(arg[3]);
      auto object = (DialogDrop<dialogClass> *)arg[4];
      gtk_label_set_markup(object->label(), markup1);
      auto text = g_strdup_printf("%d/%d", k, total);
      gtk_progress_bar_set_text(object->progress(), text);
      g_free(text);
      gtk_progress_bar_set_fraction(object->progress(), (double)k / (double) total);
      gtk_label_set_markup(object->actionLabel(), markup2);
      return NULL;
    }
    

  };
}
#endif

