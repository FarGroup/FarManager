Macro {
  description="Use Alt to toggle fast find mode";
  area="Shell";
  key="Alt";
  flags="NoSendKeysToPlugins";
  action=function()
    Keys("Alt<")
  end;
}

Macro {
  description="Use Alt to toggle fast find mode";
  area="Search";
  key="Alt";
  action=function()
    Keys("Esc")
  end;
}
