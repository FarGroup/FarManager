# dlgedit.hpp
## English
### Purpose
This file implements functionality related to: Одиночная строка редактирования для диалога (как наследник класса Edit)
Main functional areas: dialog handling, text editing
### Key Classes
- `Dialog`: Implements dialog handling, text editing
- `EditControl`: Implements dialog handling, text editing
- `Editor`: Implements dialog handling, text editing
- `FarColor`: Implements dialog handling, text editing
- `History`: Implements dialog handling, text editing
- `SetAutocomplete`: Implements dialog handling, text editing
### Key Functions
- `Flags()`: Constructor that initializes Flags object with provided parameters
- `Init()`: Initializes data structures and sets up initial state for operation
- `ProcessKey()`: Processes keyboard input events and updates dialog state accordingly
- `ProcessMouse()`: Handles mouse events including clicks, movement, and wheel actions in dialog
- `Show()`: Renders dialog on screen and enters message loop for user interaction
- `SetPosition()`: Updates Position in internal state or configuration
- `GetPosition()`: Retrieves Position from internal state or data structure
- `Hide()`: Hides visual element from screen without destroying it
- `ShowConsoleTitle()`: Renders dialog on screen and enters message loop for user interaction
- `SetScreenPosition()`: Updates ScreenPosition in internal state or configuration
### Summary
The `dlgedit.hpp` file provides essential functionality for dialog handling, text editing. It defines 6 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Одиночная строка редактирования для диалога (как наследник класса Edit)
Główne obszary funkcjonalne: obsługa dialogów, edycja tekstu
### Kluczowe Klasy
- `Dialog`: Implementuje obsługa dialogów, edycja tekstu
- `EditControl`: Implementuje obsługa dialogów, edycja tekstu
- `Editor`: Implementuje obsługa dialogów, edycja tekstu
- `FarColor`: Implementuje obsługa dialogów, edycja tekstu
- `History`: Implementuje obsługa dialogów, edycja tekstu
- `SetAutocomplete`: Implementuje obsługa dialogów, edycja tekstu
### Kluczowe Funkcje
- `Flags()`: Konstruktor inicjalizujący Flags obiekt z dostarczonymi parametrami
- `Init()`: Inicjalizuje struktury danych i ustawia stan początkowy dla operacji
- `ProcessKey()`: Przetwarza zdarzenia wejścia klawiatury i odpowiednio aktualizuje stan dialogu
- `ProcessMouse()`: Obsługuje zdarzenia myszy włączając kliknięcia, ruch i akcje kółka w dialogu
- `Show()`: Renderuje dialog na ekranie i wchodzi w pętlę komunikatów dla interakcji użytkownika
- `SetPosition()`: Aktualizuje Position w stanie wewnętrznym lub konfiguracji
- `GetPosition()`: Pobiera Position ze stanu wewnętrznego lub struktury danych
- `Hide()`: Ukrywa element wizualny z ekranu bez niszczenia go
- `ShowConsoleTitle()`: Renderuje dialog na ekranie i wchodzi w pętlę komunikatów dla interakcji użytkownika
- `SetScreenPosition()`: Aktualizuje ScreenPosition w stanie wewnętrznym lub konfiguracji
### Podsumowanie
Plik `dlgedit.hpp` zapewnia podstawową funkcjonalność dla obsługa dialogów, edycja tekstu. Definiuje 6 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
