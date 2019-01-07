echo '************************ xffm.org ************************************'

AC_PREREQ([2.63])

AC_INIT([xffm+],[0.9],[http://sourceforge.net/projects/xffm],[xffm+],[http://xffm.org])
AC_CONFIG_MACRO_DIR([m4])
AC_COPYRIGHT([Copyright (c) 2000-2019 Edscott Wilson Garcia. <edscott@users.sf.net>])
AM_INIT_AUTOMAKE([1.11 -Wall no-dist-gzip dist-bzip2])
# Support silent build rules. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
AM_SILENT_RULES([yes])


#XDT_CHECK_PACKAGE([LIBZIP], [libzip], [0.9])
#XDT_CHECK_PACKAGE([LIBRSVG], [librsvg-2.0], [2.26.0])

AC_CONFIG_HEADERS([autotools.h])

dnl Basic Checks
dnl check for UNIX variants
AC_ISC_POSIX

dnl check for basic programs
AC_PROG_INTLTOOL
AC_PROG_MAKE_SET

dnl Check for C compiler characteristics
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_TYPE_SIGNAL

dnl check for standard header files
#AC_CHECK_HEADERS([readline/history.h])

#AC_CHECK_HEADERS([features.h time.h utime.h unistd.h stdarg.h signal.h errno.h stdlib.h stdio.h string.h fcntl.h grp.h limits.h paths.h pwd.h regex.h varargs.h ctype.h ])
 
#AC_CHECK_HEADERS([sys/time.h sys/select.h sys/types.h sys/param.h sys/ucred sys/wait.h sys/mman.h])

#AC_CHECK_HEADERS([mntent.h])Writing objects: 100% (22/22), 2.29 KiB | 391.00 KiB/s, done.


#AC_CHECK_HEADERS([fstab.h sys/param.h sys/ucred.h sys/mount.h])

#AC_SUBST(LARGEFILE) LARGEFILE=yes
ac_uname_s=`uname -s`
echo "system: $ac_uname_s"

AC_CHECK_HEADERS([mntent.h sys/mount sys/types sys/sysctl sys/param])

dnl check for standard functions
AC_CHECK_FUNCS([setenv unsetenv])
AC_CHECK_FUNCS(strlcpy snprintf strlcat lseek snprintf vsnprintf strcasecmp strncasecmp memcpy memmove memset)
AC_CHECK_FUNCS([waitid signal sigaction])
#AC_SEARCH_LIBS([clock_gettime], [rt posix4])
AC_CHECK_FUNCS([clock_gettime localtime localtime_r gettimeofday strftime])
AC_CHECK_FUNCS([getmntinfo getfsent getmntent statfs])
AC_CHECK_FUNCS([signal sigaction])
dnl http://womble.decadent.org.uk/readdir_r-advisory.html
AC_CHECK_FUNCS([fpathconf dirfd])
AC_CHECK_MEMBERS([struct dirent.d_type], [], [], [[#include <dirent.h>]])


dnl check for standard libraries
# check for POSIX shared memory
AC_CHECK_LIB([rt],[shm_open])
# check for POSIX threads GNU_SOURCE
AC_CHECK_LIB([pthread],[pthread_tryjoin_np])
if test "$ac_cv_lib_pthread_pthread_tryjoin_np" = yes; then
  	AC_MSG_NOTICE([You have non portable GNU thread extensions. Good for you.])
	AC_SUBST(HAVE_PTHREAD_TRYJOIN)
	AC_DEFINE_UNQUOTED([HAVE_PTHREAD_TRYJOIN],[$ac_cv_lib_pthread_pthread_tryjoin_np],[enable non-standard GNU extensions])
else
# check for plain POSIX threads
        AC_CHECK_LIB([pthread],[pthread_create])
fi

AC_CHECK_PROG(cv_pkgconfig, pkg-config, yes, no)

AC_CHECK_PROGS([intltoolize],[intltoolize],no)
if test $intltoolize = "no" ; then
    AC_MSG_ERROR("intltool utilities not found. Please install intltool to build this package from source.");
fi

AC_CHECK_PROGS([gawk],[gawk],no)
if test $gawk = "no" ; then
    AC_MSG_ERROR("gawk program not found. Please install gawk to build this package from source.");
fi

#
dnl In FreeBSD gettext is in /usr/local
#RFM_LINUX_OR_BSD
#RFM_GNU_AWK

dnl Check for additional packages 
#RFM_GNU_LS
#RFM_GNU_CP
#RFM_GNU_MV
#RFM_GNU_LN
#RFM_GNU_RM
# Shred is available in BSD with "rm -P"
#RFM_GNU_SHRED
#RFM_GNU_GHOSTSCRIPT


#XDT_I18N([af am  an ar as ast  az  be@latin  be bg bn_IN bn bo br  bs ca ca@valencia crh csb cs cy da de  dz el en_AU en_CA en_GB  en_US eo es et eu fa fi  fr fur fy ga gl  gu gv  he hi hne hr hsb hu hy ia id ig io is it ja ka kg  kk km kn ko  ks ku ky  lb lg li lt lv mai mg mi mk ml mn mr ms  my nb nds ne  nl nn nso oc or  pa pl ps pt_BR pt pt_PT ro ru rw se si sk sl  sq sr@ijekavianlatin sr@ijekavian sr@ije sr@latin  sr sv   ta te tg th tk  tr tt ug uk  ur uz@cyrillic uz vi wa xh yi yo zh_CN zh_HK zh zh_TW.Big5 zh_TW zu])
#XDT_I18N([es zh_CN de fr ru])
XDT_I18N([es zh_CN fr de ru])
#XDT_I18N([es])

dnl final checks
#AC_CHECK_FILES([/usr/share/pixmaps /usr/local/share/pixmaps /usr/share/icons /usr/local/share/icons],[],[])

AC_OUTPUT([
Makefile
po/POTFILES
po/Makefile.in
])



