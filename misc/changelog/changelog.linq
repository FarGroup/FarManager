<Query Kind="Program">
  <Reference>&lt;RuntimeDirectory&gt;\System.Windows.Forms.dll</Reference>
</Query>

private static readonly Regex shortHeadingPattern = new Regex(@"^\p{L}.+ \d\d\.\d\d", RegexOptions.Compiled | RegexOptions.CultureInvariant);
private static readonly Regex headingPattern = new Regex(@"^\p{L}.+ \d\d\.\d\d\.20\d\d \d\d:\d\d:\d\d [+-]\d\d00.*$", RegexOptions.Compiled | RegexOptions.CultureInvariant);
private static readonly Regex headingPatternBuild = new Regex(@"^\p{L}.+ \d\d\.\d\d\.20\d\d \d\d:\d\d:\d\d [+-]\d\d00 - build (\d\d\d\d?)$", RegexOptions.Compiled | RegexOptions.CultureInvariant);
private static readonly Regex headingPatternNoBuild = new Regex(@"^\p{L}.+ \d\d\.\d\d\.20\d\d \d\d:\d\d:\d\d [+-]\d\d00$", RegexOptions.Compiled | RegexOptions.CultureInvariant);
private static readonly Regex numberedItem = new Regex(@"^\d[01abc]?\.(\d\.)? ", RegexOptions.Compiled | RegexOptions.CultureInvariant);
private static readonly Regex entriesNotTranslated = new Regex(@"Entries \d+-\d+ are not translated yet\.", RegexOptions.Compiled | RegexOptions.CultureInvariant);

private const string file_changelog_rus = @"D:\DEV\FarManager\far\changelog";
private const string file_changelog_eng = @"D:\DEV\FarManager\far\changelog_eng";

private const string file_changelog_pattern = @"D:\DEV\FarManager\misc\changelog\changelog_{0:D2}";

private const string file_rejected_candidates = @"D:\DEV\FarManager\misc\changelog\rejected_candidates.csv";
private const int max_foldable_string_length = 50;

private const string file_folded_records = @"D:\DEV\FarManager\misc\changelog\folded_records";

private static readonly string recordSeparator = new string('-', 80);
private const string langSeparator = "· · · · · · · · · · · · · · · · · · · · · · · · ·";

