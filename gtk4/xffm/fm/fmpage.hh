#ifndef XF_FMPAGE_HH
#define XF_FMPAGE_HH
#include "fmbuttonbox.hh"
#include "utilpathbar.hh"
#include "pathbar.hh"
#include "prompt.hh"

namespace xf {

  class Vpane : public UtilBasic {
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

        auto popover = mkTextviewMenu();
//FIXME/// add signal controller for popover menu.
         // deprecated g_object_set_data(G_OBJECT(vpane_), "diagnostics", output_);
         g_object_set_data(G_OBJECT(vpane_), "output", output_);
         g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

        auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0)); 
        boxPack0 (vbox, GTK_WIDGET(topScrolledWindow_), TRUE, TRUE, 0);
        boxPack0 (vbox, GTK_WIDGET(treeScrolledWindow_), TRUE, TRUE, 0);
        gtk_paned_set_start_child (vpane_, GTK_WIDGET(vbox));
       
        
        gtk_paned_set_end_child (vpane_, GTK_WIDGET(bottomScrolledWindow_));
        g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_scrolled_window_set_child(bottomScrolledWindow_, GTK_WIDGET(output_));
        
        return ;
    }

    private:
    GtkPopover *mkTextviewMenu(void){
      static const char *text[]= {
        _("Copy"), 
        _("Select All"), 
        NULL
      };
      GHashTable *mHash[3];
      mHash[0] = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);
      for (int i=1; i<3; i++) mHash[i] = g_hash_table_new(g_str_hash, g_str_equal);

      g_hash_table_insert(mHash[0], _("Copy"), g_strdup(EDIT_COPY));
      g_hash_table_insert(mHash[1], _("Copy"), NULL);
      g_hash_table_insert(mHash[2], _("Copy"), NULL);
      g_hash_table_insert(mHash[0], _("Select All"), g_strdup(VIEW_MORE));
      g_hash_table_insert(mHash[1], _("Select All"), NULL);
      g_hash_table_insert(mHash[2], _("Select All"), NULL);

      auto menu = Util::mkMenu(text,mHash,_("Output Menu"));
 
      return menu;
    }

  };

  class FMpage : private Util, public Vpane, public Prompt, public Pathbar {
    private:
      GtkBox *childBox_;
      gchar *path_=NULL;
      // We keep reference to Vpane object,
      // eventhough it will change. Actual reference
      // will be asociated to page box.
      // Same for Prompt.
    public:
      GtkBox *childBox(void){ return childBox_;}
      FMpage(const char *path){
        path_ = g_strdup(path);
        childBox_ = mkPageBox(path);
      }
      ~FMpage(){
        g_free(path_);
      }

      GtkBox *mkPageBox(const gchar *path){
        DBG("mkPageBox(%s)\n", path);
        gchar *tag = path_? g_path_get_basename(path_):g_strdup(".");
        auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));  
        
        if (g_file_test(path, G_FILE_TEST_IS_DIR)){
          // too soon Util::setWorkdir(path);
          g_object_set_data(G_OBJECT(box), "path", g_strdup(path));
        } else {
          g_object_set_data(G_OBJECT(box), "path", g_strdup(g_get_home_dir()));
          // too soon Util::setWorkdir(g_get_home_dir());
        }

        auto *label = gtk_label_new(tag);
        g_free(tag);
        gtk_widget_set_vexpand(GTK_WIDGET(box), TRUE);

        
        auto output = this->output();
        auto input = this->input();
        auto buttonSpace = this->buttonSpace();
        g_object_set_data(G_OBJECT(box), "buttonSpace", buttonSpace);

        g_object_set_data(G_OBJECT(input), "output", output);
        g_object_set_data(G_OBJECT(output), "input", input);

        g_object_set_data(G_OBJECT(box), "output", output);
        g_object_set_data(G_OBJECT(box), "input", input);

        auto promptBox = GTK_WIDGET(this->promptBox());
        auto vpane = GTK_WIDGET(this->vpane());
        auto pathbar = this->pathbar();
        g_object_set_data(G_OBJECT(box), "vpane", vpane);
        g_object_set_data(G_OBJECT(output), "pathbar", pathbar);
        g_object_set_data(G_OBJECT(pathbar), "child", box);

        UtilPathbar::updatePathbar(path, pathbar, true);

        boxPack0(box, GTK_WIDGET(this->pathbar()),  FALSE, TRUE, 0);
        boxPack0(box, GTK_WIDGET(this->vpane()),  TRUE, TRUE, 0);
        boxPack0(box, GTK_WIDGET(this->promptBox()),  FALSE, TRUE, 0);

        //gtk_widget_set_visible(promptBox, TRUE);

        return box;
      }

    private:


  };

}
#endif
