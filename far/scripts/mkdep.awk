BEGIN{
  print "Rebuilding dependencies..." > "/dev/stderr"
  ORS=""
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
  i = split($0, a, ".");
  filename = a[1];
  for (j=2; j < i; j++)
    filename = filename "." a[j]
  ext = a[i];

  if(ext == "cpp" || ext == "c")
    ext=obj;
  if(ext == "rc")
    ext=rc;
  if(ext == "hpp")
  {
    ext="hpp";
    print filename "." ext ":";
  }
  else
  {
    print out dirsep filename "." ext ":";
    print " " $0;
  }
  while((getline lnsrc < ($0)) > 0)
  {
    if(substr(lnsrc,1,length("#include \"")) == "#include \"")
    {
      lnsrc=gensub(/^#include[ \t]*\"([^\"]+)\".*$/, "\\1", "g", lnsrc);
      if(lnsrc != "" && lnsrc != $0)
        print " " lnsrc;
    }
  }
  print "\n\n"
}
