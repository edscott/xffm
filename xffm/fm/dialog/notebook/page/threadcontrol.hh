#ifndef THREADCONTROL_HH
#define THREADCONTROL_HH
#define DEBUG_THREADS 1
//FIXME: include these into class template
namespace xf {
template <class Type>
class ThreadControl {
    
    typedef struct wait_t{
	ThreadControl *thread_control_p;
	pthread_t thread;
    }wait_t;

    typedef struct heartbeat_t{
	gboolean condition;
	pthread_mutex_t *mutex;
	pthread_cond_t *signal;
	pthread_t thread;
	gchar *path;
	GFileTest test;
    } heartbeat_t;

private:
    gint thread_count;
    pthread_mutex_t inc_dec_mutex;
#ifdef DEBUG_THREADS        
    pthread_mutex_t reference_mutex;
    GHashTable *thread_hash;
    GList *thread_list;
#endif



public:

    ThreadControl(void){
	thread_count = 0;
	inc_dec_mutex=PTHREAD_MUTEX_INITIALIZER;
#ifdef DEBUG_THREADS            
	reference_mutex = PTHREAD_MUTEX_INITIALIZER;
	thread_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
	thread_list = NULL;
#endif
    }

    ~ThreadControl(void){
	// FIXME: this should run in detached thread and only complete
	//        when all threads are destroyed.
	while (thread_count){
	    TRACE("~ThreadControl: %p waiting for %d threads\n", (void *)this, thread_count);
	    sleep(1);
	}
	pthread_mutex_destroy(&inc_dec_mutex);
#ifdef DEBUG_THREADS            
	pthread_mutex_destroy(&reference_mutex);
	g_hash_table_destroy(thread_hash);
	g_list_free(thread_list);
#endif
    }


    gint
    thread_create(const gchar *dbg_text, void *(*thread_f)(void *), void *data, gboolean joinable)
    {
	TRACE("thread_create: %s\n", dbg_text);
	pthread_t thread;
	gint retval = pthread_create(&thread, NULL, thread_f, data);
	if (retval){
	    ERROR("threadcontrol.hh::thread_create(): %s\n", strerror(retval));
	    return retval;
	}
	if (joinable){
	    pthread_t wait_thread; 
	    wait_t *wait_p = (wait_t *)calloc(1, sizeof(wait_t));
	    wait_p->thread_control_p = this;
	    wait_p->thread = thread;
	    thread_reference(&(wait_p->thread), dbg_text);
	    
	    pthread_create (&wait_thread, NULL, wait_f, (void *)wait_p);
	    pthread_detach(wait_thread);
	    return 0;
	} 
	pthread_detach(thread);
	return 0;
    }

    void
    thread_unreference(pthread_t *thread){
	if (thread) inc_dec_view_ref(FALSE);
#ifdef DEBUG_THREADS
	pthread_mutex_lock(&reference_mutex);
	if (!thread_hash) ERROR("threadcontrol.hh::thread_unreference: hash is null!\n");
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
	//no more... g_free(thread);
    }
    
