#
# plugins.awk
# Преобразование plugin.hpp в дистрибутивный вид.
#
# gawk -f plugins.awk -v p1=1 -v p2=70 -v p4=build# plugin.hpp > plugin1.hpp
# gawk -f plugins.awk -v p1=1 -v p2=70 -v p4=build# plugin.pas > plugin1.pas
#
#  #ifdef FAR_USE_INTERNALS
#    то, что должно быть скрыто
#  #else // ELSE FAR_USE_INTERNALS
#    замена!
#  #endif // END FAR_USE_INTERNALS
#

BEGIN {
  Skip=0
}

{
  if(index($0,"<%YEAR%>"))
    $0=gensub(/<%YEAR%>/,strftime("%Y"),"g");

  if(index($0,"<%VERSION%>"))
    $0=gensub(/<%VERSION%>/,sprintf("%d.%d build %d",p1,p2,p4),"g");

  if(index(toupper(FILENAME),".HPP") > 0)
  {
    if(substr($1,1,3) == "#if" && $2 == "FAR_USE_INTERNALS")
    {
       Skip++;
    }
    else if($4 == "FAR_USE_INTERNALS")
    {
      if($3 == "ELSE")
      {
        if(Skip==1)
          Skip--;
      }
      else if($3 == "END")
      {
        Skip--;
        if(Skip < 0)
          Skip=0
      }
    }
    else if(!Skip)
    {
      if(index($0,"Revision:") > 0)
      {
        # /* $Revision: 1.68 04.12.2000 $ */
        printf "/* Revision: %s %s $ */\n",$3,strftime("%x")
      }
      else if(index($0,"#define FARMANAGERVERSION_") > 0)
      {
        if (index($0,"MAJOR") > 0)
          printf "#define FARMANAGERVERSION_MAJOR %d\n",p1
        else if (index($0,"MINOR") > 0)
          printf "#define FARMANAGERVERSION_MINOR %d\n",p2
        else if (index($0,"REVISION") > 0)
          printf "#define FARMANAGERVERSION_REVISION %d\n",p3
        else if (index($0,"BUILD") > 0)
          printf "#define FARMANAGERVERSION_BUILD %d\n",p4
        else if (index($0,"STAGE") > 0)
          printf "#define FARMANAGERVERSION_STAGE %s\n",p5
      }
      else
        print $0
    }
  }
  else
  {
    if($2 == "FAR_USE_INTERNALS}")
    {
       Skip=1;
    }
    else if($4 == "FAR_USE_INTERNALS")
    {
      if($3 == "ELSE" || $3 == "END")
      {
        Skip=0;
      }
    }
    else if(!Skip)
    {
      if(index($0,"$Revision:") > 0)
      {
        # * $Revision: 1.68 04.12.2000 $
        printf "* $Revision: %s %s $\n",$3,strftime("%x")
      }
      else if(index($0,"FARMANAGERVERSION :=") > 0)
      {
        printf "  FARMANAGERVERSION := MakeFarVersion (%d,%d,%d);\n",p1,p2,p4
      }
      else
        print $0
    }
  }
}
