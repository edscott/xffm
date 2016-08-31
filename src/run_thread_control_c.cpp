
#include "run_thread_control_c.hpp"

run_thread_control_c::run_thread_control_c(void * data): run_output_c(data){
}

#if 0

static gint thread_count = 0;

static void
inc_dec_view_ref(view_t *view_p, gboolean increment){
    RfmRWLock *lock;
    rfm_global_t *rfm_global_p = rfm_global();
    if (view_p) {
	lock = &(view_p->mutexes.view_lock);
    } else {
	if (!rfm_global_p) {
	    TRACE("increment/decrement view_ref(): invalid view_p (rfm_global_p==NULL)\n");
	    return; 
	}
	NOOP( "*+*+ global window lock\n");
	lock = &(rfm_global_p->window_lock);
    } 
    // XXX since this is main thread business (as well
    //     as child thread), reader lock may block...
    //     but this is very rare condition.
    if (increment){
	if (rfm_get_gtk_thread() == g_thread_self()){
	    while (!rfm_rw_lock_reader_trylock(lock)) gtk_main_iteration();
	} else 	rfm_rw_lock_reader_lock(lock);
    }
    else rfm_rw_lock_reader_unlock(lock);
    static pthread_mutex_t inc_dec_mutex=PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&inc_dec_mutex);
    if (increment) {
	if (view_p) view_p->flags.thread_count++;
	thread_count++;
    } else {
 	if (view_p) view_p->flags.thread_count--;
	thread_count--;
    }
    gint view_count = 1;
    if (view_p) view_count = view_p->flags.thread_count;
    if (thread_count == 0 || view_count == 0){
	// Janitor signal (This wakes up the janitor).
	TRACE("thread unreference sending janitor the signal. (%d,%d)\n",
		thread_count, view_count);
	g_cond_signal(rfm_global_p->janitor_signal);
    }
    pthread_mutex_unlock(&inc_dec_mutex);
}

static void *
wait_f(gpointer data){
    void **argv =data;
    view_t *view_p = argv[0];
    GThread *thread = argv[1];
    g_free(argv);
    g_thread_join(thread);
    rfm_thread_unreference(view_p, thread);
    return NULL;
}

#ifdef DEBUG_TRACE
#define DEBUG_THREADS
#endif

#ifdef DEBUG_THREADS
static GSList *view_ref_list = NULL;
static pthread_mutex_t *
get_view_ref_mutex(void){
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return &mutex;
}

void
rfm_thread_unreference_quiet(view_t *view_p, GThread *thread){
    if (thread) inc_dec_view_ref(view_p, FALSE);
}
#else
void
rfm_thread_unreference_quiet(view_t *view_p, GThread *thread){
    return rfm_thread_unreference(view_p, thread);
}
#endif


void
rfm_thread_unreference(view_t *view_p, GThread *thread){
    if (thread) inc_dec_view_ref(view_p, FALSE);
#ifdef DEBUG_THREADS
    pthread_mutex_t *view_ref_mutex=get_view_ref_mutex();
    pthread_mutex_lock(view_ref_mutex);
    const gchar *removed_text = get_thread_text(thread);

    TRACE( "- view decrement: 0x%x (%s) view ref = %d\n", 
	    GPOINTER_TO_INT(thread), removed_text,
	    g_slist_length(view_ref_list)-1);
	
    view_ref_list = g_slist_remove(view_ref_list, thread);
    g_hash_table_remove(thread_hash, thread);

#ifdef DEBUG_TRACE
    gchar *trace_text = NULL;
    
    GSList *t = view_ref_list;
    if (g_slist_length(view_ref_list)) for(;t && t->data; t=t->next){
	gchar *dbg_text = g_hash_table_lookup(thread_hash, t->data);
	if (dbg_text) {
	    gchar *g =
		g_strdup_printf("%s 0x%x (%s)", 
			trace_text?trace_text:"- decrement pending:",
			GPOINTER_TO_INT(t->data), dbg_text);
	    g_free(trace_text);
	    trace_text = g;
	}
    } else {
	trace_text = g_strdup("- view decrement: no threads pending");
    }
    if (trace_text) {
	TRACE( "%s (%d)\n", trace_text, g_slist_length(view_ref_list));
	g_free(trace_text);
    }
#endif
    pthread_mutex_unlock(view_ref_mutex);

#endif
}

void
rfm_thread_reference(view_t *view_p, GThread *thread, const gchar *dbg_text){
	if (thread) inc_dec_view_ref(view_p, TRUE);
#ifdef DEBUG_THREADS
	if (dbg_text){
	    TRACE( "- view increment: 0x%x:0x%x (%s) view ref = %d\n",
		    GPOINTER_TO_INT(view_p), GPOINTER_TO_INT(thread), dbg_text, g_slist_length(view_ref_list)+1);
	    pthread_mutex_t *view_ref_mutex=get_view_ref_mutex();
	    pthread_mutex_lock(view_ref_mutex);
	    if (!thread_hash){
		thread_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
	    }
	    view_ref_list = g_slist_append(view_ref_list, thread);
	    g_hash_table_replace(thread_hash, thread, (void *)dbg_text);
	    pthread_mutex_unlock(view_ref_mutex);
	}
#endif
}

// If view is NULL, the lock is on the window, not any particular view.
GThread *
rfm_view_thread_create(
	view_t *view_p,
	gpointer(*thread_f)(gpointer), gpointer data, 
	const gchar *dbg_text){
    rfm_global_t *rfm_global_p = rfm_global();

    if (rfm_global_p){
        g_mutex_lock(rfm_global_p->status_mutex);
        gint status = rfm_global_p->status;
        g_mutex_unlock(rfm_global_p->status_mutex);
	if (status == STATUS_EXIT) {
	    DBG("rfm_view_thread_create: rfm_global_p->status == STATUS_EXIT\n");
	    return NULL;
	}
    }
    
    if (view_p){
        TRACE("rfm_view_thread_create()...\n");
        if (!rfm_view_list_lock(view_p, "rfm_view_thread_create")) {
            return NULL;
        }
	gint status = view_p->flags.status;
	if (status == STATUS_EXIT) {
            rfm_view_list_unlock("rfm_view_thread_create");
	    return NULL;	
	}
    }

#ifdef DEBUG_TRACE
    if (dbg_text){
	TRACE("Thread: %s\n", dbg_text);
    }
#endif
    GThread *thread = rfm_thread_create (dbg_text, thread_f, data, TRUE);
    if (thread){
	rfm_thread_reference(view_p, thread, dbg_text);
	void **arg = (void **)malloc(2*sizeof(void *));
	arg[0] = view_p;
	arg[1] = thread;
	rfm_thread_create ("wait_f", wait_f, arg, FALSE);
    }
    if (view_p) rfm_view_list_unlock("rfm_view_thread_create");
    return thread;
}

#endif
