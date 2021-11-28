Macro {
  description="Autodetect code page";
  area="Editor Viewer";
  key="ShiftF9";
  flags="NoSendKeysToPlugins";
  action=function()
    Keys("ShiftF8 Home Enter")
  end;
}

Macro {
  description="Cycle through all codepages";
  area="Viewer";
  key="F9";
  flags="NoSendKeysToPlugins";
  action=function()
    Keys("ShiftF8 Down")
    if Object.Bof then -- skip Automatic detection
      Keys("Down")
    end
    Keys("Enter")
  end;
}

Macro {
  description="Cycle through codepages (excl. Unicode)";
  area="Editor";
  key="F9";
  flags="NoSendKeysToPlugins";
  action=function()
    Keys("ShiftF8")
    repeat
      Keys("Down")
      -- skip "Automatic detection" and Unicode CPs
      if not (Object.Bof or Menu.Value:find("UTF%-")) then
        Keys("Enter")
      end
    until Menu.Id~=far.Guids.CodePagesMenuId
  end;
}
