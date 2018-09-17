#ifndef TYPES_H
# define TYPES_H


#define RANGE_CALLBACK(X)  G_CALLBACK((gboolean (*)(GtkRange *,GtkScrollType,gdouble,gpointer)) X)
#define SIZE_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,GdkRectangle *,gpointer)) X)
#define EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEventKey *,gpointer)) X)
#define MENUITEM_CALLBACK(X)  G_CALLBACK((void (*)(GtkMenuItem *,gpointer)) X)
#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkButton *,gpointer)) X)
#define WIDGET_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,gpointer)) X)
#define CONTEXT_CALLBACK(X)  (gboolean (*)(gpointer)) X

#define FILTER_HISTORY g_get_user_data_dir(),"xffm+","xffind.filter",NULL
#define GREP_HISTORY g_get_user_data_dir(),"xffm+","xffind.grep",NULL
#define PATH_HISTORY g_get_user_data_dir(),"xffm+","xffind.path",NULL

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
