#ifndef FFUNCTIONS_HH
# define FFUNCTIONS_HH
namespace xf {
  class Ffunctions {
    public:
      
     static const char **ftypes(void){
       static const char *ftypes_[] = {
         N_("Regular"),
         N_("Directory"),
         N_("Symbolic Link"),
         N_("Socket"),
         N_("Block device"),
         N_("Character device"),
         N_("FIFO"),
         N_("Any"),
         NULL
       };
       return ftypes_;
     }
     static const char **ft(void) {
       static const char *ft_[] = {
          "reg",
          "dir",
          "sym",
          "sock",
          "blk",
          "chr",
          "fifo",
          "any",
          NULL
       };
       return ft_;
     }


    public:
      
    protected:

    static void 
    sensitivizeSpin (GtkCheckButton *check, gpointer data){
        TRACE("*** sensitivizeSpin, widget = %p\n", data);
        GtkWidget *widget = GTK_WIDGET(data);
        gtk_widget_set_sensitive(widget, gtk_check_button_get_active(check));
    }

    static void
    sensitivize ( GtkEntryBuffer* self, guint position, gchar* chars, guint n_chars, void *data){
      auto box = GTK_WIDGET(data);
      auto text = gtk_entry_buffer_get_text(self);
      gtk_widget_set_sensitive(box, strlen(text)>0);
    }





  };
  
}
#endif
