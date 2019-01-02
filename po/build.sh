#!/bin/bash
pwd
cd po
pwd
echo "Installing po with Makefile.autotools"
make install --makefile=Makefile.autotools
