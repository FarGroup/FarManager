# clipboard.cpp
## English
### Purpose
This file implements functionality related to: Работа с буфером обмена.
Main functional areas: clipboard operations, singleton implementation
### Key Classes
- `clipboard_guard`: Implements clipboard operations, singleton implementation
### Key Functions
- `Open()`: Opens clipboard for exclusive access to perform read/write operations
- `Clear()`: Empties clipboard contents and releases associated memory
- `SetText()`: Places text content into the system or internal clipboard
- `SetVText()`: Places text content into the system or internal clipboard
- `SetHDROP()`: Updates HDROP in internal state or configuration
- `GetText()`: Retrieves text content from the clipboard
- `GetVText()`: Retrieves text content from the clipboard
- `CreateInstance()`: Creates and initializes new Instance instance
- `SetClipboardText()`: Places text content into the system or internal clipboard
- `SetClipboardVText()`: Places text content into the system or internal clipboard
### Summary
The `clipboard.cpp` file provides essential functionality for clipboard operations, singleton implementation. It defines 1 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Работа с буфером обмена.
Główne obszary funkcjonalne: operacje schowka, implementacja singletona
### Kluczowe Klasy
- `clipboard_guard`: Implementuje operacje schowka, implementacja singletona
### Kluczowe Funkcje
- `Open()`: Otwiera schowek dla wyłącznego dostępu aby wykonać operacje odczytu/zapisu
- `Clear()`: Opróżnia zawartość schowka i zwalnia powiązaną pamięć
- `SetText()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
- `SetVText()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
- `SetHDROP()`: Aktualizuje HDROP w stanie wewnętrznym lub konfiguracji
- `GetText()`: Pobiera zawartość tekstową ze schowka
- `GetVText()`: Pobiera zawartość tekstową ze schowka
- `CreateInstance()`: Tworzy i inicjalizuje nowy Instance instancję
- `SetClipboardText()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
- `SetClipboardVText()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
### Podsumowanie
Plik `clipboard.cpp` zapewnia podstawową funkcjonalność dla operacje schowka, implementacja singletona. Definiuje 1 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
