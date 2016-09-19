echo '************************ xffm.org ************************************'

m4_define([rfm_major_version], [5])
m4_define([rfm_minor_version], [3])
m4_define([rfm_micro_version], [16])
m4_define([rfm_nano_version], [3])
m4_define([rfm_version],
          [rfm_major_version.rfm_minor_version.rfm_micro_version.rfm_nano_version])
          
m4_define([libdbh_file_compatibility], ["DBH_2.0/64bit"])
m4_define([rfm_package], [librfm5])
m4_define([rfm_bugreport], [https://sourceforge.net/p/xffm/_list/tickets])
m4_define([rfm_package_copyright], ["Copyright (c) 2000-2014 Edscott Wilson Garcia. <edscott@users.sf.net>"])
m4_define([rfm_description],["Rodent utility primary library"]) 
m4_define([rfm_homepage],["http://xffm.org/librfm"]) 
m4_define([rfm_tarball],[rfm_package()-rfm_version().tar.gz]) 
# 
m4_define([rfm_master_site],["http://sourceforge.net/projects/xffm/files/librfm"]) 
m4_define([rfm_src_uri],[rfm_master_site()/rfm_version()/rfm_tarball()]) 
m4_define([rfm_license],["  GNU General Public License version 3  "])


AC_PREREQ([2.63])

AC_INIT([xffm+],[3.20.0],[http://sourceforge.net/projects/xffm],[xffm+],[http://xffm.org])
AC_CONFIG_MACRO_DIR([m4])
AC_COPYRIGHT([Copyright (c) 2000-2016 Edscott Wilson Garcia. <edscott@users.sf.net>])
# hmm. AM_PROG_AR make automake look for install-sh in $home...
# unless preceded by AC_PROG_INSTALL
AC_PROG_INSTALL
AM_PROG_AR
LT_INIT(disable-static)
AM_INIT_AUTOMAKE([1.11 -Wall no-dist-gzip dist-bzip2])
# Support silent build rules. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
AM_SILENT_RULES([yes])
#AM_MAINTAINER_MODE([enable])

AC_DEFINE(_GNU_SOURCE,[1],[Use GNU extensions if available])
AC_SUBST([_GNU_SOURCE])
AC_DEFINE([COPYRIGHT],[Copyright (c) 2000-2016 Edscott Wilson Garcia. <edscott@users.sf.net>],[Copyright information])
AC_SUBST([COPYRIGHT])
AC_DEFINE([PACKAGE_COPYRIGHT],[Copyright (c) 2000-2016 Edscott Wilson Garcia. <edscott@users.sf.net>],[Package copyright])
AC_SUBST([PACKAGE_COPYRIGHT])
AC_SUBST(PKG_CONFIG_FILE) 

AC_ARG_WITH(la_files, [AC_HELP_STRING([--with-la-files], [Do not remove libtool generated .la files])])
AM_CONDITIONAL(WITH_LA_FILES, [test "$with_la_files" == "yes"])
    

if test $prefix = NONE; then
   echo "prefix = $ac_default_prefix"
else
   echo "prefix = $prefix"
   ac_default_prefix=$prefix
fi  

if test $exec_prefix = NONE; then
    exec_prefix=$ac_default_prefix
fi


AC_DEFINE_UNQUOTED([PREFIX],["$ac_default_prefix"],[Installation prefix])
AC_SUBST([PREFIX])

MODULE_DIR="xffm+/rmodules"
AC_SUBST(MODULE_DIR)
AC_MSG_NOTICE([Module directory: $MODULE_DIR])


AC_SUBST([XFFM_MODULE_DIR])

# FIXME: this test is too hacky
if test $libdir = '${exec_prefix}/lib' ; then
    echo "using default libdir"
    XFFM_MODULE_DIR="$exec_prefix/lib/$MODULE_DIR"

else if test $libdir = '${exec_prefix}/lib64' ; then
    echo "using default 64 libdir"
    XFFM_MODULE_DIR="$exec_prefix/lib64/$MODULE_DIR"
else
    XFFM_MODULE_DIR="$libdir/$MODULE_DIR"
fi
fi
echo "RFM_MODULE_DIR= $RFM_MODULE_DIR"
echo "libdir  = $libdir"
#echo "datadir  = $datadir"
echo "exec_prefix = $exec_prefix"
echo "RFM_MODULE_DIR = $RFM_MODULE_DIR"

AC_MSG_NOTICE([---- You may download libtubo from http://sf.net/projects/xffm/files/libtubo ----])
XDT_CHECK_PACKAGE([TUBO], [tubo], [5.0.13])
 
# XXX FIXME: check for gdbm


dnl check if gtk3 is available

# avoid gtk+-3.10.x    
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.22.5])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.18.0])
XDT_CHECK_PACKAGE([CAIRO], [cairo], [1.12.6])

AC_SUBST(LT_VERS)
AC_SUBST(GTKPC)GTKPC=gtk+-3.0

XDT_CHECK_PACKAGE([LIBZIP], [libzip], [0.9])
XDT_CHECK_PACKAGE([PANGO], [pango], [1.28.0])
XDT_CHECK_PACKAGE([GTHREAD], [gthread-2.0], [2.22.5])
XDT_CHECK_PACKAGE([GMODULE], [gmodule-2.0], [2.22.5])
XDT_CHECK_PACKAGE([LIBRSVG], [librsvg-2.0], [2.26.0])
XDT_CHECK_PACKAGE([LIBXML], [libxml-2.0], [2.4.0])

AC_CONFIG_HEADERS([config.h])

# Configurable options at script execution
# Enable Debug (and core dumps)
RFM_DEBUG
# Enable Trace (not really useful)
RFM_TRACE
# Enable core dumps
RFM_CORE

RFM_ENABLE_PLUGIN([BCRYPT], [bcrypt], [Blow fish encryption], [module], [yes])
RFM_ENABLE_PLUGIN([COMPLETION], [completion], [Bash completion], [module], [yes])
RFM_ENABLE_PLUGIN([COMBOBOX], [ecombo], [Enhanced combo boxes], [module], [yes])
RFM_ENABLE_PLUGIN([PROPERTIES], [props], [Properties dialog], [module], [yes])
RFM_ENABLE_PLUGIN([MIME], [mime], [Mime resolution], [module], [yes])
RFM_ENABLE_PLUGIN([MMAGIC], [mmagic], [Mime previews], [module (requires mime module)], [yes])
RFM_ENABLE_PLUGIN([ZMAGIC], [zmagic], [Zip image previews], [module (requires mime mmagic modules)], [yes])

#RFM_CORE_ON
RFM_MAX_PREVIEW_THREADS

DESCRIPTION=rfm_description()
AC_SUBST([DESCRIPTION])
AC_SUBST([HOMEPAGE])HOMEPAGE=http://xffm.org/
AC_DEFINE_UNQUOTED([HOMEPAGE],[http://xffm.org/],[http://sf.net/projects/xffm])

#SRC_URI=rfm_src_uri()
#AC_SUBST([SRC_URI])
#LICENSE=rfm_license()
#AC_SUBST([LICENSE])
#AC_DEFINE_UNQUOTED([LICENSE],[rfm_license],[rfm_license])

#BUGMAIL=rfm_bugreport
#AC_SUBST([BUGMAIL])
#AC_DEFINE_UNQUOTED([BUGMAIL],[rfm_bugreport],[send bug reports to])
#MASTER_SITE=rfm_master_site()
#AC_SUBST([MASTER_SITE])

dnl Basic Checks
dnl check for UNIX variants
AC_ISC_POSIX

dnl check for basic programs
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL
AC_PROG_MAKE_SET

dnl Check for C compiler characteristics
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_LONG_DOUBLE
AC_C_VOLATILE
AC_HEADER_DIRENT
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_TYPE_SIGNAL


dnl check for standard header files
AC_HEADER_MAJOR
AC_HEADER_STDC
AC_CHECK_HEADERS([readline/history.h])

AC_CHECK_HEADERS([features.h time.h utime.h unistd.h stdarg.h signal.h errno.h stdlib.h stdio.h string.h fcntl.h grp.h limits.h paths.h pwd.h regex.h varargs.h ctype.h ])
 
AC_CHECK_HEADERS([sys/time.h sys/select.h sys/types.h sys/param.h sys/ucred sys/wait.h sys/mman.h])

AC_CHECK_HEADERS([mntent.h])

AC_CHECK_HEADERS([fstab.h sys/param.h sys/ucred.h sys/mount.h])
AC_CHECK_HEADERS([windows.h])
AM_CONDITIONAL(WINDOZE, [test "$ac_cv_header_windows_h" = yes])

AC_SUBST(LARGEFILE) LARGEFILE=yes
ac_uname_s=`uname -s`
echo "system: $ac_uname_s"


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

dnl Check for required packages
# check for libmagic (from "file" utility)
AC_CHECK_LIB([magic],[magic_file])
if test "$ac_cv_lib_magic_magic_file" = "no" ; then
    AC_MSG_ERROR([*** Shucks! *** Error: libmagic header files were not found.  Debian package is  "libmagic-dev" or you can obtain the original author´s latest version by anonymous FTP on ftp.astron.com in the directory /pub/file/file-X.YZ.tar.gz. The Debian version adds a number of new magic entries. It can be obtained from every site carrying a Debian distribution (that is ftp.debian.org and mirrors)
    In OpenSUSE/Fedora, the package is called "file-devel".])
fi

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
RFM_LINUX_OR_BSD
RFM_GNU_AWK

dnl Check for additional packages 
RFM_GNU_LS
RFM_GNU_CP
RFM_GNU_MV
RFM_GNU_LN
RFM_GNU_RM
# Shred is available in BSD with "rm -P"
RFM_GNU_SHRED
RFM_GNU_GHOSTSCRIPT


dnl check for additional system characteristics
RFM_CHECK_DISTRO

#XDT_I18N([af am  an ar as ast  az  be@latin  be bg bn_IN bn bo br  bs ca ca@valencia crh csb cs cy da de  dz el en_AU en_CA en_GB  en_US eo es et eu fa fi  fr fur fy ga gl  gu gv  he hi hne hr hsb hu hy ia id ig io is it ja ka kg  kk km kn ko  ks ku ky  lb lg li lt lv mai mg mi mk ml mn mr ms  my nb nds ne  nl nn nso oc or  pa pl ps pt_BR pt pt_PT ro ru rw se si sk sl  sq sr@ijekavianlatin sr@ijekavian sr@ije sr@latin  sr sv   ta te tg th tk  tr tt ug uk  ur uz@cyrillic uz vi wa xh yi yo zh_CN zh_HK zh zh_TW.Big5 zh_TW zu])
XDT_I18N([es])

dnl final checks
AC_CHECK_FILES([/usr/share/pixmaps /usr/local/share/pixmaps /usr/share/icons /usr/local/share/icons],[],[])

#AC_PROG_PS

AC_CHECK_PROG(cv_dpkg, dpkg, yes, no)
AM_CONDITIONAL(WANT_DEBIAN, [test "$cv_dpkg" = yes])

AC_OUTPUT([
xffm+.settings
Makefile
po/POTFILES
po/Makefile.in
src/Makefile
share/Makefile
share/mime/Makefile
share/icons/Makefile
share/icons/scalable/Makefile
share/icons/scalable/emblems/Makefile
share/icons/scalable/stock/Makefile
modules/Makefile
])

cat xffm+.settings

RFM_MAKE_NOTICE


