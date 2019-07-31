#ifndef XF_MIMETYPE_HH
#define XF_MIMETYPE_HH

// We can use either libmagic or perl mimetype, depending on configuration


namespace xf {
template <class Type>
class MimeType {
#ifdef MIMETYPE_PROGRAM
private:
    static gchar *
    mime (const gchar *command){
	FILE *pipe = popen (command, "r");
	if(pipe == NULL) {
	    ERROR("Cannot pipe from %s\n", command);
	    return NULL;
	}
#define MIMETYPE_LINE 256
	gchar *retval = NULL;
        gchar line[MIMETYPE_LINE];
        line[MIMETYPE_LINE - 1] = 0;
	if (!fgets (line, MIMETYPE_LINE - 1, pipe)) {
	    ERROR("!fgets (line, MIMETYPE_LINE - 1, pipe)\n");
        } 
	else 
	{
	    retval = g_strdup(line);
	    if (strchr(retval, '\n')) *strchr(retval, '\n') = 0;
	}
        pclose (pipe);
	return retval;
    }

public:
    static gchar *
    mimeType (const gchar *file){
        gchar *retval = MimeSuffix<Type>::mimeType(file);
        if (retval) {
	    TRACE("mimeType: %s --> %s\n", file, retval);
            return retval;
        }

	gchar *command = g_strdup_printf("%s -L --output-format=\"%%m\" \"%s\"", 
		MIMETYPE_PROGRAM, file);
	TRACE("MIMETYPE_PROGRAM mimeType command: %s\n", command);
 	retval = mime(command);
	g_free(command);
        if (retval){ 
	    TRACE("MIMETYPE_PROGRAM mimeType: %s --> %s\n", file, retval);
	    if (retval) {
		if (strchr(file, '.') && strlen(strrchr(file, '.')+1)){
		    MimeSuffix<Type>::add2sfx_hash(strrchr(file, '.')+1, retval);
		} else {
		    MimeSuffix<Type>::add2sfx_hash(file, retval);
		}
	    }
	} else retval = g_strdup("unknown mimetype");
	return retval;
   } 

    // FIXME: could use language code -l code, --language=code, maybe
    static gchar *
    mimeFile (const gchar *file){
	gchar *command = g_strdup_printf("%s -d -L --output-format=\"%%d\" \"%s\"", MIMETYPE_PROGRAM, file);
 	gchar *retval = mime(command);
	g_free(command);
	return retval;
   } 

    static gchar *
    mimeMagic (const gchar *file){
	//only magic:
	gchar *command = g_strdup_printf("%s -L -M --output-format=\"%%m\" \"%s\"", MIMETYPE_PROGRAM, file);
	gchar *retval = mime(command);
	g_free(command);
	return retval;
    }


#endif

};
}
#endif
