struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  int DefaultButton;
  char *Data;
};

struct Options{
  short IgnoreQuotes; // Правило: "Игнорировать скобки, заключенные в кавычки"
  short IgnoreAfter;    // Правило: "Игнорировать за скобкой"
  short BracketPrior; // Правило: "Приоритет скобок"
  short JumpToPair;     // Правило: "Перейти на парную скобку при отметке блока"
  short Beep;
  short Reserved[2];
  char  QuotesType[21]; // типы кавычек
  char  Brackets1[21];  // одинарные скобки
  char  Brackets2[41];  // двойные скобки
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
