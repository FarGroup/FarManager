-- Options
OptCaseSensitive=true
-- End of options

F=far.Flags
color = far.AdvControl(F.ACTL_GETCOLOR, far.Colors.COL_EDITORTEXT)
color.ForegroundColor, color.BackgroundColor = color.BackgroundColor, color.ForegroundColor
colorguid=win.Uuid "507CFA2A-3BA3-4f2b-8A80-318F5A831235"
words={}

Macro
  area:"Editor"
  key:"F5"
  description:"Color Word Under Cursor"
  action:->
    ei=editor.GetInfo!
    id=ei.EditorID
    if words[id] then words[id]=nil
    else      
      pos=ei.CurPos
      line=editor.GetString!.StringText
      if pos<=line\len()+1
        slab=pos>1 and line\sub(1,pos-1)\match('[%w_]+$') or ""
        tail=line\sub(pos)\match('^[%w_]+') or ""
        if slab~="" or tail~="" then words[id]=OptCaseSensitive and slab..tail or (slab..tail)\lower!

Event
  group:"EditorEvent"
  action:(id,event,param)->
    if event==F.EE_REDRAW
      if words[id]
        ei=editor.GetInfo!
        start,finish=ei.TopScreenLine,math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines
        for ii=start,finish
          line,pos=editor.GetString(-1,ii).StringText,1
          while true
            jj,kk,curr=line\cfind("([%w_]+)",pos)
            if not jj then break
            if not OptCaseSensitive then curr=curr\lower!
            if curr==words[id] then editor.AddColor ei.EditorID,ii,jj,kk,F.ECF_AUTODELETE,color,100,colorguid
            pos=kk+1
    elseif event==F.EE_CLOSE then words[id]=nil
