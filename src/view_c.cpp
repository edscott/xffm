#include "view_c.hpp"
// Public:
view_c::view_c(GtkWidget *notebook_p) : notebook_page_c(notebook_p){
    init();
    
}


view_c::~view_c(void){
    pthread_mutex_destroy(&population_mutex);
    pthread_cond_destroy(&population_cond);
    pthread_rwlock_destroy(&population_lock);
}


///////////////////////////// Private:
void
view_c::init(void){
    gint result;
    population_mutex = PTHREAD_MUTEX_INITIALIZER;
    population_cond = PTHREAD_COND_INITIALIZER;
    result = pthread_rwlock_init(&population_lock, NULL);

    if (result){
        cerr << "view_c::init(): " << strerror(result) << "\n";
        throw 1;
    }
    population_condition = 0;



}

