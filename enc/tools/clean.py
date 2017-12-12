"""
Aux buildbot script to cleanup Encyclopedia build directory
"""


execfile("config.inc.py")

import shutil
from os.path import isdir


if isdir(DEST):
  shutil.rmtree(DEST)

print "Cleanup done."
