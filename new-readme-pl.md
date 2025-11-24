# Far Manager (Rozszerzony Opis)

| [English](new-readme.md) | [Русский](README-RU.md) | [Wersja skrócona](README-PL.md) |
| - | - | - |

[![Header][logo-img]][logo-url]

| | AppVeyor | Azure |
| - | - | - |
| VS | [![AppVeyor][VS-AppVeyor-img]][VS-AppVeyor-url] | [![Azure Pipelines][VS-Azure-img]][VS-Azure-url] |
| GCC | [![AppVeyor][GCC-AppVeyor-img]][GCC-AppVeyor-url] | TBD |
| Clang | [![AppVeyor][Clang-AppVeyor-img]][Clang-AppVeyor-url] | TBD |

## 1. Czym jest Far Manager?

Far Manager to menedżer plików i archiwów działający w trybie tekstowym w systemie Windows. Skupia się na szybkości, produktywności z klawiatury oraz rozszerzalności poprzez system wtyczek. Domyślnie umożliwia:

- Przeglądanie i zarządzanie plikami, folderami oraz archiwami
- Podgląd i edycję plików tekstowych oraz binarnych
- Kopiowanie, przenoszenie, zmianę nazw, usuwanie z rozbudowaną logiką nadpisywania
- Pracę z zasobami sieciowymi, FTP / SFTP (wtyczki)
- Automatyzację zadań poprzez makra i skrypty

Jeśli potrzebujesz krótkiego opisu, zobacz `README-PL.md`.

## 2. Kluczowe Założenia

