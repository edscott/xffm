#ifndef RUN_OUTPUT_C_HPP
#define RUN_OUTPUT_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"
#include "csh_completion_c.hpp"

class run_output_c: public csh_completion_c, virtual utility_c {
    public:
        run_output_c(void *);
        ~run_output_c(void);
        gchar *exit_string(gchar *);
    protected:
        gchar *start_string_argv(gchar **, pid_t);

        void push_hash(pid_t, gchar *);


    private:
        gchar *arg_string(char **arg);
        gchar *arg_string_format(char **);
        gchar *run_start_string(gchar *, pid_t, gboolean);
        gchar *pop_hash(pid_t);
        GHashTable *c_string_hash;
        pthread_mutex_t string_hash_mutex;
        gchar *start_string(gchar *, pid_t, gboolean);
};


#endif
