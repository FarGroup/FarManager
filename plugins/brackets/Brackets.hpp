struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  int Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
};

struct Options{
  short IgnoreQuotes;	// �ࠢ���: "�����஢��� ᪮���, �����祭�� � ����窨"
  short IgnoreAfter;    // �ࠢ���: "�����஢��� �� ᪮����"
  short BracketPrior;	// �ࠢ���: "�ਮ��� ᪮���"
  short JumpToPair;     // �ࠢ���: "��३� �� ����� ᪮��� �� �⬥⪥ �����"
  short Beep;
  short Reserved[2];
  char  QuotesType[21]; // ⨯� ����祪
  char  Brackets1[21];  // ������� ᪮���
  char  Brackets2[41];  // ������ ᪮���
  char  Dummy;
} Opt;

enum{
  BrZERO,
  BrOne,
  BrTwo,
  BrColorer,
  BrOneMath,
};

static struct PluginStartupInfo Info;
static char PluginRootKey[80];

int Config();

