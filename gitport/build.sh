#!/bin/sh

cd $(dirname $0)

if [ ! -d ./env ]; then
	python3 -m venv env
	pip install -r requirements.txt
fi

env/bin/python mkhistory.py ./oldmeta.yaml .. oldhist-fast-export.txt || exit $?

exit 0

# cvs2git
# hg2git

env/bin/python merge.py oldhist-fast-export.txt cvs.txt hg.txt >fullhist.txt

if [ -d ./gitout ]; then
	rm -rf ./gitout
fi

cd gitout
git init --bare

git fast-import <../fullhist.txt