private static readonly (string anchor, (Regex eng, Regex rus)[] patterns)[] translations =
{
	("delet", MakeTranlations(
		(@"cannot delete a macro", @"невозможно удалить макрос"),
		(@"FAR freeze on directories delete", @"зависание ФАРа при удалении каталогов")
	)),
	("clean", MakeTranlations(
		(@"ANSI code cleanup in", @"чистка анси кода из"),
		(@"cleanup", @"почистим немного"),
		(@"some more cleaning in", @"ещ(е|ё) немного чистки в"),
		(@"is cleaned a little bit", @"немного почищен"),
		(@"some cleanup", @"немного разной чистки")
	)),
	("macro", MakeTranlations(
		(@"refactoring (in|of) macro system", @"рефакторинг в системе макросов"),
		(@"minor refactoring in macro system", @"мелкий рефакторинг в макросистеме"),
		(@"changes related to macros", @"изменения, связанные с макросами"),
		(@"fix for nested macros", @"починим вложенные макросы"),
		(@"menu creation in a macro", @"создание меню в макросе"),
		(@"editing of macros", @"правка макросов"),
		(@"history didn't work in macros", @"не работала история в макросах"),
		(@"macro function Date\(\) did't work", @"не работала макрофункция Date\(\)"),
		(@"incorrect saving of REG_MULTI_SZ macros", @"некорректное сохранение REG_MULTI_SZ макросов")
	)),
	("add", MakeTranlations(
		(@"(an )?additions? (to|in)", @"дополнени(е|я) (к|в)"),
		(@"add .+ to <arc> mask\.", "добавляем .+ к маске <arc>"),
		(@"add(ed)?", @"добав(лена|лен|ить)"),
		(@"adding \w+ to", @"добавим \w+ в"),
		(@"some additions in", @"немного добавок в"),
		(@"another addition to","ещ(е|ё) одна добавка к"),
		(@"add [\w\s""]+ option", @"добавлена опция [\w\s""]+"),
		(@"structure is added", @"добавлена структура"),
		(@"\S+ function is added", @"добавлена функция \S+"),
		(@"refactoring - add some const", @"рефакторинг - добавим немного const"),
		(@"refactoring.\ Adding some 'const'ness", @"рефакторинг\. Добавим константности"),
		(@"add(ing)? some (asserts and )?'?const(-|')ness", @"добавим (ассертов и )?константности"),
		(@"is added", @"добавлена общая"),
		(@"ESPT_SETBOM and EOPT_BOM are added to API", @"в апи добавлены ESPT_SETBOM и EOPT_BOM"),
		(@"added \S+ to the project", @"добавил \S+ в проект")
	)),
	("refactor", MakeTranlations(
		(@"refactoring( of)?", @"рефакторинг"),
		(@"(minor|a bit of|slight) refactoring", @"(мелкий|небольшой) рефакторинг"),
		(@"refactoring(,| and) renamings?", @"рефакторинг(,| и) переименования"),
		(@"refactoring - rename & move", @"рефакторинг - переименования & переносы"),

		(@"slight refactoring of", @"порефакторим немного"),
		(@"other refactoring", @"прочий рефакторинг"),
		(@"other minor refactoring", @"прочий мелкий рефакторинг"),
		(@"(and )?some more refactoring( in)?", @"(и )?ещ(е|ё) немного рефакторинга( в)?"),

		(@"refactoring of the window manager", @"рефакторинг менеджера окон"),
		(@"minor refactoring of dialog engine", @"небольшой рефакторинг диалогового движка"),
		(@"refactoring: processing of keypresses", @"рефакторинг: обработка нажатий клавиш"),
		(@"continue refactoring of clipboard",@"продолжим рефакторинг буфера обмена"),
		(@"refactoring of custom sorting", @"рефакторинг кастомной сортировки"),
		(@"refactoring of descriptions", @"рефакторинг описаний"),

		(@"continuing( the)? refactoring( of)?", @"продолж(им|аем) рефакторинг"),
		(@"refactoring, continuation of", @"рефакторинг, продолжение"),
		(@"random refactoring", @"рандомный рефакторинг"),

		(@"minor refactoring related to #\d+", @"связанный с #\d+ мелкий рефакторинг"),
		(@"related to #\d+ refactoring", @"связанный с #\d+ рефакторинг")
	)),
	("except", MakeTranlations(
		(@"improve exceptions processing", @"улучшения в обработке исключительных ситуаций"),
		(@"some improvements in exception handling", @"уточнения обработки исключений"),
		(@"exception handling improvements", @"уточнения обработки исключений"),
		(@"corrections? (of|in) (SEH )?exceptions? (processing|handling)", @"уточнени(я|е) обработки (SEH-)?исключений"),
		(@"correction of processing exceptions for", @"уточнение обработки исключений для"),
		(@"exception on mf\.fsplit\('a\.:',0x\d+\+0x\d+\) call", @"исключение при вызове mf\.fsplit\('a\.:',0x\d+\+0x\d+\)")
	)),
	("crash", MakeTranlations(
		(@"crash (in|after|at)", @"(падение|валится) (в|после|при)"),
		(@"forum:( far /import)? crash( in)?", @"с форума: паде?ние (при|в)( far /import)?"),
		(@"caused crash", @"приводили к падению"),
		(@"correction of \d+: crash on exit", @"уточнение \d+: падение при выходе"),
		(@"crash on regex replace", @"падение при замене с регексами"),
		(@"crash in info panel", @"падение в инфопанели"),
		(@"crash after changes in api", @"после изменений в api падал"),
		(@"crash after commands like", @"падение после команды вида"),
		(@"crash while copying", @"падение во время копирования")
	)),
	("couple", MakeTranlations(
		(@"in a couple of places", @"в паре мест"),
		(@"a couple of things regarding to", @"пара мелочей в"),
		(@"a couple of typos in", @"пара опечаток в"),
		(@"another couple of (trifles|improvements of|drawing optimizations)", @"ещ(е|ё) пара (мелочей|уточнений|оптимизаций отрисовки)"),
		(@"(correction|continuation) of \d+( and|,) a couple of issues", @"(уточнение|продолжение) \d+( и|,) ещ(е|ё) пара мелочей"),
		(@"a couple of issues in (the copier|wrapper)", @"пара мелочей (в копире|во враппере)"),
		(@"more on %pathext% \+ a couple of issues", @"ещ(е|ё) на тему %pathext% \+ пара мелочей"),
		(@"a couple of leaks in filters", @"пара утечек в фильтрах")
	)),
	("leak", MakeTranlations(
		(@"memory leak in", @"мемори лик в"),
		(@"correction of \d+ \(memory leak\)", @"уточнение \d+ \(была утечка памяти\)")
	)),
	("fix", MakeTranlations(
		(@"fix (of|for|in) [\d.]+", @"(поправка к|исправление для) [\d.]+"),
		(@"(a )?fix(es)?( of| for| in)?", @"(уточнени(е|я)|поправка|поправлена|коррекция)( в| для)?"),
		(@"fixing( [\d.]+)?", @"(по)?чиним( [\d.]+)?"),
		(@"fix ""far \/\? > file"" and ""far > file""", @"починим ""far \/\? > file"" и ""far > file"""),
		(@"same fix as in [\d.]+ but for", @"фикс как и в [\d.]+ только для"),
		(@"correction of \d+, fix VS2010 build", @"уточнение \d+, восстановление сборки VS2010"),
		(@"fix in the Viewer settings dialog", @"фикс диалога Viewer settings"),
		(@"correction of \d+ - fixing broken export", @"уточнение \d+ - чиним поломаный экспорт"),
		(@"fixed error in \d+", @"коррекция ошибки внесённой в \d+"),
		(@"small fixes in the wrapper", @"мелкие правки во враппере"),
		(@"fixed memory leak in", @"убран memory leak в"),
		(@"dependencies are fixed in makefile", @"в makefile поправлены зависимости"),
		(@"projects dependencies are fixed", @"поправлены зависимости в проектах"),
		(@"fix of #\d+ for non-existing codepages", @"уточнение \d+ для несуществующих кодировок")
	)),
	("guid", MakeTranlations(
		(@"makeup \(UCase\) for GUID", @"косметика \(UCase\) для GUID"),
		(@"correct GUID for Network plugin", @"правильный guid Network-плагина"),
		(@"PluginMenuItem: Guid is replaced with Guids", @"в PluginMenuItem Guid заменён на Guids"),
		(@"guid for ""Plugin information"" dialog", @"guid для диалога ""Информация о плагине"""),
		(@"GUID for the login and password query dialog", @"GUID для диалога запроса логина и пароля"),
		(@"GUIDs needed", @"требуются гуиды"),
		(@"need GUIDs", @"нужны GUID-ы")
	)),
	("plugin", MakeTranlations(
		(@"for plugins", @"для плугинов"),
		(@"(“|`)const(”|') in", @"const в"), // plugin api.
		(@"getting rid of NM in", @"избавляемся от NM в"), // plugin.hpp.
		(@"FAR_LUA is removed from", @"убраны FAR_LUA из"), // plugin.hpp.
		(@"private field in", @"поле Private в"), // PluginStartupInfo.
		(@"plugin cache bugs", @"глюки с кешем плагинов"),
		(@"correction of \d+ for plugin panels", @"уточнение \d+ для плагиновых панелей"),
		(@"plugins loading correction", @"уточнение загрузки плагинов"),
		(@"correction of plugin loading process", "уточение процесса загрузки плагинов"),
		(@"correction of plugins? unloading", @"уточнение выгрузки плагинов"),
		(@"crash on plugin loading\/unloading", @"падение при загрузке\/выгрузке плагина"),
		(@"error loading plugins from cache", @"ошибка загрузки плагинов из кеша"),
		(@"correction of \d+ for ANSI plugins", @"уточнение \d+ для анси плагинов"),
		(@"small corrections in plugin manager", @"мелкии корекции в менеджере плагинов"),
		(@"export of pluginhotkeys\.db was broken\.", @"не работал экспорт pluginhotkeys\.db"),
		(@"all native plugins are broken", @"все нативные плагины перестали работать")
	)),
	("using", MakeTranlations(
		(@"using", @"используем"),
		(@"correction of \d+, using", @"уточнение \d+, заюзаем"),
		(@"using standard stacks and queues", @"используем стандартные стеки и очереди.")
	)),
	("correct", MakeTranlations(
		(@"corrections?( of| for| in)?", @"(уточнени(е|я)|поправка|коррекция|корректировка)( в| к| для| работы)?"),
		(@"correction( of)? \d+ for", @"уточнение \d+ для"),
		(@"correction of [\d.]+ and [\d.]+", @"уточнение [\d.]+ и [\d.]+"),
		(@"forum: corrections for", @"с форума: уточнения для"),
		(@"(corrections?( of)? ){2}", @"(уточнени(е|я) ){2}"),
		(@"(corrections?( of)? ){3}", @"(уточнени(е|я) ){3}"),
		(@"a (couple of|few) ""?corrections""?( of| in| to| on)?", @"пара ""?(уточнений|исправлений)""?( в)?"),
		(@"correction\+ of", @"уточнение\+"),

		(@"(yet )?(another|(one )?more|once more,) corrections?( of| for| on)?", @"(и )?ещ(ё|е)( одно| раз| пара)? (уточнени(е|й)|поправк(и|а))( в| к| для)?"),
		(@"more correct ""once more""", @"более корректный ""ещ(е|ё) раз"""),
		(@"another (couple|pair) of corrections( in| to| for)?", @"ещ(ё|е) пара (уточнений|исправлений)( в)?"),
		(@"correction of [\d.]+ (once more|again)", @"уточнение [\d.]+ ещ(ё|е) раз"),

		(@"correction and continuation of", @"уточнение и продолжение"),
		(@"corrections? (of|in) \w+ and", @"уточнени(е|я)( в)? \w+ и"),
		(@"correction of( \w+)? (function(ing)?|processing)( \w+)?", @"уточнение (работы|обработки)( \w+)?"),

		(@"size correction for", @"коррекция размеров для"),
		(@"types? correction( etc\.)?", @"коррекция типов( и т\. п\.)?"),
		(@"correction of types in", @"коррекция типов в"),
		(@"(wrapper|parser|translation) corrections?", @"уточнени(е|я) (враппера|парсера|перевода)"),
		(@"correction of (wrapper|\w+ class)", @"уточнение (враппера|класса \w+)"),
		(@"correction of \d+ for (menu|the clock|end key)", @"уточнение \d+ для (меню|часов|клавиши end)"),
		(@"corrections in SysLog`s", @"уточнения SysLog`ов"),
		(@"correct the leak detector", @"уточнение работы leak-детектора"),

		(@"build is corrected", @"поправлена сборка"),
		(@"corrected gcc and x\d+ builds", @"поправлена сборка gcc и x\d+"),

		(@"corrections? of build( #)?", @"(уточнени(е|я)|коррекци(я|и)) (build|билда)( #)?"),
		(@"corrections? of build \d+", @"уточнени(е|я) \d+ билда"),
		(@"corrections? of the previous build", @"уточнени(е|я) пред(ыдущего|\.) билда"),
		(@"cosmetic corrections? of the previous build", @"косметическ(ие|ая) (уточнени(е|я)|правк(и|а)) пред(ыдущего|\.) билда"),

		(@"corrections?( of| in) help", @"уточнени(е|я) справки"),
		(@"minor help corrections", @"мелкие уточнения справки"),
		(@"minor corrections in help files", @"мелкие уточнения в файлах справки"),
		(@"corrections of hlf, lng and documentation", @"уточнения hlf, lng и документации"),

		(@"incorrect drawing in", @"неверная отрисовка в"),
		(@"incorrect (AKey\(\)\/)?\$AKey (functioning|work)", @"(неправильная|некорректная) работа (AKey\(\)\/)?\$AKey"),
		(@"incorrect work", @"некорректная работа"),
		(@"incorrect \w+ redraw", @"некорректная отрисовка \w+")
	)),
	("continu", MakeTranlations(
		(@"continuation of", @"продолжение"),
		(@"continuation of continuation of", @"продолжение продолжения"),
		(@"continuation of \d+ and", @"продолжение \d+ и"),
		(@"continuation of \d+ once more", @"и ещ(е|ё) раз продолжение \d+"),
		(@"continuation of \d+ once more", @"продолжение \d+ ещ(е|ё) раз"),
		(@"continu(ing|ed)", @"продолж(аем|им|ение)"),
		(@"[#\d]+ is continued", @"продолжение [#\d]+"),
		(@"continuing \d+\. [\d/]+ bytes less", @"продолжим \d+\. ещ(е|ё) минус [\d/]+ байт"),
		(@"continuing the wrapper abstraction", @"продолжаем абстрагировать враппер"),
		(@"continuing playing with", @"продолжаем играться с"),
		(@"(printf elimination is continuing|continuing printf elimination|continuing elimination of printf)", @"продолжаем давить printf")
	)),
	("remov", MakeTranlations(
		(@"(is |are )?removed", @"(удал(е|ё)н(а|ы)?|убран(а|о|ы)?)"),
		(@"support (is |are )?removed", @"(удал(е|ё)н(а|ы)?|убран(а|о|ы)?) поддержка"),
		(@"is removed", @"больше нет"),
		(@"removed?", @"(долой|удалим)"),
		(@"are removed from wrapper", @"из врапера убраны"),
		(@"policies are removed", @"удалены политики"),
		(@"unnecessary volatile is removed", @"убран ненужный volatile"),
		(@"[\w.]+ (parameter|option|field) is removed", @"(параметр|опция|поле) [\w.]+ удал(ена|ено|ён)"),
		(@"remove FARMACRO_KEY_EVENT", @"событие FARMACRO_KEY_EVENT удалено"),
		(@"refactoring \(enum MACROMODEAREA is removed\)", @"рефакторинг \(удалён enum MACROMODEAREA\)")
	)),
	("gone", MakeTranlations(
		(@"is gone", @"больше нет"),
		(@"flag FMSG_DOWN has gone", @"флага FMSG_DOWN больше нет")
	)),
	("complet", MakeTranlations(
		(@"completion of", @"завершение")
	)),
	("updat", MakeTranlations(
		(@"is slightly updated", @"немного обновлен"),
		(@"SQLite updated to", @"обновим SQLite до"),
		(@"updated a bit", @"Обновил немного"),
		(@"updates in", @"обновления в")
	)),
	("work", MakeTranlations(
		(@"((did|does) not|didn't) work( in)?", @"не работа(л|ет)( в)?"),
		(@"[\w/]+ command didn't work", @"не работала команда [\w/]+"),
		(@"forum: Del doesn't work in", @"с форума: не работает Del при"),
		(@"modifiers did not work with the", @"не работали модификаторы с"),
		(@"DisableOutput flag does not work", @"не работает флаг DisableOutput"),
		(@"event did not work", @"не работало сообщение"),
		(@"did not work in wrapper", @"не работала во врапере"),
		(@"run with CtrlAltEnter did not work", @"не работал запуск по CtrlAltEnter"),
		(@"[\w""\\ ]+ does not work in command line", @"не работает [\w""\\ ]+ в командной строке"),
		(@"after \d+, alt-d did not work in editor", @"после \d+ не работало alt-d в редакторе"),
		(@"wiping of empty files would not work", @"не работал вайп пустых файлов"),
		(@"gcc debug x\d+ build did not work", @"не работал gcc debug x\d+ билд."),
		(@"mload\(\) function did not work", @"не работала функция mload\(\)"),
		(@"!@! metacharacter doesn't work", @"не работает метасимвол !@!"),
		(@"folder encryprion didn't work", @"не работало шифрование папок"),
		(@"a few workarounds to support", @"пара workaround-ов для поддержки"),
		(@"(a )?workaround (to|for)( support)?", @"костыль для поддержки"),
		(@"a workaround for a bug in", @"обход бага в"),
		(@"workaround for another VS bug", @"обход очередного бага VS"),
		(@"can work without", @"может работать без"),
		(@"CtrlUp and CtrlDown work in shortcuts menu", @"в shortcuts menu работают CtrlUp и CtrlDown")
	)),
	("malfunction", MakeTranlations(
		(@"(was )?malfunction(al)?", @"не работа(л|ет)"),
		(@"search malfunction in", @"не работал поиск в")
	)),
	("more", MakeTranlations(
		(@"more", @"(продолжение|больше)"),
		(@"and \d+ once more", @"и ещ(е|ё) раз \d+"),
		(@"(and )?(once |one |some |even )?more( time)?", @"(и )?(ещ(е|ё)|снова)( раз| немного)?"),
		(@"(some )?more( on| of)?", @"ещ(е|ё)( немного| всякое)?( на тему)?"),
		(@"more on \w+ and \w+", @"ещ(е|ё) на тему \w+ и \w+"),
		(@"more on \w+( again)?", @"(и )?(ещ(е|ё)|снова) всякое про \w+"),
		(@"more changes on", @"ещ(е|ё) правки"),
		(@"more types in", @"больше типов в"),
		(@"more on info panel", @"ещ(е|ё) всякое на тему инфопанели"),
		(@"more \d+, CtrlPgDn on files", @"ещ(е|ё) \d+, CtrlPgDn на файлах"),
		(@"more tests", @"больше тест(ов|ы)"),
		(@"getting rid of some more memcpy and memset", @"задавим ещ(е|ё) немного memcpy и memset"),
		(@"no more", @"больше нет")
	)),
	("again", MakeTranlations(
		(@"(and )?(once |even )?again", @"(и )?(ещ(е|ё)|снова)( раз| немного)?"),
		(@"and \w+ again", @"и ещ(е|ё) раз про \w+"),
		(@"again, a fix for", @"снова поправка для"),
		(@"was broken again", @"снова поломался"),
		(@"once again, the epic about", @"и снова эпопея о"),
		(@"is pure C compatible again", @"снова совместим с pure C")
	)),
	("minor", MakeTranlations(
		(@"minor fixes in the wrapper", @"мелкие правки во враппере"),
		(@"minor fixes in elevation and copier", @"косметические правки в elevation и копире"),
		(@"a minor fix to get rid of calling GeneralCfg", @"небольшое избавление от дергания GeneralCfg")
	)),
	("little", MakeTranlations(
		(@"a little of", @"немного")
	)),
	("misc", MakeTranlations(
		(@"misc on", @"ещ(е|ё) всякое на тему"),
		(@"misc stuff", @"мелочи разные"),
		(@"misc", @"разная мелочь")
	)),
	("makeup", MakeTranlations(
		(@"(other )?makeup( for)?", @"(прочая )?косметика( для)?"),
		(@"some makeup", @"косметика и около")
	)),
	("error", MakeTranlations(
		(@"(forum: )?error in", @"(с форума: )?ошибка в"),
		(@"settings import error", @"ошибка при импорте настроек"),
		(@"F\d+ copying error", @"ошибка копирования по F\d+"),
		(@"visual C\+\+ build error", @"не собиралось в visual C\+\+"),
		(@"x\d+ compilation error in", @"ошибка компиляции x\d+ версии в"),
		(@"ANSI wrapper error", @"ошибка анси-враппера"),
		(@"PCTL_GETPLUGININFORMATION error", @"ошибка PCTL_GETPLUGININFORMATION"),
		(@"error in ECTL_SETKEYBAR when", @"ошибка в ECTL_SETKEYBAR, когда"),
		(@"Dlg.GetValue error", @"ошибка с Dlg.GetValue"),
		(@"regexps processing error", @"ошибка обработки регэкспов"),
		(@"error in metacharacters processing", @"ошибка при обработке метасимволов"),
		(@"error when launching", @"ошибка при запуске"),
		(@"incorrect text of system error", @"неправильный текст системной ошибки")
	)),
	("bug", MakeTranlations(
		(@"((another |a )couple of )?bugs (in|for)", @"((ещ(е|ё) )?пара )?(баг|глюк)(и|ов)( в)?"),
		(@"(a )?(small |minor )?bug in", @"(мелкий )?глю(чо)?к в"),
		(@"another bug at the same place", @"и ещ(е|ё) один баг там же"),
		(@"visual bug in", @"визуальный глюк в"),
		(@"fix minor bugs in dialogs", @"исправлены мелкие баги в диалогах"),
		(@"bugs in wrapper", @"баги во враппере"),
		(@"another bug in menus", @"ещ(е|ё) один глюк в меню"),
		(@"bugs in menus invoked via the wrapper", @"глюки в меню вызваных через враппер"),
		(@"drawing bug in editor", @"глюк отрисовки в редакторе"),
		(@"(some|forgotten) debug(ging)? (code|junk)( in \d+)?( :\))?", @"(забытый )?отладочный (код|мусор)( в \d+)?( :\))?"),
		(@"some code was left after debugging :\)", @"забытый отладочный код :\)"),
		(@"debug (target )?(couldn't be )?buil(d|t)( fail(ure)?)?( is fixed)?", @"(не собирал(ся|ась)|исправлена) (дебаг|debug)(\s|-)?(сборка)?")
	)),
	("optim", MakeTranlations(
		(@"a few minor optimisations", @"пара мелких оптимизаций"),
		(@"is optimized slightly\.", @"небольшая оптимизация"),
		(@"(small )?optimization (for|of)", @"(небольшая )?оптимизация"),
		(@"optimization of several api\* wrappers", @"оптимизация некоторых api\*-обёрток"),
		(@"(lng|directory|folders) (read|re-reading) optimi(s|z)ation", @"оптимизация (чтения|перечитывания) (lng|каталогов|папки)"),
		(@"editor (drawing|search) optimization", @"оптимизация (отрисовки|поиска) (в )?редактор(а|е)"),
		(@"reading\/setting file owners optimisation", @"оптимизация чтения\/установки владельцев файлов")
	)),
	("renam", MakeTranlations(
		(@"some renamings?", @"немного переименований"),
		(@"[\w/""]+ key renamed to", @"ключ [\w/""]+ переименован в"),
		(@"is renamed to", @"переименована? в")
	)),
	("repl", MakeTranlations(
		(@"replaces", @"на замену"),
		(@"some replacement of BOOL to bool", @"немного замены BOOL на bool"),
		(@"some space to tab replacements", @"немного замены space на tab")
	)),
	("improv", MakeTranlations(
		(@"continuation of \d+ - improve error handling", @"продолжение \d+ - уточнения обработки ошибок"),
		(@"improved gcc build", @"улучшение gcc сборки")
	)),
	("roll", MakeTranlations(
		(@"rollback( of)?", @"откат(им)?"),
		(@"rollback [\d.]+ and [\d.]+", @"откат [\d.]+ и [\d.]+"),
		(@"partial rollback of \d+ \(Undo\/Redo in editor\)", @"частичный откат \d+ \(Undo\/Redo в редакторе\)"),
		(@"rollback of \d+ - false alarm \)", @"откат \d+ - ложная тревога \)")
	)),
	("revert", MakeTranlations(
		(@"(partial )?revert( of)?", @"(частичный )?откат")
	)),
	("move", MakeTranlations(
		(@"(sqlite code )?moved to", @"(sqlite-товый код )?переехал(а|о)? в"),
		(@"ReadConfig\(\) call is moved ""higher""", @"вызов ReadConfig\(\) перенесен ""выше""")
	)),
	("arrang", MakeTranlations(
		(@"rearrangement of", @"переделки в"),
		(@"rearrangement of struct KeyBarTitles \(part", @"переделка struct KeyBarTitles \(часть")
	)),
	("warn", MakeTranlations(
		(@"warnings?( in| etc)?", @"в(о|а)рнинги?( в| и т\.п\.)?"),
		(@"warning and near", @"ворнинг и около"),
		(@"support for VS \d+ \(CTP\), with warnings so far", @"поддежка VS \d+ \(CTP\), пока с варнингами")
	)),
	("version", MakeTranlations(
		(@"new versions of", @"новые версии"),
		(@"(\w+ |a )?new (\w+ )?version( (of|for) \w+)?", @"новая версия \w+"),
		(@"(\w+ and \w+ )?new (\w+ and \w+ )?versions( of \w+ and \w+)?", @"новые версии \w+ и \w+"),
		(@"(minimum )?gcc (min )?version (-|is)", @"минимальная версия gcc -"),
		(@"far version is now", @"версия фара теперь")
	)),
	("broken", MakeTranlations(
		(@"after \d+, WindowMode was broken", @"после \d+ криво работал WindowMode"),
		(@"adapters were broken after \d+", @"после \d+ не работали адаптеры"),
		(@"broken Message with FMSG_ALLINONE flag", @"сломали Message с флагом FMSG_ALLINONE"),
		(@"fixing the broken", @"чиним поломанное"),
		(@"input of RAlt\+character was broken in \d+", @"в \d+ сломали ввод RAlt\+символ"),
		(@"in \d+, folder shortcuts were broken", @"в \d+ сломались folder shortcuts"),
		(@"grabber is broken", @"поломался grabber"),
		(@"from Denis Kosy: Mantis#\d+: Execute broken", @"от Denis Kosy: Mantis#\d+: Execute поломалась")
	)),
	("break", MakeTranlations(
	)),
	("problem", MakeTranlations(
		(@"(forum: )?problems with", @"(с форума: )?проблемы с"),
		(@"HEX search problem", @"проблема HEX поиска")
	)),
	("help", MakeTranlations(
		(@"some help translation", @"немного перевода справки"),
		(@"some translation for English help", @"немного перевода английской справки"),
		(@"help translation from", @"перевод хелпа от"),
		(@"help (on|for)", @"хелп для"),
		(@"added help on", @"добавил хэлп по"),
		(@"grammar in help", @"грамматика в справке"),
		(@"a bug in help", @"глюк в хелпе"),
		(@"various fixes in help", @"различные исправления в хэлпе"),
		(@"help topic about filters is added", @"добавил хэлп о фильтрах"),
		(@"help topic about user menu is updated", @"обновил хэлп про юзер меню"),
		(@"updated help on RegExp`s\.", @"добавка в help`ы по RegExp`ам")
	)),
	("typo", MakeTranlations(
		(@"(a |fix )?typo (in|for)", @"опечатка (в|для)"),
		(@"(a |fix )?typo", @"опечатка"),
		(@"(yet )?another typo", @"ещ(е|ё) опечатка"),
		(@"typo in string constructor", @"опечатка в конструкторе стринга")
	)),
	("port" /* support / import / export / unimportant / portion */, MakeTranlations(
		(@"support", @"поддержка"),
		(@"win\d+ versions of mingw are supported again", @"снова поддерживаются win\d+-версии mingw"),
		(@"support for compilation with", @"поддержка компиляции в"),
		(@"support for git in", @"поддержка git в"),
		(@"support for the next VS\d+ update", @"поддержка очередного апдейта VS\d+"),
		(@"unicode support in xml file names", @"поддержка unicode в именах xml"),
		(@"(export\/)?import( from XML)? for", @"(экспорт\/)?импорт( из XML)? для")
	)),
	("build", MakeTranlations(
		(@"build", @"-?сборка"),
		(@"build with", @"собираем с"),
		(@"builds with", @"сборка под"),
		(@"build requires", @"для сборки требуются"),
		(@"gcc build problems", @"проблемы сборки в gcc"),
		(@"fix [\w\d]+ build", @"(ис|по)правлена сборка в [\w\d]+"),
		(@"fix for vc-build", @"поправлена vc-сборка"),
		(@"documentation and build corrections", @"утончения документации и сборки"),
		(@"build failure( in)?", @"не собирал(ось|ся)( в| с)?"),
		(@"didn't build in [\w\d]+ \(at least\)", @"не собирался \(как минимум\) в [\w\d]+"),
		//(@"a better way (does|may) exist", @"(возможно есть )?способ (лучше|есть)"), // gcc build
		(@"see build", @"см\. сборку"),
		(@"in the previous build", @"в предыдушем билде"),
		(@"ucd, sqlite and tinyxml build is redone", @"переделана сборка ucd, sqlite и tinyxml")
	)),
	("project", MakeTranlations(
		(@"projects for", @"проекты для")
	)),
	("use", MakeTranlations(
		(@"use [\w:]+ \(if present\)", @"используем [\w:]+ \(если есть\)"),
		(@"use Catch2 for unit tests", @"для юнит-тестов используется Catch2"),
		(@"don't use", @"не используем"),
		(@"use native thread_local in gcc", @"используем нативный thread_local в gcc"),
		(@"\w+ is removed for uselessness", @"удал(е|ё)н(а|ы)? \w+ за (ненужностью|ненадобностью)"),
		(@"(also, some )?unused code is removed", @"(заодно )?удал(ен|им|ил) (немного )?не\s?используем(ый|ого) кода?"),
		(@"removing some unused code", @"удалим немного неиспользуемого кода"),
		(@"removed? unused( code( from)?)?", @"(убраны?|удалён) неиспользуемы(й|е)( код( в)?)?"),
		(@"some useless code is removed", @"убран ненужный код"),
		(@"wiping unused code", @"удалён неиспользуемый код"),
		(@"useless code was forgotten", @"забыл ненужный код"),
		(@"unused code removed in window manager", @"удалён неиспользуемый код в оконном менеджере")
	)),
	("default", MakeTranlations(
		(@"is off by default", @"по умолчанию выключено"),
		(@"is on by default", @"по умолчанию включен"),
		(@"ClearType configuration is on by default", @"настройка ClearType по дефолту включена"),
		(@"is Windows by default", @"по умолчанию Windows"),
		(@"correction of \d+ - OEM CP by default", @"уточнение \d+ - OEM CP по умолчанию")
	)),
	("now", MakeTranlations(
	)),
	("several", MakeTranlations(
		(@"in several (places|inner functions)", @"в нескольких (местах|внутрених функциях)")
	)),
	(default, MakeTranlations(
		(@"(and )?((an)?other |a )?(couple of |few |pair of |some (more )?)?(minor |trivial )?(changes|fixes|issues|trivia|trifles|things)( in)?",
			@"(и )?(ещ(е|ё) )?(пар(а|у)|несколько|всякие|прочие|разные|некоторые|незначительные) (мелоч(ей|и)|изменения)( к| в)?"),
		(@"(some )?(cosmetic changes?|makeup)( to| in)?", @"(разные )?космети(ка|ческие)( изменения| мелочи)?( в)?"),

		(@"a few bool's", @"немного bool"),

		(@"""cd ~"" behavior", @"Поведение ""cd ~"""),

		(@"regression in", @"регрессия"),
		(@"regressions of #\d+ are fixed", @"поправлены регрессы \d+"),

		(@"consequences", @"""последствия"""),

		(@"the 2nd attempt", @"попытка №2"),
		(@"finished", @"доделал"),

		(@"translation for", @"перевод для"),

		(@"is now in [\w\\]+ format", @"теперь в формате [\w\\]+"),
		(@"EditorColor.Color type is now","тип EditorColor.Color теперь"),

		(@"in sqlite: done", @"в sqlite - готово"),
		(@"in sqlite statements for the history", @"в sqlite statements для истории"),

		(@"caching LookupPrivilegeValue results", @"кеширование результатов LookupPrivilegeValue"),
		(@"ECTL_REDRAW call from dialog", @"вызов ECTL_REDRAW из диалога"),
		(@"empty combo box line", @"пустая строка комбобокса"),
		(@"forum: an issue with panels in", @"с форума: проблема с панелями"),
		(@"adaptation of grabber to", @"адаптация граббера к"),
		(@"as in the editor", @"как в редакторе"),
		(@"always returned", @"всегда возвращал"),
		(@"the same for", @"тоже самое для"),

		(@"Platform=x64 for VS2010", @"для VS2010 Platform=x64"),
		(@"vc compilation for", @"компиляция vc для"),
		(@"NO_RELEASE_PDB flag for make", @"флаг NO_RELEASE_PDB для мэйка"),
		(@"vc has no vswscanf", @"в vc vswscanf нет"),
		(@"MSVC \d+ compatibility", @"совместимость с MSVC \d+"),
		(@"for [\d.]+ only", @"только для версии [\d.]+"),

		(@"skips", @"пропускает"),
		(@"is numbered from", @"нумеруется с"),
		(@"and dialogs", @"и диалоги"),
		(@"still (reminds of|recalls) itself", @"ещ(е|ё) даёт о себе знать"),

		(@"(from the )?forum", @"с форума"),

		(@"tilde", @"тильда"),
		(@"regarding", @"в части"),

		(@"\band\b", @"\bи\b"),
		(@"\bfrom\b", @"\bот\b"),
		(@"\bin\b", @"\b(в|при)\b"),
		(@"\bfor\b", @"\bдля\b"),
		(@"(and )?another one", @"и ещ(е|ё)"),
		(@"FAR freeze", @"зависание FAR"),
		(@"system parameters", @"системные параметры"),
		(@"tests", @"тест(ов|ы)"),
		(@"tabs in", @"табуляции в"),
		(@"ampersand in", @"амперсанд в"),
		(@"garbage in", @"мусор в"),
		(@"s", @"`ы")
	)),
};

