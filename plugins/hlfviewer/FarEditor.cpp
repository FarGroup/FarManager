#ifndef __FarEditor_cpp
#define __FarEditor_cpp

void RestorePosition(void)
{
  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurPos;
  esp.TopScreenLine=ei.TopScreenLine;
  esp.LeftPos=ei.LeftPos;
  esp.CurTabPos=-1;
  esp.Overtype=-1;
  Info.EditorControl(ECTL_SETPOSITION,&esp);
}

BOOL IsHlf(void)
{
  BOOL ret=FALSE;

  Info.EditorControl(ECTL_GETINFO,&ei);
  memset(&esp,-1,sizeof(esp));
  egs.StringNumber=-1;
  int total=(ei.TotalLines<3)?ei.TotalLines:3;

  if(total>2) for(esp.CurLine=0;esp.CurLine<total;esp.CurLine++)
  {
    Info.EditorControl(ECTL_SETPOSITION,&esp);
    Info.EditorControl(ECTL_GETSTRING,&egs);
    if(!FSF.LStrnicmp(".Language=",egs.StringText,10))
    {
      ret=TRUE;
      break;
    }
  }
  RestorePosition();
  return ret;
}

const char *FindTopic(void)
{
  const char *ret=NULL;
  const char *tmp;

  Info.EditorControl(ECTL_GETINFO,&ei);
  memset(&esp,-1,sizeof(esp));
  egs.StringNumber=-1;
  for(esp.CurLine=ei.CurLine;esp.CurLine>=0;esp.CurLine--)
  {
    Info.EditorControl(ECTL_SETPOSITION,&esp);
    Info.EditorControl(ECTL_GETSTRING,&egs);
    tmp=egs.StringText;
    if(lstrlen(tmp)>1 && *tmp=='@' && *(tmp+1)!='-' && *(tmp+1)!='+')
    {
      ret=tmp+1;
      break;
    }
  }
  RestorePosition();

  return ret;
}

#endif
