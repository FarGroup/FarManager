# scrbuf.hpp
## English
### Purpose
This file implements functionality related to: Буферизация вывода на экран, весь вывод идет через этот буфер
Main functional areas: screen operations
### Key Classes
- `ScreenBuf`: Implements screen operations
- `flush_type`: Implements screen operations
### Key Functions
- `DebugDump()`: Executes DebugDump operation as part of screen operations
- `AllocBuf()`: Executes AllocBuf operation as part of screen operations
- `Lock()`: Executes Lock operation as part of screen operations
- `Unlock()`: Executes Unlock operation as part of screen operations
- `GetLockCount()`: Retrieves LockCount from internal state or data structure
- `SetLockCount()`: Updates LockCount in internal state or configuration
- `ResetLockCount()`: Executes ResetLockCount operation as part of screen operations
- `MoveCursor()`: Moves data or object from current location to new location
- `GetCursorPos()`: Retrieves CursorPos from internal state or data structure
- `SetCursorType()`: Updates CursorType in internal state or configuration
### Summary
The `scrbuf.hpp` file provides essential functionality for screen operations. It defines 2 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Буферизация вывода на экран, весь вывод идет через этот буфер
Główne obszary funkcjonalne: operacje ekranowe
### Kluczowe Klasy
- `ScreenBuf`: Implementuje operacje ekranowe
- `flush_type`: Implementuje operacje ekranowe
### Kluczowe Funkcje
- `DebugDump()`: Wykonuje DebugDump operację jako część operacje ekranowe
- `AllocBuf()`: Wykonuje AllocBuf operację jako część operacje ekranowe
- `Lock()`: Wykonuje Lock operację jako część operacje ekranowe
- `Unlock()`: Wykonuje Unlock operację jako część operacje ekranowe
- `GetLockCount()`: Pobiera LockCount ze stanu wewnętrznego lub struktury danych
- `SetLockCount()`: Aktualizuje LockCount w stanie wewnętrznym lub konfiguracji
- `ResetLockCount()`: Wykonuje ResetLockCount operację jako część operacje ekranowe
- `MoveCursor()`: Przenosi dane lub obiekt z bieżącej lokalizacji do nowej lokalizacji
- `GetCursorPos()`: Pobiera CursorPos ze stanu wewnętrznego lub struktury danych
- `SetCursorType()`: Aktualizuje CursorType w stanie wewnętrznym lub konfiguracji
### Podsumowanie
Plik `scrbuf.hpp` zapewnia podstawową funkcjonalność dla operacje ekranowe. Definiuje 2 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
