#ifndef XF_TERM_DIALOG
#define XF_TERM_DIALOG


#include "common/types.h"
#include "common/icons.hh"
#include "common/pixbuf.hh"
#include "common/gtk.hh"
#include "common/tooltip.hh"
#include "common/dialog.hh"
#include "common/util.hh"

#include "common/print.hh"
#include "common/completion/csh.hh"

namespace xf 
{
template <class Type>
class termDialog;

template <class Type>
class completionSignals{
public:
    static gboolean
    window_keyboard_event (GtkWidget *window, GdkEventKey * event, gpointer data)
    {
        DBG("window_keyboard_event\n");
        auto dialog_p = (termDialog<Type> *)data;
        auto notebook = dialog_p->notebook();
        auto page_p = dialog_p->currentPageObject();
        auto input = page_p->status();
        auto output = page_p->diagnostics();
        // do the completion thing
        page_p->keyboard_event(event);
        return TRUE;
    }
};    
    
template <class Type>
class termDialog : public Dialog<Type>{
    using print_c = Print<double>;
    using util_c = Util<double>;
  /*  using gtk_c = Gtk<double>;
    using pixbuf_c = Pixbuf<double>;
    using pixbuf_icons_c = Icons<double>;
    using tooltip_c = Tooltip<double>;*/
 //   using dialog_c = xfDialog<double>;
 //
private:

 

    ///
public:
    //termDialog(const gchar *title, const gchar *icon):Dialog<Type>(title, icon){
    termDialog(void){
	// Here we override the default start up of the Dialog class template
	//DBG("%s %s\n", title, icon);
        gchar *workdir = g_get_current_dir();
        this->setDialogSize(600,400);
        this->setVpanePosition(0);
        this->setTabIcon("utilities-terminal");
        auto page = this->currentPageObject();
        page->setPageWorkdir(workdir);
        print_c::print(this->diagnostics(), "tag/red", g_strdup("Hello world!\n"));
        //DBG("current page = %d\n", this->currentPage());
        //this->setPageLabel(this->currentPage(), "foo");
    }
 
private:


};
}



#endif
