#!/usr/bin/env python3

"""
Make web suitable Encyclopedia
"""

# based on tool.make_chm.py pythonized by techtonik // gmail.com
# modifications by Far Group

from config import *

from os import makedirs, walk, listdir
from os.path import isdir, join, commonprefix, normpath, exists
from string import Template
import shutil
import logging
import re

logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)-6s %(message)s")
logging.addLevelName("WARN", 30)

#: just a shortcut
log = logging.info
warn = logging.warn

def copytree(src, dst, symlinks=False, ignore=None):
  if not exists(dst):
    makedirs(dst)
  for item in listdir(src):
    s = join(src, item)
    d = join(dst, item)
    if isdir(s):
      shutil.copytree(s, d, symlinks, ignore)
    else:
      shutil.copy2(s, d)
#end of copytree

def make_inet_lang(lang):
  """@param lang : either 'rus*' or 'eng*'"""
  lang_code = lang[0:2]

  log("------------------------------------")
  log("preparing %s " % lang_code)

  inet_meta_dir = join(DEST_INET, lang_code, "meta")
  makedirs(inet_meta_dir)

  log("copying files")
  copytree("%s/enc_%s/images" % (ROOT_DIR, lang), "%s/images" % (DEST_INET))
  copytree("%s/enc_%s/meta" % (ROOT_DIR, lang), "%s/%s" % (DEST_INET, lang_code))

# end def make_inet_lang(lang):



log("preparing INET build")
log("-- cleaning build dir " + DEST)
if isdir(DEST): shutil.rmtree(DEST)
makedirs(DEST)
logfile = logging.FileHandler(BUILD_INET_LOG, "w", encoding="utf-8")
logging.getLogger().addHandler(logfile)


log("-- output dir " + DEST_INET)
makedirs(DEST_INET)
makedirs(join(DEST_INET,"images"))
makedirs(join(DEST_INET,"styles"))

make_inet_lang("rus")
#make_inet_lang("eng")

log("-- copying index files")
shutil.copy(ROOT_DIR+"/tools/inet/index.html", DEST_INET)
shutil.copy(ROOT_DIR+"/tools/inet/farenclogo.gif", join(DEST_INET,"images"))
shutil.copy(ROOT_DIR+"/tools/inet/styles.css", join(DEST_INET,"styles"))

log("-- done. check build log at %s" % BUILD_INET_LOG)
