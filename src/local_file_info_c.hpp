#ifndef LOCAL_FILE_INFO_C_HPP
#define LOCAL_FILE_INFO_C_HPP
#include "xffm+.h"
#include "utility_c.hpp"
#include "gtk_c.hpp"

class local_file_info_c: virtual utility_c {
    public:
        local_file_info_c(gtk_c *);
        ~local_file_info_c(void);
        gchar *get_path_info (GtkTreeModel *, GtkTreePath *);

    private:
        gchar *path_info(const gchar *, struct stat *, const gchar *, const gchar *, const gchar *);
        gint count_files (const gchar *);
        gint count_hidden_files (const gchar *);
        gchar ftypelet (mode_t);
        
        gchar *user_string (struct stat *);
        gchar *group_string (struct stat *);
        gchar *mode_string (mode_t mode);
        gchar *date_string (time_t);
        gchar *sizetag (off_t, gint);

        pthread_mutex_t user_string_mutex;
        pthread_mutex_t group_string_mutex;
        pthread_mutex_t date_string_mutex;

        gtk_c *local_gtk_p;
        
        


};
#endif
