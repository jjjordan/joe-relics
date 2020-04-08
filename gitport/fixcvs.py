#!/usr/bin/env python
# This will do a couple three things:
# - Remove the first three commands
# - Translate bare SF usernames => realnames+emails
# - Shift mark index so marks don't collide with hg output
# - Fork . and main/ at "coroutines" revision
#   - Duplicate further commits where both are touched
# - Create annotated tags for each release

import util

from fastimport.parser import ImportParser
from fastimport.commands import TagCommand
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
            i += 1

            fix_blobmark(c)
            if c.name == b'commit':
                fix_person(c)

            # Track everything until we hit the coroutines commit.
            if c.name == b'commit' and c.message == b'semiautomatic variables\n':
                break

            t.feed(c)

            # Look for untagged releases
            if c.name == b'commit':
                out.append(c)
                tag = find_release_tag(c)
                if tag is not None:
                    out.append(tag)
            elif c.name == b'reset':
                tag = fix_tag(c, t)
                if tag is not None:
                    out.append(tag)
            else:
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

                        # Look for untagged releases
                        tag = find_release_tag(mstr)
                        if tag is not None:
                            out.append(tag)
                if coro is not None:
                    coro = util.rebase(t, coro)
                    if coro is not None:
                        coro.mark = str(cmk).encode('utf-8')
                        cmk += 1
                        t.feed(coro)
                        out.append(coro)
            elif c.name == b'reset':
                tag = fix_tag(c, t)
                if tag is not None:
                    out.append(tag)
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

def fix_tag(cmd, tracker):
    if cmd.ref not in tagxlate:
        print(cmd.ref)
        return None

    version = tagxlate[cmd.ref]
    changelog = b''
    if version in changelogs:
        changelog = util.readfile(changelogs[version]).encode('utf-8')
    
    commit = tracker.commits[cmd.from_.lstrip(b':')].commit
    author = commit.author or commit.committer
    
    return TagCommand(
        id=releasepfx + version.encode('utf-8'),
        from_=cmd.from_,
        tagger=author,
        message=changelog)

def find_release_tag(commit):
    if commit.committer is not None and commit.committer[2] in committags:
        version = committags[commit.committer[2]]
        changelog = b''
        if version in changelogs:
            changelog = util.readfile(changelogs[version]).encode('utf-8')
        
        return TagCommand(
            id=releasepfx + version.encode('utf-8'),
            from_=b':' + commit.mark,
            tagger=commit.author or commit.committer,
            message=changelog)

releasepfx = b'refs/tags/releases/'

tagxlate = {
    b'refs/tags/joe_2_9': 'joe-2.9',
    b'refs/tags/joe_2_9_1': 'joe-2.9.1',
    b'refs/tags/joe_2_9_2': 'joe-2.9.2',
    b'refs/tags/joe_2_9_4': 'joe-2.9.4',
    b'refs/tags/joe_2_9_5': 'joe-2.9.5',
    b'refs/tags/joe_2_9_6': 'joe-2.9.6',
    b'refs/tags/joe_2_9_7_pre2': 'joe-2.9.7-pre2',
    b'refs/tags/joe_2_9_7_pre3': 'joe-2.9.7-pre3',
    b'refs/tags/joe_2_9_7': 'joe-2.9.7',
    b'refs/tags/joe_2_9_8_pre1': 'joe-2.9.8.pre1',
    b'refs/tags/joe_2_9_8': 'joe-2.9.8',
    b'refs/tags/joe_3_0': 'joe-3.0',
    b'refs/tags/joe_3_1': 'joe-3.1',
    b'refs/tags/joe-3_4': 'joe-3.4',
    b'refs/tags/joe_3_5': 'joe-3.5',
}

committags = {
    1225670707: 'joe-3.7',
    1225072633: 'joe-3.6',
    1116368961: 'joe-3.3',
    1111461759: 'joe-3.2',
}

changelogs = {
    "joe-3.7": "NEWS.cvs:1-58",
    "joe-3.6": "NEWS.cvs:62-155",
    "joe-3.5": "NEWS.cvs:159-182",
    "joe-3.4": "NEWS.cvs:186-278",
    "joe-3.3": "NEWS.cvs:282-301",
    "joe-3.2": "NEWS.cvs:305-370",
    "joe-3.1": "NEWS.cvs:374-427",
    "joe-3.0": "NEWS.cvs:430-441",
    "joe-2.9.8": "NEWS.cvs:444-451",
    "joe-2.9.8-pre1": "NEWS.cvs:454-461",
    "joe-2.9.7": "NEWS.cvs:464-466",
    "joe-2.9.7-pre3": "NEWS.cvs:469-473",
    "joe-2.9.7-pre2": "NEWS.cvs:476-480",
    "joe-2.9.7-pre1": "NEWS.cvs:483-488",
    "joe-2.9.6": "NEWS.cvs:498-520",
    "joe-2.9.5": "NEWS.cvs:524-527",
    "joe-2.9.4": "NEWS.cvs:531-533",
    "joe-2.9": "NEWS.cvs:537-537",
}

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
