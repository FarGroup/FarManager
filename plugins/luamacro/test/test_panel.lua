-- test_panel.lua
local asrt = assert(loadfile(far.PluginStartupInfo().ModuleDir.."test\\assert.lua")) ()


local function test_XPanel(pan) -- (@pan: either APanel or PPanel)
  asrt.bool (pan.Bof)
  asrt.num  (pan.ColumnCount)
  asrt.num  (pan.CurPos)
  asrt.str  (pan.Current)
  asrt.num  (pan.DriveType)
  asrt.bool (pan.Empty)
  asrt.bool (pan.Eof)
  asrt.bool (pan.FilePanel)
  asrt.bool (pan.Filter)
  asrt.bool (pan.Folder)
  asrt.str  (pan.Format)
  asrt.num  (pan.Height)
  asrt.str  (pan.HostFile)
  asrt.num  (pan.ItemCount)
  asrt.bool (pan.Left)
  asrt.bool (pan.LFN)
  asrt.num  (pan.OPIFlags)
  asrt.str  (pan.Path)
  asrt.str  (pan.Path0)
  asrt.bool (pan.Plugin)
  asrt.str  (pan.Prefix)
  asrt.bool (pan.Root)
  asrt.num  (pan.SelCount)
  asrt.bool (pan.Selected)
  asrt.num  (pan.Type)
  asrt.str  (pan.UNCPath)
  asrt.bool (pan.Visible)
  asrt.num  (pan.Width)

  if pan == APanel then
    Keys "End"  asrt.istrue(pan.Eof)
    Keys "Home" asrt.istrue(pan.Bof)
  end
end

local test_APanel = function() test_XPanel(APanel) end
local test_PPanel = function() test_XPanel(PPanel) end


local function test_Panel_Item()
  local index = 0 -- 0 is the current element, otherwise element index
  for pan=0,1 do
    asrt.str    (Panel.Item(pan,index,0))  -- file name
    asrt.str    (Panel.Item(pan,index,1))  -- short file name
    asrt.num    (Panel.Item(pan,index,2))  -- file attributes
    asrt.str    (Panel.Item(pan,index,3))  -- creation time
    asrt.str    (Panel.Item(pan,index,4))  -- last access time
    asrt.str    (Panel.Item(pan,index,5))  -- modification time
    asrt.numint (Panel.Item(pan,index,6))  -- size
    asrt.numint (Panel.Item(pan,index,7))  -- packed size
    asrt.bool   (Panel.Item(pan,index,8))  -- selected
    asrt.num    (Panel.Item(pan,index,9))  -- number of links
    asrt.num    (Panel.Item(pan,index,10)) -- sort group
    asrt.str    (Panel.Item(pan,index,11)) -- diz text
    asrt.str    (Panel.Item(pan,index,12)) -- owner
    asrt.num    (Panel.Item(pan,index,13)) -- crc32
    asrt.num    (Panel.Item(pan,index,14)) -- position when read from the file system
    asrt.numint (Panel.Item(pan,index,15)) -- creation time
    asrt.numint (Panel.Item(pan,index,16)) -- last access time
    asrt.numint (Panel.Item(pan,index,17)) -- modification time
    asrt.num    (Panel.Item(pan,index,18)) -- number of streams
    asrt.numint (Panel.Item(pan,index,19)) -- size of streams
    asrt.str    (Panel.Item(pan,index,20)) -- change time
    asrt.numint (Panel.Item(pan,index,21)) -- change time
    asrt.isfalse(pcall(Panel.Item,pan,index,22))
    asrt.num    (Panel.Item(pan,index,23))
  end
end


local function test_Panel_SetPath()
  -- store
  local adir_old = panel.GetPanelDirectory(nil,1).Name
  local pdir_old = panel.GetPanelDirectory(nil,0).Name
  --test
  local pdir = "c:\\windows"
  local adir = "c:\\windows\\system32"
  local afile = "cmd.exe"
  asrt.istrue(Panel.SetPath(1, pdir))
  asrt.istrue(Panel.SetPath(0, adir, afile))
  asrt.eq (pdir, panel.GetPanelDirectory(nil,0).Name:lower())
  asrt.eq (adir, panel.GetPanelDirectory(nil,1).Name:lower())
  asrt.eq (panel.GetCurrentPanelItem(nil,1).FileName:lower(), afile)
  -- restore
  asrt.istrue(Panel.SetPath(1, pdir_old))
  asrt.istrue(Panel.SetPath(0, adir_old))
end

-- N=Panel.Select(panelType,Action[,Mode[,Items]])
local function test_Panel_Select()
  local adir_old = panel.GetPanelDirectory(nil,1) -- store active panel directory

  local PS = asrt.func(Panel.Select)
  local RM,ADD,INV,RST = 0,1,2,3 -- Action
  local MODE

  local dir = asrt.str(win.GetEnv("FARHOME"))
  asrt.istrue(panel.SetPanelDirectory(nil,1,dir))
  local pi = asrt.table(panel.GetPanelInfo(1))
  local ItemsCount = asrt.num(pi.ItemsNumber)-1 -- don't count ".."
  assert(ItemsCount>=10, "not enough files to test")

  --------------------------------------------------------------
  MODE = 0
  asrt.eq(ItemsCount,PS(0,ADD,MODE)) -- select all
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(ItemsCount, pi.SelectedItemsNumber)

  asrt.eq(ItemsCount,PS(0,RM,MODE)) -- clear all
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(ItemsCount,PS(0,INV,MODE)) -- invert
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(ItemsCount, pi.SelectedItemsNumber)

  asrt.eq(0,PS(0,INV,MODE)) -- invert again (return value is the selection count, contrary to docs)
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 1
  asrt.eq(1,PS(0,ADD,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(1, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,RM,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,INV,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(1, pi.SelectedItemsNumber)

  asrt.eq(1,PS(0,INV,MODE,5))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 2
  local list = win.JoinPath(dir,"FarEng.hlf").."\nFarEng.lng" -- the 1-st file with path, the 2-nd without
  asrt.eq(2,PS(0,ADD,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(2, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,RM,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,INV,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(2, pi.SelectedItemsNumber)

  asrt.eq(2,PS(0,INV,MODE,list))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  --------------------------------------------------------------
  MODE = 3
  local mask = "*.hlf;*.lng"
  local count = 0
  for i=1,pi.ItemsNumber do
    local item = asrt.table(panel.GetPanelItem(nil,1,i))
    if far.CmpNameList(mask, item.FileName) then count=count+1 end
  end
  assert(count>1, "not enough files to test")

  asrt.eq(count,PS(0,ADD,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(count, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,RM,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,INV,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(count, pi.SelectedItemsNumber)

  asrt.eq(count,PS(0,INV,MODE,mask))
  pi = asrt.table(panel.GetPanelInfo(1))
  asrt.eq(0, pi.SelectedItemsNumber)

  panel.SetPanelDirectory(nil,1,adir_old) -- restore active panel directory
end


local function test_all()
  test_APanel()
  test_PPanel()
  test_Panel_Item()
  test_Panel_SetPath()
  test_Panel_Select()

  asrt.eq (Panel.FAttr(0, ":"), -1)
  asrt.eq (Panel.FAttr(1, ":"), -1)

  asrt.eq (Panel.FExist(0, ":"), 0)
  asrt.eq (Panel.FExist(1, ":"), 0)

  asrt.func (Panel.SetPos)
  asrt.func (Panel.SetPosIdx)
end


return {
  test_all = test_all;
}
