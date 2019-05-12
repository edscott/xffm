#ifndef PARSER_HH
#define PARSER_HH
namespace xf {

GMarkupParseContext *mainContext;
GMarkupParseContext *typetagContext;
GMarkupParseContext *propertyContext;
GMarkupParseContext *fileContext;
FILE *input;
gchar line[2048];

gchar *sourceFile;
gchar *templates;
gchar *extraIncludes=NULL;

GMarkupParser mainParser = {
    mainStart,
    NULL, // mainEnd,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser typeTagParser = {
    startTypeTag,
    NULL,   //endTypeTag,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser propertyParser = {
    startProperty,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser fileParser = {
    startFiles,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

template <class Type>
class Parser {

public:
    static void
    parseXML (const gchar * file) {
	GError *error = NULL;

	TRACE("glib_parser(icon-module): parsing %s\n", file);

	mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
	typetagContext = g_markup_parse_context_new (&typeTagParser, (GMarkupParseFlags)0, &typeTagParent, NULL);
	propertyContext = g_markup_parse_context_new (&propertyParser, (GMarkupParseFlags)0, NULL, NULL);
	fileContext = g_markup_parse_context_new (&fileParser, (GMarkupParseFlags)0, GINT_TO_POINTER(1), NULL);

	input = fopen (file, "r");
	if(!input) {
	    TRACE ("cannot open %s\n", file);
	    return;
	}
	while(!feof (input) && fgets (line, 2048, input)) {
	    line[2048] = 0;
	    if (strstr(line, "<files name=") || strstr(line, "</files")){
		if (strstr(line, "<files name=")){
		    g_markup_parse_context_parse (fileContext, line, strlen(line), &error);
		} else {
		    auto tpath = gtk_tree_row_reference_get_path(referenceParent);
		    gtk_tree_path_up(tpath);
		    gtk_tree_row_reference_free(referenceParent);
		    referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
		    recurseCount--;
	       }
			
	    } else {
		g_markup_parse_context_parse (mainContext, line, strlen(line), &error);
	    }
	}
	fclose (input);

	g_markup_parse_context_free (typetagContext);
	g_markup_parse_context_free (propertyContext);
	g_markup_parse_context_free (fileContext);
	g_markup_parse_context_free (mainContext);
    }

private:

    static void
    startTypeTag (GMarkupParseContext * context,
		   const gchar * element_name,
		   const gchar ** attribute_names, 
		   const gchar ** attribute_values, 
		   gpointer data, 
		   GError ** error) 
    {
	TRACE ("start -> %s\n",element_name); 
	if(strcmp (element_name, "typetag")!= 0 ){
	    fprintf(stderr, "strcmp (element_name, \"typetag\")!= 0 (%s)\n", element_name);
	    return;
	}

	const gchar **name = attribute_names;
	const gchar **value = attribute_values;
	auto parent = (GtkTreeIter *)data; 
	static GtkTreeIter iter;

	for (; name && *name; name++, value++){
	    if (strcmp(*name, "name")==0){
		gtk_tree_store_append(treeStore, &iter, parent);
		gtk_tree_store_set(treeStore, &iter, TYPEDEF_NAME, *value, -1);
		TRACE( "TypeTag %s=%s\n", *name, *value); 
	    } 
	    if (strcmp(*name, "inherits")==0){
		//gtk_tree_store_append(treeStore, &iter, parent);
		gtk_tree_store_set(treeStore, &iter, TYPETAG_INHERITS, *value, -1);
		TRACE( "TypeTag %s=%s\n", *name, *value); 
	    }
	    if (strcmp(*name, "source")==0){
		//gtk_tree_store_append(treeStore, &iter, parent);
		gtk_tree_store_set(treeStore, &iter, PROPERTY_SOURCE, *value, -1);
		TRACE( "TypeTag %s=%s\n", *name, *value); 
	    }
	    if (strcmp(*name, "focus")==0){
		//gtk_tree_store_append(treeStore, &iter, parent);
		gtk_tree_store_set(treeStore, &iter, ICON2, getEmblem((*value)[0]), -1);
		TRACE( "focus %s=%s\n", element_name, *value); 
	    }
	    if (strcmp(*name, "realpath")==0) {
		gtk_tree_store_set(treeStore, &iter, REALPATH, *value, -1);
	    }
       }
	if (tmpParent) gtk_tree_iter_free(tmpParent);
	tmpParent = gtk_tree_iter_copy(&iter);
	return;
    }
    int recurseCount = 0;
    static void
    startFiles(GMarkupParseContext * context,
		   const gchar * element_name,
		   const gchar ** attribute_names, 
		   const gchar ** attribute_values, 
		   gpointer data, 
		   GError ** error) 
    {
	TRACE ("start -> %s\n",element_name); 
		
	if(strcmp (element_name, "files")!= 0 ){
	    fprintf(stderr, "strcmp (element_name, \"files\")!= 0 (%s)\n", element_name);
	    return;
	}
	recurseCount++;

	
	const gchar **name = attribute_names;
	const gchar **value = attribute_values;
	
	TRACE ("%d: %s %s\n",recurseCount, element_name, *value); 
	for (; name && *name; name++, value++){
	    GtkTreeIter parent;
	    GtkTreeIter sourceiter;
	    if (strcmp(*name, "name")==0) {
		GtkTreePath *tpath = gtk_tree_row_reference_get_path(referenceParent);
		gtk_tree_model_get_iter(GTK_TREE_MODEL(treeStore), &parent, tpath);
		gtk_tree_store_append(treeStore, &fileChild, &parent);
		gtk_tree_path_free(tpath);
		tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore), &fileChild);
		gtk_tree_row_reference_free(referenceParent);
		referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
		auto g = g_strdup("");
		for (auto i=0;i<recurseCount; i++){
		    auto gg = g_strconcat(g, "   ", NULL);
		    g_free(g);
		    g=gg;
		}
		auto k = g_strdup_printf("%s%s", g, *value);
		gtk_tree_store_set(treeStore, &fileChild,
		   TYPEDEF_NAME, k, -1);
		   //PROPERTY_NAME, *value, -1);
		g_free(g);
		g_free(k);
	    }
	    if (strcmp(*name, "realpath")==0) {
		gtk_tree_store_set(treeStore, &fileChild, PROPERTY_SOURCE, *value, -1);
		gtk_tree_store_set(treeStore, &fileChild, REALPATH, *value, -1);
	    }
	    
	}

	return;
    }


    static void
    mainStart (GMarkupParseContext * context,
		   const gchar * element_name,
		   const gchar ** attribute_names, 
		   const gchar ** attribute_values, 
		   gpointer data, 
		   GError ** error) 
    {
	TRACE ("start -> %s\n",element_name); 

	if(strcmp (element_name, "structure")==0 ){
	    const gchar **name = attribute_names;
	    const gchar **value = attribute_values;
	    for (; name && *name; name++, value++){
		if (strcmp(*name, "source")==0)sourceFile = g_strdup(*value);
		if (strcmp(*name, "templates")==0)templates = g_strdup(*value);
		if (strcmp(*name, "include")==0)
		    extraIncludes = g_strdup_printf("\nInclude: %s",*value);
	    }
	    return;
	}
	if(strcmp (element_name, "property")==0 ){
	    // Simple one line elements:
	    startProperty (propertyContext,
		   element_name,
		   attribute_names, 
		   attribute_values, 
		   &propertyParent, 
		   error);
	    return;
	}
	if(strcmp (element_name, "typetag")==0 ){
	    startTypeTag (typetagContext,
		   element_name,
		   attribute_names, 
		   attribute_values, 
		   &typeTagParent, 
		   error);
	    // Next lines:
	    while(!feof (input)
		    && 
		    fgets (line, 2048, input) 
		    &&
		    !strstr(line,"</typetag>"))
	    {
		// Simple one line elements:
		line[2048] = 0;
		TRACE("start->\n%s<-end\n", line);
		g_markup_parse_context_parse (propertyContext, line, strlen(line), error);
	    }
	    return;
	}
	return;
    }
    
    static void
    startProperty (GMarkupParseContext * context,
		   const gchar * element_name,
		   const gchar ** attribute_names, 
		   const gchar ** attribute_values, 
		   gpointer data, 
		   GError ** error) 
    {
	auto parent = (GtkTreeIter *)data; 

	TRACE ("start -> %s\n",element_name); 
	if(strcmp (element_name, "property")!= 0 ){
	    fprintf(stderr, "strcmp (element_name, \"property\")!= 0 (%s)\n", element_name);
	    return;
	}
	// Simple property into allProperties
	const gchar **name = attribute_names;
	const gchar **value = attribute_values;
	gboolean validValueIter = FALSE;
	for (; name && *name; name++, value++){
	    if (strcmp(*name, "name")==0){
		TRACE("Property %s=%s\n", *name, *value); 
	    } else {
		TRACE( "         %s=%s\n", *name, *value); 
	    }
	    GtkTreeIter nameiter;
	    GtkTreeIter valueiter;
	    GtkTreeIter sourceiter;
	    if (strcmp(*name, "name")==0) {
		if (data) {
		    gtk_tree_store_append(treeStore, &nameiter, (GtkTreeIter *)data);
		} else {
		    gtk_tree_store_append(treeStore, &nameiter, tmpParent);
		}
		auto g = g_strdup_printf("   %s", *value);
		gtk_tree_store_set(treeStore, &nameiter,
		   TYPEDEF_NAME, g, -1);
		g_free(g);
	    }
	    if (strcmp(*name, "value")==0) {
		gtk_tree_store_append(treeStore, &valueiter, &nameiter);
		validValueIter = TRUE;
		auto g = g_strdup_printf("       %s", *value);
		gtk_tree_store_set(treeStore, &valueiter,
		   TYPEDEF_NAME, g, -1);
		g_free(g);
	    }
	    if (validValueIter && strcmp(*name, "source")==0) {
		auto g = g_strdup_printf("%s", *value);
		gtk_tree_store_set(treeStore, &valueiter,
		   PROPERTY_SOURCE, g, -1);
		g_free(g);
	    }
	    if (strcmp(*name, "realpath")==0) {
		gtk_tree_store_set(treeStore, &nameiter, REALPATH, *value, -1);
		gtk_tree_store_set(treeStore, &valueiter, REALPATH, *value, -1);
	    }

	}
	return;
    }
    
};
   
}


#endif
