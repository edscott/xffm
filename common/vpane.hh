#ifndef XF_VPANE
#define XF_VPANE
#include "diagnostics.hh"

namespace xf {

template <class Type>
class Vpane : public Diagnostics<Type>{
public:
    Vpane(void){
	vpane_ = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	gtk_paned_set_wide_handle (vpane_, TRUE);
	auto top_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane_), "top_scrolled_window", top_scrolled_window);
	 
	auto bottom_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	 g_object_set_data(G_OBJECT(vpane_), "bottom_scrolled_window", bottom_scrolled_window);
	gtk_paned_pack1 (vpane_, GTK_WIDGET(top_scrolled_window), FALSE, TRUE);
	gtk_paned_pack2 (vpane_, GTK_WIDGET(bottom_scrolled_window), TRUE, TRUE);

	g_object_set(G_OBJECT(vpane_), "position-set", TRUE, NULL);
	gint max, current;
	
        this->insertDiagnostics(GTK_CONTAINER(bottom_scrolled_window));
        
        
        
        gtk_widget_show_all(GTK_WIDGET(vpane_));
        return;
    }
    
    void setVpanePosition(gint position){
	gtk_paned_set_position (vpane_, position);
        gint max;
	g_object_get(G_OBJECT(vpane_), "max-position", &max, NULL);
 	g_object_set_data(G_OBJECT(vpane_), "oldCurrent", GINT_TO_POINTER(position));
	g_object_set_data(G_OBJECT(vpane_), "oldMax", GINT_TO_POINTER(max));   
    }
    
    GtkPaned *vpane(void){ return vpane_;}
    void insertVPane(GtkBox *hViewBox){
	gtk_box_pack_start (hViewBox, GTK_WIDGET(vpane_), TRUE, TRUE, 0);
    }


private:
    GtkPaned *vpane_;


};
}




#endif
