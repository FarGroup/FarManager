Macro {
  area="Shell";
  key="ShiftF1";
  description="Select archive format"; -- multiarc
  action = function()
    Keys("ShiftF1")
    if Area.Dialog then
      Keys("ShiftF1")
    end
  end;
}

