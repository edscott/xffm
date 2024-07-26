#ifndef XF_FMPAGE_HH
#define XF_FMPAGE_HH
#include "prompt.hh"
#include "fmbuttonbox.hh"

namespace xf {

  class Vpane{
    private:
    GtkPaned *vpane_;
    GtkTextView *output_;
    GtkScrolledWindow *topScrolledWindow_;
    GtkScrolledWindow *treeScrolledWindow_;
    GtkScrolledWindow *bottomScrolledWindow_; 

    public:
    GtkPaned *vpane(void){return vpane_;}
    GtkTextView *output(void){return output_;}
    GtkScrolledWindow *treeScrolledWindow(void){return treeScrolledWindow_;}
    GtkScrolledWindow *topScrolledWindow(void){return topScrolledWindow_;}
    GtkScrolledWindow *bottomScrolledWindow(void){return bottomScrolledWindow_;}

    Vpane(void){
        vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
        gtk_paned_set_wide_handle (vpane_, TRUE);
        //gtk_paned_set_wide_handle (vpane_, FALSE);
        topScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        treeScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        
        bottomScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        output_ = Util::newTextView();

         // deprecated g_object_set_data(G_OBJECT(vpane_), "diagnostics", output_);
         g_object_set_data(G_OBJECT(vpane_), "output", output_);
         g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0)); 
        Util::boxPack0 (vbox, GTK_WIDGET(topScrolledWindow_), TRUE, TRUE, 0);
        Util::boxPack0 (vbox, GTK_WIDGET(treeScrolledWindow_), TRUE, TRUE, 0);
        gtk_paned_set_start_child (vpane_, GTK_WIDGET(vbox));
       
        
        gtk_paned_set_end_child (vpane_, GTK_WIDGET(bottomScrolledWindow_));
        g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_scrolled_window_set_child(bottomScrolledWindow_, GTK_WIDGET(output_));
        
        return ;
    }

  };

  class FMpage {
    private:
      gchar *path_=NULL;
      // We keep reference to Vpane object,
      // eventhough it will change. Actual reference
      // will be asociated to page box.
      // Same for Prompt.
      Vpane *vpane_object_;
      Prompt *prompt_object_;
    public:
      FMpage(void){
      }
      ~FMpage(){
        g_free(path_);
      }

      GtkBox *mkPageBox(const gchar *path){
        path_ = g_strdup(path);
        gchar *tag = path_? g_path_get_basename(path_):g_strdup(".");
        auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));  
        auto *label = gtk_label_new(tag);
        g_free(tag);
        gtk_widget_set_hexpand(GTK_WIDGET(box), TRUE);
        vpane_object_ = new(Vpane);
        prompt_object_ = new(Prompt);
        g_object_set_data(G_OBJECT(box), "vpane_object", vpane_object_);
        g_object_set_data(G_OBJECT(box), "prompt_object", prompt_object_);

        Util::boxPack0(box, GTK_WIDGET(vpane_object_->vpane()),  TRUE, TRUE, 0);
        Util::boxPack0(box, GTK_WIDGET(prompt_object_->promptBox()),  FALSE, FALSE, 0);

        return box;
      }

    private:


  };

}
#endif
