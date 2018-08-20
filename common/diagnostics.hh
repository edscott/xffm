#ifndef XF_DIAGNOSTICS
#define XF_DIAGNOSTICS

namespace xf {
template <class Type>
class Diagnostics {
public:
        Diagnostics(void){
	diagnostics_ = GTK_TEXT_VIEW(gtk_text_view_new ());
	gtk_text_view_set_monospace (diagnostics_, TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(diagnostics_), FALSE);
	gtk_text_view_set_wrap_mode (diagnostics_, GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (diagnostics_, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (diagnostics_), 2);
	return;
    }

    GtkTextView *diagnostics(void){ return diagnostics_;}
    void insertDiagnostics(GtkContainer *container){
        gtk_container_add (container, GTK_WIDGET(diagnostics_));
    }


private:
    GtkTextView *diagnostics_;
};

}



#endif