- Tryb tekstowy: szybkie renderowanie, pełna kontrola z klawiatury
- Rozszerzalność: bogate [API wtyczek](https://api.farmanager.com/)
- Stabilność: dojrzała baza kodu rozwijana od lat
- Personalizacja: kolory, profile sortowania, lokalizacja, makra

## 3. Najważniejsze Funkcje

| Kategoria | Wyróżnienia |
|-----------|-------------|
| Nawigacja | Dwa panele, pasek dysków, szybkie wyszukiwanie, zakładki |
| Edycja | Wbudowany edytor, podświetlanie składni (wtyczki), duże pliki |
| Podgląd | Elastyczny viewer (hex / Unicode), szybki podgląd |
| Archiwa | Przezroczyste przeglądanie (ZIP, RAR itd. via wtyczki) |
| Zdalne | FTP / zasoby sieciowe / panele tymczasowe |
| Automatyzacja | Silnik makr, skrypty, hooki zdarzeń |
| Personalizacja | Grupy kolorów, profile sortowania, tryby paneli |
| Lokalizacja | Wielojęzykowy interfejs i szablon zasobów |

## 4. Architektura (Zarys)

```text
+---------------------------+
| Far.exe (Core)           |
|  Panele / UI / Logika    |
+-------------+-------------+
              |
       Interfejsy Wtyczek (C/C++)
              |
   +----------+----------+------------------+
   | Archiwa  | Sieć     | Viewer / Edytor  | ... (20+ innych)
   +----------+----------+------------------+
```

- Rdzeń w C++ (MSVC / GCC / Clang)
- Wtyczki jako zewnętrzne DLL
- Zasoby językowe z szablonu `farlang.templ.m4` (nie edytuj `.lng` ręcznie)

## 5. Budowanie Far Manager

Wiele metod: Visual Studio, nmake, GCC/MinGW. Patrz `far/build.sh` oraz `_build/vc/all.sln`.

### 5.1 Visual Studio (MSBuild)

```pwsh
cd _build/vc
msbuild /property:Configuration=Release /property:Platform=x64 all.sln
```

Wyjście: `_build/vc/_output/product/Release.x64/Far.exe`

### 5.2 nmake (MSVC w wierszu poleceń)

```pwsh
& "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" amd64
cd far
nmake /f makefile_vc AMD64=1
```

Flagi:

- `DEBUG=1` — wersja debug
- `CLANG=1` — kompilacja clang-cl
- `PYTHON=1` — generator Python dla plików językowych

### 5.3 GCC / MinGW

```pwsh
cd far
mingw32-make -j 4 -f makefile_gcc
```

Cross‑compile:

```pwsh
cd far
$env:GCC_PREFIX="x86_64-w64-mingw32-"; make -j 4 -f makefile_gcc DIRBIT=64
```

### 5.4 Skrypt pomocniczy (bash)

```sh
cd far
./build.sh 64
./build.sh debug 64
```

### 5.5 Walidacja

Przed PR:

```pwsh
cd far
python tools/source_validator.py
```

Sprawdź także changelog i pliki pomocy (`misc/build-checks/`).

## 6. Wtyczki

- Katalog: `plugins/`
- Osobne budowanie: `makefile_all_vc`, `makefile_all_gcc`
- Przykłady: obsługa archiwów, panele tymczasowe, przeglądarka sieciowa

```pwsh
cd plugins
nmake /f makefile_all_vc AMD64=1
```

## 7. Lokalizacja

- Szablon: `far/farlang.templ.m4`
- Generowane: `*.lng` (nie edytuj)
- Modyfikacja: edytuj szablon, przebuduj (opcjonalnie `PYTHON=1`)
- Dokumentacja: `enc/README.md`

## 8. Makra i Automatyzacja

Test makr (po kompilacji):

```pwsh
cd _build/vc/_output/product/Debug.x64
./Far.exe -service "macro:test"
```

Referencje: katalog `enc/`.

## 9. Kluczowe Ścieżki

| Ścieżka | Cel |
|---------|-----|
| `far/` | Kod źródłowy rdzenia |
| `plugins/` | Wtyczki oficjalne |
| `misc/` | Instalator, walidatory |
| `enc/` | Dokumentacja i narzędzia CHM |
| `extra/` | Dodatki do dystrybucji |
| `_build/vc/` | Solucje Visual Studio |

## 10. Jakość i CI

Systemy CI:

- GitHub Actions: styl, changelog, help, matrix build
- AppVeyor: instalatory, testy makr, archiwa
- Azure Pipelines: pełne budowanie rozwiązania

## 11. Kontrybucje

Zacznij od `CONTRIBUTING.md`, wersje lokalne `CONTRIBUTING-PL.md`, `CONTRIBUTING-RU.md`.

Lista kontrolna:

1. Zaktualizuj `far/changelog` (góra pliku)
2. Uruchom validator stylu
3. Zbuduj Release + Debug
4. Przetestuj funkcjonalność
5. Nie edytuj plików `.lng` bezpośrednio

## 12. Zgłaszanie Błędów

<https://bugs.farmanager.com/>

Podaj: wersję, kroki reprodukcji, oczekiwane vs rzeczywiste zachowanie.

## 13. Licencja

BSD — plik `LICENSE`.

## 14. Szybki Start

```pwsh
git clone https://github.com/FarGroup/FarManager.git
cd FarManager
cd _build/vc
msbuild /property:Configuration=Release /property:Platform=x64 all.sln
cd _output/product/Release.x64
./Far.exe
```

## 15. Przydatne Linki

- Strona: <https://www.farmanager.com/>
- Fora: <https://enforum.farmanager.com/> / <https://forum.farmanager.com/>
- API Wtyczek: <https://api.farmanager.com/>

## 16. FAQ (Skrót)

| Pytanie | Odpowiedź |
|---------|-----------|
| Jak dodać nowy ciąg językowy? | Edytuj `farlang.templ.m4`, przebuduj |
| Czemu validator odrzuca plik? | Sprawdź BOM, tabulatory, include guards |
| Gdzie wersja? | `far/farversion.m4`, `far/vbuild.m4` |
| Czy mogę użyć Clang? | Tak: `CLANG=1` albo clang-cl w VS |

---

Ten rozszerzony README jest na gałęzi `demo-readme`. Podstawowa wersja pozostaje w `README-PL.md`.

[logo-img]: ./logo.svg
[logo-url]: <https://www.farmanager.com>
[VS-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/6pca73evwo3oxvr9?svg=true>
[VS-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager/history>
[GCC-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/k7ln3edp8nt5aoay?svg=true>
[GCC-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager-5lhsj/history>
[Clang-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/pvwnc6gc5tjlpmti?svg=true>
[Clang-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager-tgu1s/history>
[VS-Azure-img]: <https://img.shields.io/azure-devops/build/FarGroup/66d0ddcf-a098-4b98-9470-1c90632c4ba3/1.svg?logo=azuredevops>
[VS-Azure-url]: <https://dev.azure.com/FarGroup/FarManager/_build?definitionId=1>
