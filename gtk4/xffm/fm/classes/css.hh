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
      char *iconsBg = Settings::getString("xfterm", "iconsBg", "#ffffff");

      char *data = g_strdup_printf(
      "\
      .vbox {\
        background-color: %s;\
        color: %s;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .output {\
        background-color: %s;\
        color: %s;\
        font-family: monospace;\
      }\
      .outputview text selection {\
        background-color: blue;\
        font-family: monospace;\
        color: yellow;\
      }\
      .input {\
        background-color: %s;\
        color: %s;\
        font-family: monospace;\
      }\
      .inputview text selection  {\
        background-color: #abc2df;\
        color: black;\
        font-family: monospace;\
      }\
      .gridviewColors {\
        background-color: %s;\
        color: %s;\
        font-family: monospace;\
      }\
      .inquireButton {\
        padding-left: 0;\
        padding-right: 0;\
        padding-bottom: 0;\
        padding-top: 0;\
        min-height: 16px;\
        font-size: 12px;\
        margin-bottom: 0;\
        margin-left: 0;\
        margin-right: 0;\
        margin-top: 0;\
      }\
      .inquireBox {\
        padding-bottom: 0;\
        padding-top: 0;\
        font-size: 12px;\
        margin-bottom: 0;\
        margin-left: 0;\
        margin-right: 0;\
        margin-top: 0;\
      }\
      .inquire {\
        background-color: #cccccc;\
        padding-bottom: 0;\
        padding-top: 0;\
        font-size: 12px;\
        margin-bottom: 0;\
        margin-left: 0;\
        margin-right: 0;\
        margin-top: 0;\
      }\
      .gridviewBox {\
        margin-bottom: 0;\
        margin-left: 0;\
        margin-right: 0;\
        margin-top: 0;\
        border-spacing: 1px 1px;\
      }\
      .font1 {\
        font-size: xx-small;\
      }\
      .font2 {\
        font-size: x-small;\
      }\
      .font3 {\
        font-size: small;\
      }\
      .font4 {\
        font-size: medium;\
      }\
      .font5 {\
        font-size: large;\
      }\
      .font6 {\
        font-size: x-large;\
      }\
      .font7 {\
        font-size: xx-large;\
      }\
      .prompt {\
        background-color: #333333;\
        color: #00ff00;\
      }\
      tooltip {\
        color: black;\
        background-color: #acaaa5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .pathbarboxRed * {\
        color: red;\
        font-size: 12px;\
        background-color: #dcdad5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .pathbarbox * {\
        color: blue;\
        font-size: 12px;\
        background-color: #dcdad5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .pathbardrop * {\
        color: black;\
        background-color: green;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: black;\
      }\
      .gridNegative * {\
        color: white;\
        background-color: #acaaa5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .pathbarboxNegative * {\
        color: white;\
        font-size: 12px;\
        background-color: #acaaa5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .location * {\
        color: blue;\
        background-color: #dcdad5;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      .dropNegative * {\
        background-color: green;\
        border-width: 0px;\
        border-radius: 0px;\
        border-color: transparent;\
      }\
      ",
        inputBg, inputFg, outputBg, outputFg, inputBg, inputFg, iconsBg, iconsFg);
      g_free(outputBg);
      g_free(outputFg);
      g_free(inputBg);
      g_free(inputFg);
      g_free(iconsFg);
      g_free(iconsBg);
      gtk_css_provider_load_from_string (css_provider, data);
      g_free(data);
      return css_provider;
    }
  };
}
#endif
