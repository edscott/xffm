#ifndef COMPLETION_HH
#define COMPLETION_HH
#include <unistd.h>
#include "csh.hh"
#include "lpterm.hh"

namespace xf {
template <class Type> class Page;
template <class Type>
class Completion : public CshCompletion<Type>
                   , public LpTerm<Type>
{
    using util_c = Util<double>;
    using print_c = Print<double>;

public:
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

    void setCompletionInput(GtkTextView *input){completionInput_ = input;}
    void setCompletionOutput(GtkTextView *output){completionOutput_ = output;}

    gint
    completionKeyboardEvent( GdkEventKey * event) {
        TRACE( "lpterm_c::lpterm_keyboard_event...\n");
        if(!event) {
            g_warning ("on_status_key_press(): returning on event==0\n");
            return TRUE;
        }

        gtk_widget_grab_focus (GTK_WIDGET(completionInput_));

        
        if(event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_KP_Up) {
            if (this->is_completing()) this->csh_completion(completionInput_, completionOutput_, 1);
            else this->csh_history(completionInput_, completionOutput_, 1);
            return TRUE;
        }
        if(event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_KP_Down) {
            if (this->is_completing()) this->csh_completion(completionInput_, completionOutput_, -1);
            else this->csh_history(completionInput_, completionOutput_, -1);

            return TRUE;
        }
        // On activate, run the lpcommand.
        if((event->keyval == GDK_KEY_Return) || (event->keyval == GDK_KEY_KP_Enter)) {
	    gchar *command = print_c::get_current_text(completionInput_);
	    TRACE("activated with %s\n", command);
            this->csh_clean_start();
            const gchar *workdir = ((Page<Type> *)this)->workDir();
	    if (!workdir || !g_file_test(workdir, G_FILE_TEST_IS_DIR)){
		ERROR("completionKeyboardEvent(): invalid workdir: %s\n", workdir);
		return TRUE;
	    }
            TRACE("command at %s\n", workdir);
	    this->run_lp_command(completionOutput_, workdir, command);
	    this->csh_save_history(command);
	    print_c::clear_text(completionInput_);
	    g_free(command);
            return TRUE;
        }
        if((event->keyval == GDK_KEY_Page_Up) || (event->keyval == GDK_KEY_Page_Down)) {
            gboolean retval;
            g_signal_emit_by_name ((gpointer)completionOutput_, "key-press-event", event, &retval);
            
            return TRUE;
        }
        // tab for bash completion.
        if(event->keyval == GDK_KEY_Tab) {
            TRACE("event->keyval == GDK_KEY_Tab\n");
            BashCompletion<Type>::bash_completion(completionInput_, completionOutput_, ((Page<Type> *)this)->workDir());
            return TRUE;
        }

        // Let the internal callback do its business first.
        TRACE("Let the internal callback do its business first.\n");
        gboolean retval;
        g_signal_emit_by_name ((gpointer)completionInput_, "key-press-event", event, &retval);
        while (gtk_events_pending())gtk_main_iteration();
        TRACE("Now do our stuff.\n");

        // Now do our stuff.
        
        // If cursor is back at position 0, then reset.
        this->query_cursor_position(completionInput_);

        // On right or left, change the csh completion token.
        if((event->keyval == GDK_KEY_Right) || (event->keyval == GDK_KEY_KP_Right)) {
            this->csh_dirty_start(completionInput_);
        }
        // On right or left, change the csh completion token.
        if((event->keyval == GDK_KEY_Left) || (event->keyval == GDK_KEY_KP_Left)) {
            this->csh_dirty_start(completionInput_);
        }


        if (event->keyval >= GDK_KEY_space && event->keyval <= GDK_KEY_asciitilde){
            this->csh_dirty_start(completionInput_);
        }
        return retval;
    }
//////////////////////////////////////////////////////////
  
private:
    GtkTextView *completionOutput_;
    GtkTextView *completionInput_;

};
}
#endif