private static List<string> translation_candidates = new List<string>(100);

private static Dictionary<string, List<string>> suppressed_candidates =
	translations.Where(group => group.anchor != default).ToDictionary(group => group.anchor, group => new List<string>());

private static readonly HashSet<string> known_irregular_headers = new HashSet<string>
{
	"t-rex 28.04.2011 21:33:34 +0200 - build 2012 future",
	"t-rex 19.04.2011 01:09:36 +0200 - build 1972 - Chag Sameach!",
	"t-rex 04.11.2009 17:16:24 +0200 - build 1200 (UTF-16 LE :)",
	"t-rex 19.12.2008 01:41:01 +0200 - build 666 :) мой",
	"t-rex 25.06.2008 22:48:27 +0200 - build 522 Турция vs. Россия :)",
	"t-rex 14.03.2008 19:16:30 +0200 - build 666-666/3",
};

private static (Regex eng, Regex rus)[] MakeTranlations(params (string eng, string rus)[] patterns)
{
	return patterns.Select(pattern =>
	(
		new Regex($@"\s?{pattern.eng}\s?", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase),
		new Regex($@"\s?{pattern.rus}\s?", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase)
	))
	.ToArray();
}


void Main()
{
	if (translation_candidates.Count > 0)
	{
		throw new Exception($"translation_candidates.Count: {translation_candidates.Count}");
	}

	if (suppressed_candidates.Any(sc => sc.Value.Count > 0))
	{
		throw new Exception("suppressed_candidates is not empty!");
	}

	var count_rus = CheckChangelog(file_changelog_rus);
	var count_eng = CheckChangelog(file_changelog_eng);

	var changelog_rus = ReadChangelog(file_changelog_rus);
	var changelog_eng = ReadChangelog(file_changelog_eng);

	if (count_rus != changelog_rus.Count || count_eng != changelog_eng.Count)
	{
		throw new Exception("Counts do not match after Read");
	}

	WriteChangelog(file_changelog_rus, changelog_rus);
	WriteChangelog(file_changelog_eng, changelog_eng);

	if (CombineChangelog(changelog_rus, changelog_eng, 10000) != count_eng)
	{
		throw new Exception("count_eng does not match after CombineChangelog");
	}
}

