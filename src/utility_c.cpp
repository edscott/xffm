#include <gtk/gtk.h>
#include "utility_c.hpp"


////////////////////////////////////////////////////////////////////////////////////////////

utility_c::utility_c(void){
}

utility_c::~utility_c(void){
}

static void *
hide_text_f (void * data) {
    // to be executed by gtk thread
    GtkWidget * widget = (GtkWidget *)data;
    if(widget == NULL) return NULL;
    
    GtkWidget *vpane = (GtkWidget *)g_object_get_data(G_OBJECT(widget), "vpane");
    if(!vpane) return FALSE;
    gtk_paned_set_position (GTK_PANED (vpane), 10000);
        
    return NULL;
}

void
utility_c::hide_text (GtkWidget * widget) {
    if(widget == NULL) return;
    context_function(hide_text_f, (void *)widget);
}

static void *
clear_text_f (void *data) {
    // to be executed by gtk thread
    GtkWidget * widget = (GtkWidget *)data;
    GtkTextIter start,
      end;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW ((widget)));
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    if (widget==NULL) {
	// This is not applicable to diagnostics_window:
	hide_text_f (widget);
    }
    g_object_ref (G_OBJECT(buffer)); 
    gtk_text_view_set_buffer(GTK_TEXT_VIEW ((widget)), gtk_text_buffer_new(NULL));
    g_object_ref_sink (G_OBJECT(buffer));
    g_object_unref (G_OBJECT(buffer)); 
    return NULL;
}

void
utility_c::clear_text (GtkWidget * widget) {
    if(widget == NULL) return;
    context_function(clear_text_f, (void *)widget);
}


static gboolean
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

void *
utility_c::context_function(void * (*function)(gpointer), void * function_data){
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
    void *result=GINT_TO_POINTER(-1);
    void *arg[] = {
        (void *)function,
        (void *)function_data,
        (void *)&mutex,
        (void *)&signal,
        (void *)&result
    };
    gboolean owner = g_main_context_is_owner(g_main_context_default());
    if (owner){
	context_function_f(arg);
    } else {
	g_main_context_invoke(NULL, context_function_f, arg);
	pthread_mutex_lock(&mutex);
	if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
	pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&signal);
    return result;
}

gchar *
utility_c::utf_string (const gchar * t) {
    if(!t) { return g_strdup (""); }
    if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
    /* so we got a non-UTF-8 */
    /* but is it a valid locale string? */
    gchar *actual_tag;
    actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
    if(actual_tag)
        return actual_tag;
    /* So it is not even a valid locale string... 
     * Let us get valid utf-8 caracters then: */
    const gchar *p;
    actual_tag = g_strdup ("");
    for(p = t; *p; p++) {
        // short circuit end of string:
        gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
        if(g_utf8_validate (p, -1, NULL)) {
            gchar *qq = g_strconcat (actual_tag, p, NULL);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        } else if(r) {
            gchar *qq = g_strconcat (actual_tag, r, NULL);
            g_free (r);
            g_free (actual_tag);
            actual_tag = qq;
            break;
        }
        // convert caracter to utf-8 valid.
        gunichar gu = g_utf8_get_char_validated (p, 2);
        if(gu == (gunichar) - 1) {
            gu = g_utf8_get_char_validated ("?", -1);
        }
        gchar outbuf[8];
        memset (outbuf, 0, 8);
        gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
        if(outbuf_len < 0) {
            //DBG ("utility_c::utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
        }
        gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
        g_free (actual_tag);
        actual_tag = qq;
    }
    return actual_tag;
}

