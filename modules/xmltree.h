#ifndef XMLTREE_H
#define XMLTREE_H

#define XMLTREE_key_type 0x01
#define XMLTREE_string_type 0x02

#ifdef XMLTREE_C
#else
#define xmltree_t void
#define xmltree_item void
#define xmltree_attribute void

///////////// "XMLTREE_new" ///////////////////////////
#define XMLTREE_new() \
    rfm_void(RFM_MODULE_DIR, "xmltree", "xmltree_new")

///////////// "XMLTREE_free" ///////////////////////////
#define XMLTREE_free(X) \
    rfm_natural(RFM_MODULE_DIR, "xmltree", X, "xmltree_free")

///////////// "XMLTREE_run" ///////////////////////////
#define XMLTREE_run(X) \
    rfm_natural(RFM_MODULE_DIR, "xmltree", X, "xmltree_run")
///////////// "XSDTREE_run" ///////////////////////////
#define XSDTREE_run(X) \
    rfm_natural(RFM_MODULE_DIR, "xmltree", X, "xsdtree_run")
#define XSDTREE_show_hidden(X) \
    rfm_natural(RFM_MODULE_DIR, "xmltree", GINT_TO_POINTER(X), "xsdtree_show_hidden")

///////////// "XMLTREE_set_echo" ///////////////////////////
#define XMLTREE_set_echo(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X, Y, Z, "xmltree_set_echo")

///////////// "XMLTREE_text_activates_top_attribute" ///////////////////////////
#define XMLTREE_text_activates_top_attribute(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X, GINT_TO_POINTER(Y), "text_activates_top_attribute")


///////////// "XMLTREE_set_title" ///////////////////////////
#define XMLTREE_set_title(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X, Y, "xmltree_set_title")

///////////// "XMLTREE_set_xml" ///////////////////////////
#define XMLTREE_set_xml(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X, Y, "xmltree_set_xml")

///////////// "XMLTREE_set_schema" ///////////////////////////
#define XMLTREE_set_schema(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X, Y, "xmltree_set_schema")

///////////// "XMLTREE_set_editable_attribute" ///////////////////////////
#define XMLTREE_set_editable_attribute(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X, Y, GINT_TO_POINTER(Z), "xmltree_set_editable_attribute")

/////////////// tag wrappers ///////////////////////////////////////////
#define XMLTREE_get_tag_item(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X,Y,Z, "xmltree_get_tag_item")

#define XMLTREE_tag_item_add(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X,Y,Z, "xmltree_tag_item_add")

#define XMLTREE_get_tag_item_list(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X,Y,Z, "xmltree_get_tag_item_list")

#define XMLTREE_get_attribute(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X,Y, "xmltree_get_attribute")

#define XMLTREE_get_attribute_value(X) \
    rfm_natural(RFM_MODULE_DIR, "xmltree", X, "xmltree_get_attribute_value")

#define XMLTREE_set_namespace(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X,Y, "xmltree_set_namespace")

#define XMLTREE_set_attribute_parent(X,Y) \
    rfm_rational(RFM_MODULE_DIR, "xmltree", X,Y, "xmltree_set_attribute_parent")

#define XMLTREE_set_defaults_function(X,Y,Z) \
    rfm_complex(RFM_MODULE_DIR, "xmltree", X,Y,Z, "xmltree_set_defaults_function")

#define XMLTREE_attribute_item_add(X,Y,Z) \
        rfm_complex(RFM_MODULE_DIR, "xmltree", X,Y,(void *)Z, "xmltree_attribute_item_add"); 

#endif
#endif
