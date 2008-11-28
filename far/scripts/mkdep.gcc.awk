BEGIN{
  print "Rebuild dependencies..." > "/dev/stderr"
  if(out == "")
    out="./GCC/obj";
}
{
  i = split($0, a, ".");
  filename = a[1];
  for (j=2; j < i; j++)
    filename = filename "." a[j]
  ext = a[i];

  if(ext == "cpp" || ext == "c")
    ext="o";
  if(ext == "rc")
    ext="rc.o";
  if(ext == "hpp")
  {
    ext="hpp";
    print filename "." ext " : \\";
  }
  else
  {
    print out "/" filename "." ext " : \\";
    print "\t" $0 " \\";
  }
  while((getline lnsrc < ($0)) > 0)
  {
    if(substr(lnsrc,1,length("#include \"")) == "#include \"")
    {
      lnsrc=gensub(/^#include[ \t]?\"([^\"]+)\"/, "\\1", "g", lnsrc);
      if(lnsrc != "")
        print "\t" lnsrc " \\";
    }
  }
  print "\n\n"
}
