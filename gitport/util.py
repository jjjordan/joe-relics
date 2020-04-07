from fastimport.commands import FileDeleteCommand

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
                    print("Warning: found file delete for untracked file")
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
    
    # Diff...
    fcmds = []
    for newpath, newfile in nxt.files.items():
        if newpath in base.files:
            oldfile = base.files[newpath]
            if oldfile.contents != newfile.contents or oldfile.mode != newfile.mode:
                fcmds.append(newfile.modify)
        else:
            fcmds.append(newfile.modify)
    for oldpath, oldfile in base.files.items():
        if oldpath not in nxt.files:
            fcmds.append(FileDeleteCommand(oldpath))
    # Not doing renames :-\
    
    if len(fcmds) == 0:
        return None

    return cmt.copy(file_iter=fcmds)
