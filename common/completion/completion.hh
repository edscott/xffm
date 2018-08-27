#ifndef COMPLETION_HH
#define COMPLETION_HH
#include <unistd.h>
#include "csh.hh"
#include "common/print.hh"
#include "term/lpterm.hh"

namespace xf {
template <class Type>
class Completion : public CshCompletion<Type>, public LpTerm<Type>{
    using util_c = Util<double>;
    using print_c = Print<double>;

public:
    Completion(void){
        workdir_ = NULL;
    }
    static     gchar *
    get_terminal_name (const gchar *path) {
        gchar *iconname;
        if(!path) {
            iconname = util_c::utf_string (g_get_host_name());
        } else if(g_path_is_absolute(path) &&
                g_file_test (path, G_FILE_TEST_EXISTS)) {
            gchar *basename = g_path_get_basename (path);
            gchar *pathname = g_strdup (path);
            gchar *b = util_c::utf_string (basename);   // non chopped
            util_c::chop_excess (pathname);
            gchar *q = util_c::utf_string (pathname);   // non chopped

            g_free (basename);
            g_free (pathname);
            //iconname = g_strconcat (display_host, ":  ", b, " (", q, ")", NULL);
            iconname = g_strconcat (b, " (", q, ")", NULL);
            g_free (q);
            g_free (b);
        } else {
            iconname = util_c::utf_string (path);
            util_c::chop_excess (iconname);
        }

        return (iconname);
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
	    gchar *command = print_c::get_current_text(input_);
	    DBG("activated with %s\n", command);
            this->csh_clean_start();
	    chdir(workdir_);
	    this->run_lp_command(output_, workdir_, command);
	    this->csh_save_history(command);
	    print_c::clear_text(input_);
	    g_free(command);
	    chdir(workdir_);

            // FIXME run_lp_command();
	    // like this:
	    // lpterm_c::run_lp_command(input_, output_);
	    // 
	    // or better:
	    // Type::run_lp_command(input_, output_);
	    //
	    // and on other types, a void function in class
	    //
	    // here we must go into the lpterm class template, run command
	    // and setup run button...
	    // to do this... lpterm class template must be composed of static
	    // functions. As such, no problemo to call from here
	    // but if this completion class template is used without
	    // lp command, then this command must change according to <Type>
	    //run_c::thread_run(output_, , FALSE);
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
//////////////////////////////////////////////////////////
    //FIXME: lpterm command stuff
  
private:
    GtkTextView *output_;
    GtkTextView *input_;
    gchar *workdir_;

};
}
#endif


