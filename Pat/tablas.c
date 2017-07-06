
#include <time.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbh.h>
#include <locale.h>

#define GOAL 50
#define WORDS_DBH "words.dbh"
#define Random(num)  (double)(num)*(double)rand()/(double)RAND_MAX
// Reach score of GOAL (default, override with argv[1])
// Each right answer +1
// Each wrong answer -2

gint goal=GOAL;

typedef struct object_t {
    time_t start;
    gint goal;
    union {
	gint answer;
	gint records;
    };
    gchar *word;
    gchar *def;
    gint right_answers;
    gint wrong_answers;
    gint score;
    GtkEntry *answer_entry;
    GtkWidget *happy;
    GtkWidget *wrong;
    GtkLabel *operation;
    GtkLabel *help;
    GtkLabel *score_label;
    GtkLabel *last_op_label;
    GtkWidget *last_op_box;
    GtkWidget *reset;
    GtkWidget *window;
} object_t;

static object_t math_object;
static object_t words_object;

gchar *operation_string=NULL;
//#if 0
static gchar *
utf_string (const gchar * t) {
    const gchar *charset;
    g_get_charset (&charset);
    
    if(!t) { return g_strdup (""); }
    if(g_utf8_validate (t, -1, NULL)) { 
	//fprintf(stderr, "valid utf: %s\n", t);
	return g_strdup (t);
    }
    GError *error=NULL;
    gchar *s=
    g_convert (t, strlen(t),"UTF-8", "ISO-8859-1",
           NULL,NULL,
           &error);
    if (error){
       	fprintf(stderr, "eeror: %s\n",error->message);
	g_error_free(error);
    }
    return (s);
}
//#endif

static void
set_last_result(object_t *object){
    if (!operation_string) return;
    gchar *g = g_strdup_printf("       %s  <span color=\"green\" size=\"xx-large\">%d</span>     ",
	    operation_string,object->answer);
    gtk_label_set_markup(object->last_op_label, g);
    g_free(g);
    gtk_widget_show(object->last_op_box);
}

static void set_operation_words(object_t *object){
    gint record = Random(object->records);
    if (!record) record++;
    unsigned char key_length[256];
    DBHashTable *dbh = dbh_new(WORDS_DBH, key_length, DBH_READ_ONLY);
    dbh_genkey(DBH_KEY(dbh), DBH_KEYLENGTH(dbh), record);
    dbh_load(dbh);
    

    char *palabra;
    palabra=DBH_DATA(dbh);
    //int *number = (int *)palabra;
    palabra += sizeof(unsigned int); /* hay un entero antes de los textos */
    char *definicion = palabra + strlen(palabra) + 1;
    char *utfpalabra = utf_string(palabra);
    char *utfdefinicion = utf_string(definicion);
    char *operation_string = g_strdup_printf("<b><span size=\"xx-large\">%s = </span></b>", utfpalabra);

    gtk_label_set_markup(words_object.operation, operation_string);
    g_free(words_object.word);
    g_free(words_object.def);
    words_object.word = utfpalabra;
    words_object.def = utfdefinicion;
    dbh_close(dbh);
    g_free(operation_string);
    operation_string = g_strdup_printf("<span color=\"blue\" size=\"xx-large\">%s</span>", utfdefinicion);
    gtk_label_set_markup(words_object.help, operation_string);
    g_free(operation_string);

    
 
}

static void set_operation_math(void){
    gint r = rand(); 
    if (r < RAND_MAX/2) r=0;
    else r=1;
    gint r1 = (rand() % 10) +1; 
    gint r2 = (rand() % 10) +1; 
    if (r) {
	// multiplication
	g_free(operation_string);
	operation_string = g_strdup_printf("<span size=\"xx-large\"><b>%d <span color=\"blue\" size=\"xx-large\">X</span> %d  = </b></span>", r1, r2);
	math_object.answer=r1*r2;
	gtk_label_set_markup(math_object.operation, operation_string);
    } else {
	// division
	math_object.answer=r1;
	g_free(operation_string);
	operation_string = g_strdup_printf("<span size=\"xx-large\"> <b>%d <span color=\"blue\" >/</span> %d = </b></span>", r1*r2, r2);
	gtk_label_set_markup(math_object.operation, operation_string);
    }
}

