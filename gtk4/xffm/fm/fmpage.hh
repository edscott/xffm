#ifndef XF_FMPAGE_HH
#define XF_FMPAGE_HH
#include "fmbuttonbox.hh"
#include "pathbar.hh"
#include "prompt.hh"
#include "outputMenu.hh"

namespace xf {

  class Vpane : public UtilBasic {
    private:
    GtkPaned *vpane_;
    GtkTextView *output_;
    GtkScrolledWindow *gridScrolledWindow_;
    GtkScrolledWindow *treeScrolledWindow_;
    GtkScrolledWindow *outputScrolledWindow_; 

    public:
    GtkPaned *vpane(void){return vpane_;}
    GtkTextView *output(void){return output_;}
    GtkScrolledWindow *treeScrolledWindow(void){return treeScrolledWindow_;}
    GtkScrolledWindow *gridScrolledWindow(void){return gridScrolledWindow_;}
    GtkScrolledWindow *outputScrolledWindow(void){return outputScrolledWindow_;}

    Vpane(void){
        vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
        gtk_paned_set_wide_handle (vpane_, TRUE);
        //gtk_paned_set_wide_handle (vpane_, FALSE);
        gridScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        treeScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        
        outputScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        output_ = Util::newTextView();

        auto myOutputMenu = new Menu<OutputMenu>;
        auto title = g_strconcat("<span color=\"blue\">", _("Output"),_(" TTY"), "</span>", NULL);
        auto outputMenu = myOutputMenu->getMenu(title);
        g_free(title);
        delete myOutputMenu;
        Util::addMenu(outputMenu, GTK_WIDGET(output_));
       
        g_object_set_data(G_OBJECT(vpane_), "output", output_);
        g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

        //auto vbox = GTK_BOX(gtk_box_new (GTK_ORIENTATION_VERTICAL, 0)); 
        //boxPack0 (vbox, GTK_WIDGET(gridScrolledWindow_), TRUE, TRUE, 0);
        //boxPack0 (vbox, GTK_WIDGET(treeScrolledWindow_), TRUE, TRUE, 0);
        //gtk_paned_set_start_child (vpane_, GTK_WIDGET(vbox));
        gtk_paned_set_start_child (vpane_, GTK_WIDGET(gridScrolledWindow_));
        
        


        gtk_widget_set_visible(GTK_WIDGET(treeScrolledWindow_), FALSE);

       
        
        gtk_paned_set_end_child (vpane_, GTK_WIDGET(outputScrolledWindow_));
        g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_scrolled_window_set_child(outputScrolledWindow_, GTK_WIDGET(output_));
        gtk_paned_set_position(vpane_, 10000);
#if 0
        //auto gfile = g_file_new_for_path("/");
        auto gfile = g_file_new_for_path(g_get_home_dir());
        //auto gfile = g_file_new_for_path(Util::getWorkdir());
        auto dList = gtk_directory_list_new(NULL, gfile);
        while (gtk_directory_list_is_loading(dList)) {
          DBG("gtk_directory_list_is_loading...\n");
          Util::flushGTK();
        }
        auto num = g_list_model_get_n_items(G_LIST_MODEL(dList));
        DBG("gtk_directory_list_is_loading done: items=%d\n", num);
        for (int i=0; i<num; i++){
          auto info = G_FILE_INFO(g_list_model_get_item(G_LIST_MODEL(dList), i));
          // file, type, name
          GFileType tipo = G_FILE_TYPE_UNKNOWN;
          if (g_file_info_has_attribute(info, "standard::type")) tipo = g_file_info_get_file_type (info);
          else {
            // XXX no good if no standard::type
            //if (g_file_info_get_is_symlink(info) )tipo = G_FILE_TYPE_SYMBOLIC_LINK;
          }
auto s = g_file_info_get_attribute_as_string (info, "standard::type");
          GFile *z = G_FILE(g_file_info_get_attribute_object(info, "standard::file"));
          DBG("g_file_info_get_attribute_file_path(name)=%s (%s), tipo=%d (%s), ->%s (%p) %s\n",
              
              g_file_info_get_name(info),
              (const char *)g_object_get_data(G_OBJECT(info), "standard::name"), //nah
              
              tipo,s,
              g_file_info_get_attribute_as_string(info, "standard::file"),
              g_file_info_get_attribute_object(info, "standard::file"),
              g_file_get_path(z)
              ); 
         /* auto **v = g_file_info_list_attributes (info, NULL);
          for (char **p=v; p && *p; p++){
            DBG("Attribute: %s\n", *p);
          }
          g_strfreev(v);*/


         /* auto path = g_file_get_path(file);
          DBG("path=%s\n", path);
          g_free(path);*/
          g_free(info);
        }

//GtkMultiSelection*
       // auto gridview = IconView::getGridView(path);
     /*   auto selectionModel = GTK_SELECTION_MODEL(gtk_multi_selection_new(G_LIST_MODEL(dList)));
        
        auto gridview = gtk_grid_view_new(selectionModel, NULL);*/
       // gtk_scrolled_window_set_child(topScrolledWindow_, GTK_WIDGET(gridview));

#endif
    /*    auto treeExpander = gtk_tree_expander_new();
        gtk_scrolled_window_set_child(treeScrolledWindow_, GTK_WIDGET(treeExpander));

        gtk_widget_add_css_class(treeExpander, "xficons");*/
        
        return ;
    }

