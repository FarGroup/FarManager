local args = ...
local M = args.M
local ffi = require "ffi"
local C = ffi.C
local F = far.Flags
local PPIF_SELECTED = F.PPIF_SELECTED
local FILE_ATTRIBUTE_DIRECTORY = 0x00000010
local MCODE_F_SETCUSTOMSORTMODE = 0x80C68
local band, bor = bit.band, bit.bor -- 32 bits, be careful
local tonumber = tonumber

local CustomSortModes = {} -- key=integer, value=table

-- shellsort [ http://lua-users.org/wiki/LuaSorting ]
-- Written by Rici Lake. The author disclaims all copyright and offers no warranty.
-- For convenience, shellsort returns its first argument.

local incs = { 1391376,
               463792, 198768, 86961, 33936,
               13776, 4592, 1968, 861, 336,
               112, 48, 21, 7, 3, 1 }

local function shellsort(t, n, before, save, restore, swap)
  for _, h in ipairs(incs) do
    for i = h + 1, n do
      local v = save(t,i-1)
      for j = i - h, 1, -h do
        if not before(v, t, j-1) then break end
        swap(t, i-1, t, j-1); i = j
      end
      restore(t, i-1, v)
    end
  end
  return t
end

-- qsort [ extracted from Lua 5.1 distribution (file /test/sort.lua) ]
local function qsort(x,l,u,f)
  if l<u then
    local m=math.random(u-(l-1))+l-1 -- choose a random pivot in range l..u
    x[l],x[m]=x[m],x[l] -- swap pivot to first position
    local t=x[l]        -- pivot value
    m=l
    local i=l+1
    while i<=u do
      -- invariant: x[l+1..m] < t <= x[m+1..i-1]
      if f(x[i],t) then
        m=m+1
        x[m],x[i]=x[i],x[m]  -- swap x[i] and x[m]
      end
      i=i+1
    end
    x[l],x[m]=x[m],x[l]      -- swap pivot to a valid place
    -- x[l+1..m-1] < x[m] <= x[m+1..u]
    qsort(x,l,m-1,f)
    qsort(x,m+1,u,f)
  end
end

local BooleanProperties = {
  "InvertByDefault", "NoSortEqualsByName"
}
local TernaryProperties = {
  "DirectoriesFirst", "SelectedFirst", "RevertSorting", "SortGroups"
}

local function LoadCustomSortMode (nMode, Settings)
  assert(type(nMode)=="number" and nMode==math.floor(nMode) and nMode>=100 and nMode<=0x7FFFFFFF)
  if Settings then
    assert(type(Settings)=="table")
    assert(type(Settings.Compare)=="function")

    local t = { Compare=Settings.Compare }
    if type(Settings.InitSort)=="function" then t.InitSort=Settings.InitSort end
    if type(Settings.EndSort)=="function" then t.EndSort=Settings.EndSort end

    for _,v in ipairs(BooleanProperties) do t[v] = not not Settings[v] end
    for _,v in ipairs(TernaryProperties) do t[v] = Settings[v]==0 and 0 or Settings[v]==1 and 1 or 2 end

    if type(Settings.Description)=="string" then t.Description = Settings.Description end
    if type(Settings.Indicator)=="string" then
      t.Indicator = Settings.Indicator
      local len = t.Indicator:len()
      if len<2 then t.Indicator = t.Indicator..(" "):rep(2-len) end
    else
      t.Indicator = "  "
    end
    CustomSortModes[nMode] = t
  else
    CustomSortModes[nMode] = nil
  end
end

local function SetCustomSortMode (nMode, whatpanel)
  local Settings = CustomSortModes[nMode]
  if Settings then
    whatpanel = whatpanel==1 and 1 or 0
    far.MacroCallFar(MCODE_F_SETCUSTOMSORTMODE, whatpanel, nMode, Settings.InvertByDefault)
  end
end

local function CustomSortMenu()
  local items, bkeys = {}, {{BreakKey="C+RETURN"},{BreakKey="CS+RETURN"}}
  for k,v in pairs(CustomSortModes) do
    items[#items+1] = { text=v.Description and tostring(v.Description) or M.PSDefaultMenuItemText..k; Mode=k; }
  end
  table.sort(items, function(a,b) return a.Mode < b.Mode end)
  local r, pos = far.Menu({Title=M.PSMenuTitle}, items, bkeys)
  if r then
    if r.BreakKey == "C+RETURN" then
      SetCustomSortMode(items[pos].Mode,1)
    elseif r.BreakKey == "CS+RETURN" then
      SetCustomSortMode(items[pos].Mode,0)
      SetCustomSortMode(items[pos].Mode,1)
    else
      SetCustomSortMode(r.Mode,0)
    end
  end
end

local function GetSortModes()
  local t = {}
  for mode in pairs(CustomSortModes) do
    t[#t+1]=mode; t[#t+1]=mode; t[#t+1]=mode;
  end
  table.sort(t)
  for k=1,#t,3 do
    local mode = t[k]
    t[k+1] = CustomSortModes[mode].InvertByDefault
    t[k+2] = CustomSortModes[mode].Description or "Sort mode "..mode
  end
  t.n = #t
  return t
end

ffi.cdef[[
  struct FileListItem;
  typedef struct
  {
    struct FileListItem        *Data;
    size_t                      DataSize;
    void                      (*FileListToPluginItem)(const struct FileListItem*, struct SortingPanelItem*);
    void                      (*FileListToPluginItem2)(const struct FileListItem*, size_t, struct SortingPanelItem*);
    struct FileListItem*      (*save)(struct FileListItem*, size_t);
    void                      (*restore)(struct FileListItem*, size_t, struct FileListItem*);
    void                      (*swap)(struct FileListItem*, size_t, struct FileListItem*, size_t);
    int                         SortGroups;
    int                         SelectedFirst;
    int                         DirectoriesFirst;
    int                         SortMode;
    int                         RevertSorting;
    int                         NumericSort;
    int                         CaseSensitiveSort;
    HANDLE                      hSortPlugin;
  } CustomSort;
]]

-- local function Utf16Buf (str)
--   str = win.Utf8ToUtf16(str)
--   local buf = ffi.new("wchar_t[?]", #str/2+1)
--   ffi.copy(buf, str, #str)
--   return buf
-- end

-- local function wcscmp(p1,p2)
--   local pos = 0
--   while p1[pos] == p2[pos] do
--     if p1[pos] == 0 then return 0 end
--     pos = pos + 1
--   end
--   return p1[pos] - p2[pos]
-- end

local function IsTwoDots(p) return p[0]==46 and p[1]==46 and p[2]==0 end
local function Empty(p) return p[0]==0 end

-- called from Far
local function SortPanelItems (params)
  jit.flush()
  params = ffi.cast("CustomSort*", params)
  local tSettings = CustomSortModes[tonumber(params.SortMode)]
  if not tSettings then return end

  local Compare = tSettings.Compare
  local SortEqualsByName = not tSettings.NoSortEqualsByName

  local pi1 = ffi.new("struct SortingPanelItem")
  local pi2 = ffi.new("struct SortingPanelItem")

  local SortGroups, SelectedFirst, DirectoriesFirst, RevertSorting

  if tSettings.SortGroups==0 then SortGroups=false
  elseif tSettings.SortGroups==1 then SortGroups=true
  else SortGroups = params.SortGroups ~= 0
  end

  if tSettings.SelectedFirst==0 then SelectedFirst=false
  elseif tSettings.SelectedFirst==1 then SelectedFirst=true
  else SelectedFirst = params.SelectedFirst ~= 0
  end

  if tSettings.DirectoriesFirst==0 then DirectoriesFirst=false
  elseif tSettings.DirectoriesFirst==1 then DirectoriesFirst=true
  else DirectoriesFirst = params.DirectoriesFirst ~= 0
  end

  if tSettings.RevertSorting==0 then RevertSorting=false
  elseif tSettings.RevertSorting==1 then RevertSorting=true
  else RevertSorting = params.RevertSorting ~= 0
  end

  local outParams = {
    SortGroups        = params.SortGroups ~= 0;
    SelectedFirst     = params.SelectedFirst ~= 0;
    DirectoriesFirst  = params.DirectoriesFirst ~= 0;
    RevertSorting     = params.RevertSorting ~= 0;
    NumericSort       = params.NumericSort ~= 0;
    CaseSensitiveSort = params.CaseSensitiveSort ~= 0;
  }

  local function Before(e1, e2, off)
    params.FileListToPluginItem(e1, pi1)
    params.FileListToPluginItem2(e2, off, pi2)
    ----------------------------------------------------------------------------
    -- TODO: fix in Far
    if IsTwoDots(pi1.FileName) then
      if not IsTwoDots(pi2.FileName) then return true end

      if Empty(pi1.AlternateFileName) or IsTwoDots(pi1.AlternateFileName) then
        if not (Empty(pi2.AlternateFileName) or IsTwoDots(pi2.AlternateFileName)) then return true end
      else
        if Empty(pi2.AlternateFileName) or IsTwoDots(pi2.AlternateFileName) then return false end
      end

    elseif IsTwoDots(pi2.FileName) then
      return false
    end
    ----------------------------------------------------------------------------
    if DirectoriesFirst then
      local a1,a2 = band(tonumber(pi1.FileAttributes), FILE_ATTRIBUTE_DIRECTORY),
                    band(tonumber(pi2.FileAttributes), FILE_ATTRIBUTE_DIRECTORY)
      if a1 ~= a2 then return a1 > a2 end
    end
    ----------------------------------------------------------------------------
    if SelectedFirst then
      local s1,s2 = band(tonumber(pi1.Flags),PPIF_SELECTED), band(tonumber(pi2.Flags),PPIF_SELECTED)
      if s1 ~= s2 then return s1 > s2 end
    end
    ----------------------------------------------------------------------------
    if SortGroups then
      if pi1.SortGroup ~= pi2.SortGroup then return pi1.SortGroup < pi2.SortGroup end
    end
    ----------------------------------------------------------------------------
    local r = Compare(pi1, pi2, outParams)
    if r ~= 0 then
      if RevertSorting then r = -r end
      return r < 0
    else
      if SortEqualsByName then
        return 1 == C.CompareStringW(C.LOCALE_USER_DEFAULT, bor(C.NORM_IGNORECASE,C.SORT_STRINGSORT),
                                     pi1.FileName, -1, pi2.FileName, -1)
      end
    end
    return false
  end

  if tSettings.InitSort then tSettings.InitSort(outParams) end

  shellsort(params.Data, tonumber(params.DataSize), Before, params.save, params.restore, params.swap)
  -- qsort(params.Data, 0, tonumber(params.DataSize)-1, Before)

  if tSettings.EndSort then tSettings.EndSort() end

  return tSettings.Indicator
end

return {
  SortPanelItems=SortPanelItems,
  LoadCustomSortMode=LoadCustomSortMode,
  SetCustomSortMode=SetCustomSortMode,
  CustomSortMenu=CustomSortMenu,
  GetSortModes=GetSortModes,
  DeleteSortModes=function() CustomSortModes={} end,
}
