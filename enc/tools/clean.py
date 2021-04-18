#!/usr/bin/env python3

"""
Aux buildbot script to cleanup Encyclopedia build directory
"""


from config import *

import shutil
from os.path import isdir


if isdir(DEST):
  shutil.rmtree(DEST)

print("Cleanup done.")