private static int CheckChangelog(string file)
{
	var count = 0;

	foreach (var line in File.ReadLines(file))
	{
		if (!shortHeadingPattern.Match(line).Success)
		{
			continue;
		}

		count++;

		if (!headingPattern.Match(line).Success)
		{
			throw new Exception($"Not a header in {file}:{Environment.NewLine}{line}");
		}

		if (!headingPatternBuild.Match(line).Success && !headingPatternNoBuild.Match(line).Success && !known_irregular_headers.Contains(line))
		{
			throw new Exception($"Irregular header in {file}:{Environment.NewLine}{line}");
		}
	}

	$"Checked {count} records in {file}".Dump();
	return count;
}

private static List<(string heading, List<List<string>> paragraphs)> ReadChangelog(string file)
{
	var changelog = new List<(string heading, List<List<string>> paragraphs)>();
	var record = (heading: (string)null, paragraphs: new List<List<string>>());
	var paragraph = new List<string>();

	foreach (var line in File.ReadLines(file))
	{
		if (headingPattern.Match(line).Success)
		{
			FlushRecord(ref paragraph, record, changelog);
			record.heading = line;
			record.paragraphs = new List<List<string>>();
			continue;
		}

		if (string.IsNullOrWhiteSpace(line))
		{
			FlushParagraph(ref paragraph, record);
			continue;
		}

		paragraph.Add(line);
	}

	FlushRecord(ref paragraph, record, changelog);

	$"Read {changelog.Count} records in {file}".Dump();

	return changelog;
}

