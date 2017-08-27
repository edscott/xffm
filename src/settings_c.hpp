#ifndef SETTINGS_C_HPP
#define SETTINGS_C_HPP

#include "xffm+.h"
#include "gtk_c.hpp"

class  settings_c: public environment {
    public:
        settings_c(data_c *);
        gboolean run_settings_dialog(void);
        gboolean mcs_set_var (const gchar *, const gchar *);
        gboolean mcs_shm_stop (void);
        gboolean mcs_shm_start (void);
        gboolean options_dialog(const gchar *);
        
    private:
        GtkWidget *settings_dialog=NULL;
        gint shm_settings_serial = 0;
        gint settings_timer = 0;

        McsManager *mcs_manager;
        McsPlugin *mp;
        gboolean running;


};

#endif
