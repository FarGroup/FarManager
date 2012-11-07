"""
Make web suitable Encyclopedia
"""

# based on tool.make_chm.py pythonized by techtonik // gmail.com
# modificatios by Far Group

execfile("config.inc.py")

from os import makedirs, walk
from os.path import isdir, join, commonprefix,  normpath
from string import Template
import shutil
import logging
import subprocess
import re
import operator

logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)-6s %(message)s")
logging.addLevelName("WARN", 30)

#: just a shortcut
log = logging.info
warn = logging.warn



def make_inet_lang(lang):
  """@param lang : either 'rus*' or 'eng*'"""
  lang_code = lang[0:2]

  log("------------------------------------")
  log("preparing %s " % lang_code)

  inet_meta_dir = join(DEST_INET, lang_code, "meta")
  makedirs(inet_meta_dir)

  log("exporting from svn")
  command = "svn export -q --force %s/enc_%s/images %s/images" % (ROOT_DIR, lang, DEST_INET)
  subprocess.call(command, shell=True)
  command = "svn export -q --force %s/enc_%s/meta %s/%s/meta" % (ROOT_DIR, lang, DEST_INET, lang_code)
  subprocess.call(command, shell=True)

  # build empty directory tree
  inet_html_dir = join(DEST_INET, lang_code)
  for root, dirs, files in walk(inet_meta_dir):
    for d in dirs:
      makedirs(join(root.replace(inet_meta_dir, inet_html_dir), d))

  log("-- translating meta into html")
  # filter files and replace "win32/.." links with calls to MSDN
  link_match = re.compile(r'href[\s"\'=\/\.]*?win32\/(?P<funcname>[^"\']*?)(\.html)?[\'"].*?>(?P<linkend>.*?<\/a>)', re.I)
  link_replace = Template(r'href="$notfound?\g<funcname>">\g<linkend>')
  id = 0
  for root, dirs, files in walk(inet_meta_dir):
    for f in files:
      relpath = normpath(join(root, "..", "xxx")) # ".." because we need to get rid of 1 level because of "meta""
      plen = len(commonprefix([ join(inet_html_dir, "notfound.html"), relpath ]))
      relpath = relpath[plen:].replace('\\','/')
      notfound = "../" * relpath.count('/') + "notfound.html"
      infile  = open(join(root, f))
      outfile = open(join(root.replace(inet_meta_dir, inet_html_dir), f), "w")
      for line in infile:
        while link_match.search(line):
          line = link_match.sub(link_replace.substitute(notfound=notfound), line)
          id += 1
        outfile.write(line)
      infile.close()
      outfile.close()
  log("total %d win32 links" % id)

  log("-- cleaning meta")
  shutil.rmtree(inet_meta_dir)

# end def make_inet_lang(lang):



log("preparing INET build")
log("-- cleaning build dir")
if isdir(DEST): shutil.rmtree(DEST)
makedirs(DEST)
logfile = logging.FileHandler(BUILD_INET_LOG, "w")
logging.getLogger().addHandler(logfile)


log("-- making directory tree")
makedirs(DEST_INET)
makedirs(join(DEST_INET,"images"))
makedirs(join(DEST_INET,"styles"))

make_inet_lang("rus3.work")
#make_inet_lang("eng")

log("-- copying index files")
shutil.copy("inet/index.html", DEST_INET)
shutil.copy("inet/farenclogo.gif", join(DEST_INET,"images"))
shutil.copy("inet/styles.css", join(DEST_INET,"styles"))

log("-- done. check build log at %s" % BUILD_INET_LOG)
