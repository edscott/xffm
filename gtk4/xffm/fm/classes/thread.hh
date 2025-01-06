#ifndef THREADCONTROL_HH
#define THREADCONTROL_HH
#undef DEBUG_THREADS
//#define DEBUG_THREADS 1

#ifdef DEBUG_THREADS
# define DBG_T(...)  {fprintf(stderr, "DBG_T> "); fprintf(stderr, __VA_ARGS__);}
#else
# define DBG_T(...)   { (void)0; }
#endif

#define THREADPOOL ((ThreadPool *)threadPoolObject)

      typedef struct threadInfo_t {
        void* (*function)(void*);
        void *data;
        pthread_t thread;
      } threadInfo_t;

namespace xf {

class ThreadPool {
    int maxThreads = 0;
    GList *waitList = NULL;  
    GList *threadPool = NULL;
    pthread_mutex_t threadPoolMutex = PTHREAD_MUTEX_INITIALIZER;
    //pthread_cond_t threadPoolCond = PTHREAD_COND_INITIALIZER;
public:
      ThreadPool(void){
        pthread_t threadLeader;
        pthread_create(&threadLeader, NULL, threadPoolRun_f, this);
        pthread_detach(threadLeader);
      }

  void lock(void){
    pthread_mutex_lock(&threadPoolMutex);
  }
  void unlock(void){
    pthread_mutex_unlock(&threadPoolMutex);
  }

  void clear(void){
    pthread_mutex_lock(&threadPoolMutex);
      for (auto l=threadPool; l && l->data; l=l->next){
        g_free(l->data);
      }
      g_list_free(threadPool);
      threadPool = NULL;
    pthread_mutex_unlock(&threadPoolMutex);
    
    //pthread_cond_wait(&threadPoolCond, &threadPoolMutex); //deadlocks
  }
       void getMaxThreads(void){
        if (maxThreads > 0) return;
        if (g_file_test("/proc/cpuinfo", G_FILE_TEST_EXISTS)){
          FILE *in = fopen("/proc/cpuinfo", "r");
          char buffer[256];
          while (fgets(buffer, 256, in) && ! feof(in)){
            if (strstr(buffer, "\n")) *strstr(buffer, "\n") = 0;
            if (strstr(buffer, "siblings")){
              auto v = g_strsplit(buffer, ":", -1);
              maxThreads = atoi(v[1]);
              g_strfreev(v);
              if (maxThreads > 64) maxThreads = 64;
              if (maxThreads < 4) maxThreads = 4;
              break;
            }
          }
          fclose(in);
        } else maxThreads = 8;

        maxThreads *= 2;
              TRACE("maxThreads set to %d\n", maxThreads);
      }

      void add(void* (*function)(void*), void *data){
#ifdef ENABLE_THREAD_POOL
        auto info = (threadInfo_t *)calloc(1, sizeof(threadInfo_t));
        info->function = function;
        info->data = data;
        pthread_mutex_lock(&threadPoolMutex);
        threadPool = g_list_append(threadPool, info);
        pthread_mutex_unlock(&threadPoolMutex);
#endif
      }



      int size(void){
        pthread_mutex_lock(&threadPoolMutex);
        auto sizePending = g_list_length(threadPool);
        auto sizeWaiting = g_list_length(waitList);
        pthread_mutex_unlock(&threadPoolMutex);
        return sizePending;
      }
private:
      static void *threadPoolRun_f(void *data){
        auto p = (ThreadPool *)data;
        p->threadPoolRun();
        return NULL;
      }
      void threadPoolRun(void){
#ifdef ENABLE_THREAD_POOL
        TRACE("Thread::threadPoolRun...\n");
        int active = 0;
        getMaxThreads();
        //pthread_t threads[maxThreads];
        threadInfo_t *info;
        
        int count=0;
        int lastCount = 0;
        while (1){
          if (active >= maxThreads) break;
          // Fireup initial thread group
          while (active < maxThreads){
            TRACE("Threadpool loop %d\n", count);
            pthread_mutex_lock(&threadPoolMutex);
            if (g_list_length(threadPool)) {
                auto first = g_list_first(threadPool);
                info = (threadInfo_t *)first->data;
                threadPool = g_list_remove(threadPool, info);
                waitList = g_list_append(waitList, info);

                pthread_create(&(info->thread), NULL, 
                    info->function, 
                    info->data);
                active++;
                TRACE("initial: thread %d spawned...\n", active);
                pthread_mutex_unlock(&threadPoolMutex);
            } else {
                pthread_mutex_unlock(&threadPoolMutex);
              break;
            }
          }
          // break sends here.
          // Wait for thread and fireup group element replacement.
          // Join maxThreads before spawning any more.
          // FIXME use non blocking join. Join any thread.
          while (active > 0){
            void *retval;
            auto first = g_list_first(waitList);
            auto info =(threadInfo_t *)first->data;
            pthread_join(info->thread, &retval);
            waitList = g_list_remove(waitList, info);
            g_free(info);

            TRACE("thread %d joined...\n", active);
            lastCount = count;
            count++;
            active--;
            pthread_mutex_lock(&threadPoolMutex);
            if (g_list_length(threadPool)) {
              // fire up another, if in queue.
              auto next = g_list_first(threadPool);
              info = (threadInfo_t *)next->data;
              threadPool = g_list_remove(threadPool, info);
              waitList = g_list_append(waitList, info);
              pthread_create(&(info->thread), NULL, 
                  info->function, 
                  info->data);
              active++;
              TRACE("final: thread %d spawned...\n", active);
              pthread_mutex_unlock(&threadPoolMutex);
           } else {
              pthread_mutex_unlock(&threadPoolMutex);
           }
           // active is cero here.
           //pthread_cond_signal(&threadPoolCond);

          }
          TRACE("sleep 250...\n");
          usleep(250);
          if (count != lastCount){
            TRACE("Threads processed so far: %d\n", count);
            lastCount = count;
          }
        }
        active = 0;
#else
        DBG("Threadpool is disabled\n");
#endif
        return;
      }
};
 
