#!/usr/bin/env python3

"""
Make projects files for building Far Manager Encyclopedia in .CHM format
"""

# pythonized by techtonik // gmail.com
# modifications by Far Group

# keywords for HHK are generated from "<a name=>" and "<h1>"

# contents tree for HHC is generated from /html/index.html following links one level down.
# links in each file are followed only between <!-- HHC --> comments, for each "<h3>" a new "folder" is created,
# for each "<a href=>" a new topic with some additional logic that prevents following unwanted links (only width=40% links are followed under h3 sections)
# also,  for "<h3>",  text for the title is taken upto to the first comma


from config import *

from os import makedirs, walk, listdir
from os.path import isdir, join, exists
from string import Template
import shutil
import logging
import re
import operator

logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)-6s %(message)s")
logging.addLevelName("WARN", 30)

#: just a shortcut
log = logging.info
warn = logging.warning

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

def make_chm_lang(lang):
  """@param lang : either 'rus*' or 'eng*'"""
  lang_code = lang[0:2]

  log("------------------------------------")
  log("preparing %s " % lang_code)

  log("copying files")
  copytree("%s/enc_%s" % (ROOT_DIR, lang), "%s/%s" % (DEST_CHM, lang_code))

  chm_lang_dir = join(DEST_CHM, lang_code)
  makedirs(join(chm_lang_dir, "html"))

  # build empty directory tree
  chm_meta_dir = join(chm_lang_dir, "meta")
  chm_html_dir = join(chm_lang_dir, "html")
  for root, dirs, files in walk(chm_meta_dir):
    for d in dirs:
      makedirs(join(root.replace(chm_meta_dir, chm_html_dir), d))

  log("-- translating meta into html")

  header_match = '<meta http-equiv="Content-Type" Content="text/html; charset=utf-8">'
  header_replace = '<meta http-equiv="Content-Type" Content="text/html; charset=Windows-1251">'

  for root, dirs, files in walk(chm_meta_dir):
    for f in files:
      infile  = open(join(root, f), encoding="utf-8-sig")
      outfile = open(join(root.replace(chm_meta_dir, chm_html_dir), f), "w", encoding="windows-1251")
      header_replaced = False
      for line in infile:
        if not header_replaced and header_match in line:
          line = line.replace(header_match, header_replace)
          header_replaced = True

        outfile.write(line)
      infile.close()
      outfile.close()

  log("-- cleaning meta")
  shutil.rmtree(chm_meta_dir)

  log("-- creating CHM contents")
  contents_filename = join(chm_lang_dir, "plugins%s.hhc" % lang[0])
  match_h3 = re.compile("<h3>(?P<title>.*?)(</h3>|,)", re.I)
  match_link_no_h3 = re.compile(r'<a.+?href\s*=\s*(?P<quote>[\'\"])(.+?)(?P=quote).*?>(.+?)</a>', re.I)
  match_link_after_h3 = re.compile(r'.?width\=\"40\%\".?<a.+?href\s*=\s*(?P<quote>[\'\"])(.+?)(?P=quote).*?>(.+?)</a>', re.I)

  cntnts = open(contents_filename, "w", encoding="windows-1251")
  cntnts.write(
"""<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta http-equiv="Content-Type" Content="text/html; charset=Windows-1251">
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
  <LI> <OBJECT type="text/sitemap">
  <param name="Name" value="Programming Far Manager plugins - Encyclopedia for Developers">
  <param name="Local" value="html/index.html">
  </OBJECT>
  <UL>

""")
  in_hhc1 = 0
  f1 = open(join(chm_html_dir, "index.html"), encoding="windows-1251")
  log("Scanning %s" % f1.name)
  for l1 in f1:
    if (l1.find("HHC") != -1):
      if (in_hhc1==0):
        in_hhc1 = 1
        continue
      break
    elif (in_hhc1==0):
      continue
    for rl in match_link_no_h3.findall(l1):
      log("  New link: %s" % rl[2])
      cntnts.write(
"""      <LI> <OBJECT type="text/sitemap">
      <param name="Name" value="%s">
      <param name="Local" value="html/%s">
      </OBJECT>
""" % (rl[2], rl[1]))
      link_dir = rl[1]
      link_dir = link_dir[:link_dir.find("/")]
      in_hhc2 = 0
      in_h3 = 0
      in_link = 0
      f2 = open(join(chm_html_dir, rl[1]), encoding="windows-1251")
      log("Scanning %s" % f2.name)
      for l2 in f2:
        if (l2.find("HHC") != -1):
          if (in_hhc2 == 0):
            in_hhc2 = 1
            continue
          break
        elif (in_hhc2 == 0):
          continue

        for rh in match_h3.findall(l2):
          if (in_h3 == 1):
            log("    Close section")
            cntnts.write("          </UL>\n");
          else:
            log("  Open section")
            cntnts.write("      <UL>\n");

          in_h3 = 1
          cntnts.write(
"""          <LI> <OBJECT type="text/sitemap">
          <param name="Name" value="%s">
          </OBJECT>
          <UL>
""" % (rh[0]))
          log("    Open section: %s" % rh[0])

        match_link = (match_link_no_h3, match_link_after_h3)[in_h3 == 1]
        for rl in match_link.findall(l2):
          if (in_h3 == 0 and in_link == 0):
            log("  Open section")
            cntnts.write("      <UL>\n");
          in_link = 1
          cntnts.write(
"""                <LI> <OBJECT type="text/sitemap">
                <param name="Name" value="%s">
                <param name="Local" value="html/%s/%s">
                </OBJECT>
""" % (rl[2], link_dir, rl[1]))
          log("      New topic: %s" % rl[2])

      if (in_h3 == 1):
        log("    Close section")
        cntnts.write("          </UL>\n")

      if (in_h3 == 1 or in_link == 1):
        log("  Close section")
        cntnts.write("      </UL>\n")

  cntnts.write(
"""
  </UL>
</UL>
</BODY></HTML>
""")

  log("-- creating CHM indexes")
  # indexes are extracted from <h1> and <a name="">..</a>
  # articles are not included in index

  index_filename = join(chm_lang_dir, "plugins%s.hhk" % lang[0])
  match_h1 = re.compile("<h1>(?P<title>.*?)</h1>", re.I)
  match_aname = re.compile(r'<a.+?name\s*=\s*(?P<quote>[\'\"])(.+?)(?P=quote).*?>(.+?)</a>', re.I)
  strip_re = re.compile(r'&quot;|[/<>\'"]', re.I)
  title_list = []
  macro_list = []
  for root, dirs, files in walk(chm_html_dir):
    if root.startswith(join(chm_html_dir, "articles")):
      continue
    macro_flag = "macro" in root
    for fn in files:
      if not fn.endswith(".html") or fn in ["faq.html"]:
        continue
      relflink = join(root[root.find("html"):], fn).replace('\\', '/')
      f = open(join(root, fn), encoding="windows-1251")
      for line in f:
        if not macro_flag:
          target_list = title_list
        else:
          target_list = macro_list
        for rh in match_h1.findall(line):
          target_list.append([relflink, strip_re.sub("", rh)])
        for ra in match_aname.findall(line):
          target_list.append([relflink+"#"+ra[1], strip_re.sub("", ra[2])])
      f.close()
  print

  titles = [t[1] for t in title_list]
  for ix, iv in enumerate(macro_list):
    if iv[1] in titles:
      macro_list[ix][1] = iv[1]+" (Macros)"

  title_list.extend(macro_list)
  title_list.sort(key=lambda x: x[1].lower())

  # processing duplicates
  for i,x in enumerate(title_list):
    if i and x[1] == title_list[i-1][1]:
      # non-anchored link from <h1> take precendence if filename is the same
      if "#" in x[0]:
        dup1 = x[0]
        dup2 = title_list[i-1][0]
      else:
        dup1 = title_list[i-1][0]
        dup2 = x[0]
      if "#" not in dup1 or "#" in dup2 or not (dup1.startswith(dup2) and len(dup2) == dup1.find("#")):
        warn("duplicate index : %s" % (x[1]))
        warn("  %s" % (x[0]))
        warn("  %s" % (title_list[i-1][0]))
      else:
        # remove anchored duplicate
        if dup1 == x[0]:
          del title_list[i]
        else:
          del title_list[i-1]

  # fcreep: mark all macros with (Macros) explicitly
  # fcreep: add distinguished meta labels to macros

  idx = open(index_filename, "w", encoding="windows-1251")
  idx.write(
"""<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta http-equiv="Content-Type" Content="text/html; charset=Windows-1251">
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<UL>
""")

  for title in title_list:
    idx.write(
"""  <LI> <OBJECT type="text/sitemap">
    <param name="Name" value="%s">
    <param name="Local" value="%s">
    </OBJECT>
""" % (title[1], title[0]))

  idx.write(
"""</UL>
</BODY></HTML>
""")
# end def make_chm_lang(lang):

log("preparing CHM build")
log("-- cleaning build dir")
if isdir(DEST): shutil.rmtree(DEST)
makedirs(DEST)
logfile = logging.FileHandler(BUILD_CHM_LOG, "w", encoding="utf-8")
logging.getLogger().addHandler(logfile)


log("-- making directory tree")
makedirs(DEST_CHM)

make_chm_lang("rus")
#make_chm_lang("eng")

log("-- done. check build log at %s" % BUILD_CHM_LOG)
