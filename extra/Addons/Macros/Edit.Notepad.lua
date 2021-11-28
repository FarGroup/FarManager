local notepad = {
  CtrlF = "F7",                 --Find...
  F3    = "ShiftF7",            --Find Next
  CtrlH = "CtrlF7",             --Replace...
  CtrlG = "AltF8",              --Go To...
  --
  CtrlN = "ShiftF4 CtrlY Enter",--New File
  CtrlO = "ShiftF4",            --Open...
  CtrlS = "F2",                 --Save File
}

local keys = "";
for k,_ in pairs(notepad) do keys = keys.." "..k end

Macro {
  description="Editor: Notepad-like shortcuts";
  area="Editor";
  key=keys;
  action=function()
    Keys(notepad[mf.akey(1,1)])
  end;
}

Macro {
  description="Editor: paste time/date";
  area="Editor";
  key="F5";
  action=function()
    mf.print(os.date("%H:%M %d.%m.%Y")) --mf.date("%R %x"))
  end;
}