    private:

    

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
        TRACE("mkPageBox(%s)\n", path);
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
        auto dollar = this->dollar();
        auto buttonSpace = this->buttonSpace();
        auto gridScrolledWindow = this->gridScrolledWindow();
        auto outputScrolledWindow = this->outputScrolledWindow();

        /* gridview creation and setting is done by workdir class.
         * at updateGridView()
        auto gridview = Workdir::getGridView(path);        
        gtk_scrolled_window_set_child(gridScrolledWindow, GTK_WIDGET(gridview));*/

        g_object_set_data(G_OBJECT(box), "buttonSpace", buttonSpace);
        g_object_set_data(G_OBJECT(box), "gridScrolledWindow", gridScrolledWindow);
        g_object_set_data(G_OBJECT(box), "outputScrolledWindow", outputScrolledWindow);

        g_object_set_data(G_OBJECT(input), "output", output);
        g_object_set_data(G_OBJECT(output), "input", input);

        g_object_set_data(G_OBJECT(box), "output", output);
        g_object_set_data(G_OBJECT(box), "input", input);
        g_object_set_data(G_OBJECT(box), "dollar", dollar);

        auto fontSize = Settings::getString("xfterm", "outputSize");
        if (fontSize) gtk_widget_add_css_class(GTK_WIDGET(output), fontSize);
        g_free(fontSize);

        fontSize = Settings::getString("xfterm", "inputSize");
        if (fontSize) {
          gtk_widget_add_css_class(GTK_WIDGET(output), fontSize);
          gtk_widget_add_css_class(GTK_WIDGET(dollar), fontSize);
        }
        g_free(fontSize);


        auto promptBox = GTK_WIDGET(this->promptBox());
        auto vpane = GTK_WIDGET(this->vpane());
        auto pathbar = this->pathbar();
        g_object_set_data(G_OBJECT(box), "vpane", vpane);
        g_object_set_data(G_OBJECT(box), "pathbar", pathbar);
        g_object_set_data(G_OBJECT(output), "pathbar", pathbar);

        g_object_set_data(G_OBJECT(input), "child", box);
        g_object_set_data(G_OBJECT(output), "child", box);
        g_object_set_data(G_OBJECT(pathbar), "child", box);
        g_object_set_data(G_OBJECT(pathbar), "input", input);
        g_object_set_data(G_OBJECT(pathbar), "output", output);

        TRACE("updatePathbar(%s)\n", path);
        // FIXME:: UtilPathbar::updatePathbar(path, pathbar, true); 
        // bool is to add navigation history

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
