local JumpToSelectedFile = function (p,from,size,step)
  if p.SelectedItemsNumber>0 then
    local to=from+size*step
    for ii=from,to,step do
      local pos=ii>size and ii-size or (ii<1 and ii+size or ii)
      local item=panel.GetPanelItem(nil,1,pos)
      if bit64.band(item.Flags,far.Flags.PPIF_SELECTED)~=0 then
        local bottomItem=p.TopPanelItem+p.PanelRect.bottom-p.PanelRect.top
        local topItem=(pos>=p.TopPanelItem and pos<=bottomItem) and p.TopPanelItem or nil
        panel.RedrawPanel(nil,1,{CurrentItem=pos,TopPanelItem=topItem})
        break
      end
    end
  end
end

Macro {
  area="Shell"; key="CtrlShiftDown"; flags="Selection"; description="Jump to the next selected file"; action = function()

local p=panel.GetPanelInfo(nil, 1)
JumpToSelectedFile(p,p.CurrentItem+1,p.ItemsNumber,1)

  end;
}

Macro {
  area="Shell"; key="CtrlShiftEnd"; flags="Selection"; description="Jump to the last selected file"; action = function()

local p=panel.GetPanelInfo(nil, 1)
JumpToSelectedFile(p,p.ItemsNumber,p.ItemsNumber,-1)

  end;
}

Macro {
  area="Shell"; key="CtrlShiftHome"; flags="Selection"; description="Jump to the first selected file"; action = function()

local p=panel.GetPanelInfo(nil, 1)
JumpToSelectedFile(p,1,p.ItemsNumber,1)

  end;
}

Macro {
  area="Shell"; key="CtrlShiftUp"; flags="Selection"; description="Jump to the previous selected file"; action = function()

local p=panel.GetPanelInfo(nil, 1)
JumpToSelectedFile(p,p.CurrentItem-1,p.ItemsNumber,-1)

  end;
}

