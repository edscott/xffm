#ifndef XF_FMPAGE_HH
#define XF_FMPAGE_HH


namespace xf {

  class Vpane {
    private:
    GtkPaned *vpane_;
    GtkTextView *output_;
    GtkScrolledWindow *gridScrolledWindow_;
    GtkScrolledWindow *outputScrolledWindow_; 

    public:
    GtkPaned *vpane(void){return vpane_;}
    GtkTextView *output(void){return output_;}
    GtkScrolledWindow *gridScrolledWindow(void){return gridScrolledWindow_;}
    GtkScrolledWindow *outputScrolledWindow(void){return outputScrolledWindow_;}

    Vpane(void){
        vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
        gtk_paned_set_wide_handle (vpane_, TRUE);
        //gtk_paned_set_wide_handle (vpane_, FALSE);
        gridScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        
        outputScrolledWindow_ = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new ());
        output_ = Util<LocalDir>::newTextView();
   

        g_object_set_data(G_OBJECT(vpane_), "output", output_);
        g_object_set_data(G_OBJECT(output_), "vpane", vpane_);

        gtk_paned_set_start_child (vpane_, GTK_WIDGET(gridScrolledWindow_));
        gtk_paned_set_position(vpane_, 10000);
        
       
        gtk_paned_set_end_child (vpane_, GTK_WIDGET(outputScrolledWindow_));
        g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
        gtk_scrolled_window_set_child(outputScrolledWindow_, GTK_WIDGET(output_));
        //gtk_paned_set_position(vpane_, 10000);
        
        return ;
    }

    private:

    

  };

  class FMpage : public Vpane, public Prompt<LocalDir>, public Pathbar<LocalDir> {
    private:
      GtkBox *childBox_;
      gchar *path_=NULL;
      GtkPopover *gridMenu_=NULL;
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
        TRACE("FMpage destructor: need to call GridView destructor...\n");
        //auto gridView_p = (GridView<LocalDir> *)Child::getGridviewObject(GTK_WIDGET(childBox_));
        //delete gridView_p;
        g_free(path_);
       
      }

      GtkBox *mkPageBox(const gchar *path){
        TRACE("mkPageBox(%s)\n", path);
        gchar *tag = path_? g_path_get_basename(path_):g_strdup(".");
        auto box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));  
        
        if (g_file_test(path, G_FILE_TEST_IS_DIR)){
          g_object_set_data(G_OBJECT(box), "path", g_strdup(path));
        } else {
          g_object_set_data(G_OBJECT(box), "path", g_strdup(g_get_home_dir()));
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


        auto title = g_strconcat("",_("Output"),_(" TTY"), NULL);
        auto myOutputMenu = new Menu<OutputMenu<LocalDir> >(title);
        myOutputMenu->setMenu(GTK_WIDGET(output), GTK_WIDGET(output), Child::getWorkdir(), true);
        g_free(title);
        delete myOutputMenu;

        g_object_set_data(G_OBJECT(box), "buttonSpace", buttonSpace);
        g_object_set_data(G_OBJECT(box), "gridScrolledWindow", gridScrolledWindow);
        g_object_set_data(G_OBJECT(box), "outputScrolledWindow", outputScrolledWindow);

        g_object_set_data(G_OBJECT(input), "output", output);
        g_object_set_data(G_OBJECT(output), "input", input);

        g_object_set_data(G_OBJECT(box), "output", output);
        g_object_set_data(G_OBJECT(box), "input", input);
        g_object_set_data(G_OBJECT(box), "dollar", dollar);

        auto size = Settings::getInteger("xfterm", "fontcss");
        if (size < 1 || size > 7) size=3;
        auto css = g_strdup_printf("font%d", size);

        DBG("*** css font size  is %d: %s\n", size, css);

        gtk_widget_add_css_class (GTK_WIDGET(output), css);
        g_object_set_data(G_OBJECT(output), "css", (void *)css);

        /*auto fontSize = Settings::getString("xfterm", "outputSize");
        if (fontSize) gtk_widget_add_css_class(GTK_WIDGET(output), fontSize);
        g_free(fontSize);*/

        /*fontSize = Settings::getString("xfterm", "inputSize");
        if (fontSize) {
          gtk_widget_add_css_class(GTK_WIDGET(output), fontSize);
          gtk_widget_add_css_class(GTK_WIDGET(dollar), fontSize);
        }
        g_free(fontSize);*/

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

        Basic::boxPack0(box, GTK_WIDGET(this->pathbar()),  FALSE, TRUE, 0);
        Basic::boxPack0(box, GTK_WIDGET(this->vpane()),  TRUE, TRUE, 0);

        auto toggle = Basic::newButton(EMBLEM_TERMINAL, _("Toggle Text Mode"));
        Basic::boxPack0(this->promptBox(), GTK_WIDGET(toggle),  FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (toggle), "clicked", G_CALLBACK(MenuCallbacks<LocalDir>::toggleVpane), NULL);
        auto scale = newSizeScale(_("Font size"));
        Basic::boxPack0(this->promptBox(), GTK_WIDGET(scale),  FALSE, FALSE, 0);
        
        Basic::boxPack0(box, GTK_WIDGET(this->promptBox()),  FALSE, TRUE, 0);
        return box;
      }

    private:

    static void
    fontSize(GtkRange* self, void *data){
      auto value = floor(gtk_range_get_value(self));
      int valueI = value;

      auto css = g_strdup_printf("font%d", valueI);
      TRACE("menucallbacks.hh: fontSize() value=%d font=%s\n", valueI, css);
      auto textView = Child::getOutput();
      auto oldCss = (char *)g_object_get_data(G_OBJECT(textView), "css");
      if (oldCss != NULL){
        TRACE("retrieved oldCss=%s\n", oldCss);
        if (strcmp(oldCss, css) == 0){
           TRACE("no css change\n");
          g_free(css);
          return;
        }
        TRACE("removing css: %s\n", oldCss);
        gtk_widget_remove_css_class (GTK_WIDGET(textView), oldCss);
        //g_free(oldCss);
      }

      TRACE("adding css: %s\n", css);
      gtk_widget_add_css_class (GTK_WIDGET(textView), css);
      g_object_set_data(G_OBJECT(textView), "css", (void *)css);

      Settings::setInteger("xfterm", "fontcss", valueI);
      Basic::flushGTK();

    }
        
    GtkScale *newSizeScale(const gchar *tooltipText){
        auto size_scale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 7.0, 1.0));
        // Load saved value fron xffm+/settings.ini file (if any)
        auto size = Settings::getInteger("xfterm", "fontcss");
        if (size < 1 || size > 7) size=3;
        double value = size;
        DBG("***range set value=%lf\n", value);
        gtk_range_set_value(GTK_RANGE(size_scale), value);

        gtk_range_set_increments (GTK_RANGE(size_scale), 1.0, 1.0);
        gtk_widget_set_size_request (GTK_WIDGET(size_scale),50, -1);

        gtk_scale_set_value_pos (size_scale,GTK_POS_BOTTOM);
        Basic::setTooltip (GTK_WIDGET(size_scale),tooltipText);   
        g_signal_connect(G_OBJECT(size_scale), "value-changed", G_CALLBACK(fontSize), NULL);
        return size_scale;
    }



  };

}
#endif