    // g_file_test_with_timeout
    gboolean
    file_test_with_wait(const gchar *path, GFileTest test){
	if (!path) return FALSE;
	if (!g_path_is_absolute(path)) return FALSE;
	TRACE( "file_test_with_wait(): %s\n", path);

	heartbeat_t *heartbeat_p = (heartbeat_t *)calloc(1,sizeof(heartbeat_t));
	if (!heartbeat_p) {
	    g_warning("malloc heartbeat_p: %s\n",strerror(errno));
	    return FALSE;
	}

	heartbeat_p->mutex=(pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
	if (!heartbeat_p->mutex){
	    g_warning("malloc heartbeat_p->mutex: %s\n",strerror(errno));
	    g_free(heartbeat_p);
	    return FALSE;
       }
	pthread_mutex_init(heartbeat_p->mutex, NULL);
	
	heartbeat_p->signal=(pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
	if (!heartbeat_p->signal){
	    g_warning("malloc heartbeat_p->signal: %s\n",strerror(errno));
	    pthread_mutex_destroy(heartbeat_p->mutex);
	    g_free(heartbeat_p->mutex);
	    g_free(heartbeat_p);
	    return FALSE;
       }
	pthread_cond_init(heartbeat_p->signal,NULL);

	heartbeat_p->condition = 0;
	heartbeat_p->path = g_strdup(path);
	heartbeat_p->test = test;

	pthread_mutex_lock(heartbeat_p->mutex);
	TRACE("Creating wait thread for heartbeat_g_file_test_with_timeout\n");
	// This thread does not affect view nor window.
	gint r =
	    pthread_create (&heartbeat_p->thread, NULL, heartbeat_g_file_test, (void *)heartbeat_p);
	
	if (r==0 && !heartbeat_p->condition) {
	    if (!cond_timed_wait(path, heartbeat_p->signal, heartbeat_p->mutex, 2)) {
		pthread_mutex_unlock(heartbeat_p->mutex);
		ERROR("threadcontrol.hh::file_test_with_wait(): dead heartbeat, aborting\n");
		// Dead heartbeat:
		// Fire off a wait and cleanup thread.
		// nonjoinable just to pick up zombie
		pthread_t wait_thread;
		pthread_create (&wait_thread, NULL, wait_on_thread, (void *)heartbeat_p);
		pthread_detach(wait_thread);
		return FALSE;
	    }
	    DBG("file_test_with_wait(): wait complete within time.\n");
	}
	pthread_mutex_unlock(heartbeat_p->mutex);
	return (TRUE);

    }

    ////////////////////////////////////////////////////////////////////


    static void *
    wait_f(void *data){
	wait_t *wait_p =(wait_t *)data;
	void *retval;
	pthread_join(wait_p->thread, &retval);
	wait_p->thread_control_p->thread_unreference(&(wait_p->thread));
	g_free(wait_p);
	return NULL;
    }

    static void *
    heartbeat_g_file_test(gpointer data){
	heartbeat_t *heartbeat_p = (heartbeat_t *)data;

	// This function call may block
	TRACE("heartbeat doing stat %s\n", heartbeat_p->path);
	struct stat st;
	errno=0;
	if (lstat(heartbeat_p->path, &st) < 0) {
	    DBG("threadcontrol.hh::heartbeat_g_file_test(): lstat %s (%s)\n",
		heartbeat_p->path, strerror(errno));
	    errno=0;
	    return NULL;
	}
	
	// If test is not for symlink, and item is a symlink,
	// then follow the symlink for the test.
	if (S_ISLNK(st.st_mode)){
	    if (heartbeat_p->test == G_FILE_TEST_IS_SYMLINK){
		return GINT_TO_POINTER(TRUE);
	    }
	    errno=0;
	    if (stat(heartbeat_p->path, &st) < 0) {
		DBG("threadcontrol.hh::heartbeat_g_file_test(): lstat %s (%s)\n",
		    heartbeat_p->path, strerror(errno));
		errno=0;
		return NULL;
	    }
	}

	gboolean retval = FALSE;
	switch (heartbeat_p->test){
	    case G_FILE_TEST_EXISTS: retval = TRUE; break;
	    case G_FILE_TEST_IS_REGULAR: retval = S_ISREG(st.st_mode); break;
	    case G_FILE_TEST_IS_EXECUTABLE:  
		retval = ((st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) && S_ISREG(st.st_mode));
		break;
	    case G_FILE_TEST_IS_SYMLINK: retval = S_ISLNK(st.st_mode); break;
	    case G_FILE_TEST_IS_DIR: retval = S_ISDIR (st.st_mode); break;

	}
	
	pthread_mutex_lock(heartbeat_p->mutex);
	heartbeat_p->condition = TRUE;
	pthread_mutex_unlock(heartbeat_p->mutex);
	TRACE("heartbeat signal %d\n", retval);
	pthread_cond_signal(heartbeat_p->signal);
	return GINT_TO_POINTER(retval);

    }

    static 
    void *wait_on_thread(gpointer data){
	heartbeat_t *heartbeat_p = (heartbeat_t *)data;
	void *value;
	pthread_join(heartbeat_p->thread, &value);

	pthread_mutex_destroy(heartbeat_p->mutex);
	g_free(heartbeat_p->mutex);
	pthread_cond_destroy(heartbeat_p->signal);
	g_free(heartbeat_p->signal);

	g_free (heartbeat_p->path);
	g_free (heartbeat_p);
	return value;
    }


private:

    gboolean 
    cond_timed_wait(const gchar *path, pthread_cond_t *signal, pthread_mutex_t *mutex, gint seconds){
	struct timespec tv;
	memset(&tv, 0, sizeof (struct timespec));
	tv.tv_sec = time(NULL) + seconds;

	gint r = pthread_cond_timedwait(signal, mutex, &tv);
	
	gboolean retval = TRUE;
	if (r){
	    retval = FALSE;
	    g_warning("%s: %s\n", path, strerror(errno));
	}
	return retval;
    }


    void
    inc_dec_view_ref(gboolean increment){
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
    thread_reference(pthread_t *thread, const gchar *dbg_text){
	if (thread) inc_dec_view_ref(TRUE);

#ifdef DEBUG_THREADS
	if (!thread_hash) ERROR("threadcontrol.hh::thread_reference: hash is null!\n");
	TRACE( "- view increment: 0x%x:0x%x (%s) view ref = %d\n",
		GPOINTER_TO_INT(this), GPOINTER_TO_INT(thread), 
		dbg_text, g_list_length(thread_list)+1);

	pthread_mutex_lock(&reference_mutex);
	thread_list = g_list_prepend(thread_list, thread);
	g_hash_table_replace(thread_hash, thread, (dbg_text)?(void *)dbg_text:(void *)"no debug text");
	pthread_mutex_unlock(&reference_mutex);
#endif
    }

};
}
#endif

