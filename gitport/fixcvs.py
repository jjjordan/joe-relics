#!/usr/bin/env python
# This will do a couple three things:
# - Squash together the first 3 commits (wherein they're figuring out the repo)
# - Translate bare SF usernames => realnames+emails
# - Fork . and main/ at "coroutines" revision
#   - Duplicate further commits where both are touched

import util

from fastimport.parser import ImportParser
from fastimport.helpers import repr_bytes

def main(blobs, cmds, outfile):
    t = util.Tracker()
    out = []
    with open(blobs, 'rb') as f:
        p = ImportParser(f)
        for c in p.iter_commands():
            t.feed(c)
            out.append(c)

    with open(cmds, 'rb') as f:
        p = ImportParser(f)
        l = list(p.iter_commands())
        
        # Ignore the first 3 commands (false start)
        i = 2
        for c in l[3:]:
            i += 1
            # Track everything until we hit the coroutines commit.
            if c.name == b'commit' and c.message == b'semiautomatic variables\n':
                break
            if c.name == b'commit':
                fix_commit(c)
            t.feed(c)
            out.append(c)
        
        # Branch coroutine/
        bc = l[i]
        fix_commit(bc)
        bc.ref = b'refs/heads/coroutine'
        bc.from_ = b':' + t.last.commit.mark
        t.feed(bc)
        out.append(bc)
        
        # Here begins the madness.  We need to fork commits to master and coroutine.
        for c in l[i+1:]:
            if c.name == b'commit':
                fix_commit(c)
                coro, mstr = fork_commit(c)
                if mstr is not None:
                    mstr = util.rebase(t, mstr)
                    if mstr is not None:
                        t.feed(mstr)
                        out.append(mstr)
                if coro is not None:
                    coro = util.rebase(t, coro)
                    if coro is not None:
                        t.feed(coro)
                        out.append(coro)
            else:
                out.append(c)
    
    with open(outfile, 'wb') as f:
        for c in out:
            f.write(repr_bytes(c))
            f.write(b'\n')
    
    return True

def fix_commit(c):
    # Translate names...
    pass

def fork_commit(c):
    coro = c.copy()
    mstr = c.copy()
    
    # I'm getting away with an awful lot here: the CVS output does not contain any
    # 'from' past the coroutines commit, so rather than duplicate and track marks
    # I'm just going to remove them and rely on branch state.
    coro.mark = None
    mstr.mark = None

    coro.ref = b'refs/heads/coroutine'
    mstr.ref = b'refs/heads/master'
    
    coro.file_iter = [f for f in coro.iter_files() if not file_name(f).startswith(b'main/')]
    mstr.file_iter = [strip_main(f) for f in mstr.iter_files() if file_name(f).startswith(b'main/')]
    
    if len(coro.file_iter) == 0:
        coro = None
    if len(mstr.file_iter) == 0:
        mstr = None

    return coro, mstr

def file_name(fcmd):
    if fcmd.name == b'filemodify' or fcmd.name == b'filedelete':
        return fcmd.path
    if fcmd.name == b'filecopy':
        return fcmd.dest_path
    if fcmd.name == b'filerename':
        return fcmd.new_path

def strip_main(fcmd):
    if fcmd.name == b'filemodify' or fcmd.name == b'filedelete':
        fcmd.path = fcmd.path[5:]
    if fcmd.name == b'filecopy':
        fcmd.src_path = fcmd.src_path[5:]
        fcmd.dest_path = fcmd.dest_path[5:]
    if fcmd.name == b'filerename':
        fcmd.old_path = fcmd.old_path[5:]
        fcmd.new_path = fcmd.new_path[5:]
    return fcmd

if __name__ == '__main__':
    import sys
    res = main(*sys.argv[1:])
    if type(res) == int:
        sys.exit(res)
    else:
        sys.exit(0 if res else 1)
