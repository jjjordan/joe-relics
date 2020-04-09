#!/usr/bin/env python

import dateutil.parser
import email.parser
import hashlib
import jinja2
import os
import stat
import yaml

from fastimport.commands import BlobCommand, CommitCommand, FileModifyCommand, FileRenameCommand, FileDeleteCommand, TagCommand
from fastimport.helpers import repr_bytes
from dateutil import tz
from Levenshtein import distance
from util import diff_score, readfile

MIN_DIFF_SCORE = 0.8
BLOB_BASE_MARK = 100000
COMMIT_BASE_MARK = 200000

def main(spec, rootpath, outfile):
    with open(spec, 'r') as f:
        y = yaml.safe_load(f)
    
    versions = [Version("..", v) for v in y['versions']]
    lastset = FileSet(None)
    files = {}
    cmds = []

    hfiles = HistoricalFiles(".")
    mk = COMMIT_BASE_MARK
    
    for i, v in enumerate(versions):
        if v.fileset is not None:
            hfiles.apply_files(v.fileset, versions[:i+1])
            cmds.extend(apply_changes(v, lastset, files, mk))
            mk += 1
            lastset = v.fileset
    
    # Write it out
    with open(outfile, 'wb') as f:
        for c in cmds:
            f.write(repr_bytes(c))
            f.write(b'\n')
    
    return True

def apply_changes(version, prev, files, mark):
    adds, removes, changes = diff_filesets(prev, version.fileset)
    find_renames(adds, removes, changes)
    
    # Write out files
    for f in adds + [c[1] for c in changes]:
        if f.hash not in files:
            # Generate a new id
            id = str(len(files) + BLOB_BASE_MARK).encode('utf-8')
            files[f.hash] = b':' + id
            
            yield BlobCommand(id, f.read())
    
    filecmds = []
    filecmds.extend(FileModifyCommand(f.repopath.encode('utf-8'), f.mode, files[f.hash], None) for f in adds)
    filecmds.extend(FileRenameCommand(f.repopath.encode('utf-8'), g.repopath.encode('utf-8')) for f, g in changes if f.repopath != g.repopath)
    filecmds.extend(FileModifyCommand(g.repopath.encode('utf-8'), g.mode, files[g.hash], None) for f, g in changes if f.hash != g.hash)
    filecmds.extend(FileDeleteCommand(f.repopath.encode('utf-8')) for f in removes)
    
    # Build the commit ...
    yield CommitCommand(
        mark=str(mark).encode('utf-8'),
        ref=b"refs/heads/master",
        author=get_author(version),
        committer=get_committer(version),
        message=version.fileset.commitmsg,
        from_=None,
        merges=None,
        file_iter=filecmds)
    
    yield TagCommand(
        id="releases/{}-{}".format(version.name.lower(), version.version).encode('utf-8'),
        from_=':{}'.format(mark).encode('utf-8'),
        tagger=get_author(version),
        message=version.changelog.encode('utf-8') if version.changelog is not None else b'')

def diff_filesets(prev, cur):
    prevfiles = {f.repopath: f for f in prev.files}
    curfiles = {f.repopath: f for f in cur.files}
    adds = []
    removes = []
    changes = []
    for f in cur.files:
        if f.repopath in prevfiles:
            o = prevfiles[f.repopath]
            if o.hash != f.hash:
                changes.append((o, f))
        else:
            adds.append(f)
    for f in prev.files:
        if f.repopath not in curfiles:
            removes.append(f)

    return adds, removes, changes

def find_renames(adds, removes, changes):
    # Score candidate moves
    poss = []
    if len(removes) > 0 and len(adds) > 0:
        print("Searching for renames from {} to {}".format(repr([f.repopath for f in removes]), repr([f.repopath for f in adds])))

    for r in removes:
        for a in adds:
            sc = diff_score(r.read(), a.read())
            if sc > MIN_DIFF_SCORE:
                poss.append((sc, r, a))

    poss.sort(key=lambda x: (x[0], -distance(x[1].repopath, x[2].repopath)))
    poss.reverse()

    # Convert remove+add => rename
    for score, rf, af in poss:
        if rf in removes and af in adds:
            print("Detected file rename: {} => {} [{}]".format(rf.repopath, af.repopath, score))
            adds.remove(af)
            removes.remove(rf)
            changes.append((rf, af))
        else:
            print("Rejected file rename (preempted): {} => {} [{}]".format(rf.repopath, af.repopath, score))

