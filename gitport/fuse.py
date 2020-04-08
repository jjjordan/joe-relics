#!/usr/bin/env python3
# This fuses together disjoint git fast-import streams into a single repository.

import util

from fastimport.parser import ImportParser
from fastimport.helpers import repr_bytes

def main(*flist):
    infiles = flist[:-1]
    outfile = flist[-1]
    
    # Global tracker
    gt = util.Tracker()
    output = []
    
    for infile in infiles:
        # Local tracker
        lt = util.Tracker()
        
        print("Processing {} ...".format(infile))
        with open(infile, 'rb') as f:
            p = ImportParser(f)
            for cmd in p.iter_commands():
                if cmd.name == b'commit':
                    # Stitch together if this is the first commit to an
                    # existing branch.
                    if cmd.ref in gt.branches and cmd.ref not in lt.branches:
                        prev = gt.branches[cmd.ref]
                        lt.feed(cmd)
                        c = util.rebase_branch(prev, lt.last, cmd)
                        if c is None:
                            print("OOPS - Not implemented!")
                            return False
                        if c.from_ is not None:
                            if c.merges is not None:
                                c.merges.append(c.from_)
                            else:
                                c.merges = [c.from_]
                        c.from_ = None
                        gt.feed(c)
                        output.append(c)
                    else:
                        lt.feed(cmd)
                        gt.feed(cmd)
                        output.append(cmd)
                else:
                    lt.feed(cmd)
                    gt.feed(cmd)
                    output.append(cmd)
    
    with open(outfile, 'wb') as f:
        for c in output:
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
