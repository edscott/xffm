//#define DEBUG_TRACE
#include "xffm+.h"
#include "thread_control_c.hpp"

#include "view_c.hpp"

thread_control_c::thread_control_c(void * data){
    view_v = data;
    thread_count = 0;
    inc_dec_mutex=PTHREAD_MUTEX_INITIALIZER;
#ifdef DEBUG_THREADS            
    reference_mutex = PTHREAD_MUTEX_INITIALIZER;
    thread_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
    thread_list = NULL;
#endif
}

thread_control_c::~thread_control_c(void){
    // FIXME: this should run in detached thread and only complete
    //        when all threads are destroyed.
    while (thread_count){
        TRACE("thread_control_c::~thread_control_c: %p waiting for %d threads\n", (void *)this, thread_count);
        sleep(1);
    }
    pthread_mutex_destroy(&inc_dec_mutex);
#ifdef DEBUG_THREADS            
    pthread_mutex_destroy(&reference_mutex);
    g_hash_table_destroy(thread_hash);
    g_list_free(thread_list);
#endif
}


void
thread_control_c::inc_dec_view_ref(gboolean increment){
    // do this in a thread so gtk thread does not block
    pthread_mutex_lock(&inc_dec_mutex);
    if (increment) {
	thread_count++;
    } else {
	thread_count--;
    }
    pthread_mutex_unlock(&inc_dec_mutex);
}


void
thread_control_c::thread_unreference(pthread_t *thread){
    if (thread) inc_dec_view_ref(FALSE);
#ifdef DEBUG_THREADS
    pthread_mutex_lock(&reference_mutex);
    if (!thread_hash) DBG("thread_control_c::thread_unreference: hash is null!\n");
    const gchar *removed_text = (const gchar *)g_hash_table_lookup(thread_hash, (void *)thread);

    TRACE( "- view decrement: 0x%x (%s) view ref = %d\n", 
	    GPOINTER_TO_INT(thread), removed_text,
	    g_list_length(thread_list)-1);
	
    thread_list = g_list_remove(thread_list, thread);
    g_hash_table_remove(thread_hash, thread);

#ifdef DEBUG_TRACE
    gchar *trace_text = NULL;
    
    GList *t = thread_list;
    if (g_list_length(thread_list)) for(;t && t->data; t=t->next){
	const gchar *dbg_text = (const gchar *)g_hash_table_lookup(thread_hash, t->data);
        gchar *g = g_strdup_printf("%s 0x%x (%s)", 
                    trace_text?trace_text:"- decrement pending:",
                    GPOINTER_TO_INT(t->data), dbg_text);
        g_free(trace_text);
        trace_text = g;
    } else {
	trace_text = g_strdup("- view decrement: no threads pending");
    }
    if (trace_text) {
	TRACE( "%s (%d)\n", trace_text, g_list_length(thread_list));
	g_free(trace_text);
    }
#endif
    pthread_mutex_unlock(&reference_mutex);

#endif
    g_free(thread);
}

void
thread_control_c::thread_reference(pthread_t *thread, const gchar *dbg_text){
    if (thread) inc_dec_view_ref(TRUE);

#ifdef DEBUG_THREADS
    if (!thread_hash) DBG("thread_control_c::thread_reference: hash is null!\n");
    TRACE( "- view increment: 0x%x:0x%x (%s) view ref = %d\n",
            GPOINTER_TO_INT(this), GPOINTER_TO_INT(thread), 
            dbg_text, g_list_length(thread_list)+1);

    pthread_mutex_lock(&reference_mutex);
    thread_list = g_list_prepend(thread_list, thread);
    g_hash_table_replace(thread_hash, thread, (dbg_text)?(void *)dbg_text:(void *)"no debug text");
    pthread_mutex_unlock(&reference_mutex);
#endif
}

typedef struct wait_t{
    thread_control_c *thread_control_p;
    pthread_t *thread;
}wait_t;

static void *
wait_f(void *data){
    wait_t *wait_p =(wait_t *)data;
    void *retval;
    pthread_join(*(wait_p->thread), &retval);
    wait_p->thread_control_p->thread_unreference(wait_p->thread);
    g_slice_free(wait_t, wait_p);
    return NULL;
}


// If view is NULL, the lock is on the window, not any particular view.
pthread_t *
thread_control_c::thread_create(const gchar *dbg_text, void *(*thread_f)(void *), void *data, gboolean joinable)
{

    TRACE("thread_control_c::thread_create: %s\n", dbg_text);

    pthread_t *thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    if (!thread) return NULL;

// FIXME: memleak of thread and wait_thread???
    gint retval = pthread_create(thread, NULL, thread_f, data);
    thread_reference(thread, dbg_text);
    if (retval){
        g_warning("thread_control_c::thread_create(): %s\n", strerror(retval));
        g_free(thread);
        return NULL;
    }
    if (joinable){
//        pthread_t *wait_thread = (pthread_t *)calloc(1, sizeof(pthread_t));  // leak?
        pthread_t wait_thread; 
        wait_t *wait_p = g_slice_new(wait_t);
        wait_p->thread_control_p = this;
        wait_p->thread = thread;
        
	pthread_create (&wait_thread, NULL, wait_f, (void *)wait_p);
	//thread_reference(wait_thread, "wait_thread");
        pthread_detach(wait_thread);
    } else {
        pthread_detach(*thread);
	thread_unreference(thread); // this frees
    }

    return thread;
}

