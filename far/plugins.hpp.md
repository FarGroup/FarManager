# plugins.hpp
## English
### Purpose
This file implements functionality related to: Работа с плагинами (низкий уровень, кое-что повыше в filelist.cpp)
Main functional areas: plugin interface
### Key Classes
- `CallPluginInfo`: Implements plugin interface
- `Dialog`: Implements plugin interface
- `Editor`: Implements plugin interface
- `FileEditor`: Implements plugin interface
- `Panel`: Implements plugin interface
- `Plugin`: Implements plugin interface
- `PluginManager`: Implements plugin interface
- `SaveScreen`: Implements plugin interface
- `Viewer`: Implements plugin interface
- `delayed_deleter`: Implements plugin interface
### Key Functions
- `plugin()`: Performs specific operation
- `panel()`: Performs specific operation
- `set_panel()`: Sets or assigns data
- `delayed_delete()`: Performs specific operation
- `NotifyExitLuaMacro()`: Performs specific operation
- `ClosePanel()`: Closes a resource
- `GetOpenPanelInfo()`: Retrieves or returns data
- `GetFindData()`: Retrieves or returns data
- `FreeFindData()`: Performs specific operation
- `GetVirtualFindData()`: Retrieves or returns data
### Summary
The `plugins.hpp` file provides essential functionality for plugin interface. It defines 14 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Работа с плагинами (низкий уровень, кое-что повыше в filelist.cpp)
Główne obszary funkcjonalne: interfejs wtyczek
### Kluczowe Klasy
- `CallPluginInfo`: Implementuje interfejs wtyczek
- `Dialog`: Implementuje interfejs wtyczek
- `Editor`: Implementuje interfejs wtyczek
- `FileEditor`: Implementuje interfejs wtyczek
- `Panel`: Implementuje interfejs wtyczek
- `Plugin`: Implementuje interfejs wtyczek
- `PluginManager`: Implementuje interfejs wtyczek
- `SaveScreen`: Implementuje interfejs wtyczek
- `Viewer`: Implementuje interfejs wtyczek
- `delayed_deleter`: Implementuje interfejs wtyczek
### Kluczowe Funkcje
- `plugin()`: Wykonuje specyficzną operację
- `panel()`: Wykonuje specyficzną operację
- `set_panel()`: Ustawia lub przypisuje dane
- `delayed_delete()`: Wykonuje specyficzną operację
- `NotifyExitLuaMacro()`: Wykonuje specyficzną operację
- `ClosePanel()`: Zamyka zasób
- `GetOpenPanelInfo()`: Pobiera lub zwraca dane
- `GetFindData()`: Pobiera lub zwraca dane
- `FreeFindData()`: Wykonuje specyficzną operację
- `GetVirtualFindData()`: Pobiera lub zwraca dane
### Podsumowanie
Plik `plugins.hpp` zapewnia podstawową funkcjonalność dla interfejs wtyczek. Definiuje 14 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
