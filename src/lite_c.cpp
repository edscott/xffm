#include "lite_c.hpp"


lite_c::lite_c(void) {
}

lite_c::~lite_c(void){
    g_hash_table_destroy(lite_type_hash);
    g_hash_table_destroy(lite_key_hash);
}


//////////////////////////////////////////////////////////////////////


