#!/usr/bin/python
"""
Aux buildbot script to cleanup Encyclopaedia build directory
"""


execfile("config.inc.py")

import shutil
from os.path import isdir


if isdir(DEST):
  shutil.rmtree(DEST)

print "Cleanup done."
