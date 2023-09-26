|[English](CONTRIBUTING.md)|[Русский](CONTRIBUTING-RU.md)|
|-|-|

### WSPÓŁPRACA

> Poniższe sekcje pomogą Ci zapoznać się z naszym procesem rozwoju.

#### Przesyłanie poprawek

Kiedy poczujesz że Twój kod jest przydatny i zechcesz go wnieść do projektu,
prosimy kierować się poniższymi wytycznymi:

1. Jedna logiczna zmiana na poprawkę, im mniejsza łatka, tym łatwiej nam
   ją sprawdzić i zatwierdzić.
2. Staraj się zachować spójność z ogólnym stylem kodu.
3. Podaj szczegółowy dziennik zmian dla swojej łatki.
4. Jeżeli Twoja poprawka wymaga aktualizacji dokumentacji (pomoc, Encyklopedia, itp.),
   prosimy o dołączenie potrzebnych informacji.
   Proszę utworzyć łatkę na najnowszym kodzie programu z repozytorium.
5. Poprawki należy przesyłać jako żądania aktualizacji repozytorium
   lub jako pliki diff do [system zgłaszania błędów](https://bugs.farmanager.com)
   lub [forum](https://forum.farmanager.com/viewforum.php?f=54).
6. Jeżeli planujesz tworzyć duże poprawki lub chcesz być na bieżąco rozwijać Far
   Manager, zapisz się do [listy mailingowej dla programistów](https://groups.google.com/group/fardeven)
   (<fardeven@googlegroups.com>).
7. Osoby często poprawiające kod będą uprawione do pełnego dostępu do repozytorium.


#### Kompilacja

```
cd far
```

1. Aby skompilować za pomocą Visual Studio można użyj projektu IDE lub pliku makefile.<br/>
   Przykład dla kompilacji z msbuild i vcxproj:<br/>
     `msbuild /property:Configuration=Release;platform=x64 far.vcxproj`<br/>
   Przykład dla nmake i makefile:<br/>
     `nmake /f makefile_vc`

2. Aby skompilować za pomocą GCC można użyć pliku makefile.<br/>
   Przykład dla MinGW i makefile:<br/>
     `mingw32-make -f makefile_gcc`

> Sprawdź także komentarze w plikach makefile_* dla dodatkowych parametrów kompilacji.


#### Dziennik zmian - plik `changelog`

1. Wszystkie komentarze do zatwierdzonych zmian powinny być zapisywane pliku
   `changelog` (po angielsku).
   Komentarze w kodzie powinny pozostać tylko jeżeli uważasz, że kod nie jest
   oczywisty i nie będzie zrozumiały dla "przyszłych pokoleń".
2. Najnowsze zmiany znajdują się na górze.
3. Każdy wpis rozpoczyna się nagłówkiem w formacie:
```
--------------------------------------------------------------------------------
warp 2006-12-05 01:39:38+03:00 - build 2149
```
4. Zmiany nie zawsze wymagają podniesienia numeru kompilacji (np. zmiany
   kosmetyczne lub niezwiązane z kodem).
5. Przykładowe makro do generowania nagłówka:
   [ChangelogHeader.lua](./misc/changelog/ChangelogHeader.lua)

#### farversion.m4

1. Ten plik zawiera informacje używane do generowania wersji Far:
   * `SPECIAL_VERSION`, ciąg znaków, który (jeżeli jest ustawiony), oznacza
   kompilację jako specjalną.
      - Przeznaczony dla wersji Far, których kod nie został jeszcze zatwierdzony
        w repozytorium, abyśmy my i użytkownicy nie byli zdezorientowani.
      - Jeżeli nie zostanie ustawiona, typ kompilacji zostanie zdefiniowany
        przez zmienną środowiskową FARMANAGER_BUILD_TYPE. Jeżeli nie jest ona
        ustawiona, typem kompilacji będzie Private.
        Obsługiwane typy kompilacji można znaleźć w wyliczeniu VERSION_STAGE w pliku
        plugin.hpp.
   * `VERSION_MAJOR` - główna wersja Far (np. 3).
   * `VERSION_MINOR` - podrzędna wersja Far (np. 0).
   * `VERSION_REVISION` - rewizja wersji Far (np. 0).
   * `VERSION_BUILD` - ustawiana w pliku `vbuild.m4`.


#### vbuild.m4

1. Jeżeli numer kompilacji w pliku `vbuild.m4` został zmieniony, to po zatwierdzeniu
   zmian w repozytorium należy uruchomić `tag_build.bat`.


#### Dodawanie nowych linii do plików językowych

1. Pliki `lng` są generowane automatycznie.
   Wszystkich zmian należy dokonywać w pliku `farlang.templ.m4`.
   Jeżeli nie wiesz jak przetłumaczyć swoje zmiany na inne języki,
   użyj angielskiej wersji z przedrostkiem `upd:`.


#### x64 - przeprowadzenie poprawnej kompilacji dla x86 i x64

1. DWORD_PTR, LONG_PTR, itd. są używane zamiast DWORD/long/itd. w następujących
   przypadkach:

   - potrzebne tam, gdzie wcześniej używano typu int/long/dword/or_any_other_non_pointer_type
      i przypisano do niego wartość wskaźnika.
   - ...
   - ...


> Przypomnienia, zadania, notatki powinny trafić do [systemu zgłaszania błędów](https://bugs.farmanager.com).
