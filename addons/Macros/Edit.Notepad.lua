Macro {
  area="Editor"; key="CtrlF"; flags=""; description="Editor: Find..."; action = function()
Keys('F7')
  end;
}

Macro {
  area="Editor"; key="F3"; flags=""; description="Editor: Find Next"; action = function()
Keys('ShiftF7')
  end;
}

Macro {
  area="Editor"; key="CtrlH"; flags=""; description="Editor: Replace..."; action = function()
Keys('CtrlF7')
  end;
}

Macro {
  area="Editor"; key="CtrlG"; flags=""; description="Editor: Go To..."; action = function()
Keys('AltF8')
  end;
}

Macro {
  area="Editor"; key="F5"; flags=""; description="Editor: Paste Time/Date"; action = function()
mf.print(mf.date('%R %x'))
  end;
}

Macro {
  area="Editor"; key="CtrlN"; flags=""; description="Editor: New File"; action = function()
Keys('ShiftF4 Enter')
  end;
}

Macro {
  area="Editor"; key="CtrlO"; flags=""; description="Editor: Open..."; action = function()
Keys('ShiftF4')
  end;
}

Macro {
  area="Editor"; key="CtrlS"; flags=""; description="Editor: Save File"; action = function()
Keys('F2')
  end;
}

