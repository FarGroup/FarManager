# platform.fs.hpp
## English
### Purpose
This file implements functionality related to: */
Main functional areas: file system operations
### Key Classes
- `Chunk`: Implements file system operations
- `current_directory_guard`: Implements file system operations
- `file`: Implements file system operations
- `file_status`: Implements file system operations
- `file_walker`: Implements file system operations
- `find_data`: Implements file system operations
- `find_file_handle_closer`: Implements file system operations
- `find_handle_closer`: Implements file system operations
- `find_volume_handle_closer`: Implements file system operations
- `process_current_directory_guard`: Implements file system operations
### Key Functions
- `set_current_directory_syncronisation()`: Updates _current_directory_syncronisation in internal state or configuration
- `AlternateFileName()`: Constructor that initializes AlternateFileName object with provided parameters
- `SetAlternateFileName()`: Updates AlternateFileName in internal state or configuration
- `HasAlternateFileName()`: Tests whether AlternateFileName condition is true or property exists
- `is_standard_letter()`: Tests whether _standard_letter condition is true or property exists
- `get_number()`: Retrieves _number from internal state or data structure
- `get_letter()`: Retrieves _letter from internal state or data structure
- `get_device_path()`: Retrieves _device_path from internal state or data structure
- `get_win32nt_device_path()`: Retrieves _win32nt_device_path from internal state or data structure
- `get_root_directory()`: Retrieves _root_directory from internal state or data structure
### Namespaces
- `detail`
- `drive`
- `low`
- `os`
- `state`
### Summary
The `platform.fs.hpp` file provides essential functionality for file system operations. It defines 10 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: */
Główne obszary funkcjonalne: operacje systemu plików
### Kluczowe Klasy
- `Chunk`: Implementuje operacje systemu plików
- `current_directory_guard`: Implementuje operacje systemu plików
- `file`: Implementuje operacje systemu plików
- `file_status`: Implementuje operacje systemu plików
- `file_walker`: Implementuje operacje systemu plików
- `find_data`: Implementuje operacje systemu plików
- `find_file_handle_closer`: Implementuje operacje systemu plików
- `find_handle_closer`: Implementuje operacje systemu plików
- `find_volume_handle_closer`: Implementuje operacje systemu plików
- `process_current_directory_guard`: Implementuje operacje systemu plików
### Kluczowe Funkcje
- `set_current_directory_syncronisation()`: Aktualizuje _current_directory_syncronisation w stanie wewnętrznym lub konfiguracji
- `AlternateFileName()`: Konstruktor inicjalizujący AlternateFileName obiekt z dostarczonymi parametrami
- `SetAlternateFileName()`: Aktualizuje AlternateFileName w stanie wewnętrznym lub konfiguracji
- `HasAlternateFileName()`: Testuje czy AlternateFileName warunek jest prawdziwy lub właściwość istnieje
- `is_standard_letter()`: Testuje czy _standard_letter warunek jest prawdziwy lub właściwość istnieje
- `get_number()`: Pobiera _number ze stanu wewnętrznego lub struktury danych
- `get_letter()`: Pobiera _letter ze stanu wewnętrznego lub struktury danych
- `get_device_path()`: Pobiera _device_path ze stanu wewnętrznego lub struktury danych
- `get_win32nt_device_path()`: Pobiera _win32nt_device_path ze stanu wewnętrznego lub struktury danych
- `get_root_directory()`: Pobiera _root_directory ze stanu wewnętrznego lub struktury danych
### Przestrzenie nazw
- `detail`
- `drive`
- `low`
- `os`
- `state`
### Podsumowanie
Plik `platform.fs.hpp` zapewnia podstawową funkcjonalność dla operacje systemu plików. Definiuje 10 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
