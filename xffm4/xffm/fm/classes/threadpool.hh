#ifndef THREADPOOL_HH
#define THREADPOOL_HH
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
      new Thread("ThreadPool", threadPoolRun_f, (void *)this);
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
        TRACE("Threadpool is disabled\n");
#endif
        return;
      }
};

}
#endif

