# configdb.hpp
## English
### Purpose
This file implements functionality related to: хранение настроек в базе sqlite.
Main functional areas: configuration management
### Key Classes
- `AssociationsConfig`: Implements configuration management
- `ColorsConfig`: Implements configuration management
- `FarColor`: Implements configuration management
- `GeneralConfig`: Implements configuration management
- `HierarchicalConfig`: Implements configuration management
- `HistoryConfig`: Implements configuration management
- `PluginsCacheConfig`: Implements configuration management
- `PluginsHotkeysConfig`: Implements configuration management
- `VersionInfo`: Implements configuration management
- `async_delete`: Implements configuration management
### Key Functions
- `GetValue()`: Retrieves specific configuration value by key name
- `ValuesEnumerator()`: Executes ValuesEnumerator operation as part of configuration management
- `get()`: Retrieves specific configuration value by key name
- `bool()`: Executes bool operation as part of configuration management
- `KeysEnumerator()`: Executes KeysEnumerator operation as part of configuration management
- `EnumKeys()`: Executes EnumKeys operation as part of configuration management
- `EnumValues()`: Executes EnumValues operation as part of configuration management
- `ToSettingsType()`: Updates configuration value for specified key
- `MasksEnumerator()`: Executes MasksEnumerator operation as part of configuration management
- `EnumMasks()`: Executes EnumMasks operation as part of configuration management
### Namespaces
- `concurrency`
- `detail`
- `os`
### Summary
The `configdb.hpp` file provides essential functionality for configuration management. It defines 24 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: хранение настроек в базе sqlite.
Główne obszary funkcjonalne: zarządzanie konfiguracją
### Kluczowe Klasy
- `AssociationsConfig`: Implementuje zarządzanie konfiguracją
- `ColorsConfig`: Implementuje zarządzanie konfiguracją
- `FarColor`: Implementuje zarządzanie konfiguracją
- `GeneralConfig`: Implementuje zarządzanie konfiguracją
- `HierarchicalConfig`: Implementuje zarządzanie konfiguracją
- `HistoryConfig`: Implementuje zarządzanie konfiguracją
- `PluginsCacheConfig`: Implementuje zarządzanie konfiguracją
- `PluginsHotkeysConfig`: Implementuje zarządzanie konfiguracją
- `VersionInfo`: Implementuje zarządzanie konfiguracją
- `async_delete`: Implementuje zarządzanie konfiguracją
### Kluczowe Funkcje
- `GetValue()`: Pobiera specific configuration wartość by key name
- `ValuesEnumerator()`: Wykonuje ValuesEnumerator operację jako część zarządzanie konfiguracją
- `get()`: Pobiera specific configuration wartość by key name
- `bool()`: Wykonuje bool operację jako część zarządzanie konfiguracją
- `KeysEnumerator()`: Wykonuje KeysEnumerator operację jako część zarządzanie konfiguracją
- `EnumKeys()`: Wykonuje EnumKeys operację jako część zarządzanie konfiguracją
- `EnumValues()`: Wykonuje EnumValues operację jako część zarządzanie konfiguracją
- `ToSettingsType()`: Aktualizuje configuration wartość for specified key
- `MasksEnumerator()`: Wykonuje MasksEnumerator operację jako część zarządzanie konfiguracją
- `EnumMasks()`: Wykonuje EnumMasks operację jako część zarządzanie konfiguracją
### Przestrzenie nazw
- `concurrency`
- `detail`
- `os`
### Podsumowanie
Plik `configdb.hpp` zapewnia podstawową funkcjonalność dla zarządzanie konfiguracją. Definiuje 24 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
