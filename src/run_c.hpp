#ifndef RUN_C_HPP
#define RUN_C_HPP
#include "xffm+.h"
#include "run_output_c.hpp"
#include "csh_completion_c.hpp"
class run_c: public run_output_c {
    public:
        run_c(void *);
    protected:
        GPid thread_run(const gchar *command);
        GPid thread_run(const gchar **arguments);
    private:
 
};



#endif
