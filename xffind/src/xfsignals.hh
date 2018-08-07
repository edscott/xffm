#ifndef SIGNALS__HH
# define SIGNALS__HH

#include "types.h"
#include "xfprint.hh"
#include "xfrun.hh"

namespace xf
{


template <class Type>
class Signals: public Run<Type>{
    using gtk_c = Gtk<double>;
    using print_c = Print<double>;
    using run_c = Run<double>;
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


    static void
    onClearButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(data);
	std::cerr<<"fixme: signals::onClearButton\n";
        print_c::clear_text(diagnostics);

    }

    static void
    onEditButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
	std::cerr<<"fixme: signals::onEditButton\n";
        print_c::print_status(diagnostics, g_strdup("fixme: signals::onEditButton testing run\n"));
        run_c::thread_run(diagnostics, "ls -l");
    }

    static void
    onCloseButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCloseButton\n";
        GtkWidget *dialog = GTK_WIDGET(data);
        gtk_widget_hide(dialog);
        while (gtk_events_pending()) gtk_main_iteration();
        exit(1);
    }

    static void
    onCancelButton (GtkWidget * button, gpointer data) {
	std::cerr<<"fixme: signals::onCancelButton\n";
    }

    static void
    onFindButton (GtkWidget * button, gpointer data) {
	GtkTextView *diagnostics = GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(data), "diagnostics"));
	std::cerr<<"fixme: signals::onFindButton\n";
        print_c::print(diagnostics, g_strdup("1.fixme: signals::\n"));
        print_c::print(diagnostics, "tag/green", g_strdup("2.fixme: signals::\n"));
        print_c::print_debug(diagnostics, g_strdup("3.fixme: signals::\n"));
        print_c::print_error(diagnostics, g_strdup("4.fixme: signals::\n"));
        print_c::print_icon(diagnostics, "edit-find",  g_strdup("5.fixme: signals::\n"));
        print_c::print_icon_tag(diagnostics, "edit-find",  "tag/red",g_strdup("fixme: signals::\n"));
    }

};

}
#endif
