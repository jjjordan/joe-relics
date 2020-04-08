#!/usr/bin/env python3
# This only needs to squash together the first several commits to maintain a clean
# history back to the CVS code.  hg-fast-export can handle name, tag, and crlf
# translation.

import util

from fastimport.commands import CommitCommand, FileModifyCommand
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
    
    with open(hgout, 'wb') as f:
        f.write(repr_bytes(commit))
        f.write(b'\n')
        for c in l[cutoff+1:]:
            f.write(repr_bytes(c))
            f.write(b'\n')
    
    return True

if __name__ == '__main__':
    import sys
    res = main(*sys.argv[1:])
    if type(res) == int:
        sys.exit(res)
    else:
        sys.exit(0 if res else 1)
