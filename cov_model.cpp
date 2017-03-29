
static int stat_func (void){
    struct stat *st;
    __coverity_free__(st);
    return 1;
}


