local sound = 0x30 -- MB_ICONEXCLAMATION

local Panels = "Shell QView Tree Info Search"

Macro {
  description="Go to previous folder in history";
  area=Panels;
  key="AltBS";
  action=function()
    Keys("AltF12")
    if Object.Bof then
      mf.beep(sound)
      Keys("Esc")
      return
    end
    Keys("Up ShiftEnter")
  end
}

Macro {
  description="Go to next folder in history";
  area=Panels;
  key="AltShiftBS";
  action=function()
    Keys("AltF12")
    if Object.Eof then
      mf.beep(sound)
      Keys("Esc")
      return
    end
    Keys("Down ShiftEnter")
  end
}