private static void FlushRecord(
	ref List<string> paragraph,
	(string heading, List<List<string>> paragraphs) record,
	List<(string heading, List<List<string>> paragraphs)> changelog)
{
	if (record.heading != null)
	{
		FlushParagraph(ref paragraph, record);
		changelog.Add(record);
	}
}

private static void FlushParagraph(
	ref List<string> paragraph,
	(string heading, List<List<string>> paragraphs) record)
{
	if (paragraph.Count > 0)
	{
		record.paragraphs.Add(paragraph);
		paragraph = new List<string>();
	}
}

public static void WriteChangelog(string file, List<(string heading, List<List<string>> paragraphs)> changelog)
{
	using (var writer = new CnagelogWriter(file, 1))
	{
		foreach (var record in changelog)
		{
			writer.WriteRecord(record, with_separator: false);
		}
	}
}

private static int CombineChangelog(
	List<(string heading, List<List<string>> paragraphs)> changelog_rus,
	List<(string heading, List<List<string>> paragraphs)> changelog_eng,
	int chunk_size)
{
	using (var changelog = new CnagelogWriter(
		file_changelog_pattern,
		(int.Parse(headingPatternBuild.Match(changelog_rus[0].heading).Groups[1].Value) + chunk_size - 1) / chunk_size))
	{
		int count_eng;

		using (var folded = new CnagelogWriter(file_folded_records, 1))
		{
			count_eng = CombineChangelog(changelog, changelog_rus, changelog_eng, chunk_size, folded);
		}

		File.WriteAllLines(
			file_rejected_candidates,
			translation_candidates
				.Concat(suppressed_candidates
					.OrderBy(group => group.Key)
					.SelectMany(group => group.Value.Prepend($"{Environment.NewLine}---- {group.Key} ----"))));

		return count_eng;
	}
}

