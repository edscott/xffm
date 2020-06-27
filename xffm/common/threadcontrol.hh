#ifndef THREADCONTROL_HH
#define THREADCONTROL_HH
#define DEBUG_THREADS 1

namespace xf {
    gint thread_count = 0;
    pthread_mutex_t inc_dec_mutex=PTHREAD_MUTEX_INITIALIZER;
#ifdef DEBUG_THREADS        
    pthread_mutex_t reference_mutex=PTHREAD_MUTEX_INITIALIZER;
    GHashTable *thread_hash = NULL;
    GList *thread_list = NULL;
#endif

template <class Type>
class Thread {

pthread_t *runThread_;
pthread_t *waitThread_;
gchar *dbg_text_;
public:
    Thread(const gchar *dbg_text, void *(*thread_f)(void *), void *data) {
        dbg_text_ = g_strdup(dbg_text);
        runThread_ = (pthread_t *)calloc(1, sizeof(pthread_t *));
        waitThread_ = (pthread_t *)calloc(1, sizeof(pthread_t *));
        if (thread_create(dbg_text, thread_f, data)){
            throw 1;
        }
    }
    
    ~Thread(void){
        g_free(runThread_);
        g_free(waitThread_);
        g_free(dbg_text_);
    }

private:

    pthread_t *
    runThread(void){ return this->runThread_;}

    pthread_t *
    waitThread(void){ return this->waitThread_;}

    gint
    thread_create(const gchar *dbg_text, void *(*thread_f)(void *), void *data)
    {
        TRACE("thread_create: %s\n", dbg_text);
        //auto thread = (pthread_t *)calloc(1,sizeof(pthread_t *));
        gint retval = pthread_create(runThread_, NULL, thread_f, data);
        if (retval){
            ERROR("threadcontrol.hh::thread_create(): %s\n", strerror(retval));
            return retval;
        }
        thread_reference(runThread_, dbg_text);
        pthread_create (waitThread_, NULL, wait_f, (void *)this);
        pthread_detach(*waitThread_);
        return 0;
    }
    
    static void *
    wait_f(void *data){
        auto thread_p = (Thread<Type> *)data;
        void *retval;
        auto runThread = thread_p->runThread();
        pthread_join(*runThread, &retval);
        Thread<Type>::thread_unreference(runThread);
        delete(thread_p);
        return NULL;
    }


    static void
    inc_dec_view_ref(gboolean increment,pthread_t *thread, const gchar *dbg_text ){
        // do this in a thread so gtk thread does not block
        pthread_mutex_lock(&inc_dec_mutex);
        if (increment) {
            thread_count++;
            DBG("Thread count is %d added 0x%x (%s)\n", thread_count, GPOINTER_TO_INT(thread), dbg_text);
        } else {
            thread_count--;
            DBG("Thread count is %d removed 0x%x (%s)\n", thread_count, GPOINTER_TO_INT(thread), dbg_text);
        }
        pthread_mutex_unlock(&inc_dec_mutex);
    }

    static void
    thread_reference(pthread_t *thread, const gchar *dbg_text){
        if (thread) inc_dec_view_ref(TRUE, thread, dbg_text);
        TRACE("Thread count: %d 0x%x (%s)\n", thread_count, GPOINTER_TO_INT(thread), dbg_text);

#ifdef DEBUG_THREADS
        if (thread_hash == NULL) {
            thread_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal,NULL, g_free);
        }
        if (!thread_hash) ERROR("threadcontrol.hh::thread_reference: hash is null!\n");

        pthread_mutex_lock(&reference_mutex);
        g_hash_table_replace(thread_hash, thread, 
                (dbg_text)?
                (void *)g_strdup(dbg_text):(void *)g_strdup("no debug text"));
        pthread_mutex_unlock(&reference_mutex);
#endif
    }

