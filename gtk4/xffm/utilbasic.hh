#ifndef UTILBASIC_HH
#define UTILBASIC_HH

namespace xf {
  class UtilBasic{
    public:
  static 
  void boxPack0(  
      GtkBox* box,
      GtkWidget* child,
      gboolean expand,
      gboolean fill,
      guint padding)
  {
    GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(box));
    gtk_box_append(box, child);
    gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_START);
    // other options: GTK_ALIGN_START, GTK_ALIGN_END, GTK_ALIGN_BASELINE
    if (orientation == GTK_ORIENTATION_HORIZONTAL){
      gtk_widget_set_hexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_start(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_end(GTK_WIDGET(child), padding);
    } else if (orientation == GTK_ORIENTATION_VERTICAL){
      gtk_widget_set_vexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_top(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_bottom(GTK_WIDGET(child), padding);
    } else {
      fprintf(stderr, "boxPack0(): programming error. Exit(2)\n");
      exit(2);
    }
  }
  static gchar *
  utf_string (const gchar * t) {
      if(!t) { return g_strdup (""); }
      if(g_utf8_validate (t, -1, NULL)) { return g_strdup (t); }
      /* so we got a non-UTF-8 */
      /* but is it a valid locale string? */
      gchar *actual_tag;
      actual_tag = g_locale_to_utf8 (t, -1, NULL, NULL, NULL);
      if(actual_tag)
          return actual_tag;
      /* So it is not even a valid locale string... 
       * Let us get valid utf-8 caracters then: */
      const gchar *p;
      actual_tag = g_strdup ("");
      for(p = t; *p; p++) {
          // short circuit end of string:
          gchar *r = g_locale_to_utf8 (p, -1, NULL, NULL, NULL);
          if(g_utf8_validate (p, -1, NULL)) {
              gchar *qq = g_strconcat (actual_tag, p, NULL);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          } else if(r) {
              gchar *qq = g_strconcat (actual_tag, r, NULL);
              g_free (r);
              g_free (actual_tag);
              actual_tag = qq;
              break;
          }
          // convert caracter to utf-8 valid.
          gunichar gu = g_utf8_get_char_validated (p, 2);
          if(gu == (gunichar) - 1) {
              gu = g_utf8_get_char_validated ("?", -1);
          }
          gchar outbuf[8];
          memset (outbuf, 0, 8);
          gint outbuf_len = g_unichar_to_utf8 (gu, outbuf);
          if(outbuf_len < 0) {
              ERROR ("utf_string: unichar=%d char =%c outbuf_len=%d\n", gu, p[0], outbuf_len);
          }
          gchar *qq = g_strconcat (actual_tag, outbuf, NULL);
          g_free (actual_tag);
          actual_tag = qq;
      }
      return actual_tag;
  }
  static void *context_function(void * (*function)(gpointer), void * function_data){
      pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_cond_t signal = PTHREAD_COND_INITIALIZER; 
      auto result=GINT_TO_POINTER(-1);
      void *arg[] = {
          (void *)function,
          (void *)function_data,
          (void *)&mutex,
          (void *)&signal,
          (void *)&result
      };
      gboolean owner = g_main_context_is_owner(g_main_context_default());
      if (owner){
          context_function_f(arg);
      } else {
          g_main_context_invoke(NULL, CONTEXT_CALLBACK(context_function_f), arg);
          pthread_mutex_lock(&mutex);
          if (result == GINT_TO_POINTER(-1)) pthread_cond_wait(&signal, &mutex);
          pthread_mutex_unlock(&mutex);
      }
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&signal);
      return result;
  }
  static gboolean
  context_function_f(gpointer data){
      void **arg = (void **)data;
      void * (*function)(gpointer) = (void* (*)(void*))arg[0];
      gpointer function_data = arg[1];
      pthread_mutex_t *mutex = (pthread_mutex_t *)arg[2];
      pthread_cond_t *signal = (pthread_cond_t *)arg[3];
      auto result_p = (void **)arg[4];
      void *result = (*function)(function_data);
      pthread_mutex_lock(mutex);
      *result_p = result;
      pthread_cond_signal(signal);
      pthread_mutex_unlock(mutex);
      return FALSE;
  }
  };
}
#endif

