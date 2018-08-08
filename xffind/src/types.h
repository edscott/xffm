#ifndef TYPES_H
# define TYPES_H

#define EVENT_CALLBACK(X)  G_CALLBACK((gint (*)(GtkWidget *,GdkEventKey *,gpointer)) X)
#define BUTTON_CALLBACK(X)  G_CALLBACK((void (*)(GtkButton *,gpointer)) X)
#define WIDGET_CALLBACK(X)  G_CALLBACK((void (*)(GtkWidget *,gpointer)) X)
#define CONTEXT_CALLBACK(X)  (gboolean (*)(gpointer)) X

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
