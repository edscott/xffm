# - Try to find libtubo
# Once done, this will define
#
#  TUBO_FOUND - system has GTK+ 3.
#  TUBO_INCLUDE_DIRS - the GTK+ 3. include directories
#  TUBO_LIBRARIES - link these to use GTK+ 3.
#
# Copyright (C) 2018 edscott wilson garcia <edscott@xfce.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# If tubo is installed in alternate directory, make sure to set 
# the PKG_CONFIG_PATH environment variable.
find_package(PkgConfig)
pkg_check_modules(TUBO tubo)
pkg_get_variable(INCLUDEDIRS tubo includedir)
set(TUBO_INCLUDE_DIRS "${INCLUDEDIRS}")


set(VERSION_OK TRUE)
if (TUBO_VERSION)
    if (TUBO_FIND_VERSION_EXACT)
        if (NOT("${TUBO_FIND_VERSION}" VERSION_EQUAL "${TUBO_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${TUBO_VERSION}" VERSION_LESS "${TUBO_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TUBO TUBO_INCLUDE_DIRS TUBO_LIBRARIES VERSION_OK)
