/*
TMPCLASS.HPP

Temporary panel plugin class header file

*/

#ifndef __TMPCLASS_HPP__
#define __TMPCLASS_HPP__

#define REMOVE_FLAG 1

class TmpPanel
{
  private:
    void SortList();
    void RemoveDups();
    void RemoveEmptyItems();
    void UpdateItems(int ShowOwners,int ShowLinks);
    int IsOwnersDisplayed (const struct PanelInfo &PInfo);
    int IsLinksDisplayed (const struct PanelInfo &PInfo);
    void ProcessRemoveKey();
    void ProcessSaveListKey();
    void ProcessPanelSwitchMenu();
    void SwitchToPanel (int NewPanelIndex);
    void FindSearchResultsPanel();
    void SaveListFile (const char *Path);
    int IsCurrentFileCorrect (char *pCurFileName);

    PluginPanelItem *TmpPanelItem;
    int TmpItemsNumber;
    int LastOwnersRead;
    int LastLinksRead;
    int UpdateNotNeeded;
  public:
    TmpPanel();
    ~TmpPanel();
    int PanelIndex;
//    int OpenFrom;
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(const char *Dir,int OpMode);

    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    HANDLE BeginPutFiles();
    void CommitPutFiles (HANDLE hRestoreScreen, int Success);
    int PutOneFile (PluginPanelItem &PanelItem);

    int SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber);
    int ProcessEvent(int Event,void *Param);
    int ProcessKey(int Key,unsigned int ControlState);
    static int CheckForCorrect(const char *Dir,FAR_FIND_DATA *FindData,int OpenFrom);
    void IfOptCommonPanel(void);

};

#endif /* __TMPCLASS_HPP__ */
