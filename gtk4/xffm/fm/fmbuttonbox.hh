#ifndef XF_FMBUTTONBOX_HH
#define XF_FMBUTTONBOX_HH
namespace xf {

  class EmptyButtonBox{
    public:
    GtkBox *mkVbuttonBox(){
      return GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));  
    }
  };
  class FMbuttonBox{
    private:
      GtkBox *vButtonBox_;
    public:
    GtkBox *mkVbuttonBox(){
 
        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        
        gtk_widget_add_css_class (GTK_WIDGET(vButtonBox_), "vbox" );


        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);

        const char *bIcon[]={SEARCH, OPEN_TERMINAL, OPEN_FILEMANAGER, GO_HOME, DRIVE_HARDDISK, TRASH_ICON, GLIST_ADD, GLIST_REMOVE, NULL};
        const char *bText[]={_("Search"),_("Open terminal"),_("Open a New Window"),_("Home Directory"),_("Disk Image Mounter"),_("Trash bin"),_("Reset image size"),_("Reset image size"), NULL};
        auto q = bText;
        for (auto p=bIcon; p && *p; p++, q++){
          auto button = Util::newButton(*p, *q);
          Util::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
        }
        
        Util::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);

        return hbox;
    }
  };
}

#endif
