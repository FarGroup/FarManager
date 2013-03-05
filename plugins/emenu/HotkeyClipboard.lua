-- Hotkeys to execute cut/copy/paste commands: Ctrl-X, Ctrl-C, Ctrl-V

Macro {
  area="Shell"; key="CtrlC"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:copy") Keys("Enter")
  end;
}

Macro {
  area="Shell"; key="CtrlV"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:paste") Keys("Enter")
  end;
}

Macro {
  area="Shell"; key="CtrlX"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:cut") Keys("Enter")
  end;
}

Macro {
  area="Tree"; key="CtrlC"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:copy") Keys("Enter")
  end;
}

Macro {
  area="Tree"; key="CtrlV"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:paste") Keys("Enter")
  end;
}

Macro {
  area="Tree"; key="CtrlX"; flags=""; action = function()
Keys("Esc") Far.DisableHistory(0) print("rclk_cmd:cut") Keys("Enter")
  end;
}