private static int CombineChangelog(
	CnagelogWriter changelog,
	List<(string heading, List<List<string>> paragraphs)> changelog_rus,
	List<(string heading, List<List<string>> paragraphs)> changelog_eng,
	int chunk_size,
	CnagelogWriter folded_records)
{
	string.Empty.Dump();

	using (var records_eng = changelog_eng.GetEnumerator())
	{
		var count_eng = 0;
		var count_folded = 0;
		records_eng.MoveNext();

		foreach (var record_rus in changelog_rus)
		{
			var record_eng = records_eng.Current;

			if (record_eng.heading == record_rus.heading)
			{
				count_eng++;
				records_eng.MoveNext();

				var entries_not_translated = CheckForEntriesNotTranslated(record_eng.paragraphs);

				changelog.WriteRecord(record_eng, with_separator: true);

				if (AreFoldable(record_eng.paragraphs, record_rus.paragraphs))
				{
					count_folded++;
					folded_records.WriteRecord(record_eng, with_separator: true);
					folded_records.WriteTranslation(record_rus);
				}
				else
				{
					changelog.WriteTranslation(record_rus);
					
					PrintTranslationCandidates(record_eng.paragraphs, record_rus.paragraphs, record_rus.heading);
				}

				if (entries_not_translated != null)
				{
					changelog.WriteParagraph(entries_not_translated);
				}
			}
			else
			{
				changelog.WriteRecord(record_rus, with_separator: true);
			}

			var match = headingPatternBuild.Match(record_rus.heading);
			if (match.Success && int.Parse(match.Groups[1].Value) % chunk_size == 0)
			{
				changelog.NextChunk();
			}
		}

		$"Folded {count_folded} records".Dump();
		$"Wrote {count_eng} records from changelog_eng".Dump();
		"Last seen record in changelog_eng".Dump();
		records_eng.Current.heading.Dump();

		if (records_eng.Current.heading != null)
		{
			throw new Exception("records_eng was not exhausted.");
		}

		return count_eng;
	}
}

