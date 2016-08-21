#ifndef UTILITY_C_HPP
#define UTILITY_C_HPP
#include "xffm+.h"

#include <pthread.h>
#include <string.h>

class utility_c{
    public:
        utility_c(void);
        ~utility_c(void);

        void *context_function(void * (*function)(gpointer), void * function_data);
        gchar *utf_string (const gchar *);
	const gchar *chop_excess (gchar *);
    protected:
    private:


};

#endif