    static void
    thread_unreference(pthread_t *thread){
#ifdef DEBUG_THREADS
        pthread_mutex_lock(&reference_mutex);
        if (thread_hash == NULL) {
            thread_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
        }
        if (!thread_hash) ERROR("threadcontrol.hh::thread_unreference: hash is null!\n");
        gchar *removed_text = g_strdup((const gchar *)g_hash_table_lookup(thread_hash, (void *)thread));
        TRACE( "Thread count %d: 0x%x (%s)\n", 
                g_list_length(thread_list)-1,
                GPOINTER_TO_INT(thread), removed_text);
            
        g_hash_table_remove(thread_hash, thread);
        pthread_mutex_unlock(&reference_mutex);
#endif
        if (thread) inc_dec_view_ref(FALSE, thread, removed_text);
        g_free(removed_text);
        //no more... g_free(thread);
    }
    


    
    typedef struct heartbeat_t{
        gboolean condition;
        pthread_mutex_t *mutex;
        pthread_cond_t *signal;
        pthread_t thread;
        gchar *path;
        GFileTest test;
    } heartbeat_t;

 
public:
    // g_file_test_with_timeout
    // This should only be for network items.
    static gboolean
    fileTest(const gchar *path, GFileTest test){
        if (!path) return FALSE;
	DBG("*** Thread::fileTest(%s)\n", path);
	

        if (!g_path_is_absolute(path)) return FALSE;
        TRACE( "Thread::fileTest(): %s\n", path);

        heartbeat_t *heartbeat_p = (heartbeat_t *)calloc(1,sizeof(heartbeat_t));
        if (!heartbeat_p) {
            g_warning("calloc heartbeat_p: %s\n",strerror(errno));
            return FALSE;
        }

        heartbeat_p->mutex=(pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        if (!heartbeat_p->mutex){
            g_warning("calloc heartbeat_p->mutex: %s\n",strerror(errno));
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
        TRACE("Thread:: fileTest: Creating wait thread for heartbeat_g_file_test_with_timeout\n");
        // This thread does not affect view nor window.
        gint r =
            pthread_create (&heartbeat_p->thread, NULL, heartBeatTest, (void *)heartbeat_p);
        
        if (r==0 && !heartbeat_p->condition) {
            if (!condTimedWait(path, heartbeat_p->signal, heartbeat_p->mutex, 2)) {
                pthread_mutex_unlock(heartbeat_p->mutex);
                ERROR("Thread::fileTest(%s): dead heartbeat, aborting\n",heartbeat_p->path);
                // Dead heartbeat:
                // Fire off a wait and cleanup thread.
                // nonjoinable just to pick up the zombie
                pthread_t wait_thread;
                pthread_create (&wait_thread, NULL, waitOnThread, (void *)heartbeat_p);
                pthread_detach(wait_thread);
                return FALSE;
            }
            DBG("Thread::fileTest(): wait complete within time.\n");
        }
        pthread_mutex_unlock(heartbeat_p->mutex);
        return (TRUE);

    }

    ////////////////////////////////////////////////////////////////////
private:

    static void *
    heartBeatTest(gpointer data){
        heartbeat_t *heartbeat_p = (heartbeat_t *)data;

        // This function call may block
        TRACE("heartbeat doing stat %s\n", heartbeat_p->path);
        struct stat st;
        errno=0;
        if (lstat(heartbeat_p->path, &st) < 0) {
            DBG("threadcontrol.hh::heartBeatTest(): lstat %s (%s)\n",
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
                DBG("threadcontrol.hh::heartBeatTest(): lstat %s (%s)\n",
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
    void *waitOnThread(gpointer data){
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

    static gboolean 
    condTimedWait(const gchar *path, pthread_cond_t *signal, pthread_mutex_t *mutex, gint seconds){
        struct timespec tv;
        memset(&tv, 0, sizeof (struct timespec));
        tv.tv_sec = time(NULL) + seconds;

        gint r = pthread_cond_timedwait(signal, mutex, &tv);
        
        gboolean retval = TRUE;
        if (r){
            retval = FALSE;
            TRACE("Thread::condTimedWait returning FALSE for: %s\n", path);
        }
        return retval;
    }
};



}
#endif