private static List<string> CheckForEntriesNotTranslated(List<List<string>> paragraphs)
{
	var last_paragraph = paragraphs[paragraphs.Count - 1];

	if (last_paragraph.Count == 3
	&& last_paragraph[0] == recordSeparator
	&& last_paragraph[2] == recordSeparator
	&& entriesNotTranslated.Match(last_paragraph[1]).Success)
	{
		paragraphs.RemoveAt(paragraphs.Count - 1);
		return last_paragraph;
	}

	return null;
}

private static bool AreFoldable(List<List<string>> paragraphs_eng, List<List<string>> paragraphs_rus)
{
	if (paragraphs_eng.Count != paragraphs_rus.Count) return false;

	return paragraphs_eng.Zip(paragraphs_rus, AreFoldable).All(foldable => foldable);
}

private static bool AreFoldable(List<string> eng, List<string> rus)
{
	if (eng.SequenceEqual(rus)) return true;

	if (eng.Count != 1 || rus.Count != 1) return false;

	NumberedItem(eng[0], rus[0], out string eng_text, out string rus_text);

	return translations.Any(group =>
		(group.anchor == default || eng_text.IndexOf(group.anchor, StringComparison.OrdinalIgnoreCase) >= 0)
		&& group.patterns.Any(pat =>
			string.Compare(
				pat.eng.Replace(eng_text, string.Empty, 1),
				pat.rus.Replace(rus_text, string.Empty, 1),
				StringComparison.InvariantCultureIgnoreCase) == 0));
}

