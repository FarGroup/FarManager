Macro {
  area="Shell"; key="Space"; flags="EmptyCommandLine"; description="Panel: Use Space to select files"; action = function()
if APanel.Visible or PPanel.Visible then Keys("Ins") else Keys("Space") end
  end;
}

