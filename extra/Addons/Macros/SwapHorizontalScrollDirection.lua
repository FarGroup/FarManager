local function LeftToRight()
    Keys(mf.akey(1,1):gsub("Left", "Right"):gsub("Num4", "Num6"))
end

local function RightToLeft()
    Keys(mf.akey(1,1):gsub("Right", "Left"):gsub("Num6", "Num4"))
end

Macro {
  area="Viewer";
  description="Swap horizontal scroll direction";
  key=
    "Left Num4 ShiftNum4"               -- Horizontal scroll in Text and Hex modes
    .. " CtrlLeft CtrlNum4"             -- Horizontal scroll by 20 in Text mode; roll the contents in Dump and Hex modes
    .. " MsWheelLeft AltMsWheelLeft";   -- Translates to "Left" repeated as necessary
  action = LeftToRight;
}

Macro {
  area="Viewer";
  description="Swap horizontal scroll direction";
  key=
    "Right Num6 ShiftNum6"              -- Horizontal scroll in Text and Hex modes
    .. " CtrlRight CtrlNum6"            -- Horizontal scroll by 20 in Text mode; roll the contents in Dump and Hex modes
    .. " MsWheelRight AltMsWheelRight"; -- Translates to "Right" repeated as necessary
  action = RightToLeft;
}

Macro {
  area="Menu";
  description="Swap horizontal scroll direction";
  key=
    "AltLeft AltNum4 MsWheelLeft"                   -- Horizontal scroll all items by 1 position
    .. " CtrlAltLeft CtrlAltNum4 CtrlMsWheelLeft"   -- Horizontal scroll all items by 20 positions
    .. " AltShiftLeft AltShiftNum4"                 -- Horizontal scroll the current item by 1 position
    .. " CtrlShiftLeft CtrlShiftNum4";              -- Horizontal scroll the current item by 20 positions
  action = LeftToRight;
}

Macro {
  area="Menu";
  description="Swap horizontal scroll direction";
  key=
    "AltRight AltNum6 MsWheelRight"                 -- Horizontal scroll all items by 1 position
    .. " CtrlAltRight CtrlAltNum6 CtrlMsWheelRight" -- Horizontal scroll all items by 20 positions
    .. " AltShiftRight AltShiftNum6"                -- Horizontal scroll the current item by 1 position
    .. " CtrlShiftRight CtrlShiftNum6";             -- Horizontal scroll the current item by 20 positions
  action = RightToLeft;
}

Macro {
  area="Shell";
  description="Swap horizontal scroll direction";
  flags="EmptyCommandLine";
  key= "AltLeft";   -- Horizontal scroll of file names and descriptions
  action = LeftToRight;
}

Macro {
  area="Shell";
  description="Swap horizontal scroll direction";
  flags="EmptyCommandLine";
  key= "AltRight";  -- Horizontal scroll of file names and descriptions
  action = RightToLeft;
}
