# platform.concurrency.hpp
## English
### Purpose
This file implements functionality related to: Threads, mutexes, events, critical sections etc.
Main functional areas: core functionality
### Key Classes
- `critical_section`: Implements core functionality
- `event`: Implements core functionality
- `i_shared_mutex`: Implements core functionality
- `mutex`: Implements core functionality
- `shared_mutex`: Implements core functionality
- `state`: Implements core functionality
- `synced_queue`: Implements core functionality
- `thread`: Implements core functionality
- `timer`: Implements core functionality
- `timer_closer`: Implements core functionality
### Key Functions
- `make_name()`: Executes make_name operation as part of core functionality
- `lock()`: Executes lock operation as part of core functionality
- `unlock()`: Executes unlock operation as part of core functionality
- `thread()`: Executes thread operation as part of core functionality
- `get_id()`: Retrieves _id from internal state or data structure
- `joinable()`: Executes joinable operation as part of core functionality
- `detach()`: Executes detach operation as part of core functionality
- `join()`: Executes join operation as part of core functionality
- `check_joinable()`: Validates data integrity and checks for correctness
- `finalise()`: Executes finalise operation as part of core functionality
### Namespaces
- `detail`
- `os`
### Summary
The `platform.concurrency.hpp` file provides essential functionality for core functionality. It defines 11 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Threads, mutexes, events, critical sections etc.
Główne obszary funkcjonalne: podstawowa funkcjonalność
### Kluczowe Klasy
- `critical_section`: Implementuje podstawowa funkcjonalność
- `event`: Implementuje podstawowa funkcjonalność
- `i_shared_mutex`: Implementuje podstawowa funkcjonalność
- `mutex`: Implementuje podstawowa funkcjonalność
- `shared_mutex`: Implementuje podstawowa funkcjonalność
- `state`: Implementuje podstawowa funkcjonalność
- `synced_queue`: Implementuje podstawowa funkcjonalność
- `thread`: Implementuje podstawowa funkcjonalność
- `timer`: Implementuje podstawowa funkcjonalność
- `timer_closer`: Implementuje podstawowa funkcjonalność
### Kluczowe Funkcje
- `make_name()`: Wykonuje make_name operację jako część podstawowa funkcjonalność
- `lock()`: Wykonuje lock operację jako część podstawowa funkcjonalność
- `unlock()`: Wykonuje unlock operację jako część podstawowa funkcjonalność
- `thread()`: Wykonuje thread operację jako część podstawowa funkcjonalność
- `get_id()`: Pobiera _id ze stanu wewnętrznego lub struktury danych
- `joinable()`: Wykonuje joinable operację jako część podstawowa funkcjonalność
- `detach()`: Wykonuje detach operację jako część podstawowa funkcjonalność
- `join()`: Wykonuje join operację jako część podstawowa funkcjonalność
- `check_joinable()`: Waliduje integralność danych i sprawdza poprawność
- `finalise()`: Wykonuje finalise operację jako część podstawowa funkcjonalność
### Przestrzenie nazw
- `detail`
- `os`
### Podsumowanie
Plik `platform.concurrency.hpp` zapewnia podstawową funkcjonalność dla podstawowa funkcjonalność. Definiuje 11 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
