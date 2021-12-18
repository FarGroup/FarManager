-- Options
OptCaseSensitive=true
-- End of options

F=far.Flags
color = far.AdvControl(F.ACTL_GETCOLOR, far.Colors.COL_EDITORTEXT)
color.ForegroundColor, color.BackgroundColor = color.BackgroundColor, color.ForegroundColor
colorguid=win.Uuid "507CFA2A-3BA3-4f2b-8A80-318F5A831235"
lines={}
selStarts={}
selTypes={}

Macro
  area:"Editor"
  key:"F5"
  description:"Color Word Under Cursor"
  action:->
    ei=editor.GetInfo!
    id=ei.EditorID
    if lines[id] then lines[id]=nil
    else
      -- We don't want to search a vertical block of the zero width, let's check instead if there is a word under cursor:
      if Editor.Sel(0,4)~=1 and Editor.Sel(0,1)==Editor.Sel(0,3)
        selStarts[id],selTypes[id]=nil,nil
        pos=ei.CurPos
        line=editor.GetString!.StringText
        if pos<=line\len!+1
          slab=pos>1 and line\sub(1,pos-1)\match('[%w_]+$') or ""
          tail=line\sub(pos)\match('^[%w_]+') or ""
          if slab~="" or tail~=""
            lines[id]={}
            lines[id][1]=OptCaseSensitive and slab..tail or (slab..tail)\lower!
      else
        -- If the first line is empty, we are going to crop it. Let's reset the offset first:
        selTypes[id],selStarts[id]=Editor.Sel(0,4),Editor.SelValue\match("(.-)\r?\n")~="" and Editor.Sel(0,1) or 1
        -- Let's crop any empty leading and trailing lines and pad the last line with LF so that `gmatch` doesn't miss it:
        for line in Editor.SelValue\gsub("^[\r\n]*(.-)[\r\n]*$","%1\n")\gmatch("(.-)\r?\n")
          -- If the first line is empty after the crop, then the entire block is empty; there is nothing to highlight:
          if not lines[id] then if line=="" then break else lines[id]={}
          table.insert(lines[id],OptCaseSensitive and line or line\lower!)

Event
  group:"EditorEvent"
  action:(id,event,param)->
    if event==F.EE_REDRAW
      if lines[id]
        ei=editor.GetInfo id

        if selTypes[id]== nil
          start,finish=ei.TopScreenLine,math.min ei.TopScreenLine+ei.WindowSizeY,ei.TotalLines
          for ii=start,finish
            line,pos=editor.GetString(id,ii).StringText,1
            while true
              jj,kk,curr=line\cfind("([%w_]+)",pos)
              if not jj then break
              if not OptCaseSensitive then curr=curr\lower!
              if curr==lines[id][1] then editor.AddColor id,ii,jj,kk,F.ECF_AUTODELETE,color,100,colorguid
              pos=kk+1
          return

        minY=ei.TopScreenLine-#lines[id]+1
        maxY=math.min ei.TopScreenLine+ei.WindowSizeY-1,ei.TotalLines-#lines[id]+1
        for y=minY,maxY
          x=1
          while true
            local first,last
            for dY=1,#lines[id]
              line=OptCaseSensitive and editor.GetString(id,y+dY-1).StringText or editor.GetString(id,y+dY-1).StringText\lower!
              if selTypes[id]==2 then line=mf.strpad(line,lines[id][dY]\len!+selStarts[id]-1)
              if dY==1
                first,last=line\find(lines[id][dY],x,true)
                if not first then break
              else
                dX=selTypes[id]==1 and 1-selStarts[id] or 0
                -- If the first line of the pattern matches, but any of the following lines doesn't, then break the `dY` cycle and continue the search from the next column in the current line of screen:
                if lines[id][dY]~=line\sub(first+dX,first+dX+lines[id][dY]\len!-1)
                  last,x=nil,first+1
                  break
            -- If the first line of the pattern doesn't match, then break the `while` cycle and continue the search in the next line of screen:
            if not first then break
            if last
              for dY=1,#lines[id]
                dX=dY>1 and selTypes[id]==1 and 1-selStarts[id] or 0
                editor.AddColor id,y+dY-1,first+dX,first+dX+lines[id][dY]\len!-1,F.ECF_AUTODELETE,color,100,colorguid
              -- If the line of the pattern is empty, then the `last` variable is 0, and we use the `first` variable instead to avoid a loop:
              x=1+math.max first,last
    elseif event==F.EE_CLOSE then
      lines[id]=nil
      selStarts[id]=nil
      selTypes[id]=nil
