-- test_editor.lua
local asrt = assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\assert.lua")) ()

local F = far.Flags
local band = bit64.band

local function OpenTempFile(DeleteOnClose)
  local fname = asrt.str(far.MkTemp())
  local flags = { EF_NONMODAL=1; EF_IMMEDIATERETURN=1; EF_DISABLEHISTORY=1; }
  if DeleteOnClose then flags.EF_DELETEONCLOSE = 1; end

  asrt.eq(F.EEC_MODIFIED, editor.Editor(fname,nil,nil,nil,nil,nil,flags))
  asrt.istrue(Area.Editor)
  local EI = asrt.table(editor.GetInfo())
  asrt.eq(EI.TotalLines, 1)
  return fname
end

local function test_Sel_Cmdline(args)
  for _, str in ipairs(args) do
    asrt.istrue(Area.Shell)
    panel.SetCmdLine(nil, str)

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local line, pos, w, h = 1, 13, 5, 1

    panel.SetCmdLineSelection(nil, pos, pos+w)
    asrt.eq(Editor.Sel(act,0), line)
    asrt.eq(Editor.Sel(act,1), pos)
    asrt.eq(Editor.Sel(act,2), line)
    asrt.eq(Editor.Sel(act,3), pos + w)
    asrt.eq(Editor.Sel(act,4), 1)
    --------------------------------------------------------------------------------------------------
    act = 1
    panel.SetCmdLineSelection(nil, pos, pos+w)

    asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
    asrt.eq(panel.GetCmdLinePos(), pos)

    asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
    asrt.eq(panel.GetCmdLinePos(), pos + w + 1)
    --------------------------------------------------------------------------------------------------
    for act = 2,3 do
      panel.SetCmdLinePos(nil,pos)             -- set block start
      asrt.eq(1, Editor.Sel(act,0))          -- +++
      panel.SetCmdLinePos(nil,pos + w)         -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))          -- +++

      asrt.eq(Editor.Sel(0,0), 1)
      asrt.eq(Editor.Sel(0,1), pos)
      asrt.eq(Editor.Sel(0,2), 1)
      asrt.eq(Editor.Sel(0,3), pos + w - 1)
      asrt.eq(Editor.Sel(0,4), 1)
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)

    panel.SetCmdLine(nil, "")
  end
end

local function test_Sel_Dialog(args)
  Keys("F7 Del")
  asrt.istrue(Area.Dialog)
  local inf = actl.GetWindowInfo()
  local hDlg = asrt.udata(inf.Id)
  local EditPos = asrt.num(hDlg:GetFocus())

  for _, str in ipairs(args) do
    hDlg:SetText(EditPos, str)

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local tSel = { BlockType = F.BTYPE_STREAM; BlockStartLine = 1; BlockStartPos = 13,
                   BlockWidth = 5; BlockHeight = 1; }

    hDlg:SetSelection(EditPos, tSel)
    asrt.eq(Editor.Sel(act,0), tSel.BlockStartLine)
    asrt.eq(Editor.Sel(act,1), tSel.BlockStartPos)
    asrt.eq(Editor.Sel(act,2), tSel.BlockStartLine)
    asrt.eq(Editor.Sel(act,3), tSel.BlockStartPos + tSel.BlockWidth - 1) -- [-1: DIFFERENT FROM EDITOR]
    asrt.eq(Editor.Sel(act,4), tSel.BlockType)
    --------------------------------------------------------------------------------------------------
    act = 1
    hDlg:SetSelection(EditPos, tSel)

    asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
    local pos = asrt.table(hDlg:GetCursorPos(EditPos))
    asrt.eq(pos.Y+1, tSel.BlockStartLine)
    asrt.eq(pos.X+1, tSel.BlockStartPos)

    asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
    pos = asrt.table(hDlg:GetCursorPos(EditPos))
    asrt.eq(pos.Y+1, tSel.BlockStartLine)
    asrt.eq(pos.X+1, tSel.BlockStartPos + tSel.BlockWidth)
    --------------------------------------------------------------------------------------------------
    for act = 2,3 do
      hDlg:SetCursorPos(EditPos, {X=10; Y=0})  -- set block start
      asrt.eq(1, Editor.Sel(act,0))          -- +++
      hDlg:SetCursorPos(EditPos, {X=20; Y=0})  -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))          -- +++

      asrt.eq(Editor.Sel(0,0), 1)
      asrt.eq(Editor.Sel(0,1), 10+1)
      asrt.eq(Editor.Sel(0,2), 1)
      asrt.eq(Editor.Sel(0,3), 20)
      asrt.eq(Editor.Sel(0,4), 1)
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)
  end

  Keys("Esc")
