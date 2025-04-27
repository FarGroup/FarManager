Macro {
  description="Menu: CtrlClear toggles vertical alignment between 'All found entries' and 'Home'";
  area="Menu";
  key="CtrlClear";
  action = function()
    -- Menu.HorizontalAlignment:
    --  0 Menu items are not aligned
    --  1 Menu items are left-aligned
    --  2 Menu items are right-aligned
    --  4 Menu items are aligned at annotations, i.e., the found search pattern in editor's Find All menu
    -- -1 The active object is not a menu
    if Menu.HorizontalAlignment == 4 then
      Keys("AltHome")
    else
      Keys("CtrlClear")
    end
  end;
}
