#ifndef SIGNALS__HH
# define SIGNALS__HH

typedef struct radio_t {
    GtkBox *box;
    GtkToggleButton *toggle[5];
} radio_t;

namespace xf
{


template <class Type>
class Signals{
    typedef Gtk<double> gtk_c;
public:
    static void 
    sensitivize (GtkToggleButton *togglebutton, gpointer data){
	GtkWidget *widget = GTK_WIDGET(data);
	gtk_widget_set_sensitive(widget, gtk_toggle_button_get_active(togglebutton));
    }

    static void 
    sensitivize_radio (GtkToggleButton *togglebutton, gpointer data){
	if (!data) return;
	radio_t *radio_p = (radio_t *)data;
	gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), FALSE);
	GtkToggleButton **tb_p = radio_p->toggle;
	for (; tb_p && *tb_p; tb_p++){
	    if (gtk_toggle_button_get_active(*tb_p)){
		gtk_widget_set_sensitive(GTK_WIDGET(radio_p->box), TRUE);
	    }
	}
    }

    static void
    command_help (GtkWidget * button, gpointer data) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	const gchar *message = (const gchar *)data;
	std::cerr<<"fixme: signals::command_help\n";
	gtk_c::quick_help(dialog_, message);
    }

    static void
    on_buttonHelp (GtkWidget * button, gpointer data) {
	GtkWindow *dialog_=GTK_WINDOW(g_object_get_data(G_OBJECT(button), "dialog_"));
	const gchar *message = (const gchar *)data;
	std::cerr<<"fixme: signals::on_buttonHelp\n";
	gtk_c::quick_help(dialog_, message);
    }

};

}
#endif
