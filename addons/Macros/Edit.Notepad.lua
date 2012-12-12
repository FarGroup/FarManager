Macro {
  area="Editor"; key="CtrlF"; flags="DisableOutput"; description="Editor: Find..."; action = function()
Keys('F7')
  end;
}

Macro {
  area="Editor"; key="F3"; flags="DisableOutput"; description="Editor: Find Next"; action = function()
Keys('ShiftF7')
  end;
}

Macro {
  area="Editor"; key="CtrlH"; flags="DisableOutput"; description="Editor: Replace..."; action = function()
Keys('CtrlF7')
  end;
}

Macro {
  area="Editor"; key="CtrlG"; flags="DisableOutput"; description="Editor: Go To..."; action = function()
Keys('AltF8')
  end;
}

Macro {
  area="Editor"; key="F5"; flags="DisableOutput"; description="Editor: Paste Time/Date"; action = function()
mf.print(mf.date('%R %x'))
  end;
}

Macro {
  area="Editor"; key="CtrlN"; flags="DisableOutput"; description="Editor: New File"; action = function()
Keys('ShiftF4 Enter')
  end;
}

Macro {
  area="Editor"; key="CtrlO"; flags="DisableOutput"; description="Editor: Open..."; action = function()
Keys('ShiftF4')
  end;
}

Macro {
  area="Editor"; key="CtrlS"; flags="DisableOutput"; description="Editor: Save File"; action = function()
Keys('F2')
  end;
}

