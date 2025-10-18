# plugins.cpp
## English
### Purpose
This file implements functionality related to: Работа с плагинами (низкий уровень, кое-что повыше в filelist.cpp)
Main functional areas: plugin interface
### Key Classes
- `PluginData`: Implements plugin interface
- `PluginMenuItemData`: Implements plugin interface
- `layout`: Implements plugin interface
- `plugin_panel_holder`: Implements plugin interface
### Key Functions
- `GetHotKeyPluginKey()`: Retrieves HotKeyPluginKey from internal state or data structure
- `GetPluginHotKey()`: Retrieves PluginHotKey from internal state or data structure
- `EnsureLuaCpuCompatibility()`: Executes EnsureLuaCpuCompatibility operation as part of plugin interface
- `bool()`: Executes bool operation as part of plugin interface
- `m_PluginsLoaded()`: Executes m_PluginsLoaded operation as part of plugin interface
- `ScTree()`: Executes ScTree operation as part of plugin interface
- `analyse()`: Executes analyse operation as part of plugin interface
- `set_analyse()`: Updates _analyse in internal state or configuration
- `File()`: Executes File operation as part of plugin interface
- `AddHotkey()`: Adds new Hotkey to collection or list
### Summary
The `plugins.cpp` file provides essential functionality for plugin interface. It defines 4 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Работа с плагинами (низкий уровень, кое-что повыше в filelist.cpp)
Główne obszary funkcjonalne: interfejs wtyczek
### Kluczowe Klasy
- `PluginData`: Implementuje interfejs wtyczek
- `PluginMenuItemData`: Implementuje interfejs wtyczek
- `layout`: Implementuje interfejs wtyczek
- `plugin_panel_holder`: Implementuje interfejs wtyczek
### Kluczowe Funkcje
- `GetHotKeyPluginKey()`: Pobiera HotKeyPluginKey ze stanu wewnętrznego lub struktury danych
- `GetPluginHotKey()`: Pobiera PluginHotKey ze stanu wewnętrznego lub struktury danych
- `EnsureLuaCpuCompatibility()`: Wykonuje EnsureLuaCpuCompatibility operację jako część interfejs wtyczek
- `bool()`: Wykonuje bool operację jako część interfejs wtyczek
- `m_PluginsLoaded()`: Wykonuje m_PluginsLoaded operację jako część interfejs wtyczek
- `ScTree()`: Wykonuje ScTree operację jako część interfejs wtyczek
- `analyse()`: Wykonuje analyse operację jako część interfejs wtyczek
- `set_analyse()`: Aktualizuje _analyse w stanie wewnętrznym lub konfiguracji
- `File()`: Wykonuje File operację jako część interfejs wtyczek
- `AddHotkey()`: Dodaje nowy Hotkey do kolekcji lub listy
### Podsumowanie
Plik `plugins.cpp` zapewnia podstawową funkcjonalność dla interfejs wtyczek. Definiuje 4 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
