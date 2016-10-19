#ifndef THREAD_CONTROL_C_HPP
#define THREAD_CONTROL_C_HPP
#include "xffm+.h"
#include <pthread.h>


// This class is to avoid crashes when view_c objects are
// destroyed but threads are still active. All threads
// associated to views must be done before view can
// be destroyed.
//
// This is for background monitor functions,
// preview generations and other background
// functionality.
// 
#define DEBUG_THREADS 1

class thread_control_c{
    public:
        thread_control_c(void *);
        ~thread_control_c(void);
        gint thread_create(const gchar *, void *(*)(void *), void *, gboolean); 
        void thread_unreference(pthread_t *);
        gboolean file_test_with_wait(const gchar *, GFileTest);
    protected:
    private:
        gboolean cond_timed_wait(const gchar *, pthread_cond_t *, pthread_mutex_t *, gint);
        void *view_v;
        gint thread_count;
        void thread_reference(pthread_t *, const gchar *);
        pthread_mutex_t inc_dec_mutex;
        void inc_dec_view_ref(gboolean);
#ifdef DEBUG_THREADS        
	pthread_mutex_t reference_mutex;
        GHashTable *thread_hash;
        GList *thread_list;
#endif

};

#endif
