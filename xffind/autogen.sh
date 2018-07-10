#!/bin/sh
package="fgr+"

echo "Creating system specific build files for $package..."

#echo "** libtoolize -icf ***"
 libtoolize -f -c >/dev/null; 
#echo "** aclocal -I m4 ***"
 aclocal -I m4 >/dev/null;
#echo "** autoheader ***"
 autoheader >/dev/null;
#echo "** autoconf ***"
 autoconf >/dev/null;
#echo "** automake --add-missing ***"
 automake --add-missing --copy >/dev/null;
 if test x"$1" = "x" ; then
   echo "You may now execute ./configure for $package"
 else
   ./configure $1 $2 $3 $4 $5 $6 $7 $8 $9
 fi

 exit 0
