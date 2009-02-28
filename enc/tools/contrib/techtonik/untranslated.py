# -*- coding:windows-1251 -*-

"""Find files with russian letters in specified directory

If files contain Russian letters they are considered untranslated
unless <!-- NLC --> marker is present at the same line in HTML code

"""
# pythonized by techtonik // gmail.com


import codecs
import os
import re
import sys

#sys.stdout.write("ыва".decode("windows-1251"))

if len(sys.argv) < 2:
  print __doc__
  print "Usage: program <enc_eng_dir>"
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


ru = re.compile("[а-яА-Я] *")
skip_mark = "<!-- NLC -->"
for root,dirs,files in os.walk(sys.argv[1]):
  if ".svn" in dirs:
    dirs.remove(".svn")
  for f in files:
    if f.endswith(".gif"):
      continue
    rucount = 0
    for l in open(os.path.join(root, f), "r"):
      if l.find(skip_mark) != -1:
        continue
      rutext = "".join(ru.findall(l))
      rucount += len(rutext)
      #if rutext: print rutext #.decode("cp1251")
    if rucount:
      print "%s - %d russian symbols" % (os.path.join(root, f), rucount)
      
  #print root,dirs,files