#ifndef RUN_BUTTON_C_HPP
#define RUN_BUTTON_C_HPP
#include "run_thread_control_c.hpp"

class run_button_c {
    public:
        run_button_c(void *, const gchar *, pid_t);
        ~run_button_c(void);
    protected:
    private:
        const gchar *rfm_shell(void);//FIXME
        pid_t controller;
        pid_t pid;
        pid_t grandchild;
        gchar *command;
        gchar *icon;
        GtkWidget *button;
        void *view_v;

};

#endif
