dnl 5.2.4

AC_DEFUN([RFM_SET_PKG_CONF_DIR],[
if test $prefix = NONE; then
   PREF=$ac_default_prefix
else
   PREF=$prefix
fi  
AC_ARG_WITH(pkglibdata, [AC_HELP_STRING([--with-pkglibdata], [Place pkgconfig .pc files at libdata/pkgconfig, relative to prefix])])
AC_ARG_WITH(pkgdatadir, [AC_HELP_STRING([--with-pkgdatadir], [Place pkgconfig .pc files at (datadir)/pkgconfig])])
PKG_CONF_DIR=
if test x"$with_pkglibdata" != x ; then
    PKG_CONF_DIR="$PREF/libdata/pkgconfig"
else
  if test x"$with_pkgshare" != x ; then
    PKG_CONF_DIR="$PREF/share/pkgconfig"
  else
    DIRS="lib libdata share"
    TOP_COUNT=0
    for DIR in $DIRS
    do
        TARGET=$PREF/$DIR/pkgconfig
        if test -e $TARGET ; then
            echo "Searching for pkgconfig directory $TARGET... found."
            R=`ls $TARGET | grep \.pc\$`
                echo "score: $TARGET -> ${#R}"
                if test ${#R} -gt $TOP_COUNT ; then
                    TOP_COUNT=${#R}
                    PKG_CONF_DIR=$TARGET
                fi
        else
            echo "Searching for pkgconfig directory $TARGET... not found."
        fi
    done
    if test ${#PKG_CONF_DIR} -eq 0 ; then
        PKG_CONF_DIR="$libdir/pkgconfig"
        echo "*** Could not determine pkgconfig target. Using default $PKG_CONF_DIR"
        echo "    target. If this is not correct, use either --with_pkglibdata or"
        echo "    --with_pkgshare option and rerun configure"
    else
        echo "    Using $PKG_CONF_DIR as target for pkgconfig file."
        echo "    If this is not correct, use either --with_pkglibdata or"
        echo "    --with_pkgshare option and rerun configure"
    fi
  fi
fi
AC_SUBST([PKG_CONF_DIR]) PKG_CONF_DIR="$libdir/pkgconfig"
]
)


dnl RFM_ENABLE_PLUGIN(varname, optionname, option string, option string 2, [default])
dnl
dnl This macro automatically adds a commandline switch based on the "optionname"
dnl parameter (--enable-optionname/--disable-optionname), which allows the
dnl user to explicitly control whether this option should be
dnl enabled or not. The "option string" parameter gives a brief description
dnl about this switch.
dnl
dnl If the user chose to enable this option defines a WITH_"varname" for
dnl use in Makefile.am
dnl
AC_DEFUN([RFM_ENABLE_PLUGIN],
[

  AC_ARG_ENABLE([$2],[AC_HELP_STRING([--enable-$2], [Create "$3" $4 [[default=$5]]])]) 
  AM_CONDITIONAL([WANT_$1_P], [test x$enable_$2 = xyes || ( test x$enable_$2 = x && test x$5=xyes )])
  AC_SUBST(WITH_$1) 
  if test x$enable_$2 = xno; then
	WITH_$1=no
  else
    if test x$enable_$2 = xyes; then
	WITH_$1=yes
    else
	WITH_$1=$5
    fi
  fi
])

AC_DEFUN([RFM_CHECK_DISTRO],
[
AC_MSG_CHECKING(distro);
result=`ls -l /sbin/yast 2>/dev/null | grep yast2`
if test x"${result}" = x; then
    AC_MSG_RESULT([ not Suse.])
    AC_SUBST(LIBRSVG_NAME) LIBRSVG_NAME=librsvg2
    AC_SUBST(DISTRO_X) DISTRO_X=libSM-devel
    AC_SUBST(DISTRO_X_VERSION) DISTRO_X_VERSION=1.2
    AC_SUBST(SUSE_RPM) SUSE_RPM=no
else 
    AC_MSG_RESULT([ Suse.])
    AC_SUBST(LIBRSVG_NAME) LIBRSVG_NAME=librsvg
    AC_SUBST(DISTRO_X) DISTRO_X=xorg-x11-devel
    AC_SUBST(DISTRO_X_VERSION) DISTRO_X_VERSION=7.6
    AC_SUBST(SUSE_RPM) SUSE_RPM=yes
fi

AC_MSG_CHECKING(for gentoo emerge)
AC_CHECK_PROG(cv_emerge, emerge, yes, no)
AM_CONDITIONAL(WANT_GENTOO, [test "$cv_emerge" != no])

AC_MSG_CHECKING(for debian dpkg)
AC_CHECK_PROG(cv_dpkg, dpkg, yes, no)
AM_CONDITIONAL(WANT_DEBIAN, [test "$cv_dpkg" != no])

])

AC_DEFUN([RFM_CORE],
[
AC_ARG_WITH([core],[AC_HELP_STRING([--with-core], [Enable core dumps])]) 
if test "$with_core" = "yes"
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	AC_MSG_NOTICE([You enabled core dumps. Good for you.])
	CFLAGS="-O0 -ggdb"
elif test "$with_core" != ""
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_MSG_NOTICE([Your specific kind of core dumps is $with_core])
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	CFLAGS="-O0 -ggdb"

else
	AC_SUBST(WITH_CORE) WITH_CORE=no
	AC_MSG_NOTICE([No core dumps for you.])
fi
])


AC_DEFUN([RFM_DEBUG],
[
AC_ARG_WITH([debug], [AC_HELP_STRING([--with-debug], [Enable debug])])
if test "$with_debug" = "yes"
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	AC_SUBST(WITH_DEBUG) WITH_DEBUG=yes
	AC_SUBST(DEBUG)
	AC_DEFINE_UNQUOTED([DEBUG],[$with_debug],[enable DEBUG])
	AC_MSG_NOTICE([You enabled debug. Good for you.])
	DB=--with-debug
	CFLAGS="-O0 -ggdb"
elif test "$with_debug" != ""
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	AC_SUBST(WITH_DEBUG) WITH_DEBUG=yes
	AC_MSG_NOTICE([Your specific kind of debug is $with_debug])
	AC_SUBST(DEBUG)
	AC_DEFINE_UNQUOTED([DEBUG],[$with_debug],[enable DEBUG])
	CFLAGS="-O0 -ggdb"

else
	AC_SUBST(WITH_DEBUG) WITH_DEBUG=no
	AC_MSG_NOTICE([No debug for you.])
	CFLAGS="-O2"
fi
])

AC_DEFUN([RFM_TRACE],
[
AC_ARG_WITH([trace], [AC_HELP_STRING([--with-trace],[Enable trace])])
if test "$with_trace" = "yes"
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	AC_MSG_NOTICE([You enabled trace. Good for you.])
	AC_SUBST(WITH_TRACE) WITH_TRACE=yes
	DEBUG_TRACE=yes
	AC_SUBST(DEBUG_TRACE)
	AC_DEFINE_UNQUOTED([DEBUG_TRACE],[$with_trace],[enable DEBUG_TRACE])
	TR=--with-trace
elif test "$with_trace" != ""
then
	AC_SUBST(WITH_CORE) WITH_CORE=yes
	AC_SUBST(CORE)
	AC_DEFINE_UNQUOTED([CORE],[$with_core],[enable CORE])
	AC_SUBST(WITH_TRACE) WITH_TRACE=yes
	DEBUG_TRACE=yes
	AC_SUBST(DEBUG_TRACE)
	AC_DEFINE_UNQUOTED([DEBUG_TRACE],[$with_trace],[enable DEBUG_TRACE])
	AC_MSG_NOTICE([Your specific kind of trace is $with_trace])
else
	AC_SUBST(WITH_TRACE) WITH_TRACE=no
	AC_SUBST(DEBUG_TRACE)
	AC_MSG_NOTICE([No trace for you.])
fi
])

AC_DEFUN([RFM_GNU_GREP],[
AC_MSG_CHECKING([for GNU grep])
GAWK_VERSION=`grep --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`

if test $INDEX -gt  0 ; then
    AC_MSG_RESULT([GNU grep found])
    GNU_GREP=1
    AC_SUBST(WITH_GNU_GREP) WITH_GNU_GREP=yes
    AC_SUBST(GNU_GREP)
    AC_DEFINE_UNQUOTED([GNU_GREP],[$GNU_GREP],[enable GNU grep])
else
    AC_SUBST(WITH_GNU_GREP) WITH_GNU_GREP=no
    AC_MSG_RESULT([grep is not GNU. Some features will be define by BSD!])
fi

])

AC_DEFUN([RFM_GNU_LS],
[
AC_MSG_CHECKING([for GNU ls])
GAWK_VERSION=`ls --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`

#   WITH_GNU_LS subst is for librfm-settings.in, configure script output.
#   GNU_LS subst is for .h.in preparations
if test $INDEX -gt  0 ; then
    AC_MSG_RESULT([GNU ls found])
    AC_SUBST(WITH_GNU_LS) WITH_GNU_LS=yes
    AC_SUBST(GNU_LS) GNU_LS="#define GNU_LS 1"
else
    AC_SUBST(GNU_LS) GNU_LS="/*#define GNU_LS */"
    AC_SUBST(WITH_GNU_LS) WITH_GNU_LS=no
    AC_MSG_RESULT([ls is not GNU. Some features will be define by BSD ls.])
fi
])

AC_DEFUN([RFM_GNU_CP],
[
AC_MSG_CHECKING([for GNU cp])
GAWK_VERSION=`cp --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`

if test $INDEX -gt  0 ; then
    AC_MSG_RESULT([GNU cp found])
    AC_SUBST(WITH_GNU_CP) WITH_GNU_CP=yes
    AC_SUBST(GNU_CP) GNU_CP="#define GNU_CP 1"
else
    AC_SUBST(WITH_GNU_CP) WITH_GNU_CP=no
    AC_SUBST(GNU_CP) GNU_CP="/*#define GNU_CP */"
    AC_MSG_RESULT([cp is not GNU. Some features will be define by BSD!])
fi
])

AC_DEFUN([RFM_GNU_MV],
[
AC_MSG_CHECKING([for GNU mv])
GAWK_VERSION=`mv --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`

if test $INDEX -gt  0 ; then
    AC_MSG_RESULT([GNU mv found])
    AC_SUBST(WITH_GNU_MV) WITH_GNU_MV=yes
    AC_SUBST(GNU_MV) GNU_MV="#define GNU_MV 1"
else
    AC_SUBST(WITH_GNU_MV) WITH_GNU_MV=no
    AC_SUBST(GNU_MV) GNU_MV="/*#define GNU_MV */"
    AC_MSG_RESULT([mv is not GNU. Some features will be define by BSD!])
fi
])

AC_DEFUN([RFM_GNU_LN],
[
AC_MSG_CHECKING([for GNU ln])
GAWK_VERSION=`ln --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`

if test $INDEX -gt  0 ; then
    AC_MSG_RESULT([GNU ln found])
    AC_SUBST(WITH_GNU_LN) WITH_GNU_LN=yes
    AC_SUBST(GNU_LN) GNU_LN="#define GNU_LN 1"
else
    AC_SUBST(WITH_GNU_LN) WITH_GNU_LN=no
    AC_SUBST(GNU_LN) GNU_LN="/*#define GNU_LN */"
    AC_MSG_RESULT([ln is not GNU. Some features will not be enabled!])
fi
])

AC_DEFUN([RFM_GNU_RM],
[
AC_MSG_CHECKING([for GNU rm])
GAWK_VERSION=`rm --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`
#echo "INDEX=$INDEX"

if test $INDEX -gt 0 ; then
    AC_MSG_RESULT([GNU rm found])
    AC_SUBST(WITH_GNU_RM) WITH_GNU_RM=yes
    AC_SUBST(GNU_RM) GNU_RM="#define GNU_RM 1"
else
    AC_SUBST(WITH_GNU_RM) WITH_GNU_RM=no
    AC_SUBST(GNU_RM) GNU_RM="/*#define GNU_RM */"
    AC_MSG_RESULT([rm is not GNU. Some features will not be enabled!])
fi
])

AC_DEFUN([RFM_GNU_SHRED],
[
AC_MSG_CHECKING([for GNU shred])
GAWK_VERSION=`shred --version`
INDEX=`expr "$GAWK_VERSION" : "[.*GNU]"`
#echo "INDEX=$INDEX"

if test $INDEX -gt 0 ; then
    AC_MSG_RESULT([GNU shred found])
    AC_SUBST(WITH_GNU_SHRED) WITH_GNU_SHRED=yes
    AC_SUBST(GNU_SHRED) GNU_SHRED="#define GNU_SHRED 1"
else
    AC_SUBST(WITH_GNU_SHRED) WITH_GNU_SHRED=no
    AC_SUBST(GNU_SHRED) GNU_SHRED="/*#define GNU_SHRED */"
    AC_MSG_RESULT([shred is not GNU. Some features will not be enabled!])
fi
])

AC_DEFUN([RFM_GNU_GHOSTSCRIPT],
[
AC_MSG_CHECKING([for GNU Ghostscript])
GS_VERSION=`gs --version`
if test x"$GS_VERSION" = x; then
    AC_SUBST(WITH_GNU_GS) WITH_GNU_GS=no
    AC_MSG_RESULT([GNU Ghostscript not found])
else
    AC_SUBST(WITH_GNU_GS) WITH_GNU_GS=yes
    AC_MSG_RESULT([Ghostscript found])
fi
])


AC_DEFUN([RFM_LINUX_OR_BSD],
[
AC_MSG_CHECKING(for Linux or BSD)
#echo "system: $ac_uname_s"
os=`uname -a | grep BSD`
if test x"$os" != x ; then
    os=`uname -a | grep DragonFly`
fi

if test x"$os" != x ; then 
    AC_MSG_RESULT([System is BSD.])
    rodentman1dir=${prefix}/man/man1
    AC_MSG_CHECKING(for gmake);
    result_gmake=`gmake --version`
    if test x"${result_gmake}" = x; then
        AC_MSG_ERROR([GNU make (gmake) not found! Please install and then rerun configure.])
    else 
        echo "$result_gmake"
    fi
    AC_DEFINE_UNQUOTED([THIS_IS_BSD], [1], [Define if system is BSD ])
# In FreeBSD gettext is in /usr/local
    CFLAGS="$CFLAGS -I/usr/local/include"
    LDFLAGS="$LDFLAGS  -L/usr/local/lib"
else 
   rodentman1dir=${mandir}/man1
   ac_uname_s=`uname -s`
   if test "$ac_uname_s" = "Linux" ; then
	AC_DEFINE_UNQUOTED([THIS_IS_LINUX], [1], [Define if system is Linux kernel and posix shared memory is at /dev/shm])
	AC_MSG_RESULT([System is Linux.])
   else
        AC_MSG_RESULT([Unknown system... mingw-w64?])
   fi
   CFLAGS="$CFLAGS"
   LDFLAGS="$LDFLAGS"
fi
ac_uname_s=`uname -s`
AC_SUBST(UNAME_S) UNAME_S=$ac_uname_s
AM_CONDITIONAL([WANT_FREEBSD], [test x"$os" != x ]) 
RFM_SET_PKG_CONF_DIR
RFM_CHECK_DISTRO
])

AC_DEFUN([RFM_MAKE_NOTICE],
[
echo 'CFLAGS:' $CFLAGS
echo 'LDFLAGS:' $LDFLAGS

if test "$ac_uname_s" = "Linux"; then
 echo 'Now do a make && sudo make install && sudo ldconfig'
else
 echo 'ATTENTION BSD user: Use gmake instead of make,'
 echo 'otherwise make process will halt with an error while building'
 echo 'translations.'
 echo '...'
 echo 'Now do a gmake && sudo gmake install && sudo ldconfig'
fi

])

AC_DEFUN([RFM_GNU_AWK],
[
AC_MSG_CHECKING([for GNU Awk])
GAWK_VERSION=`gawk --version`
INDEX=`expr "$GAWK_VERSION" : "[GNU Awk]"`
#echo "INDEX=$INDEX"
if test $INDEX -gt 0 ; then
    AC_MSG_RESULT([GNU gawk found])
else
    AC_MSG_ERROR([GNU gawk not found! Please install and then rerun configure.])
fi
])

AC_DEFUN([RFM_MAX_PREVIEW_THREADS],
[
AC_ARG_WITH([mpt], [AC_HELP_STRING([--with-mpt],[max preview threads (default=4)])])
if test "$with_mpt" != ""
then
	AC_SUBST(WITH_MPT) WITH_MPT=$with_mpt
	AC_DEFINE_UNQUOTED([MAX_PREVIEW_THREADS],[$with_mpt],[maximum preview threads])
	AC_MSG_NOTICE([Maximum preview threads set to $with_mpt])
else
	AC_SUBST(WITH_MPT) WITH_MPT=4
	AC_DEFINE_UNQUOTED([MAX_PREVIEW_THREADS],[4],[maximum preview threads])
	AC_MSG_NOTICE([Maximum preview threads set to 4])
fi
])

