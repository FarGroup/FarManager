----------------------------------------------------------------
-- To install this file, copy it to %FARPROFILE%\Macros\scripts
-- directory and restart Far Manager.
----------------------------------------------------------------

Macro {
  area="Dialog"; key="AltDown";
  description="Alt+Down will open history in input lines";
  action=function() Keys("CtrlDown") end;
}

Macro {
  area="Shell"; key="Del"; flags="EmptyCommandLine";
  description="Del will delete files in the shell";
  action=function() Keys("F8") end;
}

Macro {
  area="Editor"; key="CtrlDel"; flags="EVSelection";
  description="Ctrl-Del will delete selection or words in the editor";
  action=function() Keys("CtrlD") end;
}

Macro {
  area="Editor"; key="CtrlPgDn";
  description="Ctrl-PgDn will move cursor to the end of the page";
  action=function() Keys("CtrlE") end;
}

Macro {
  area="Editor"; key="CtrlPgUp";
  description="Ctrl-PgUp will move cursor to the beginning of the page";
  action=function() Keys("CtrlN") end;
}

Macro {
  area="Editor"; key="CtrlS";
  description="Ctrl-S will save the file in the editor";
  action=function() Keys("F2") end;
}

Macro {
  area="Editor"; key="CtrlF";
  description="Ctrl-F will open search dialog in the editor";
  action=function() Keys("F7") end;
}

Macro {
  area="Editor"; key="F3";
  description="F3 will continue search";
  action=function() Keys("ShiftF7") end;
}
