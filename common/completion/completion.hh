#ifndef COMPLETION_HH
#define COMPLETION_HH
#include "csh.hh"

namespace xf {
template <class Type>
class Completion : public CshCompletion<Type>{

public:
    Completion(void){
        workdir_ = NULL;
    }

    void setInput(GtkTextView *input){input_ = input;}
    void setOutput(GtkTextView *output){output_ = output;}
    void setWorkdir(const gchar *workdir){
        g_free(workdir_);
        workdir_ = g_strdup(workdir);
        DBG("workdir set to %s\n", workdir_);
    }
    const gchar * workdir(void){
        DBG("returning workdir_%s\n", workdir_);
        return workdir_;
    }
    gint
    keyboard_event( GdkEventKey * event) {
        TRACE( "lpterm_c::lpterm_keyboard_event...\n");
        if(!event) {
            g_warning ("on_status_key_press(): returning on event==0\n");
            return TRUE;
        }
        gtk_widget_grab_focus (GTK_WIDGET(input_));

        
        if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up) {
            if (this->is_completing()) this->csh_completion(input_, output_, 1);
            else this->csh_history(input_, output_, 1);
            return TRUE;
        }
        if(event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down) {
            if (this->is_completing()) this->csh_completion(input_, output_, -1);
            else this->csh_history(input_, output_, -1);

            return TRUE;
        }
        // On activate, run the lpcommand.
        if((event->keyval == GDK_KEY_Return) || (event->keyval == GDK_KEY_KP_Enter)) {
            this->csh_clean_start();
            // FIXME run_lp_command();
            return TRUE;
        }
        if((event->keyval == GDK_KEY_Page_Up) || (event->keyval == GDK_KEY_Page_Down)) {
            gboolean retval;
            g_signal_emit_by_name ((gpointer)output_, "key-press-event", event, &retval);
            
            return TRUE;
        }
        // tab for bash completion.
        if(event->keyval == GDK_KEY_Tab) {
            BashCompletion<Type>::bash_completion(input_, output_, workdir_);
            return TRUE;
        }

        // Let the internal callback do its business first.
        TRACE("Let the internal callback do its business first.\n");
        gboolean retval;
        g_signal_emit_by_name ((gpointer)input_, "key-press-event", event, &retval);
        while (gtk_events_pending())gtk_main_iteration();
        TRACE("Now do our stuff.\n");

        // Now do our stuff.
        
        // If cursor is back at position 0, then reset.
        this->query_cursor_position(input_);

        // On right or left, change the csh completion token.
        if((event->keyval == GDK_KEY_Right) || (event->keyval == GDK_KEY_KP_Right)) {
            this->csh_dirty_start(input_);
        }
        // On right or left, change the csh completion token.
        if((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_KP_Left)) {
            this->csh_dirty_start(input_);
        }


        if (event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde){
            this->csh_dirty_start(input_);
        }
        return retval;
    }

/*
    void *
    shell_command(const gchar *c, gboolean save){
        // Make sure any sudo command has the "-A" option
        gchar *command = sudo_fix(c);
        pid_t pid = thread_run(command?command:c);
        if (!pid) return NULL; 
        run_button_c *run_button_p = NULL;
        run_button_p = new run_button_c(view_v, c, pid, run_in_shell(c));
        // We save the original sudo command, not the one modified with "-A"
        if (save) csh_save_history(c);
        g_free (command);
        return (void *)run_button_p;
    }
    */
private:
    GtkTextView *output_;
    GtkTextView *input_;
    gchar *workdir_;


};
}
#endif


