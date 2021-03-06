#ifndef RUN_BUTTON_C_HPP
#define RUN_BUTTON_C_HPP
#include "gtk_c.hpp"

class run_button_c {
    public:
        run_button_c(void *, const gchar *, pid_t, gboolean);
        ~run_button_c(void);
        void run_button_setup (GtkWidget *);

        gboolean in_shell;
        
        const gchar *get_icon_id(void);
        void set_icon_id(const gchar *);
        const gchar *get_tip(void);
        const gchar *get_command(void);
        const gchar *get_workdir(void);
        gint get_pid(void);
        gint get_grandchild(void);
        void *get_view_v(void);
        GtkWidget *make_menu(void);
        GtkWidget *get_menu(void){return menu;}
	
    protected:
    private:
        pid_t pid;
        pid_t grandchild;
        gchar *command;
        gchar *tip;
        gchar *icon_id;
        gchar *workdir;
        GtkWidget *popup_widget;
        GtkWidget *button;
        void *view_v;
    private:
        void create_menu(void);
        GtkWidget *menu;

};

#endif