    pthread_mutex_t inc_dec_mutex=PTHREAD_MUTEX_INITIALIZER;
    gint thread_count = 0;
#ifdef DEBUG_THREADS        
    pthread_mutex_t reference_mutex=PTHREAD_MUTEX_INITIALIZER;
    GHashTable *thread_hash = NULL;
    GList *thread_list = NULL;
#endif

class Thread {

private:

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

    static void
    threadCount(gboolean increment, pthread_t *thread_p, const gchar *dbg_text ){
        // do this in a thread so gtk thread does not block
       inc_dec_view_ref(increment, thread_p, dbg_text );
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
        auto thread_p = (Thread *)data;
        void *retval;
        auto runThread = thread_p->runThread();
        pthread_join(*runThread, &retval);
        Thread::thread_unreference(runThread);
        delete(thread_p);
        return NULL;
    }


    static void
    inc_dec_view_ref(gboolean increment,pthread_t *thread, const gchar *dbg_text ){
        // do this in a thread so gtk thread does not block
        pthread_mutex_lock(&inc_dec_mutex);
        if (increment) {
            thread_count++;
            DBG_T("Thread count is %d added 0x%x (%s)\n", thread_count, GPOINTER_TO_INT(thread), dbg_text);
        } else {
            thread_count--;
            DBG_T("Thread count is %d removed 0x%x (%s)\n", thread_count, GPOINTER_TO_INT(thread), dbg_text);
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
	gchar *removed_text  = NULL;
#ifdef DEBUG_THREADS
        pthread_mutex_lock(&reference_mutex);
        if (thread_hash == NULL) {
            thread_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
        }
        if (!thread_hash) ERROR("threadcontrol.hh::thread_unreference: hash is null!\n");
        removed_text = g_strdup((const gchar *)g_hash_table_lookup(thread_hash, (void *)thread));
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
        gint condition;
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
	DBG_T("*** Thread::fileTest(%s)\n", path);
        if (!path || !g_path_is_absolute(path)) return FALSE;

	// Create data structure for sharing with thread.
        heartbeat_t *heartbeat_p = (heartbeat_t *)calloc(1,sizeof(heartbeat_t));
        if (!heartbeat_p) {
            ERROR("calloc heartbeat_p: %s\n",strerror(errno));
            return FALSE;
        }

        // Create mutex.
	heartbeat_p->mutex=(pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
        if (!heartbeat_p->mutex){
            ERROR("calloc heartbeat_p->mutex: %s\n",strerror(errno));
            g_free(heartbeat_p);
            return FALSE;
        }
        pthread_mutex_init(heartbeat_p->mutex, NULL);

        // Create signal.
        heartbeat_p->signal=(pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
        if (!heartbeat_p->signal){
            ERROR("malloc heartbeat_p->signal: %s\n",strerror(errno));
            pthread_mutex_destroy(heartbeat_p->mutex);
            g_free(heartbeat_p->mutex);
            g_free(heartbeat_p);
            return FALSE;
       }
        pthread_cond_init(heartbeat_p->signal,NULL);

	// Initialize data structure.
        heartbeat_p->condition = 0;
        heartbeat_p->path = g_strdup(path);
        heartbeat_p->test = test;

	// Lock data structure with mutex.
        pthread_mutex_lock(heartbeat_p->mutex);
        TRACE("Thread:: fileTest: Creating wait thread for heartbeat_g_file_test_with_timeout\n");
        
	// Create thread to perform the file test.
        gint r =
            pthread_create (&heartbeat_p->thread, NULL, heartBeatTest, (void *)heartbeat_p);
        
	// Test for success in creating thread.
	if (r) {
	    ERROR("Thread::fileTest():Cannot create thread (%s)\n", strerror(errno));
            pthread_mutex_unlock(heartbeat_p->mutex);
	    heartbeatCleanup(heartbeat_p);
	    return FALSE;
	}


	struct timespec tv;
	auto seconds = 2;
	memset(&tv, 0, sizeof (struct timespec));
	tv.tv_sec = time(NULL) + seconds;

	r =
	    pthread_cond_timedwait(heartbeat_p->signal, heartbeat_p->mutex, &tv);
	
	if (r){
	    ERROR("Thread::condTimedWait error in condition creation: %s\n", path);
            pthread_mutex_unlock(heartbeat_p->mutex);
	    heartbeatCleanup(heartbeat_p);
	    return FALSE;
	    
	}
        
	gboolean retval;
	switch (heartbeat_p->condition) {
	    case 0:
		ERROR("Thread::fileTest(%s): timeout.\n",heartbeat_p->path);
		retval = FALSE;
	       	break;
	    case 1:
		DBG_T("Thread::fileTest(%s): wait complete within time, result = FALSE.\n", heartbeat_p->path);
		retval = FALSE;
		break;
	    case 2:
		DBG_T("Thread::fileTest(%s): wait complete within time, result = TRUE.\n", heartbeat_p->path);
		retval = TRUE;
		break;
	}
        pthread_mutex_unlock(heartbeat_p->mutex);
	// Fire off a wait and cleanup thread.
	// nonjoinable just to pick up the zombie
	pthread_t wait_thread;
	pthread_create (&wait_thread, NULL, waitOnThread, (void *)heartbeat_p);
	pthread_detach(wait_thread);
	
        return (retval);

    }

    ////////////////////////////////////////////////////////////////////
private:

    static void
    heartbeatCleanup(heartbeat_t *heartbeat_p){
	pthread_mutex_destroy(heartbeat_p->mutex);
        pthread_cond_destroy(heartbeat_p->signal);
	g_free(heartbeat_p->signal);
	g_free(heartbeat_p->mutex);
	g_free(heartbeat_p->path);
	g_free(heartbeat_p);
    }

    static void *
    heartBeatTest(gpointer data){
	gboolean retval = FALSE;
        heartbeat_t *heartbeat_p = (heartbeat_t *)data;
	auto condition = 0; // timeout (FALSE)
        // This function call may block
        TRACE("heartbeat doing stat %s\n", heartbeat_p->path);
        struct stat st;
        if (lstat(heartbeat_p->path, &st) < 0) {
            DBG_T("threadcontrol.hh::heartBeatTest(): lstat %s (%s)\n", heartbeat_p->path, strerror(errno));
	    condition = 1; // failed (FALSE)
	    goto end;
        }
        
        // If test is not for symlink, and item is a symlink,
        // then follow the symlink for the test.
        if (S_ISLNK(st.st_mode)){
            if (heartbeat_p->test == G_FILE_TEST_IS_SYMLINK){
		condition = 2; // TRUE.
                goto end;
            }
            if (stat(heartbeat_p->path, &st) < 0) {
                DBG_T("threadcontrol.hh::heartBeatTest(): stat %s (%s)\n",
                    heartbeat_p->path, strerror(errno));
		    condition = 1; // failed (FALSE)
		    goto end;
                return NULL;
            }
        }

        switch (heartbeat_p->test){
            case G_FILE_TEST_EXISTS: retval = TRUE; break;
            case G_FILE_TEST_IS_REGULAR: retval = S_ISREG(st.st_mode); break;
            case G_FILE_TEST_IS_EXECUTABLE:  
                retval = ((st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) && S_ISREG(st.st_mode));
                break;
            case G_FILE_TEST_IS_SYMLINK: retval = S_ISLNK(st.st_mode); break;
            case G_FILE_TEST_IS_DIR: retval = S_ISDIR (st.st_mode); break;

        }
	if (retval) condition = 2;
	else condition = 1;
end:
	pthread_mutex_lock(heartbeat_p->mutex);
	heartbeat_p->condition = condition;
	pthread_mutex_unlock(heartbeat_p->mutex);	    
	pthread_cond_signal(heartbeat_p->signal);
	return NULL;

    }


    static 
    void *waitOnThread(gpointer data){
        heartbeat_t *heartbeat_p = (heartbeat_t *)data;
        void *value;
        pthread_join(heartbeat_p->thread, &value);
	heartbeatCleanup(heartbeat_p);
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
            ERROR("Thread::condTimedWait error in condition creation: %s\n", path);
        }
        return retval;
    }
};



}
#endif

