struct InitDialogItem
{
  int Type;
  int X1, Y1, X2, Y2;
  int Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  int DefaultButton;
  const TCHAR *Data;
};

struct Options{
  short IgnoreQuotes;   // Правило: "Игнорировать скобки, заключенные в кавычки"
  short IgnoreAfter;    // Правило: "Игнорировать за скобкой"
  short BracketPrior;   // Правило: "Приоритет скобок"
  short JumpToPair;     // Правило: "Перейти на парную скобку при отметке блока"
  short Beep;
  short Reserved[2];
  TCHAR  QuotesType[21]; // типы кавычек
  TCHAR  Brackets1[21];  // одинарные скобки
  TCHAR  Brackets2[41];  // двойные скобки
  TCHAR  Dummy;
} Opt;

enum{
  BrZERO,                // не определено
  BrOne,                 // скобка одинарная, курсор на скобке
  BrTwo,                 // скобка двойная
  BrRight,               // скобка одинарная, курсор справа от скобки
  BrOneMath,             // "скобка" == "кавычка"
};

static struct PluginStartupInfo Info;
static TCHAR PluginRootKey[80];

int Config();
