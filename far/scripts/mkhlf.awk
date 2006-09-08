# gawk -f mkhlf.awk FarEng.hlf >

{
  if($0 == "<%INDEX%>")
  {
    ;
  }
  else
  {
    print $0
  }
}

$1 == "@Contents" && NF == 1{
  getline
  #$^#File and archive manager#
  Contents=gensub(/^\$[ \^]#(.+)#/,"\\1","g",$0)
  print $0
}

substr($1,1,1) == "@" && substr($1,2,1) != " " && $1 != "@Contents" && NF == 1 && $1 != "@Index"{
  topic=$0
  topicFound=1
}

topicFound == 1 && NF > 1{
  if($1 != "$" || $2 == "#Warning:" || $2 == "#Error:" || $2 == "#Предупреждение:" || $2 == "#Ошибка:")
  {
    topicFound=0
    topic=""
    it--
  }
  else
  {
    #$ #Режимы сортировки#
    s=gensub(/^\$[ \^]#(.+)#/,"\\1","g",$0)
    atopic[s "~" topic "@"]=topic
    topicFound=0
    topic=0;
  }
}

$0 == "<%INDEX%>" {
  print "   ~" Contents "~@Contents@"
  if(index(toupper(FILENAME),"FARENG.HLF.M4") > 0) {print ""}

  n=asorti(atopic,topic2)
  ch=" "

  for(i=1; i <= n; i++)
  {
    if(ch != substr(topic2[i],1,1))
    {
      ch=substr(topic2[i],1,1)
      print ""
    }
    print "   ~" ltrim(topic2[i])
  }
}

function ltrim (str) {
  while(match(str,/^[ \t]/))
  {
    sub(/^[ \t]/, "", str)
  }
  return str
}
