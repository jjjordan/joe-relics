#!/usr/bin/env python3
# This will:
# - Squash together the first 7 commits to maintain a clean break from CVS.
# - Make nice annotated tags for each release.
# - Convert author names (I don't like how hg-fast-export handles this).

import util
import authors

from fastimport.commands import CommitCommand, FileModifyCommand, TagCommand
from fastimport.parser import ImportParser
from fastimport.helpers import repr_bytes

def main(hgin, hgout):
    with open(hgin, 'rb') as f:
        p = ImportParser(f)
        l = list(p.iter_commands())
    
    cutoff = 6
    
    t = util.Tracker()
    for c in l[:cutoff+1]:
        t.feed(c)
    
    # Cobble together a new commit.
    last = l[cutoff]
    commit = CommitCommand(
        ref=last.ref,
        mark=last.mark,
        author=last.author,
        committer=last.committer,
        message=last.message,
        from_=None,
        merges=None,
        file_iter=[FileModifyCommand(p, f.mode, f.modify.dataref, f.modify.data) for p, f in t.last.files.items()])

    fix_authors(commit)
    
    # Still need to make annotated tags + fix authors.
    with open(hgout, 'wb') as f:
        f.write(repr_bytes(commit))
        f.write(b'\n')
        for c in l[cutoff+1:]:
            if c.name == b'reset':
                c = fix_tag(c, t)
            if c.name == b'commit':
                fix_authors(c)

            if c is not None:
                t.feed(c)
                f.write(repr_bytes(c))
                f.write(b'\n')
    
    return True

def fix_tag(c, t):
    if c.ref.startswith(b'refs/tags/joe-'):
        suffix = ''
        version = c.ref[14:].decode('utf-8')
    elif c.ref.startswith(b'refs/tags/windows-'):
        suffix = '-windows'
        version = c.ref[18:].decode('utf-8')
    else:
        return None
    
    changelog = b''
    if version in changelogs:
        changelog = util.readfile(changelogs[version]).encode('utf-8')
    
    commit = t.commits[c.from_.lstrip(b':')].commit
    author = commit.author or commit.committer
    
    return TagCommand(
        id="releases/joe-{}{}".format(version, suffix).encode('utf-8'),
        from_=c.from_,
        tagger=author,
        message=changelog)

def fix_authors(c):
    person = c.committer[0:2]
    if person in authors.hg_author_map:
        c.committer = authors.hg_author_map[person] + c.committer[2:]

changelogs = {
    '4.6': 'NEWS.md:29-92',
    '4.5': 'NEWS.md:104-171',
    '4.4': 'NEWS.md:177-197',
    '4.3': 'NEWS.md:201-266',
    '4.2': 'NEWS.md:270-388',
    '4.1': 'NEWS.md:392-625',
    '4.0': 'NEWS.md:631-666',
    '3.8': 'NEWS.md:672-768',
}

if __name__ == '__main__':
    import sys
    res = main(*sys.argv[1:])
    if type(res) == int:
        sys.exit(res)
    else:
        sys.exit(0 if res else 1)
