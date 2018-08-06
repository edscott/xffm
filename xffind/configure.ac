echo '************************ xffm.org ************************************'
# --with-core is default for RCx, but should be off for final release.
#m4_define([tag], [Core])

AC_PREREQ([2.63])

m4_define([vers], [0.0.0.1])
m4_define([package], [xffind])
m4_define([bugreport], [https://sourceforge.net/p/xffm/bugs/])
m4_define([bugmail], ["https://sourceforge.net/p/xffm/bugs/"])
m4_define([package_copyright], ["Copyright (c) 2000-2018 Edscott Wilson Garcia. <edscott@users.sf.net>"])
m4_define([summary],["Advanced user search dialog"]) 
 
m4_define([homepage],["http://xffm.org/"]) 
m4_define([tarball],[package()-vers().tar.gz]) 
# 
m4_define([master_site],[http://sourceforge.net/projects/xffm+/files]) 
m4_define([src_uri],[master_site()/vers()/tarball()]) 
m4_define([license],["  GNU General Public License version 3  "])

AC_COPYRIGHT([package_copyright()])
#AC_DEFINE([COPYRIGHT],[package_copyright()],[Copyright information])
#AUTOMAKE_OPTIONS=dist-bzip2
#AC_INIT([Rodent],[vers()],[bugreport()],[package()],[homepage()])
AC_INIT([package()],[vers],[bugreport],[package],[homepage])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT(disable-static)
AC_PROG_CXX
XDT_CHECK_PACKAGE([TUBO], [tubo], [5.0.13])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.19])
AC_SUBST(GTKPC)GTKPC=gtk+-3.0
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDLIBS)
GTK_VERSION=`$PKG_CONFIG --modversion gtk+-3.0`
GTK_CFLAGS=`$PKG_CONFIG --cflags gtk+-3.0`
GTK_LIBS=`$PKG_CONFIG --libs gtk+-3.0`


AC_DEFINE(_GNU_SOURCE,[1],[Use GNU extensions if available])
AC_SUBST([_GNU_SOURCE])
AC_DEFINE([COPYRIGHT],[package_copyright()],[Copyright information])
AC_SUBST([COPYRIGHT])
AC_DEFINE([PACKAGE_COPYRIGHT],[package_copyright()],[Package copyright])
AC_SUBST([PACKAGE_COPYRIGHT])

AC_ARG_WITH(tcmalloc, [AC_HELP_STRING([--with-tcmalloc], [enable tcmalloc (developers only)])])
AC_SUBST(TCMALLOC)
if test "$with_tcmalloc" = "yes"; then
    AC_DEFINE_UNQUOTED([TCMALLOC],[$with_tcmalloc],[compile with tcmalloc])
    TCMALLOC="-L/usr/local/lib/ -ltcmalloc"
else 
    TCMALLOC=
fi


AM_INIT_AUTOMAKE(1.11 foreign silent-rules)
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])

SRC_URI=src_uri()
AC_SUBST([SRC_URI])
LICENSE=license()
AC_SUBST([LICENSE])
AC_DEFINE_UNQUOTED([LICENSE],[license],[license])

BUGMAIL=bugmail
AC_SUBST([BUGMAIL])
AC_DEFINE_UNQUOTED([BUGMAIL],[bugmail],[send bug reports to])
MASTER_SITE=master_site()
AC_SUBST([MASTER_SITE])

dnl Basic Checks
dnl check for UNIX variants
AC_ISC_POSIX

dnl check for basic programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL
AC_PROG_MAKE_SET

dnl check for standard header files
AC_HEADER_MAJOR
AC_HEADER_STDC

AC_CHECK_HEADERS([stdarg.h varargs.h features.h rodent.h rfm_modules.h])
 
dnl .........................................................

AC_CHECK_PROG(cv_pkgconfig, pkg-config, yes, no)
dnl These tests require pkg-config. 
AC_CHECK_PROGS([intltoolize],[intltoolize],no)
if test $intltoolize = "no" ; then
    AC_MSG_ERROR("intltool utilities not found. Please install intltool to build this package from source.");
fi

AC_CHECK_PROGS([gawk],[gawk],no)
if test $gawk = "no" ; then
    AC_MSG_ERROR("gawk program not found. Please install gawk to build this package from source.");
fi


dnl Check for additional packages 
RFM_GNU_GREP

dnl Check for i18n support
dnl In FreeBSD gettext is in /usr/local
if test "$ac_uname_s" = "Linux"; then
  CFLAGS="$CFLAGS $GTK_CFLAGS"
  CXXFLAGS="$CFLAGS $GTK_CFLAGS"
  LDFLAGS="$LDFLAGS $GTK_LIBS"
else
  CFLAGS="$CFLAGS -I/usr/local/include"
  CXXFLAGS="$CFLAGS -I/usr/local/include"
fi

# Configurable options at script execution
# Enable Debug option (implies core dumps)
RFM_DEBUG

# Enable Trace option (use with care)
RFM_TRACE
# Enable core dumps option
RFM_CORE

# For now, translations are off...
linguas="es"
#linguas="af am  an ar as ast  az  be@latin  be bg bn_IN bn bo br  bs ca ca@valencia crh csb cs cy da de  dz el en_AU en_CA en_GB  en_US eo es et eu fa fi  fr fur fy ga gl  gu gv  he hi hne hr hsb hu hy ia id ig io is it ja ka kg  kk km kn ko  ks ku ky  lb lg li lt lv mai mg mi mk ml mn mr ms  my nb nds ne  nl nn no nso oc or  pa pl ps pt_BR pt pt_PT ro ru rw se si sk sl  sq sr@ijekavianlatin sr@ijekavian sr@ije sr@latin  sr sv   ta te tg th tk  tr tt ug uk  ur uz@cyrillic uz vi wa xh yi yo zh_CN zh_HK zh zh_TW.Big5 zh_TW zu"
XDT_I18N([$linguas])

AC_CHECK_PROG(cv_dpkg, dpkg, yes, no)

AC_OUTPUT([
Makefile
src/Makefile
xffind.desktop.in
xffind.settings
xffind.appdata.xml
])
#po/Makefile.in
#man/Makefile
#icons/Makefile
#icons/png/Makefile
#icons/svg/Makefile

cat fgr+.settings
echo "Build: $build"
RFM_MAKE_NOTICE



