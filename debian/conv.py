#!/usr/bin/python

import sys

def convert (from_enc, to_enc, fn):
    try:
        f = open (fn, "rb")
        txt = f.read ()
    except:
       print "Failed to open file %s" % fn
       return

    txt = txt.decode (from_enc, "replace").encode (to_enc, "replace")
    f = open (fn, "wb")
    f.write (txt)


if len (sys.argv) < 3:
    print "Usage: %s [from-encoding] [to-encoding] [files ...]" % sys.argv [0]
    sys.exit (-1)

from_enc = sys.argv [1]
to_enc = sys.argv [2]

for fn in sys.argv [3:]:
    convert (from_enc, to_enc, fn)
