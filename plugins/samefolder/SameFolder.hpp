struct OptionsName
{
	const wchar_t *Add2PlugMenu;
	const wchar_t *Add2DisksMenu;
	const wchar_t *SetMode;
};

struct Options
{
	int Add2PlugMenu;
	int Add2DisksMenu;
	int SetMode;
} Opt;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

bool ComparePPI(const PluginPanelItem* PPISrc,const PluginPanelItem* PPIDst);
const wchar_t *GetMsg(int MsgId);
