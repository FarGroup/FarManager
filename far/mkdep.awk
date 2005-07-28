BEGIN{
  FS=".";
  if(out == "")
    out="Release.vc";
}
{
  if($2 == "cpp" || $2 == "c")
    ext="obj";
  if($2 == "rc")
    ext="res";
  if($2 == "hpp")
  {
    ext="hpp";
    print ".\\" $1 "." ext " : \\";
    #if($1 "." $2 == "lang.hpp")
    #  print "\t\".\\farlang.ini\"\\";

  }
  else
  {
    print ".\\" out "\\obj\\" $1 "." ext " : \\";
    print "\t\".\\" $1 "." $2 "\"\\";
  }
  #print "Process " $1 "." $2 > "/dev/stderr"
  while((getline lnsrc < ($1 "." $2)) > 0)
  {
    if(substr(lnsrc,1,length("#include \"")) == "#include \"")
    {
      #print lnsrc > "/dev/stderr"
      lnsrc=gensub(/^#include[ \t]?\"([^\"]+)\"/, "\\1", "g", lnsrc);
      if(lnsrc != "")
        print "\t\".\\" lnsrc "\"\\";
    }
  }
  print "\n\n"
}
