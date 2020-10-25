#!/usr/bin/env python2
# -*- coding:windows-1251 -*-

"""Find files with letters in russian windows-1251 encoding.

windows-1251 is a single byte encoding with a range 0xC0-0xFF
and 0xA8,0xB8 for symbols ё and Ё respectfully. Unfortunately,
russian symbols in windows-1251 clash with russian symbols in
utf-8, where they take two bytes in the ranges:

 - 0xD081         Ё
 - 0xD090-0xD0BF  А-п
 - 0xD180-0xD18F  п-я
 - 0xD191         ё

This code ignores lines with <!-- NLC --> marker.
"""
# pythonized by techtonik // gmail.com


import codecs
import fnmatch
import os
import re
import sys

#sys.stdout.write("ыва".decode("windows-1251"))


args = [arg for arg in sys.argv[1:] if not arg.startswith('-')]
if not args:
  print(__doc__)
  print("""\
usage: program [opts] <dir>

opts:
  --lines  - show lines with matched text
""")
  sys.exit()


# set encoding for console output
#print sys.stdout.encoding
#sys.stdout = codecs.lookup("windows-1251")[-1](sys.stdout)
#print sys.stdout.encoding


#reload(sys)
#sys.setdefaultencoding("cp866")

#print "вфаывЭ".decode("cp1251")

#sw = codecs.lookup("cp866")[-1](sys.stdout)
#sw.write("sdfфывЭ")

dirs_exclude = ['.svn', '.git']
files_exclude = ['*.gif', '*.png', '*.jpg', '*.exe', '*.ico', '*.msi', '*.rar']

# https://en.wikipedia.org/wiki/Windows-1251
cp1251 = re.compile(r"[\xA8\xB8\xC0-\xFF]+")
# https://en.wikipedia.org/wiki/UTF-8#Encoding
utf8ru = re.compile(r"(\xD0[\x81\x90-\xBF]|\xD1[\x91\x80-\x8F])+")
utf8rest = re.compile(r"(\xC2[\xAB\xBB]|"  # angle quotes
                      r"\xE2\x80\x94|"     # long dash
                      r"\xC2\xA6)+")       # unicode |

skip_mark = "<!-- NLC -->"
for root,dirs,files in os.walk(args[0]):

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
    for i,l in enumerate(open(os.path.join(root, f), "r")):
      if l.find(skip_mark) != -1:
        continue
      noutf = utf8ru.sub('', l)  # remove russian utf-8 to avoid false positives
      noutf = utf8rest.sub('', noutf)  # remove other clashing utf-8 symbols
      rutext = "".join(cp1251.findall(noutf))
      rucount += len(rutext)
      if rutext and ('--lines' in sys.argv):
          sys.stdout.write("  line {}: ".format(i+1))
          sys.stdout.write(rutext.decode("windows-1251").encode('utf-8'))
          #print(rutext.decode("cp1251"))
          #print("\n    ", noutf.encode('hex')
          sys.stdout.write("\n")

    if rucount:
      print("%s - %d russian (cp1251) symbols" % (os.path.join(root, f), rucount))

  #print root,dirs,files
