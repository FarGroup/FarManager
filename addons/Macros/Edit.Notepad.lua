Macro {
  area="Editor"; key="CtrlF"; description="Editor: Find..."; action = function()
Keys('F7')
  end;
}

Macro {
  area="Editor"; key="F3"; description="Editor: Find Next"; action = function()
Keys('ShiftF7')
  end;
}

Macro {
  area="Editor"; key="CtrlH"; description="Editor: Replace..."; action = function()
Keys('CtrlF7')
  end;
}

Macro {
  area="Editor"; key="CtrlG"; description="Editor: Go To..."; action = function()
Keys('AltF8')
  end;
}

Macro {
  area="Editor"; key="F5"; description="Editor: Paste Time/Date"; action = function()
mf.print(mf.date('%R %x'))
  end;
}

Macro {
  area="Editor"; key="CtrlN"; description="Editor: New File"; action = function()
Keys('ShiftF4 Enter')
  end;
}

Macro {
  area="Editor"; key="CtrlO"; description="Editor: Open..."; action = function()
Keys('ShiftF4')
  end;
}

Macro {
  area="Editor"; key="CtrlS"; description="Editor: Save File"; action = function()
Keys('F2')
  end;
}