static void set_score(object_t *object){
    if (object->start == 0) object->start = time(NULL);
    gchar *t = g_strdup_printf("%ld:%02ld",(long)(time(NULL) - object->start)/60, (long)(time(NULL) - object->start)%60);
    gchar *g = g_strdup_printf("<span color=\"blue\" size=\"xx-large\"> correctos = <span color=\"green\">%d</span> incorrectos = <span color=\"red\">%d</span> nivel = <span color=\"blue\">%d</span>\n tiempo = %s</span>", object->right_answers, object->wrong_answers, object->score, t);
    gtk_label_set_markup(object->score_label, g);
    g_free(g);
    g_free(t);
}

static void check_word(GtkWidget *entry, void *data){
    //GtkLabel *score_label = (GtkLabel *)data;
    gchar *s = g_strdup(gtk_entry_get_text (GTK_ENTRY(entry)));
    g_strstrip(s);
   g_free(utf_string(words_object.word));
    
    if (strcmp(words_object.word,s)==0){
	gtk_widget_hide(words_object.wrong);
	//set_last_result(&words_object);
	set_operation_words(&words_object);
	words_object.right_answers++;
	words_object.score++;
	//fprintf(stderr, "OK\n");
    } else {
	//gtk_widget_hide(happy);
	gtk_widget_show(words_object.wrong);
	words_object.wrong_answers++;
	words_object.score--;
	words_object.score--;
    }
    //fprintf(stderr, "is %d == %d ? score is %d\n", x, answer, score);
    set_score(&words_object);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    while (gtk_events_pending()) gtk_main_iteration();
    if (words_object.goal == words_object.score){
	// open score message dialog here
	//gtk_main_quit();
    }
    
    g_free(s);
}

static void check_answer(GtkWidget *entry, void *data){
    //GtkLabel *score_label = (GtkLabel *)data;
    const gchar *s = gtk_entry_get_text (GTK_ENTRY(entry));
    gint x = atoi(s);
    if (x==math_object.answer){
	gtk_widget_hide(math_object.wrong);
	set_last_result(&math_object);
	//gtk_widget_show(happy);
	set_operation_math();
	math_object.right_answers++;
	math_object.score++;
    } else {
	//gtk_widget_hide(happy);
	gtk_widget_show(math_object.wrong);
	math_object.wrong_answers++;
	math_object.score--;
	math_object.score--;
    }
    //fprintf(stderr, "is %d == %d ? score is %d\n", x, answer, score);
    set_score(&math_object);
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    while (gtk_events_pending()) gtk_main_iteration();
    if (math_object.goal == math_object.score){
	// open score message dialog here
	//gtk_main_quit();
    }
}

static void
delete_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
    gtk_main_quit();
    exit(1);
}

void reset(GtkButton *button, gpointer data){
    object_t *object = (object_t *)data;
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(object->window),
                                 flags,GTK_MESSAGE_WARNING,
				 GTK_BUTTONS_YES_NO,
                                 "%s", "¿Reiniciar?");
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result)
      {
	case GTK_RESPONSE_YES:
	case GTK_RESPONSE_ACCEPT:
	 object->right_answers = 0;
	 object->wrong_answers = 0;
	 object->score = 0;
	 object->start = 0;
	 set_score(object);
	   break;
	default:
	   break;
      }
    gtk_widget_destroy (dialog);
}

