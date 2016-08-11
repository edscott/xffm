#ifndef SIGNALS_C_HPP
#define SIGNALS_C_HPP

class signals_c {
    public:
 	void 
            setup_callback(void *object, GtkWidget *widget, 
	        const gchar *signal, void *callback, void *data);
 	void 
            setup_event_callback(void *object, GtkWidget *widget, 
	        const gchar *signal, void *callback, void *data);
       
};


#endif
