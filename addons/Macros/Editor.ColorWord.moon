F=far.Flags
color = far.AdvControl(F.ACTL_GETCOLOR, far.Colors.COL_EDITORTEXT)
color.ForegroundColor, color.BackgroundColor  = color.BackgroundColor, color.ForegroundColor
olorguid=win.Uuid "507CFA2A-3BA3-4f2b-8A80-318F5A831235"
word=false

Macro
  area:"Editor"
  key:"F5"
  description:"Color Word Under Cursor"
  action:->
    if word then word=false
    else
      ei=editor.GetInfo!
      pos,row=ei.CurPos,ei.CurLine
      line=editor.GetString!.StringText
      if pos<=line\len()+1
        slab=pos>1 and line\sub(1,pos-1)\match('[%w_]+$') or ""
        tail=line\sub(pos)\match('^[%w_]+') or ""
        word=slab..tail
      if word\len()<=0 then word=false

Event
  group:"EditorEvent"
  action:(id,event,param)->
    if event==F.EE_REDRAW
      if word
        ei=editor.GetInfo!
        start,finish=ei.TopScreenLine,math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines
        for ii=start,finish
          line,pos=editor.GetString(-1,ii).StringText,1
          while true
            jj,kk,curr=line\cfind("([%w_]+)",pos)
            if not jj then break
            if curr==word then editor.AddColor ei.EditorID,ii,jj,kk,F.ECF_AUTODELETE,color,100,colorguid
            pos=kk+1
