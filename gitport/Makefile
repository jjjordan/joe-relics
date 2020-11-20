all: out/full-export.txt
	rm -rf out/git
	mkdir -p out/git
	cd out/git && git init --bare && git fast-import --export-marks=../git-marks.txt <../full-export.txt && git gc

clean:
	rm -rf out/historic.txt out/cvs-cmds.txt out/cvs-blobs.txt out/cvs-fixed.txt out/hg-exp.txt out/hg-fixed.txt out/full-export.txt out/git

out/joe-editor-hg:
	mkdir -p out
	hg clone http://hg.code.sf.net/p/joe-editor/mercurial out/joe-editor-hg

out:
	mkdir -p out

out/dummy:
	mkdir -p out/dummy
	cd out/dummy && git init --bare

env: requirements.txt
	python3 -m venv env
	env/bin/pip install -r requirements.txt

env2: requirements2.txt
	virtualenv2 env2
	env2/bin/pip install -r requirements2.txt

out/historic.txt: oldmeta.yaml mkhistory.py util.py | env out
	env/bin/python mkhistory.py ./oldmeta.yaml .. out/historic.txt

out/cvs-cmds.txt: Makefile | out
	python2 tools/cvs2svn/cvs2git --blobfile=out/cvs-blobs.txt --dumpfile=out/cvs-cmds.txt ../collection/cvs/joe-current

# TODO:
#  - Translate names
#  - Translate tags
out/hg-exp.txt: out/joe-editor-hg plugins/crlf/__init__.py plugins/sfbugs/__init__.py Makefile | out/dummy out env2
	rm -f out/hg-mapping.txt out/hg-heads.txt out/hg-status.txt out/hg-marks.txt out/hg-exp.txt
	cd out/dummy && ../../env2/bin/python ../../tools/hg-fast-export/hg-fast-export.py \
		-r ../joe-editor-hg \
		--mapping=../hg-mapping.txt \
		--heads=../hg-heads.txt \
		--status=../hg-status.txt \
		--marks=../hg-marks.txt \
		--plugin-path=../../plugins \
		--plugin=crlf \
		--plugin=sfbugs=SF- >../hg-exp.txt

out/cvs-fixed.txt: out/cvs-cmds.txt fixcvs.py util.py Makefile | env out
	env/bin/python fixcvs.py out/cvs-blobs.txt out/cvs-cmds.txt out/cvs-fixed.txt

out/hg-fixed.txt: out/hg-exp.txt fixhg.py util.py Makefile | env out
	env/bin/python fixhg.py out/hg-exp.txt out/hg-fixed.txt

out/full-export.txt: out/historic.txt out/cvs-fixed.txt out/hg-fixed.txt fuse.py util.py Makefile | env out
	env/bin/python fuse.py out/historic.txt out/cvs-fixed.txt out/hg-fixed.txt out/full-export.txt | tee out/export-log.txt
