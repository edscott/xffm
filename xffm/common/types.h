#ifndef TYPES_H
# define TYPES_H
namespace xf {
#ifdef XFFM_CC
template <class Type> class fmDialog;
#endif
template <class Type> class Notebook;
}


#define EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEvent *,gpointer)) X)
#define KEY_EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEventKey *,gpointer)) X)
#define ICONVIEW_CALLBACK(X)  G_CALLBACK((void (*)(GtkIconView *,GtkTreePath *,gpointer)) X)
#define SELECTION_RECEIVED_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GtkSelectionData *,guint,gpointer)) X)
#define RESPONSE_EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkDialog *,GdkEvent *,gpointer)) X)
#define ENTRY_CALLBACK(X)  G_CALLBACK((void (*)(GtkEntry *,gpointer)) X)
#define RANGE_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkRange *,GtkScrollType,gdouble,gpointer)) X)
#define SIZE_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkRectangle *,gpointer)) X)
#define MENUITEM_CALLBACK(X)  G_CALLBACK((void (*)(GtkMenuItem *,gpointer)) X)
#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkButton *,gpointer)) X)
#define WIDGET_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,gpointer)) X)
#define CONTEXT_CALLBACK(X)  (gboolean (*)(gpointer)) X

#define FILTER_HISTORY g_get_user_data_dir(),"xffm+","xffind.filter",NULL
#define GREP_HISTORY g_get_user_data_dir(),"xffm+","xffind.grep",NULL
#define PATH_HISTORY g_get_user_data_dir(),"xffm+","xffind.path",NULL

#define BIG_ICON_SIZE 		96 
#define MEDIUM_ICON_SIZE 	72 
#define SMALL_ICON_SIZE 	48 
#define TINY_ICON_SIZE 		24 
#define LIST_ICON_SIZE  	0

#define TINY_BUTTON	16
#define SIZE_BUTTON	22
#define SIZE_DIALOG	24
#define SIZE_ICON	48
#define SIZE_PREVIEW	96
#define SIZE_TIP	128

# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
//# define DBG(...)   { (void)0; }
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "*** ERROR> "); fprintf(stderr, __VA_ARGS__);}
# undef WARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}

typedef struct menuItem_t {
    const gchar *label;
    void *callback;
    void *callbackData;
}menuItem_t;

typedef struct radio_t {
    GtkBox *box;
    GtkToggleButton *toggle[5];
} radio_t;

typedef struct sequence_t {
    const gchar *id;
    const gchar *sequence;
} sequence_t;


typedef struct lpterm_colors_t {
    const gchar *id;
    GdkColor color;
} lpterm_colors_t;





#endif
