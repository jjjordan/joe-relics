#!/bin/bash

# URL: http://ftp.fi.netbsd.org/pub/misc/archive/alt.sources/volume91/Sep/
# Args: 91 Sep 4

monthnum=$(date --date="01 $2" +"%m")

echo $monthnum

for i in `seq 1 99`; do
	URL=http://ftp.fi.netbsd.org/pub/misc/archive/alt.sources/volume$1/$2/$1${monthnum}$(printf "%02d" $3).$(printf "%02d" $i).gz
	wget $URL || exit $?
done
