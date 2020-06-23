#ifndef XF_PAGE_SIGNALS
#define XF_PAGE_SIGNALS

namespace xf{
template <class Type> class Page;
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
        page->showIconview(1);
    }

    static void toggleToTerminal(GtkButton *button, gpointer data){
        // This will toggle into terminal
        auto page = (Page<Type> *)data;
        page->showIconview(0);
    }
    static gboolean
    rangeChangeValue(GtkRange     *range,
               GtkScrollType scroll,
               gdouble       value,
               gpointer      data){
        // minimum and maximun font size in range:
        gint min=10;
        gint max=30;

        gint round = value + 0.5;
        if (value > max) round = max;
        if (value < min) round = min;
        
        TRACE("rangeChangeValue: %lf->%d\n", value, round);
        auto page = (Page<Type> *)data;
        print_c::set_font_size(GTK_WIDGET(page->output()), round);
        print_c::set_font_size(GTK_WIDGET(page->input()), round);
        //page->parent()->resizeWindow(round, min, max);
        return FALSE;
    }

    static gboolean
    rangeOff (GtkWidget *widget,
               GdkEvent  *event,
               gpointer   data){
        auto page = (Page<Type> *)data;
        TRACE("range off\n");
        auto size = page->fontSize();
        page->parent()->resizeWindow(size);
        Settings<Type>::setInteger("xfterm", "fontSize",size);
        // for all pages, change fontSize
        auto notebook = page->parent()->notebook();
        for (int i=0; i<gtk_notebook_get_n_pages (notebook); i++){
            auto page2 = page->parent()->currentPageObject(i);
            if ((void *)page2 == (void *)page) continue;
            page2->setSizeScale(size);
            print_c::set_font_size(GTK_WIDGET(page2->output()), size);
            print_c::set_font_size(GTK_WIDGET(page2->input()), size);
        }
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
            TRACE("leave_notify_event: data cannot be NULL\n");
            return FALSE;
        }
        TRACE("leave_notify_event\n");

        auto page = (Page<Type> *)data;
        auto dialog = page->parent();
        auto baseModel = (BaseModel<Type> *)g_object_get_data(G_OBJECT(page->topScrolledWindow()),"baseModel");
        if (baseModel) {
            BaseSignals<Type>::clear_highlights(baseModel);
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
