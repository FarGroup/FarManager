struct Options
{
	int IgnoreQuotes;   // Правило: "Игнорировать скобки, заключенные в кавычки"
	int IgnoreAfter;    // Правило: "Игнорировать за скобкой"
	int BracketPrior;   // Правило: "Приоритет скобок"
	int JumpToPair;     // Правило: "Перейти на парную скобку при отметке блока"
	int Beep;
	wchar_t  QuotesType[21]; // типы кавычек
	wchar_t  Brackets1[21];  // одинарные скобки
	wchar_t  Brackets2[41];  // двойные скобки
};

enum
{
	BrZERO,                // не определено
	BrOne,                 // скобка одинарная, курсор на скобке
	BrTwo,                 // скобка двойная
	BrRight,               // скобка одинарная, курсор справа от скобки
	BrOneMath,             // "скобка" == "кавычка"
};
