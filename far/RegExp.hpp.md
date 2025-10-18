# RegExp.hpp
## English
### Purpose
This file implements functionality related to: Regular expressions
Main functional areas: regular expressions
### Key Classes
- `REOpCode`: Implements regular expressions
- `RegExp`: Implements regular expressions
- `StateStackItem`: Implements regular expressions
- `UniSet`: Implements regular expressions
- `regex_exception`: Implements regular expressions
- `regex_match`: Implements regular expressions
- `state_stack`: Implements regular expressions
### Key Functions
- `code()`: Returns operation code or encoded value representation
- `position()`: Returns current position or offset within data structure
- `to_string()`: Converts object to string representation for display or serialization
- `CalcLength()`: Calculates compiled pattern length in bytes for memory allocation
- `InnerCompile()`: Internal compilation routine that builds finite automaton from regex pattern
- `InnerMatch()`: Core matching algorithm that traverses state machine to test pattern
- `TrimTail()`: Removes redundant trailing opcodes from compiled regular expression
- `StrCmp()`: Performs string comparison operation used during pattern matching
- `Compile()`: Parses regular expression pattern string and builds internal state machine
- `Optimize()`: Optimizes compiled regular expression for faster matching performance
### Summary
The `RegExp.hpp` file provides essential functionality for regular expressions. It defines 7 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Regular expressions
Główne obszary funkcjonalne: wyrażenia regularne
### Kluczowe Klasy
- `REOpCode`: Implementuje wyrażenia regularne
- `RegExp`: Implementuje wyrażenia regularne
- `StateStackItem`: Implementuje wyrażenia regularne
- `UniSet`: Implementuje wyrażenia regularne
- `regex_exception`: Implementuje wyrażenia regularne
- `regex_match`: Implementuje wyrażenia regularne
- `state_stack`: Implementuje wyrażenia regularne
### Kluczowe Funkcje
- `code()`: Zwraca kod operacji lub zakodowaną reprezentację wartości
- `position()`: Zwraca bieżącą pozycję lub przesunięcie w strukturze danych
- `to_string()`: Konwertuje obiekt do reprezentacji łańcuchowej dla wyświetlenia lub serializacji
- `CalcLength()`: Oblicza długość skompilowanego wzorca w bajtach dla alokacji pamięci
- `InnerCompile()`: Wewnętrzna procedura kompilacji budująca automat skończony ze wzorca regex
- `InnerMatch()`: Podstawowy algorytm dopasowywania przechodzący przez automat stanów aby przetestować wzorzec
- `TrimTail()`: Usuwa nadmiarowe końcowe kody operacji ze skompilowanego wyrażenia regularnego
- `StrCmp()`: Wykonuje operację porównania łańcuchów używaną podczas dopasowywania wzorca
- `Compile()`: Parsuje łańcuch wzorca wyrażenia regularnego i buduje wewnętrzny automat stanów
- `Optimize()`: Optymalizuje skompilowane wyrażenie regularne dla szybszej wydajności dopasowywania
### Podsumowanie
Plik `RegExp.hpp` zapewnia podstawową funkcjonalność dla wyrażenia regularne. Definiuje 7 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
