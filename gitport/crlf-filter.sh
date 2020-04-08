#!/bin/sh
# Example taken from fast-export docs.
# $1 = pathname of exported file relative to the root of the repo
# $2 = Mercurial's hash of the file
# $3 = "1" if Mercurial reports the file as binary, otherwise "0"

if [ "$3" == "1" ]; then cat; else dos2unix; fi
