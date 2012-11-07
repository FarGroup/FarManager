"""
Check for broken links
"""

execfile("config.inc.py")

from os import walk
from os.path import isfile, join
import re

def check_links(dir_to_check):
  link_match = re.compile(r'(href|src)\s*=[\s"\']*([^#"\']+)', re.I)
  for root, dirs, files in walk(dir_to_check):
    for f in files:
      if (not f.endswith(".html")):
        continue
      infile  = open(join(root, f))
      for line in infile:
        for r in link_match.findall(line):
          if (r[1].startswith(("http:", "mailto:", "ftp:", "news:", "mk:"))):
            continue
          if (r[1].find("win32/") != -1):
            continue
          if (not isfile(join(root, r[1]))):
            print "%s - %s" % (join(root, f)[len(dir_to_check)+1:], r[1])
      infile.close()

check_links(join(ROOT_DIR,"enc_rus3.work","meta"))
