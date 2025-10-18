# zip.cpp
## English
### Purpose
This file implements functionality related to: Second-level plugin module for FAR Manager and MultiArc plugin
Main functional areas: core functionality
### Key Classes
- `ExtraBlockHeader`: Implements core functionality
- `NTFSAttributeHeader`: Implements core functionality
- `TimesAttribute`: Implements core functionality
- `ZIP64Descriptor`: Implements core functionality
- `ZipHdr1`: Implements core functionality
- `ZipHdr2`: Implements core functionality
- `ZipHeader`: Implements core functionality
### Key Functions
- `IsValidHeader()`: Tests whether ValidHeader condition is true or property exists
- `GetFilePosition()`: Retrieves FilePosition from internal state or data structure
- `IsZIPFileMagic()`: Tests whether ZIPFileMagic condition is true or property exists
- `IsArchive()`: Tests whether Archive condition is true or property exists
- `OpenArchive()`: Opens resource and prepares it for access operations
- `GetArcItem()`: Retrieves ArcItem from internal state or data structure
- `SetFilePointer()`: Updates FilePointer in internal state or configuration
- `CloseArchive()`: Closes resource and performs cleanup operations
- `GetSFXPos()`: Retrieves SFXPos from internal state or data structure
- `GetFormatName()`: Retrieves FormatName from internal state or data structure
### Summary
The `zip.cpp` file provides essential functionality for core functionality. It defines 7 class(es) and contains approximately 10 function(s) to support the Far Manager file manager operations.

---

## Polski (Polish)
### Cel
Ten plik implementuje funkcjonalność związaną z: Second-level plugin module for FAR Manager and MultiArc plugin
Główne obszary funkcjonalne: podstawowa funkcjonalność
### Kluczowe Klasy
- `ExtraBlockHeader`: Implementuje podstawowa funkcjonalność
- `NTFSAttributeHeader`: Implementuje podstawowa funkcjonalność
- `TimesAttribute`: Implementuje podstawowa funkcjonalność
- `ZIP64Descriptor`: Implementuje podstawowa funkcjonalność
- `ZipHdr1`: Implementuje podstawowa funkcjonalność
- `ZipHdr2`: Implementuje podstawowa funkcjonalność
- `ZipHeader`: Implementuje podstawowa funkcjonalność
### Kluczowe Funkcje
- `IsValidHeader()`: Testuje czy ValidHeader warunek jest prawdziwy lub właściwość istnieje
- `GetFilePosition()`: Pobiera FilePosition ze stanu wewnętrznego lub struktury danych
- `IsZIPFileMagic()`: Testuje czy ZIPFileMagic warunek jest prawdziwy lub właściwość istnieje
- `IsArchive()`: Testuje czy Archive warunek jest prawdziwy lub właściwość istnieje
- `OpenArchive()`: Otwiera zasób i przygotowuje go do operacji dostępu
- `GetArcItem()`: Pobiera ArcItem ze stanu wewnętrznego lub struktury danych
- `SetFilePointer()`: Aktualizuje FilePointer w stanie wewnętrznym lub konfiguracji
- `CloseArchive()`: Zamyka zasób i wykonuje operacje czyszczące
- `GetSFXPos()`: Pobiera SFXPos ze stanu wewnętrznego lub struktury danych
- `GetFormatName()`: Pobiera FormatName ze stanu wewnętrznego lub struktury danych
### Podsumowanie
Plik `zip.cpp` zapewnia podstawową funkcjonalność dla podstawowa funkcjonalność. Definiuje 7 klas(y) i zawiera około 10 funkcji wspierających operacje menedżera plików Far Manager.
