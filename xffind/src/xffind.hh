#ifndef __XFFIND__H_
# define __XFFIND__H_
namespace xf
{
template <class Type>
class xffindGUI<Type>
{
public:
    void helpFilter (GtkToggleButton * button) {
	GtkTextView *textView=GTK_TEXT_VIEW(g_object_get_data(G_OBJECT(window), "helpTextView"));
	clearText(textView);
	if (gtk_toggle_button_get_active (button)) {
	    showText(textView); 
	    printTag("blue", _(filter_text_help), NULL);
	    scrollTop(textView);
	} 
    }

    void clearText (GtkTextView *textView) {
	if (!textView){
	    std::cerr<<"DBG> clearText(): textView is null\n";
	    return;
	}
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (textView);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_delete (buffer, &start, &end);
	GtkPaned *vpane = GTK_PANED(g_object_get_data(G_OBJECT(textView), "vpane"));
        hidePane (vpane);
    }
    void hidePane (GtkPaned *vpane) {
	if(!vpane){
	    std::cerr<<"DBG> hidePane(): vpane is null\n";
	    return ;
	}
	GtkAllocation allocation;
	gtk_widget_get_allocation (vpane, &allocation);
	gtk_paned_set_position (GTK_PANED (vpane), 10000);
    }

    void showText (GtkTextView *textView) {
	if(!textView){
	    std::cerr<<"DBG> showText(): textView is null\n";
	    return ;
	}
	GtkPaned *vpane = GTK_PANED(g_object_get_data(G_OBJECT(textView), "vpane"));
	if(!vpane){
	    std::cerr<<"DBG> showText(): vpane is null\n";
	    return ;
	}
	GtkAllocation allocation;
	gtk_widget_get_allocation (vpane, &allocation);
	if (allocation.height <= 50) {
	    std::cerr<<"DBG> showText(): allocation.height <= 50\n";
	    return ;
	}
	gdouble position = gtk_paned_get_position (vpane);
	if(position > allocation.height * 0.90) {
	    gtk_paned_set_position (vpane, allocation.height * 0.75);
	}
	return ;
    }

    private:


// default values:
gint result_limit=256;
gint size_greater=16;
gint size_smaller=1024;
gint last_minutes=60;
gint last_hours=2;
gint last_days=7;
gint last_months=2;
gboolean default_recursive=TRUE;
gboolean default_recursiveH=FALSE;
gboolean default_xdev=TRUE;
gboolean default_case_sensitive=FALSE;
gboolean default_ext_regexp=FALSE;
gboolean default_look_in_binaries=FALSE;
gboolean default_line_count=FALSE;
gint default_type_index=0;
gboolean default_anywhere=TRUE;
gboolean default_match_words=FALSE;
gboolean default_match_lines=FALSE;
gboolean default_match_no_match=FALSE;
GSList *find_list = NULL;
gchar  *last_workdir = NULL;
    
gboolean have_grep = FALSE;
gboolean have_gnu_grep = FALSE;

static const gchar *
filter_text_help=
	        N_("Basic rules:\n" "\n"
                          "*  Will match any character zero or more times.\n"
                          "?  Will match any character exactly one time\n"
                          "[] Match any character within the [] \n"
                          "^  Match at beginning of string\n" 
			  "$  Match at end of string \n");
static const gchar *
grep_text_help=
		N_("Reserved characters for extended regexp are\n"
                          ". ^ $ [ ] ? * + { } | \\ ( ) : \n"
                          "In  basic regular expressions the metacharacters\n"
                          "?, +, {, |, (, and ) lose their special meaning.\n"
                          "\n"
                          "The  period  .   matches  any  single  character.\n"
                          "The caret ^ matches at the start of line.\n"
                          "The dollar $ matches at the end of line.\n" "\n"
                          "Characters within [ ] matches any single \n"
                          "       character in the list.\n"
                          "Characters within [^ ] matches any single\n"
                          "       character *not* in the list.\n"
                          "Characters inside [ - ] matches a range of\n"
                          "       characters (ie [0-9] or [a-z]).\n" "\n"
                          "A regular expression may be followed by one\n"
                          "       of several repetition operators:\n"
                          "?      The preceding item is optional and matched\n"
                          "       at most once.\n"
                          "*      The preceding item will be matched zero\n"
                          "       or more times.\n"
                          "+      The preceding item will be matched one or\n"
                          "       more times.\n"
                          "{n}    The preceding item is matched exactly n times.\n"
                          "{n,}   The preceding item is matched n or more times.\n"
                          "{n,m}  The preceding item is matched at least n times,\n"
                          "       but not more than m times.\n" "\n"
                          "To match any reserved character, precede it with \\. \n"
                          "\n"
                          "Two regular expressions may be joined by the logical or\n"
                          "       operator |.\n"
                          "Two regular expressions may be concatenated.\n" "\n"
                          "More information is available by typing \"man grep\"\n"
			  );

};

template <class Type>
class xffind: public xffindGUI<Type>
{
public:
    xffind(const char *path){
    }
    xffind(void){
    }

private:
    GtkWidget *fill_string_option_menu (GtkComboBox *om, GSList * strings);
    GtkWidget *create_find_dialog (void);
} // namespace xf
#endif
