#ifndef XF_PAGE_SIGNALS
#define XF_PAGE_SIGNALS

namespace xf{
template <class Type> class Page;
template <class Type> class Dialog;
template <class Type>
class PageSignals{
    using print_c = Print<double>;
public:
    static void scriptRun(GtkButton *button, gpointer data){
        auto page = (Page<Type> *)data;
        page->scriptRun();
    }


    static void clearText(GtkButton *button, gpointer data){
        auto page = (Page<Type> *)data;
        print_c::clear_text(page->output());
        //page->showIconview(TRUE, TRUE);
    }

    static void toggleToIconview(GtkButton *button, gpointer data){
        // This will toggle into iconview
        auto page = (Page<Type> *)data;
        page->showIconview(TRUE);
    }

    static void toggleToTerminal(GtkButton *button, gpointer data){
        // This will toggle into terminal
        auto page = (Page<Type> *)data;
        page->showIconview(FALSE);
    }
    static gboolean
    rangeChangeValue(GtkRange     *range,
               GtkScrollType scroll,
               gdouble       value,
               gpointer      data){
        gint round = value + 0.5;
        if (value > 24) round = 24;
        
        TRACE("rangeChangeValue: %lf->%d\n", value, round);
        auto page = (Page<Type> *)data;
        print_c::set_font_size(GTK_WIDGET(page->output()), round);
        print_c::set_font_size(GTK_WIDGET(page->input()), round);
        return FALSE;
    }

#ifdef XFFM_CC
        // This would be for topScrolledWindow
	// and here we eliminate any lingering tooltip window
	// (once we enable tooltip
    static void
    setWindowToolTip(GtkWindow *data, const gchar *data2){
/*
 * FIXME
	//if (data2) tooltip_path_string = g_strdup(data2);
	//set_tooltip_path_string(data2);
	auto toolTipWindow = GTK_WINDOW(data);
	gtk_widget_set_tooltip_window (GTK_WIDGET(window), toolTipWindow);
	if (toolTipWindow && G_IS_OBJECT (toolTipWindow)) {
	    //g_object_set_data(G_OBJECT(toolTipWindow), "tooltip_target", (void *)window);
	}
	*/
	return;
    }

    static gboolean
    leave_notify_event (GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data){
        if (!data) {
            DBG("leave_notify_event: data cannot be NULL\n");
            return FALSE;
        }
	DBG("leave_notify_event\n");

	auto page = (Page<Type> *)data;
        auto dialog = page->parent();
	auto baseView = (BaseView<Type> *)g_object_get_data(G_OBJECT(page->top_scrolled_window()),"baseView");
	if (baseView) {
	    baseView->clear_highlights();
	}
	//FIXME:
        //if (!view_p->all_set_up) return FALSE;

        //view_p->get_xfdir_p()->clear_highlights();
        //setWindowToolTip(NULL, NULL);
        return FALSE;
    }
#endif

};

}



#endif
