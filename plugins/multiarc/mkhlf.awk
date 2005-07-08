# awk -f mkhlf.awk -v FV1=1 -v FV2=70 [-v FV3=4] [-v FV4=Build#] FarEng.hlf >
# Если не указан FV3 или он пустой, то не генерится "beta 4", например
# Если не указан FV4 или он пустой, то значение берется из файла "vbuild"

BEGIN {
  if(length(FV3) > 0)
    FV3=sprintf(" beta %d",FV3);
  else
    FV3=""
  if(length(FV4) == 0)
  {
    getline FV4 < "vbuild"
    FV4=alltrim(FV4)
    close("vbuild")
  }
}

{
  if(index($0,'<%FV1%>'))
    $0=gensub(/<%FV1%>/,FV1,'g');
  if(index($0,'<%FV2%>'))
    $0=gensub(/<%FV2%>/,FV2,'g');
  if(index($0,'<%FV3%>'))
    $0=gensub(/<%FV3%>/,FV3,'g');
  if(index($0,'<%FV4%>'))
    $0=gensub(/<%FV4%>/,FV4,'g');
  if(index($0,'<%YEAR%>'))
    $0=gensub(/<%YEAR%>/,strftime("%Y"),'g');
  print
}
