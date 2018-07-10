dnl Version information
m4_define([summary],["Command line search tool"]) 

dnl configure.ac
dnl
m4_define([package_copyright], [Copyright (c) 2002-2014 Edscott Wilson García. ])
AC_COPYRIGHT([package_copyright])
AC_SUBST([PACKAGE_COPYRIGHT],[package_copyright])
m4_define([package], [fgr])
m4_define([vers], [5.2.5])
m4_define([bugreport], [https://sourceforge.net/p/xffm/bugs/])
m4_define([homepage],["http://xffm.org/fgr.html"]) 

AC_INIT([package()],[vers],[bugreport],[package],[homepage])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT(disable-static)

AC_ARG_WITH(centos, [AC_HELP_STRING([--with-centos], [Create CentOS spec file instead of the default OpenSUSE spec file (packagers only)])])
AC_SUBST(RPM_COMMENT)
AC_SUBST([RPM_LICENSE]) 
AC_SUBST([RPM_GROUP]) 
AC_SUBST([RPM_FDUPES_REQUIRES]) 
AC_SUBST([RPM_UDF_REQUIRES]) 

if test "$with_centos" = "yes"; then
    RPM_COMMENT="# RPM spec file for RedHat CentOS created by configure script."
    RPM_LICENSE="GPLv3"
    RPM_GROUP="User Interface/Desktops"
    RPM_FDUPES_REQUIRES=
    RPM_UDF_REQUIRES=
else 
dnl These are the default OpenSUSE values.
    RPM_COMMENT="# RPM spec file for SUSE OpenSUSE created by configure script."
    RPM_LICENSE="GPL-3.0"
    RPM_GROUP="System/GUI/Other"
    RPM_FDUPES_REQUIRES="BuildRequires: fdupes"
    RPM_UDF_REQUIRES="BuildRequires: update-desktop-files"
fi
AC_SUBST([SUMMARY])SUMMARY=summary
AC_SUBST([HOMEPAGE])HOMEPAGE=homepage


RFM_CORE
RFM_DEBUG
RFM_TRACE
dnl In FreeBSD gettext is in /usr/local
RFM_LINUX_OR_BSD

AM_INIT_AUTOMAKE(1.11 foreign silent-rules)
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

dnl Basic Checks
dnl check for UNIX variants
#AC_GNU_SOURCE
AC_ISC_POSIX

dnl check for basic programs
AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL
AC_PROG_MAKE_SET

dnl check for standard header files
AC_HEADER_MAJOR
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h stdlib.h unistd.h string.h stdarg.h glob.h sys/types.h sys/stat.h sys/wait.h sys/signal.h time.h errno.h])

# This will only work if you run aclocal with -I m4 (duh):
XDT_I18N([af ar as ast az be bg bn br bs ca crh csb cs cy da de dz el en_CA en_GB eo es et eu fa fi fr fy ga gl gu he hi hne hr hsb hu hy ia id is it ja ka km kn ko ku lt lv mai mk ml mr ms nb nds ne nl nn oc or pa pl ps pt_BR pt ro ru si sk sl sq  sr sv ta te tg th  tr ug uk uz vi wa xh zh_CN zh_HK zh zh_TW.Big5 zh_TW ])


dnl Check for C compiler characteristics
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_LONG_DOUBLE
AC_C_VOLATILE

dnl Check for signal handling
AC_CHECK_FUNCS([signal])
AC_CHECK_FUNCS([sigaction])

dnl Check for grep
dnl same note applies here as for XDT_I18N ut supra
#AC_PROG_GREP

#if test $grep != "no" ; then
    AC_MSG_CHECKING([for GNU grep])
    result=`grep --version   | grep GNU`;
#    result=`grep --version  2>/dev/null | grep GNU`;
#    echo "result=${result}"
    if test x"${result}" = x; then
	AC_MSG_RESULT([not found])
	GNU_GREP_FOUND=no
    else 
	AC_DEFINE([HAVE_GNU_GREP], [1], [Define if GNU-grep present])
	AC_MSG_RESULT([found])
	GNU_GREP_FOUND=yes
    fi
    AM_CONDITIONAL([HAVE_GNU_GREP], [test x"GNU_GREP_FOUND" = x"yes"])
#fi

ac_uname_s=`uname -s`
echo "system: $ac_uname_s"
AC_SUBST(LIBDIR) LIBDIR=$libdir;

AC_OUTPUT([
Makefile
fgr.spec
po/POTFILES
po/Makefile.in
src/Makefile
man/Makefile
])
RFM_MAKE_NOTICE



