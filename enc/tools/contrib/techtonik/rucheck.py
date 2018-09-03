#!/usr/bin/env python2
# -*- coding:windows-1251 -*-

"""Find files with letters in russian windows-1251 encoding.

If English encyclopedia files contain Russian letters they are considered
untranslated unless <!-- NLC --> marker is present at the same line in
HTML code.

"""
# pythonized by techtonik // gmail.com


import codecs
import fnmatch
import os
import re
import sys

#sys.stdout.write("���".decode("windows-1251"))

if len(sys.argv) < 2:
  print __doc__
  print "Usage: program <dir>"
  sys.exit()

# set encoding for console output
#print sys.stdout.encoding
#sys.stdout = codecs.lookup("windows-1251")[-1](sys.stdout)
#print sys.stdout.encoding


#reload(sys)
#sys.setdefaultencoding("cp866")

#print "������".decode("cp1251")

#sw = codecs.lookup("cp866")[-1](sys.stdout)
#sw.write("sdf����")

dirs_exclude = ['.svn', '.git']
files_exclude = ['*.gif', '*.png', '*.jpg', '*.exe', '*.ico', '*.msi', '*.rar']

# https://en.wikipedia.org/wiki/Windows-1251
cp1251 = re.compile(r"[\xA8\xB8\xC0-\xFF]+")
# by coincidence cp1251 codes match with lower
# byte of utf-8

skip_mark = "<!-- NLC -->"
for root,dirs,files in os.walk(sys.argv[1]):

  # exclude dirs by modifying dirs in place
  dirs[:] = [d for d in dirs if not d in dirs_exclude]

  for f in files:
    # exclude files by skipping them
    skip = False
    for pattern in files_exclude:
      if fnmatch.fnmatch(f.lower(), pattern):
        skip = True
    if skip:
      continue

    rucount = 0
    for l in open(os.path.join(root, f), "r"):
      if l.find(skip_mark) != -1:
        continue
      rutext = "".join(cp1251.findall(l))
      rucount += len(rutext)
      #if rutext: print rutext #.decode("cp1251")
    if rucount:
      print "%s - %d russian (cp1251) symbols" % (os.path.join(root, f), rucount)

  #print root,dirs,files
