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
         const char *bIcon[]={
           OPEN_TERMINAL, 
           SEARCH, 
           OPEN_FILEMANAGER, 
           GLIST_ADD, 
           GLIST_REMOVE, 
           EDIT_CLEAR,
           "media-view-subtitles",
           NULL
         };
        const char *bText[]={
          _("Open terminal"),
          _("Search"),
          _("Open a New Window"),
          _("Reset image size"),
          _("Reset image size"), 
          _("Clear Log"),
          _("Show/hide grid."),
          NULL
        };
        auto scale = Util::newSizeScale(_("Terminal font"));

        auto hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_widget_set_hexpand(GTK_WIDGET(hbox), FALSE);
        vButtonBox_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        
        gtk_widget_add_css_class (GTK_WIDGET(vButtonBox_), "vbox" );


        gtk_widget_set_hexpand(GTK_WIDGET(vButtonBox_), TRUE);

        /*const char *bIcon[]={OPEN_FILEMANAGER, GO_HOME, DRIVE_HARDDISK, TRASH_ICON, NULL};
        const char *bText[]={_("Open a New Window"),_("Home Directory"),_("Disk Image Mounter"),_("Trash bin"),_ NULL};*/

        auto q = bText;
        for (auto p=bIcon; p && *p; p++, q++){
          auto button = Util::newButton(*p, *q);
          Util::boxPack0(vButtonBox_, GTK_WIDGET(button),  FALSE, FALSE, 0);
        }
        Util::boxPack0(vButtonBox_, GTK_WIDGET(scale),  FALSE, FALSE, 0);
        
        Util::boxPack0(hbox, GTK_WIDGET(vButtonBox_),  FALSE, FALSE, 0);

        return hbox;
    }
  };
}

#endif