end

local function test_Sel_Editor(args)
  local R2T, T2R = editor.RealToTab, editor.TabToReal
  local T2R2T = function(id, y, x)
    return R2T(id, y, T2R(id, y, x)) -- needed when the column position is "inside" a tab
  end

  Keys("ShiftF4 Del Enter")
  asrt.istrue(Area.Editor)

  for _, str in ipairs(args) do
    Keys("CtrlA Del")

    for k=1,8 do
      editor.InsertString()
      editor.SetString(nil, k, str)
    end

    local act = 0
    for opt = 0,4 do
      asrt.eq(Editor.Sel(act,opt), 0) -- no block exists, zeros returned
    end

    local line, pos, w, h = 3, 13, 5, 4

    for ii=1,2 do
      local typ = ii==1 and "BTYPE_STREAM" or "BTYPE_COLUMN"
      local ref_x1 = (ii==1 and R2T or T2R2T)(nil, line, pos)
      local ref_x2 = (ii==1 and R2T or T2R2T)(nil, line-1+h, pos+w)

      asrt.istrue(editor.Select(nil, typ, line, pos, w, h)) -- select the block
      asrt.eq(Editor.Sel(act,0), line)      -- get block start line
      asrt.eq(Editor.Sel(act,1), ref_x1)    -- get block start pos
      asrt.eq(Editor.Sel(act,2), line-1+h)  -- get block end line
      asrt.eq(Editor.Sel(act,3), ref_x2)    -- get block end pos
      asrt.eq(Editor.Sel(act,4), ii)        -- get block type
    end
    --------------------------------------------------------------------------------------------------
    act = 1
    for ii=1,2 do
      local typ = ii==1 and "BTYPE_STREAM" or "BTYPE_COLUMN"
      local ref_x1 = ii==1 and pos or T2R(nil, line, pos) or pos
      local ref_x2 = ii==1 and pos+w or T2R(nil, line-1+h, pos+w)

      local inf
      asrt.istrue(editor.Select(nil, typ, line, pos, w, h)) -- select the block

      asrt.eq(1, Editor.Sel(act, 0)) -- set cursor at block start
      inf = editor.GetInfo()
      asrt.eq(inf.CurLine, line)
      asrt.eq(inf.CurPos, ref_x1)

      asrt.eq(1, Editor.Sel(act, 1)) -- set cursor next to block end
      inf = editor.GetInfo()
      asrt.eq(inf.CurLine, line-1+h)
      asrt.eq(inf.CurPos, ref_x2)
    end
    --------------------------------------------------------------------------------------------------
    local y1,x1,y2,x2 = 2,10,6,20
    for act = 2,3 do
      editor.SetPosition(nil,y1,x1)    -- set block start
      asrt.eq(1, Editor.Sel(act,0))  -- +++
      editor.SetPosition(nil,y2,x2)    -- set block end (it also selects the block)
      asrt.eq(1, Editor.Sel(act,1))  -- +++

      local ref_x1 = (act==2 and R2T or T2R2T)(nil,y1,x1)
      local ref_x2 = (act==2 and R2T or T2R2T)(nil,y2,x2)

      asrt.eq(Editor.Sel(0,0), y1)                 -- get block start line
      asrt.eq(Editor.Sel(0,1), ref_x1)             -- get block start pos
      asrt.eq(Editor.Sel(0,2), y2)                 -- get block end line
      asrt.eq(Editor.Sel(0,3), ref_x2)             -- get block end pos
      asrt.eq(Editor.Sel(0,4), act==2 and 1 or 2)  -- get block type
    end
    --------------------------------------------------------------------------------------------------
    asrt.eq(1, Editor.Sel(4)) -- reset the block
    asrt.eq(Editor.Sel(0,4), 0)
  end

  asrt.istrue(editor.Quit())
