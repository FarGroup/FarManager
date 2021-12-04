BEGIN{
  ORS=""
  bootstrap = ENVIRON["BOOTSTRAPDIR"]
  if (compiler=="gcc")
  {
    out="$(OBJDIR)";
    obj="o";
    rc="rc.o"
    dirsep="/";
  }
  else
  {
    out="$(INTDIR)";
    obj="obj";
    rc="res"
    dirsep="\\";
  }
}
{
  if (match($0, /(.+)\.(.+)/, a))
  {
    filename = a[1]
    ext = a[2]
  }

  if (match(filename, /(.+\\).+/, a))
  {
    path_part = a[1]
  }

  if(ext == "cpp" || ext == "c")
    ext=obj;
  if(ext == "rc")
    ext=rc;

  if(path_part == "" && (ext == obj || ext == rc))
  {
    print out dirsep filename "." ext ":";
    print " " $0;
  }
  else
  {
    print filename "." ext ":";
  }

  while((getline lnsrc < ($0)) > 0)
  {
    if(substr(lnsrc,1,length("#include \"")) == "#include \"")
    {
      lnsrc=gensub(/^#include[ \t]*"([^"]+)".*$/, "\\1", "g", lnsrc);
      if(lnsrc != "" && lnsrc != $0)
        if(substr(lnsrc,1,length("bootstrap/")) == "bootstrap/")
          lnsrc = bootstrap substr(lnsrc, length("bootstrap/") + 1)
        print " " path_part lnsrc;
    }
  }
  print "\n\n"
}
