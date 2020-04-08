#!/bin/sh
# Example taken from fast-export docs.
# $1 = pathname of exported file relative to the root of the repo
# $2 = Mercurial's hash of the file
# $3 = "1" if Mercurial reports the file as binary, otherwise "0"

if [ "$3" == "1" ]; then
	cat
else
	# Mercurial doesn't seem to reliably set the binary bit here, so check
	# extensions too -JJ
	case $1 in
		*.gif | *.ico | *.png)
			cat
			;;
		
		*)
			dos2unix -f
			;;
	esac
fi
