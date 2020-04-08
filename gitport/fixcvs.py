#!/usr/bin/env python
# This will do a couple three things:
# - Remove the first three commands
# - Translate bare SF usernames => realnames+emails
# - Fork . and main/ at "coroutines" revision
#   - Duplicate further commits where both are touched

import util

from fastimport.parser import ImportParser
from fastimport.helpers import repr_bytes

BLOB_BASE_MARK = 200000
BASE_MARK = 300000

def main(blobs, cmds, outfile):
    t = util.Tracker()
    out = []
    cmk = BASE_MARK
    
    with open(blobs, 'rb') as f:
        p = ImportParser(f)
        for c in p.iter_commands():
            fix_blobmark(c)
            t.feed(c)
            out.append(c)

    with open(cmds, 'rb') as f:
        p = ImportParser(f)
        l = list(p.iter_commands())
        
        # Ignore the first 3 commands (false start)
        i = 2
        for c in l[3:]:
            fix_blobmark(c)
            i += 1
            if c.name == b'commit':
                fix_person(c)
            # Track everything until we hit the coroutines commit.
            if c.name == b'commit' and c.message == b'semiautomatic variables\n':
                break
            t.feed(c)
            out.append(c)
        
        # Branch coroutine/
        bc = l[i]
        #fix_blobmark(bc) - this already happened above!
        #fix_person(bc)
        bc.ref = b'refs/heads/coroutine'
        bc.from_ = b':' + t.last.commit.mark
        t.feed(bc)
        out.append(bc)
        
        # Here begins the madness.  We need to fork commits to master and coroutine.
        for c in l[i+1:]:
            fix_blobmark(c)
            if c.name == b'commit':
                fix_person(c)
                coro, mstr = fork_commit(c)
                if mstr is not None:
                    mstr = util.rebase(t, mstr)
                    if mstr is not None:
                        mstr.mark = str(cmk).encode('utf-8')
                        cmk += 1
                        t.feed(mstr)
                        out.append(mstr)
                if coro is not None:
                    coro = util.rebase(t, coro)
                    if coro is not None:
                        coro.mark = str(cmk).encode('utf-8')
                        cmk += 1
                        t.feed(coro)
                        out.append(coro)
            elif c.name == b'reset':
                if fix_tag(c):
                    out.append(c)
            else:
                out.append(c)
    
    with open(outfile, 'wb') as f:
        for c in out:
            f.write(repr_bytes(c))
            f.write(b'\n')
    
    return True

def fix_person(c):
    person = c.committer[0:2]
    tm = c.committer[2:]
    if person[0] == b'jhallen':
        person = (b"Joseph H. Allen", b"jhallen@world.std.com")
    elif person[0] == b'vsamel':
        person = (b"Vitezslav Samel", b"samel@mail.cz")
    elif person[0] == b'marx_sk':
        person = (b"Marek 'Marx' Grac", b"xgrac@fi.muni.cz")
    elif person[0] == b'polesapart':
        person = (b"Alexandre P. Nunes", b"alex@PolesApart.dhs.org")
    elif person[0] == b'electrum':
        person = (b"David Phillips", b"electrum@users.sf.net")
    elif person[0] == b'sonic_amiga':
        person = (b"Pavel Fedin", b"sonic_amiga@rambler.ru")
    elif person[0] == b'shallot':
        person = (b"Josip Rodin", b"shallot@users.sf.net")
    
    c.committer = person + tm

def fix_blobmark(cmd):
    if cmd.name == b'blob':
        cmd.mark = translate_mark(cmd.mark)
    elif cmd.name == b'commit':
        for fi in cmd.file_iter:
            if fi.name == b'filemodify':
                fi.dataref = translate_mark(fi.dataref)

def fix_tag(cmd):
    # TODO: Fix tag names!
    return True

def translate_mark(mk):
    if not mk:
        return mk
    if mk.startswith(b':'):
        return b':' + str(int(mk[1:].decode('utf-8')) + BLOB_BASE_MARK).encode('utf-8')
    else:
        return str(int(mk.decode('utf-8')) + BLOB_BASE_MARK).encode('utf-8')

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
