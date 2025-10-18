# scrobj.hpp
## English
### Purpose
This file implements functionality related to: Parent class для всех screen objects
Main functional areas: screen operations
### Key Classes
- `SaveScreen`: Implements screen operations
- `ScreenObject`: Implements screen operations
- `ScreenObjectWithShadow`: Implements screen operations
- `SimpleScreenObject`: Implements screen operations
### Key Functions
- `ProcessKey()`: Processes Key through transformation or handling pipeline
- `ProcessMouse()`: Processes Mouse through transformation or handling pipeline
- `Hide()`: Hides visual element from screen without destroying it
- `Show()`: Displays content or makes visual element visible on screen
- `ShowConsoleTitle()`: Displays content or makes visual element visible on screen
- `SetPosition()`: Updates Position in internal state or configuration
- `GetPosition()`: Retrieves Position from internal state or data structure
- `SetScreenPosition()`: Updates ScreenPosition in internal state or configuration
- `ResizeConsole()`: Changes size dimensions while preserving content when possible
- `VMProcess()`: Processes virtual machine operations or commands
### Summary
The `scrobj.hpp` file provides essential functionality for screen operations. It defines 4 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Parent class для всех screen objects
Główne obszary funkcjonalne: operacje ekranowe
### Kluczowe Klasy
- `SaveScreen`: Implementuje operacje ekranowe
- `ScreenObject`: Implementuje operacje ekranowe
- `ScreenObjectWithShadow`: Implementuje operacje ekranowe
- `SimpleScreenObject`: Implementuje operacje ekranowe
### Kluczowe Funkcje
- `ProcessKey()`: Przetwarza Key przez potok transformacji lub obsługi
- `ProcessMouse()`: Przetwarza Mouse przez potok transformacji lub obsługi
- `Hide()`: Ukrywa element wizualny z ekranu bez niszczenia go
- `Show()`: Wyświetla zawartość lub czyni element wizualny widocznym na ekranie
- `ShowConsoleTitle()`: Wyświetla zawartość lub czyni element wizualny widocznym na ekranie
- `SetPosition()`: Aktualizuje Position w stanie wewnętrznym lub konfiguracji
- `GetPosition()`: Pobiera Position ze stanu wewnętrznego lub struktury danych
- `SetScreenPosition()`: Aktualizuje ScreenPosition w stanie wewnętrznym lub konfiguracji
- `ResizeConsole()`: Zmienia wymiary rozmiaru zachowując zawartość gdy to możliwe
- `VMProcess()`: Przetwarza operacje lub polecenia maszyny wirtualnej
### Podsumowanie
Plik `scrobj.hpp` zapewnia podstawową funkcjonalność dla operacje ekranowe. Definiuje 4 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