private static void PrintTranslationCandidates(
	List<List<string>> paragraphs_eng,
	List<List<string>> paragraphs_rus,
	string heading)
{
	if (paragraphs_eng.Count != paragraphs_rus.Count) return;

	foreach (var pars in paragraphs_eng.Zip(paragraphs_rus, (eng, rus) => (eng: eng, rus: rus)))
	{
		PrintTranslationCandidates(pars, heading);
	}
}

private static bool NumberedItem(string eng, string rus, out string eng_text, out string rus_text)
{
	eng_text = eng;
	rus_text = rus;

	if (!char.IsDigit(eng[0]) || !char.IsDigit(rus[0])) return false;

	var eng_num = numberedItem.Match(eng).Value;
	var rus_num = numberedItem.Match(rus).Value;

	if (string.IsNullOrEmpty(eng_num) || string.IsNullOrEmpty(rus_num))
	{
		throw new Exception("Malformed numbered items.");
	}

	if (eng_num != rus_num)
	{
		throw new Exception("Numbered items do not match.");
	}

	eng_text = eng.Substring(eng_num.Length);
	rus_text = rus.Substring(rus_num.Length);

	return true;
}

private static void PrintTranslationCandidates((List<string> eng, List<string> rus) pars, string heading)
{
	if (AreFoldable(pars.eng, pars.rus)) return;

	if (pars.eng.Count != 1 || pars.rus.Count != 1) return;
	if (pars.eng[0].Length > max_foldable_string_length || pars.rus[0].Length > max_foldable_string_length) return;

	if (!NumberedItem(pars.eng[0], pars.rus[0], out string eng_text, out string rus_text))
	{
		return; // Manually verified: no interesting candidates
	}

	foreach (var group in translations)
	{
		if (group.anchor != default && !Char.IsUpper(group.anchor, 0) && eng_text.IndexOf(group.anchor, StringComparison.OrdinalIgnoreCase) >= 0)
		{
			suppressed_candidates[group.anchor].Add(TranslationCandidate(eng_text, rus_text, heading));
			return;
		}
	}

	translation_candidates.Add(TranslationCandidate(eng_text, rus_text, heading));
}

private static string TranslationCandidate(string eng_text, string rus_text, string heading)
{
	return
		$@"""{
			eng_text.Replace("\"", "\"\"")
		}"",""{
			rus_text.Replace("\"", "\"\"")
		}"",""{
			heading.Replace("\"", "\"\"")
		}""";
		//$@"""{
		//	decimalNumber.Replace(eng_text, "0").Replace("\"", "\"\"")
		//}"",""{
		//	decimalNumber.Replace(rus_text, "0").Replace("\"", "\"\"")
		//}"",""{
		//	heading.Replace("\"", "\"\"")
		//}""";
}

public class CnagelogWriter : IDisposable
{
	private StreamWriter writer;

	private string file_pattern;
	private int chunk_count;

	public CnagelogWriter(string file_pattern, int expected_chunks)
	{
		this.file_pattern = file_pattern;
		this.chunk_count = expected_chunks;
		NextChunk();
	}

	public void NextChunk()
	{
		if (this.writer != null)
		{
			this.writer.Dispose();
		}

		this.writer = new StreamWriter(string.Format(this.file_pattern, --chunk_count));
	}

	public void WriteRecord((string heading, List<List<string>> paragraphs) record, bool with_separator)
	{
		if (this.writer.BaseStream.Position > 0)
		{
			this.writer.WriteLine();
		}

		if (with_separator)
		{
			this.writer.WriteLine(recordSeparator);
		}
		this.writer.WriteLine(record.heading);
		this.WriteParagraphs(record.paragraphs);

		this.writer.Flush();
	}

	public void WriteTranslation((string heading, List<List<string>> paragraphs) record)
	{
		this.writer.WriteLine();
		this.writer.WriteLine(langSeparator);
		this.WriteParagraphs(record.paragraphs);

		this.writer.Flush();
	}

	public void WriteParagraph(List<string> paragraph)
	{
		this.writer.WriteLine();
		foreach (var line in paragraph)
		{
			this.writer.WriteLine(line);
		}

		this.writer.Flush();
	}

	private void WriteParagraphs(List<List<string>> paragraphs)
	{
		foreach (var paragraph in paragraphs)
		{
			this.writer.WriteLine();
			foreach (var line in paragraph)
			{
				this.writer.WriteLine(line);
			}
		}
	}

	// IDisposable
	private bool disposed = false;

	public void Dispose()
	{
		Dispose(true);
		GC.SuppressFinalize(this);
	}

	protected virtual void Dispose(bool disposing)
	{
		if (this.disposed)
		{
			return;
		}

		if (disposing)
		{
			this.writer.Dispose();
		}

		this.disposed = true;
	}

	~CnagelogWriter()
	{
		Dispose(false);
	}
}