end

local function test_Editor_InsStr()
  asrt.func(Editor.InsStr)

  OpenTempFile(true)
  local N = 5
  for k = 1,N do
    editor.InsertString(nil, k)
    editor.SetString(nil, k, "line "..k)
  end
  local EI = editor.GetInfo()
  asrt.eq(EI.TotalLines, N+1)
  asrt.eq(EI.CurLine, N+1)

  asrt.eq(0, Editor.InsStr("aaa", N+2))

  asrt.eq(1, Editor.InsStr("aaa", N+1))
  EI = editor.GetInfo()
  asrt.eq(EI.TotalLines, N+2)
  asrt.eq(EI.CurLine, N+1)
  asrt.eq("aaa", editor.GetString(nil, N+2, 3))

  local curLine, curPos = 2, 3
  asrt.istrue(editor.SetPosition(nil, curLine, curPos))
  asrt.eq(1, Editor.InsStr("bbb")) -- insert after current line
  asrt.eq("line 1", editor.GetString(nil, 1, 3))
  asrt.eq("line 2", editor.GetString(nil, 2, 3))
  asrt.eq("bbb",    editor.GetString(nil, 3, 3))
  EI = editor.GetInfo()
  asrt.eq(EI.TotalLines, N+3)
  asrt.eq(EI.CurLine, curLine)
  asrt.eq(EI.CurPos, curPos)

  editor.Quit()
  asrt.istrue(Area.Shell)
end

