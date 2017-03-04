
#include <time.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define GOAL 50
// Reach score of GOAL (default, override with argv[1])
// Each right answer +1
// Each wrong answer -2
static    gint goal;
static    gint answer;
static    gint right_answers = 0;
static    gint wrong_answers = 0;
static    gint score = 0;
static    GtkWidget *happy;
static    GtkWidget *wrong;
static    GtkLabel *operation;
static    GtkLabel *score_label;
static    GtkLabel *last_op_label;
static 	  GtkWidget *last_op_box;
static time_t start=0;

gchar *operation_string=NULL;

static void
set_last_result(void){
    if (!operation_string) return;
    gchar *g = g_strdup_printf("       %s  <span color=\"green\">%d</span>     ",
	    operation_string,answer);
    gtk_label_set_markup(last_op_label, g);
    g_free(g);
    gtk_widget_show(last_op_box);
}

static void set_operation(void){
    gint r = rand(); 
    if (r < RAND_MAX/2) r=0;
    else r=1;
    gint r1 = (rand() % 10) +1; 
    gint r2 = (rand() % 10) +1; 
    if (r) {
	// multiplication
	g_free(operation_string);
	operation_string = g_strdup_printf("<b>%d <span color=\"blue\">X</span> %d  = </b>", r1, r2);
	answer=r1*r2;
	gtk_label_set_markup(operation, operation_string);
    } else {
	// division
	answer=r1;
	g_free(operation_string);
	operation_string = g_strdup_printf("<b>%d <span color=\"blue\">/</span> %d = </b>", r1*r2, r2);
	gtk_label_set_markup(operation, operation_string);
    }
}

static void set_score(void){
    if (start == 0) start = time(NULL);
    gchar *t = g_strdup_printf("%d:%02d", (time(NULL)-start)/60, (time(NULL)-start)%60);
    gchar *g = g_strdup_printf("correctos = <span color=\"green\">%d</span> incorrectos = <span color=\"red\">%d</span> nivel = <span color=\"blue\">%d</span>\n tiempo = %s", right_answers, wrong_answers, score, t);
    gtk_label_set_markup(score_label, g);
    g_free(g);
    g_free(t);
}

static void check_answer(GtkWidget *entry, void *data){
    GtkLabel *score_label = (GtkLabel *)data;
    const gchar *s = gtk_entry_get_text (GTK_ENTRY(entry));
    gint x = atoi(s);
    if (x==answer){
	gtk_widget_hide(wrong);
	set_last_result();
	//gtk_widget_show(happy);
	set_operation();
	right_answers++;
	score++;
    } else {
	//gtk_widget_hide(happy);
	gtk_widget_show(wrong);
	wrong_answers++;
	score--;
	score--;
    }
    //fprintf(stderr, "is %d == %d ? score is %d\n", x, answer, score);
    set_score();
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    while (gtk_events_pending()) gtk_main_iteration();
    if (goal == score){
	// open score message dialog here
	gtk_main_quit();
    }
}

static void
delete_event (GtkWidget * widget, GdkEvent * event, gpointer data) {
    gtk_main_quit();
    exit(1);
}

gint main(gint argc, gchar **argv){
    gtk_init (&argc, &argv);
    srand(time(NULL));   // should only be called once
    if (argv[1]) goal=atoi(argv[1]); else goal=GOAL;
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    operation = GTK_LABEL(gtk_label_new(""));
    GtkEntry *answer = GTK_ENTRY(gtk_entry_new());

    happy = gtk_image_new_from_icon_name ("face-laugh", GTK_ICON_SIZE_SMALL_TOOLBAR);
    wrong = gtk_image_new_from_icon_name ("edit-delete", GTK_ICON_SIZE_SMALL_TOOLBAR);

    last_op_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *score_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    last_op_label = GTK_LABEL(gtk_label_new(""));
    score_label = GTK_LABEL(gtk_label_new(""));


    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET(last_op_box), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(score_box), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(operation), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET(answer), TRUE, TRUE, 0);

    GtkWidget *aux_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (last_op_box), GTK_WIDGET(aux_box), TRUE, TRUE, 0);

    gtk_box_pack_start (GTK_BOX (aux_box), GTK_WIDGET(last_op_label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (aux_box), happy, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), wrong, TRUE, TRUE, 0);



    gtk_box_pack_start (GTK_BOX (score_box), GTK_WIDGET(score_label), TRUE, TRUE, 0);

    set_operation();
    set_score();
    gtk_widget_show_all(window);
    gtk_widget_hide(wrong);
    gtk_widget_hide(last_op_box);
   
    g_signal_connect (answer, "activate", 
            G_CALLBACK (check_answer), score_label);
    g_signal_connect (G_OBJECT (window), "delete_event",
	    G_CALLBACK (delete_event), (gpointer) NULL);
    g_signal_connect (G_OBJECT (window), "destroy", 
	    G_CALLBACK (delete_event), (gpointer) NULL);
    g_signal_connect (G_OBJECT (window), "destroy_event",
	    G_CALLBACK (delete_event), (gpointer) NULL);
    gtk_main();
    gtk_widget_hide(window);

    // create complete window


}
    
    



