#ifndef CSS_HH
#define CSS_HH

namespace xf {
  class CSS  {
public: 
    static void init(void){
      auto css = getCSSprovider();
      gtk_style_context_add_provider_for_display(gdk_display_get_default(),
          GTK_STYLE_PROVIDER(css),
          GTK_STYLE_PROVIDER_PRIORITY_USER); 
    } 
private:
    static
    GtkCssProvider * getCSSprovider(void){
      GtkCssProvider *css_provider = gtk_css_provider_new();

      char *outputBg = Settings::getString("xfterm", "outputBg", DEFAULT_OUTPUT_BG);
      char *outputFg = Settings::getString("xfterm", "outputFg", DEFAULT_INPUT_BG);
      char *inputBg = Settings::getString("xfterm", "inputBg", "#dddddd");
      char *inputFg = Settings::getString("xfterm", "inputFg", "#000000");
      char *iconsFg = Settings::getString("xfterm", "iconsFg", "#000000");
      char *iconsBg = Settings::getString("xfterm", "iconsBg", "#c0bfbc");

      char *variable = g_strdup_printf(
      "\
      .vbox {\n\
        background-color: %s;\n\
        color: %s;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .output {\n\
        background-color: %s;\n\
        color: %s;\n\
        font-family: monospace;\n\
      }\n\
      .input {\n\
        background-color: %s;\n\
        color: %s;\n\
        font-family: monospace;\n\
      }\n\
      .gridviewColors {\n\
        background-color: %s;\n\
        color: %s;\n\
        font-family: monospace;\n\
      }", inputBg, inputFg, outputBg, outputFg, inputBg, inputFg, iconsBg, iconsFg);

      g_free(outputBg);
      g_free(outputFg);
      g_free(inputBg);
      g_free(inputFg);
      g_free(iconsFg);
      g_free(iconsBg);

      char *data = g_strdup_printf(
      "\
      %s\n\
      .outputview text selection {\n\
        background-color: blue;\n\
        font-family: monospace;\n\
        color: yellow;\n\
      }\n\
      .inputview text selection  {\n\
        background-color: #abc2df;\n\
        color: black;\n\
        font-family: monospace;\n\
      }\n\
      .inquireButton {\n\
        padding-left: 0;\n\
        padding-right: 0;\n\
        padding-bottom: 0;\n\
        padding-top: 0;\n\
        min-height: 16px;\n\
        font-size: 12px;\n\
        margin-bottom: 0;\n\
        margin-left: 0;\n\
        margin-right: 0;\n\
        margin-top: 0;\n\
      }\n\
      .inquireBox {\n\
        padding-bottom: 0;\n\
        padding-top: 0;\n\
        font-size: 12px;\n\
        margin-bottom: 0;\n\
        margin-left: 0;\n\
        margin-right: 0;\n\
        margin-top: 0;\n\
      }\n\
      .inquire {\n\
        background-color: #cccccc;\n\
        padding-bottom: 0;\n\
        padding-top: 0;\n\
        font-size: 12px;\n\
        margin-bottom: 0;\n\
        margin-left: 0;\n\
        margin-right: 0;\n\
        margin-top: 0;\n\
      }\n\
      .gridviewBox {\n\
        margin-bottom: 0;\n\
        margin-left: 0;\n\
        margin-right: 0;\n\
        margin-top: 0;\n\
        border-spacing: 1px 1px;\n\
      }\n\
      .font1 {\n\
        font-size: xx-small;\n\
      }\n\
      .font2 {\n\
        font-size: x-small;\n\
      }\n\
      .font3 {\n\
        font-size: small;\n\
      }\n\
      .font4 {\n\
        font-size: medium;\n\
      }\n\
      .font5 {\n\
        font-size: large;\n\
      }\n\
      .font6 {\n\
        font-size: x-large;\n\
      }\n\
      .font7 {\n\
        font-size: xx-large;\n\
      }\n\
      .prompt {\n\
        background-color: #333333;\n\
        color: #00ff00;\n\
      }\n\
      tooltip {\n\
        color: black;\n\
        background-color: #acaaa5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .pathbarboxRed * {\n\
        color: red;\n\
        font-size: 12px;\n\
        background-color: #dcdad5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .pathbarbox * {\n\
        color: blue;\n\
        font-size: 12px;\n\
        background-color: #dcdad5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .pathbardrop * {\n\
        color: black;\n\
        background-color: green;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: black;\n\
      }\n\
      .gridNegative * {\n\
        color: white;\n\
        background-color: #acaaa5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .menuNegative * {\n\
        background-color: #acaaa5;\n\
      }\n\
      .menuPositive * {\n\
        background-color: white;\n\
      }\n\
      .pathbarboxNegative * {\n\
        color: white;\n\
        font-size: 12px;\n\
        background-color: #acaaa5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .location * {\n\
        color: blue;\n\
        background-color: #dcdad5;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      .dropNegative * {\n\
        background-color: green;\n\
        border-width: 0px;\n\
        border-radius: 0px;\n\
        border-color: transparent;\n\
      }\n\
      ", variable);
        
      //fprintf(stdout, "%s", data);
      g_free(variable);
      gtk_css_provider_load_from_string (css_provider, data);
      g_free(data);
      return css_provider;
    }
  };
}
#endif