local function test_Misc()
  OpenTempFile(true)
  local EI = asrt.table(editor.GetInfo())

  local str = ("123456789-"):rep(4)
  local str2, num

  -- test Editor.Value
  editor.SetString(nil,1,str)
  asrt.eq(Editor.Value, str)

  -- test insertion with overtype=OFF
  editor.SetString(nil,1,str)
  editor.SetPosition(nil,1,1)
  editor.InsertText(nil, "AB")
  asrt.eq(Editor.Value, "AB"..str)

  -- test insertion with overtype=ON
  editor.SetString(nil,1,str)
  Keys("Ins")
  editor.SetPosition(nil,1,1)
  editor.InsertText(nil, "CD")
  asrt.eq(Editor.Value, "CD"..str:sub(3))
  Keys("Ins")

  -- test insertion beyond EOL (overtype=ON then OFF)
  num = 20
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", true))
  str2 = str .. (" "):rep(num) .. "AB"
  for _=1,2 do
    Keys("Ins")
    editor.SetString(nil,1,str)
    editor.SetPosition(nil, 1, #str + 1 + num)
    editor.InsertText(nil, "AB")
    asrt.eq(Editor.Value, str2)
  end
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", band(EI.Options, F.EOPT_CURSORBEYONDEOL) ~= 0))

  editor.Quit()
end

-- "Several lines are merged into one".
local function test_issue_3129()
  local fname = OpenTempFile(false)
  for k=1,3 do
    editor.InsertString()
    editor.SetString(nil, k, "foo")
  end
  asrt.istrue (editor.SaveFile())
  asrt.istrue (editor.Quit())
  local fp = assert(io.open(fname))
  local k = 0
  for line in fp:lines() do
    k = k + 1
    asrt.eq (line, "foo")
  end
  fp:close()
  win.DeleteFile(fname)
  asrt.eq (k, 3)
end

local function test_Editor_Sel()
  local args = {
    ("123456789-"):rep(4),     -- plain ASCII
    ("12\t4\t6789-"):rep(4),   -- includes tabs
    ("12🔥4🔥6789-"):rep(4),   -- includes 🔥 (a multi-byte double-width character)
    ("1Ю23456789-"):rep(4),    -- insert 'Ю' (a multi-byte character) into position 2
    ("1Ю2\t4\t6789-"):rep(4),  -- ditto
    ("1Ю2🔥4🔥6789-"):rep(4),  -- ditto
  }
  test_Sel_Cmdline(args)
  test_Sel_Dialog(args)
  test_Sel_Editor(args)
end

local function test_multiple_instances()
  local flags = {EF_NONMODAL=1, EF_IMMEDIATERETURN=1, EF_DISABLEHISTORY=1, EF_DELETEONCLOSE=1}
  local N = 10
  local ids = {}
  local MakeText = function(idx) return "This is a string number "..idx end
  local MakeFName = function(idx) return far.InMyTemp("instance-"..idx) end

  -- editor.Editor
  asrt.eq(1, actl.GetWindowCount())
  for k=1,N do
    asrt.eq(editor.Editor(MakeFName(k), nil,nil,nil,nil,nil, flags), F.EEC_MODIFIED)
    ids[k] = asrt.table(editor.GetInfo()).EditorID
  end
  asrt.eq(ids[N] - ids[1], N - 1)
  asrt.eq(1 + N, actl.GetWindowCount())

  -- editor.SetString
  for k=1,N do asrt.istrue(editor.SetString(ids[k], 1, MakeText(k))) end

  -- editor.AddColor
  local colstart, colend, colbase = 2, 5, 1
  for k=1,N do
    asrt.istrue(editor.AddColor(ids[k], 1, colstart+k, colend+k, nil, colbase+k))
  end

  -- GetString, GetColor, DelColor
  for k=N,1,-1 do
    local ID = ids[k]
    -- inspect text
    local str = asrt.table(editor.GetString(ID, 1))
    asrt.eq(str.StringText, MakeText(k))
    -- inspect color
    asrt.isnil(editor.GetColor(ID, 1, 1))    -- coloritem 1 does not exist
    local clr = asrt.table(editor.GetColor(ID, 1, 0))
    asrt.eq(clr.StartPos, colstart+k)
    asrt.eq(clr.EndPos, colend+k)
    local Color = asrt.table(clr.Color)
    asrt.eq(16*Color.BackgroundColor + Color.ForegroundColor, colbase+k)
    asrt.eq(Color.Flags, F.FCF_FG_INDEX + F.FCF_BG_INDEX)
    asrt.istrue(editor.DelColor(ID, 1))
    asrt.isnil(editor.GetColor(ID, 1, 0))    -- coloritem 0 was deleted
  end

  -- editor.Select
  local selstart, selwidth = 2, 3
  for k=1,N do asrt.istrue(editor.Select(ids[k], "BTYPE_STREAM", 1, selstart + k, selwidth, 1)) end

  -- GetFileName, GetInfo, GetSelection
  for k=1,N do
    local ID = ids[k]
    -- editor.GetFileName
    asrt.eq(editor.GetFileName(ID), MakeFName(k))
    -- editor.GetInfo
    local info = asrt.table(editor.GetInfo(ID))
    asrt.eq(info.EditorID, ID)
    asrt.eq(info.BlockType, F.BTYPE_STREAM)
    asrt.eq(info.BlockStartLine, 1)
    asrt.eq(info.FileName, MakeFName(k))
    -- editor.GetSelection
    local sel = asrt.table(editor.GetSelection(ID))
    asrt.eq(sel.BlockType, F.BTYPE_STREAM)
    asrt.eq(sel.StartLine, 1)
    asrt.eq(sel.StartPos, selstart + k)
    asrt.eq(sel.EndLine, 1)
    asrt.eq(sel.EndPos, selstart + k + selwidth - 1)
  end

  -- editor.Quit
  for k = N, 1, -1 do
    asrt.istrue(editor.Quit(ids[k]))
    asrt.eq(k, actl.GetWindowCount())
  end
  asrt.istrue(Area.Shell)
end

-- commit bc3a1124
-- Editor: fix last line deletion
local function test_editor_bc3a1124()
  local flags = "EF_NONMODAL EF_IMMEDIATERETURN EF_DISABLEHISTORY"
  local fname = far.MkTemp()

  editor.Editor(fname,nil,nil,nil,nil,nil,flags)
  asrt.istrue(Area.Editor)
  Keys("Enter CtrlZ F2 Esc")
  asrt.istrue(Area.Shell)

  editor.Editor(fname,nil,nil,nil,nil,nil,flags)
  asrt.istrue(Area.Editor)
  local info = asrt.table(editor.GetInfo())
  asrt.eq(1, info.TotalLines)

  Keys("Esc")
  asrt.istrue(Area.Shell)
  asrt.istrue(win.DeleteFile(fname))
end

local function test_mantis_2571()
  OpenTempFile(true)

  -- input some text
  for k = 1,8 do
    editor.InsertString()
    editor.SetString(nil, k, "12345678")
  end
  for _ = 1,4 do editor.InsertString() end

  local EI = asrt.table(editor.GetInfo())
  asrt.eq(13, EI.TotalLines)

  -- insert a vertical block
  Keys("CtrlHome 3*AltRight 7*AltDown CtrlC CtrlU CtrlV")
  EI = asrt.table(editor.GetInfo())
  asrt.eq(15, EI.TotalLines)

  -- undo
  Keys("CtrlZ")
  EI = asrt.table(editor.GetInfo())
  asrt.eq(13, EI.TotalLines)

  -- check
  for k = 1,EI.TotalLines do
    local T = asrt.table(editor.GetString(nil, k))
    asrt.eq(T.StringText, k <= 8 and "12345678" or "")
  end

  -- clean
  Keys("F2 Esc")
  asrt.istrue(Area.Shell)
end

local function test_mantis_3367()
  OpenTempFile(true)

  -- set ESPT_CURSORBEYONDEOL option to true
  local EI = asrt.table(editor.GetInfo())
  asrt.eq(1, EI.TotalLines)
  asrt.neq(0, band(EI.CurState, F.ECSTATE_SAVED))
  local BeyondEOL = band(EI.Options, F.EOPT_CURSORBEYONDEOL) ~= 0
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", true))

  -- move right and input some characters
  Keys("8*Right a b c")
  EI = asrt.table(editor.GetInfo())
  asrt.eq(0, band(EI.CurState, F.ECSTATE_SAVED))
  local T = asrt.table(editor.GetString(nil, 1))
  asrt.eq(T.StringText, "        abc")

  -- undo the input characters
  Keys("CtrlZ")
  EI = asrt.table(editor.GetInfo())
  asrt.neq(0, band(EI.CurState, F.ECSTATE_SAVED))
  T = asrt.table(editor.GetString(nil, 1))
  asrt.eq(T.StringText, "")

  -- restore BeyondEOL and clean
  asrt.istrue(editor.SetParam(nil, "ESPT_CURSORBEYONDEOL", BeyondEOL))
  Keys("Esc")
  asrt.istrue(Area.Shell)
end

local function test_all()
  test_Editor_Sel()
--  test_Editor_InsStr()
  test_Misc()
  test_issue_3129()
--  test_multiple_instances()
  test_editor_bc3a1124()
  test_mantis_2571()
  test_mantis_3367()
end

return {
  test_all = test_all;
}
