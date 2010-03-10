struct Options
{
	int AddToDisksMenu;
	int DisksMenuDigit;
	char DefaultFolder[512];
};

struct InitDialogItem
{
	unsigned char Type;
	unsigned char X1,Y1,X2,Y2;
	unsigned char Focus;
	unsigned int Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	char *Data;
};


extern struct Options Opt;
extern char PluginRootKey[80];
extern struct PluginStartupInfo Info;

extern const char *AddToDisksMenu;
extern const char *DisksMenuDigit;
extern const char *DefaultFolder;


char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);

int Config();
void createRegistry(char *PluginRootKey1);
char *getDesktop(void);
