#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <pwd.h>
#include <gtk/gtk.h>
// FIXME: library namespace these... ptc_
#include "pthreadCalls.h"
void * 
print_f(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    print_c *print_p = (print_c *) arg[0];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *tag = (const gchar *)arg[1];
    const gchar *string = (const gchar *)arg[2];
    set_font_size (GTK_WIDGET(textview));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    if (print_p->trim_diagnostics(buffer)){
        // textview is at line limit.
    }

    GtkTextTag **tags = print_p->resolve_tags(buffer, tag);
    if(string && strlen (string)) {
        print_p->insert_string (buffer, string, tags);
    }
    g_free(tags);
    print_p->scroll_to_bottom();
    return NULL;
}

void *
print_i(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    GdkPixbuf *pixbuf = (GdkPixbuf *)arg[0];
    print_c *print_p = (print_c *) arg[1];
    GtkTextView *textview = print_p->get_diagnostics();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_insert_pixbuf (buffer, &end, pixbuf);
    return print_f(arg+1);
}
    

void *
print_d(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *d_arg[]={arg[0],(void *)"tag/italic", (void *)"DBG> "};
    print_f((void *)d_arg);
    return print_f(data);
}
    
    
void *
print_e(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg=(void **)data;
    void *e_arg1[]={arg[0],(void *)"tag/red", (void *)"*** "};
    print_f((void *)e_arg1);
    return print_f(data);
}


void *
print_s(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    print_c *print_p = (print_c *) arg[0];
    GtkTextView *textview = print_p->get_status();
    if (!GTK_IS_TEXT_VIEW(textview)) return GINT_TO_POINTER(-1);
    const gchar *string = (const gchar *)arg[1];

    set_font_size (GTK_WIDGET(textview));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    if(string && strlen (string)) {
        print_p->insert_string (buffer, string, NULL);
    }
    return NULL;
}

void *
print_sl(void *data){
    if (!data) return GINT_TO_POINTER(-1);
    void **arg = (void **)data;
    GtkLabel *status_label = (GtkLabel *)arg[0];
    const gchar *text = (const gchar *)arg[1];
    if (!GTK_IS_LABEL(status_label)) return GINT_TO_POINTER(-1);
    gtk_label_set_markup(status_label, text);
    return NULL;
}

# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <pwd.h>
#include <gtk/gtk.h>


void *
clear_text_buffer_f(void *data){
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    return NULL;
}

gboolean
context_function_f(gpointer data){
    void **arg = (void **)data;
    void * (*function)(gpointer) = (void* (*)(void*))arg[0];
    gpointer function_data = arg[1];
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
    pthread_cond_t *signal = (pthread_cond_t *)arg[3];
    void **result_p = (void **)arg[4];
    void *result = (*function)(function_data);
    pthread_mutex_lock(mutex);
    *result_p = result;
    pthread_cond_signal(signal);
    pthread_mutex_unlock(mutex);
    return FALSE;
}


