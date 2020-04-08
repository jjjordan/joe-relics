import difflib
import os

from fastimport.commands import FileDeleteCommand, FileRenameCommand
from Levenshtein import distance

MIN_DIFF_SCORE = 0.8

class Tracker:
    def __init__(self):
        self.blobs = {}
        self.commits = {}
        self.branches = {}
        self.last = None
    
    def feed(self, cmd):
        if cmd.name == b'blob':
            self.blobs[cmd.mark] = cmd
        elif cmd.name == b'commit':
            if cmd.from_ is not None:
                br = self.commits[cmd.from_.lstrip(b':')].fork()
            elif cmd.ref in self.branches:
                br = self.branches[cmd.ref].fork()
            else:
                br = BranchState()
            br.apply(cmd, self.blobs)
            br.commit = cmd
            self.last = br
            self.branches[cmd.ref] = br
            if cmd.mark is not None:
                self.commits[cmd.mark] = br
        elif cmd.name == b'reset':
            if cmd.from_ is not None:
                br = self.commits[cmd.from_.lstrip(b':')].fork()
            else:
                br = self.last.fork()
            self.branches[cmd.ref] = br
            # XXX: self.last = br ???

class BranchState:
    def __init__(self):
        self.files = {}
        self.commit = None
    
    def apply(self, commit, blobs):
        for f in commit.iter_files():
            if f.name == b'filemodify':
                st = self.files[f.path] if f.path in self.files else None
                if st == None:
                    st = FileState(f.path)
                    self.files[f.path] = st
                st.mode = f.mode
                if f.dataref is not None:
                    ref = f.dataref.lstrip(b':')
                    st.blobref = ref
                    st.contents = blobs[ref].data
                elif f.data is not None:
                    st.contents = f.data
                else:
                    print("Warning: Found a file with no contents")
                st.modify = f
            elif f.name == b'filedelete':
                st = self.files[f.path] if f.path in self.files else None
                if st is not None:
                    del self.files[f.path]
                else:
                    print("Warning: found file delete for untracked file [{}] in {} mark {}".format(f.path.decode('utf-8'), commit.ref.decode('utf-8'), commit.mark.decode('utf-8')))
            elif f.name == b'filecopy':
                st = self.files[f.src_path] if f.src_path in self.files else None
                if st is not None:
                    self.files[f.dest_path] = st.copy(f.dest_path)
                else:
                    print("Warning: found file copy for untracked file")
            elif f.name == b'filerename':
                st = self.files[f.old_path] if f.old_path in self.files else None
                if st is not None:
                    del self.files[f.old_path]
                    self.files[f.new_path] = st
                    st.path = f.new_path
                else:
                    print("Warning: found file rename for untracked file")
            elif f.name == b'deleteall':
                self.files = {}

    def fork(self):
        b = BranchState()
        for k, v in self.files.items():
            b.files[k] = v.copy(k)
        return b

class FileState:
    def __init__(self, path):
        self.path = path
        self.modify = None
        self.mode = None
        self.contents = None
        self.blobref = None
    
    def copy(self, path):
        res = FileState(path)
        res.modify = self.modify
        res.mode = self.mode
        res.contents = self.contents
        res.blobref = self.blobref
        return res

def rebase(tr, cmt):
    if cmt.from_ is not None:
        base = tr.commits[cmt.from_.lstrip(b':')]
    else:
        base = tr.branches[cmt.ref]

    nxt = base.fork()
    nxt.apply(cmt, tr.blobs)
    return rebase_branch(base, nxt, cmt)

def rebase_branch(base, nxt, cmt):
    # Diff...
    adds = []
    changes = []
    deletes = []
    
    for newpath, newfile in nxt.files.items():
        if newpath in base.files:
            oldfile = base.files[newpath]
            if oldfile.contents != newfile.contents or oldfile.mode != newfile.mode:
                changes.append(newfile)
        else:
            adds.append(newfile)
    for oldpath, oldfile in base.files.items():
        if oldpath not in nxt.files:
            deletes.append(oldfile)
    
    # Detect renames
    renames = find_renames(adds, deletes, changes)
    fcmds = []
    fcmds.extend(FileRenameCommand(d.path, a.path) for d, a in renames)
    fcmds.extend(f.modify for f in changes)
    fcmds.extend(f.modify for f in adds)
    fcmds.extend(FileDeleteCommand(f.path) for f in deletes)
    
    if len(fcmds) == 0:
        return None

    return cmt.copy(file_iter=fcmds)

def find_renames(adds, deletes, changes):
    poss = []
    if len(deletes) > 0 and len(adds) > 0:
        print("Searching for renames from {} to {}".format(repr([f.path for f in deletes]), repr([f.path for f in adds])))
    
    for d in deletes:
        for a in adds:
            score = diff_score(d.contents, a.contents)
            if score > MIN_DIFF_SCORE:
                poss.append((score, d, a))
    
    poss.sort(key=lambda x: (x[0], -distance(x[1].path, x[2].path)))
    poss.reverse()
    
    renames = []
    for score, d, a in poss:
        if d in deletes and a in adds:
            print("Detected file rename: {} => {} [{}]".format(d.path, a.path, score))
            adds.remove(a)
            deletes.remove(d)
            renames.append((d, a))
            if d.contents != a.contents:
                changes.append(a)
        else:
            print("Rejected file rename (preempted): {} => {} [{}]".format(d.path, a.path, score))
    
    return renames

def diff_score(a, b):
    la = a.splitlines(keepends=True)
    lb = b.splitlines(keepends=True)
    d = difflib.diff_bytes(difflib.context_diff, la, lb)
    sc = 0
    for ln in d:
        if ln.startswith(b'! ') or ln.startswith(b'+ ') or ln.startswith(b'- '):
            sc += 1
    return 1 - (sc / (len(la) + len(lb)))

def readfile(name, rootpath='.'):
    if name is None:
        return None
    
    if ':' not in name:
        with open(os.path.join(rootpath, name), 'r') as f:
            return f.read()
    
    path, lines = name.split(':')
    st, en = lines.split('-')
    start, end = int(st), int(en)
    with open(os.path.join(rootpath, path), 'r') as f:
        result = [line for i, line in enumerate(list(f)) if i >= (start - 1) and i < end]
    
    return ''.join(result)
