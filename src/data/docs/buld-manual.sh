#!/usr/bin/bash
#
# Build the man pages
#
cd ${0%/*}

make html 
make latexpdf
