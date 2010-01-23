const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

TCHAR *GetCommaWord(const TCHAR *Src,TCHAR *Word)
{
  int WordPos,SkipBrackets;
  if (*Src==_T('\0'))
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=_T('\0');Src++,WordPos++)
  {
    if (*Src==_T('[') && _tcschr(Src+1,_T(']'))!=NULL)
      SkipBrackets=TRUE;
    if (*Src==_T(']'))
      SkipBrackets=FALSE;
    if (*Src==_T(',') && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (_istspace(*Src))
        Src++;
      return((TCHAR*)Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return((TCHAR*)Src);
}