GtkWidget *create_object(GtkWidget *window, object_t *object, const gchar *title){
    object->goal=goal;
    object->window = window;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    object->operation = GTK_LABEL(gtk_label_new(""));
    object->help = GTK_LABEL(gtk_label_new(""));
    object->answer_entry = GTK_ENTRY(gtk_entry_new());

    object->happy = gtk_image_new_from_icon_name ("face-laugh", GTK_ICON_SIZE_SMALL_TOOLBAR);
    object->wrong = gtk_image_new_from_icon_name ("edit-delete", GTK_ICON_SIZE_SMALL_TOOLBAR);

    object->last_op_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    object->reset = gtk_button_new_with_label("Reiniciar");
    GtkWidget *score_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    object->last_op_label = GTK_LABEL(gtk_label_new(""));
    object->score_label = GTK_LABEL(gtk_label_new(""));


    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    GtkWidget *label = gtk_label_new(title);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gchar *label_markup=g_strdup_printf("<span  size=\"xx-large\">%s</span>", title);
    gtk_label_set_markup(GTK_LABEL(label), label_markup);
    g_free(label_markup);
   

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET(object->reset), FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET(object->last_op_box), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(object->help), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(score_box), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(object->operation), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(object->answer_entry), TRUE, TRUE, 0);

    GtkWidget *aux_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (object->last_op_box), GTK_WIDGET(aux_box), TRUE, TRUE, 0);

    gtk_box_pack_start (GTK_BOX (aux_box), GTK_WIDGET(object->last_op_label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (aux_box), object->happy, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), object->wrong, TRUE, TRUE, 0);



    gtk_box_pack_start (GTK_BOX (score_box), GTK_WIDGET(object->score_label), TRUE, TRUE, 0);
    set_score(object);
    return vbox;    


}

void create_math(GtkWidget *window){
    memset(&math_object, 0, sizeof(object_t));
    GtkWidget *vbox = create_object(window, &math_object, "Matemáticas");
    GtkWidget *paned =g_object_get_data(G_OBJECT(window), "paned");
    gtk_paned_add1 (GTK_PANED(paned), vbox);
    set_operation_math(); 
    g_signal_connect (math_object.answer_entry, "activate", 
            G_CALLBACK (check_answer), math_object.score_label);
    g_signal_connect(G_OBJECT(math_object.reset), "clicked", G_CALLBACK(reset), &math_object);
    
}

void create_words(GtkWidget *window){
    memset(&words_object, 0, sizeof(object_t));
    GtkWidget *vbox = create_object(window, &words_object, "Español");
    GtkWidget *paned =g_object_get_data(G_OBJECT(window), "paned");
    gtk_paned_add2 (GTK_PANED(paned), vbox);
    unsigned char key_length;
    DBHashTable *dbh = dbh_new(WORDS_DBH, &key_length, DBH_READ_ONLY);
    words_object.records = DBH_RECORDS(dbh);
    dbh_close(dbh);
    //fprintf(stderr, "records=%d\n",words_object.records);
    
    set_operation_words(&words_object); 
    g_signal_connect (words_object.answer_entry, "activate", 
            G_CALLBACK (check_word), words_object.score_label);
    g_signal_connect(G_OBJECT(words_object.reset), "clicked", G_CALLBACK(reset), &words_object);
    
}

static void
show(GtkWidget *window){
    gtk_widget_set_size_request(window, 500, 300);
    gtk_widget_show_all(window);
    gtk_widget_hide(math_object.wrong);
    gtk_widget_hide(math_object.last_op_box);
    gtk_widget_hide(words_object.wrong);
    gtk_widget_hide(words_object.last_op_box);
    

    g_signal_connect (G_OBJECT (window), "delete_event",
	    G_CALLBACK (delete_event), (gpointer) NULL);
    g_signal_connect (G_OBJECT (window), "destroy", 
	    G_CALLBACK (delete_event), (gpointer) NULL);
    g_signal_connect (G_OBJECT (window), "destroy_event",
	    G_CALLBACK (delete_event), (gpointer) NULL);
}

gint main(gint argc, gchar **argv){
    setlocale (LC_ALL, "es_MX.iso88591");
//    setlocale (LC_ALL, "");
    if (argv[1]) goal=atoi(argv[1]);
    gtk_init (&argc, &argv);
    srand(time(NULL));   // should only be called once
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *paned =gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    g_object_set_data(G_OBJECT(window), "paned", paned);
    gtk_container_add (GTK_CONTAINER (window), paned);

    create_math(window);
    create_words(window);

    show(window);
    gtk_main();
    gtk_widget_hide(window);

}
    
    



