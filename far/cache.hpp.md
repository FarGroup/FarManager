# cache.hpp
## English
### Purpose
This file implements functionality related to: Кеширование записи в файл/чтения из файла
Main functional areas: file caching
### Key Classes
- `CachedRead`: Implements file caching
### Key Functions
- `CachedRead()`: Constructor that initializes CachedRead object with file reference and buffer size for optimized disk I/O
- `AdjustAlignment()`: Adjusts buffer alignment based on physical sector size for optimal disk I/O performance
- `Read()`: Reads data from file into buffer using cache to minimize disk operations
- `Unread()`: Moves file pointer backward to allow re-reading previously consumed bytes from cache
- `Clear()`: Clears all data and resets to initial empty state
- `FillBuffer()`: Fills internal buffer by reading data from underlying file with alignment optimization
### Summary
The `cache.hpp` file provides essential functionality for file caching. It defines 1 class(es) and contains approximately 6 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Кеширование записи в файл/чтения из файла
Główne obszary funkcjonalne: buforowanie plików
### Kluczowe Klasy
- `CachedRead`: Implementuje buforowanie plików
### Kluczowe Funkcje
- `CachedRead()`: Konstruktor inicjalizujący obiekt CachedRead z referencją do pliku i rozmiarem bufora dla zoptymalizowanego I/O dysku
- `AdjustAlignment()`: Dostosowuje wyrównanie bufora na podstawie rozmiaru sektora fizycznego dla optymalnej wydajności I/O dysku
- `Read()`: Odczytuje dane z pliku do bufora używając pamięci podręcznej aby zminimalizować operacje dyskowe
- `Unread()`: Przesuwa wskaźnik pliku wstecz aby umożliwić ponowne odczytanie poprzednio skonsumowanych bajtów z pamięci podręcznej
- `Clear()`: Czyści wszystkie dane i resetuje do początkowego pustego stanu
- `FillBuffer()`: Wypełnia wewnętrzny bufor odczytując dane z bazowego pliku z optymalizacją wyrównania
### Podsumowanie
Plik `cache.hpp` zapewnia podstawową funkcjonalność dla buforowanie plików. Definiuje 1 klas(y) i zawiera około 6 funkcji wspierających operacje menedżera plików Far Manager.
