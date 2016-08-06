#include "view_c.hpp"
// Public:
view_c::view_c(void){
    init();
    
}


view_c::~view_c(void){
    pthread_mutex_destroy(population_mutex);
    g_free(population_mutex);
    pthread_cond_destroy(population_cond);
    g_free(population_cond);
    pthread_rwlock_destroy(population_lock);
    g_freepopulation_lock();
}


///////////////////////////// Private:
void
view_c::init(void){
    gint result[3];
    population_mutex = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
    population_cond = (pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
    population_lock = (pthread_rwlock_t *)calloc(1, sizeof(pthread_rwlock_t));
    if (!population_mutex || !population_cond || !population_lock) {
        cerr << "view_c::init(): calloc failed\n";
        if (!population_mutex) throw 1;
        if (!population_cond) throw 2;
        if (!population_lock) throw 3;
    }
    result[0] = pthread_mutex_init(population_mutex, NULL);
    result[1] = pthread_cond_init(population_cond, NULL);
    result[2] = pthread_rwlock_init(population_lock, NULL);


    if (result [0] | result [1] | result[2]){
        gint r = result[0]? result[0]: result[1]?result[1]:result[2];
        cerr << "view_c::init(): " << strerror(r) << "\n";
        if (result [0]) throw 4;
        if (result [1]) throw 5;
        if (result[2]) throw 6;
    }

    population_condition = 0;



}

