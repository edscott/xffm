#ifndef UTILITY_C_HPP
#define UTILITY_C_HPP
#include <gtk/gtk.h>
#include <pthread.h>

class utility_c{
    public:
        utility_c(void);
        ~utility_c(void);
        void clear_text (GtkWidget *);
        void hide_text (GtkWidget *);
    protected:
        void *context_function(void * (*function)(gpointer), void * function_data);
    private:


};

#endif
