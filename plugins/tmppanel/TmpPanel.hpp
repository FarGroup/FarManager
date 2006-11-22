/*
TMPPANEL.HPP

Temporary panel header file

*/

#ifndef __TMPPANEL_HPP__
#define __TMPPANEL_HPP__

#define COMMONPANELSNUMBER 10

typedef struct _MyInitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  DWORD Flags;
  signed char Data;
} MyInitDialogItem;

typedef struct _PluginPanels
{
  PluginPanelItem *Items;
  unsigned int ItemsNumber;
  unsigned int OpenFrom;
} PluginPanels;

extern PluginPanels CommonPanels[COMMONPANELSNUMBER];

extern unsigned int CurrentCommonPanel;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

extern int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;
extern char PluginRootKey[80];

const char *GetMsg(int MsgId);
void InitDialogItems(const MyInitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber);

int Config();
void GoToFile(const char *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, DWORD Total);

char* my_strchr(const char * string,int ch);
char* my_strrchr(const char * string,int ch);

char *ParseParam(char *& str);
void GetOptions(void);
void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd);

#endif /* __TMPPANEL_HPP__ */
