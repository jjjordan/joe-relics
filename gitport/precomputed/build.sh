#!/bin/sh

# cvs2git (cvs2svn) has many VERY outdated dependencies and output that is
# rather unlikely to change.  It is becoming increasingly difficult to run
# on modern systems and the current recommended approach is to use Docker. 
# While I always have this installed, it can be cumbersome, especially in
# Makefiles, so for now we will just commit the output.  This script can
# rebuild it in the unlikely event that it is needed, or at least show how
# the sausage is made ...

cd $(dirname $0)
LOC=$(pwd)

#cd ../tools/cvs2svn
#make docker-image || exit $?

rm -rf tmp tmp2
mkdir -p tmp tmp2

cd $LOC
docker run -it --rm \
	-v $LOC/tmp:/git \
	-v $LOC/../../collection/cvs:/cvs \
marcosmike/cvs2git cvs2git \
	--blobfile=/git/cvs-blobs.txt \
	--dumpfile=/git/cvs-cmds.txt \
	--username=cvs2git \
	/cvs/joe-current || exit $?

# Copy files from tmp->tmp2 to shed root user
cp tmp/cvs-*.txt tmp2 || exit $?
rm -rf tmp

cd tmp2
tar czf ../cvs-data.new.tgz cvs-*.txt || exit $?

cd $LOC
rm -rf cvs-data.tgz tmp2
mv cvs-data.new.tgz cvs-data.tgz