def get_author(version):
    # Format: name,email,secs-since-epoch,utc-offset-secs
    if version.msg is None:
        return (b"Joseph H. Allen", b"jhallen@world.std.com") + to_timefmt(get_time(version))
    
    fr = version.msg.get("From")
    paren = fr.index("(")
    lparen = fr.rindex(")")
    
    return (fr[paren+1:lparen].strip().encode("utf-8"), fr[:paren].strip().encode("utf-8")) + to_timefmt(get_time(version))

def get_committer(version):
    # Format: name,email,secs-since-epoch,utc-offset-secs
    return (b"Joe Allen", b"jhallenworld@gmail.com") + to_timefmt(get_time(version))

def get_time(version):
    eastern = tz.gettz("America/New_York")
    if version.msg is None:
        dt = dateutil.parser.parse("{} 12:00:00".format(version.date))
        return dt.replace(tzinfo=eastern)
    
    return dateutil.parser.parse(version.msg.get("Date")).astimezone(eastern)

def to_timefmt(dt):
    off = dt.utcoffset()
    offs = int(off.total_seconds()) if off is not None else 0
    return int(dt.timestamp()), offs

class Version:
    def __init__(self, rootpath, v):
        self.name = v.get('name', 'JOE')
        self.version = v['version']
        self.date = v.get('date', None)
        self.source = v.get('source', None)
        self.path = v.get('path', None)
        if self.path is not None:
            self.path = os.path.join(rootpath, self.path)
        self.comments = readfile(v.get('comments', None), rootpath)
        self.announce = readfile(v.get('announce', None), rootpath)
        self.changelog = readfile(v.get('changelog', None), rootpath)
        self.filedates = readfile(v.get('filedates', None), rootpath)
        self.note = v.get('note', None)
        self.fileset = FileSet(self.path) if self.path is not None else None
        self.msg = None
        if self.announce is not None:
            p = email.parser.FeedParser()
            p.feed(self.announce)
            self.msg = p.close()

class FileSet:
    def __init__(self, path):
        self.path = path
        self.files = []
        self.commitmsg = None
        
        if path is not None:
            for root, dirs, files in os.walk(path):
                for f in files:
                    filepath = os.path.join(root, f)
                    repopath = os.path.relpath(filepath, path)
                    self.files.append(File(repopath, filepath))

class File:
    def __init__(self, repopath, path):
        self.repopath = repopath
        self.path = path
        
        if path is not None:
            st = os.stat(path)
            self.mode = 0o100644 if not (st.st_mode & stat.S_IEXEC) else 0o100755
            
            with open(path, 'rb') as f:
                self.set(f.read())
        else:
            self.data = None
            self.hash = ''
            self.mode = 0o100644
    
    def read(self):
        return self.data
    
    def set(self, data):
        self.data = data
        h = hashlib.sha256()
        h.update(self.data)
        self.hash = h.hexdigest()

class HistoricalFiles:
    def __init__(self, tpath):
        self.env = jinja2.Environment(loader=jinja2.FileSystemLoader(tpath))
        self.env.filters['rtrim'] = str.rstrip
    
    def apply_files(self, fileset, versions):
        hist_f = File(".historical/README.md", None)
        hist_f.set(self.env.get_template("templates/README.md").render(versions=versions).encode('utf-8'))
        fileset.files.append(hist_f)
        
        ts_f = File(".historical/timestamps.md", None)
        ts_f.set(self.env.get_template("templates/timestamps.md").render(version=versions[-1]).encode('utf-8'))
        fileset.files.append(ts_f)
        
        fileset.commitmsg = self.env.get_template("templates/commit").render(version=versions[-1]).encode('utf-8')

if __name__ == '__main__':
    import sys
    res = main(*sys.argv[1:])
    if type(res) == int:
        sys.exit(res)
    else:
        sys.exit(0 if res else 1)
