# platform.clipboard.hpp
## English
### Purpose
This file implements functionality related to: */
Main functional areas: clipboard operations
### Key Classes
- `state`: Implements clipboard operations
### Key Functions
- `enable_ansi_to_unicode_conversion_workaround()`: Executes enable_ansi_to_unicode_conversion_workaround operation as part of clipboard operations
- `open()`: Opens clipboard for exclusive access to perform read/write operations
- `close()`: Closes clipboard and releases exclusive lock, making it available for other processes
- `clear()`: Empties clipboard contents and releases associated memory
- `set_text()`: Places text content into the system or internal clipboard
- `set_vtext()`: Places text content into the system or internal clipboard
- `set_files()`: Updates _files in internal state or configuration
- `get_text()`: Retrieves text content from the clipboard
- `get_vtext()`: Retrieves text content from the clipboard
- `capture()`: Constructor that initializes capture object with provided parameters
### Namespaces
- `os`
- `testing`
### Summary
The `platform.clipboard.hpp` file provides essential functionality for clipboard operations. It defines 1 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: */
Główne obszary funkcjonalne: operacje schowka
### Kluczowe Klasy
- `state`: Implementuje operacje schowka
### Kluczowe Funkcje
- `enable_ansi_to_unicode_conversion_workaround()`: Wykonuje enable_ansi_to_unicode_conversion_workaround operację jako część operacje schowka
- `open()`: Otwiera schowek dla wyłącznego dostępu aby wykonać operacje odczytu/zapisu
- `close()`: Zamyka schowek i zwalnia wyłączną blokadę, udostępniając go innym procesom
- `clear()`: Opróżnia zawartość schowka i zwalnia powiązaną pamięć
- `set_text()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
- `set_vtext()`: Umieszcza zawartość tekstową w schowku systemowym lub wewnętrznym
- `set_files()`: Aktualizuje _files w stanie wewnętrznym lub konfiguracji
- `get_text()`: Pobiera zawartość tekstową ze schowka
- `get_vtext()`: Pobiera zawartość tekstową ze schowka
- `capture()`: Konstruktor inicjalizujący capture obiekt z dostarczonymi parametrami
### Przestrzenie nazw
- `os`
- `testing`
### Podsumowanie
Plik `platform.clipboard.hpp` zapewnia podstawową funkcjonalność dla operacje schowka. Definiuje 1 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
