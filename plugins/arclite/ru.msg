.Language=Russian,Russian (Русский)

lang = ru
plugin.name = <(NAME)>

# Extract dialog
extract.dlg.title = Распаковка архива
extract.dlg.dst_dir = &Извлечь файлы в:
extract.dlg.ignore_errors = &Пропускать файлы с ошибками
extract.dlg.oa = Если файл существует:
extract.dlg.oa_ask = &Спросить
extract.dlg.oa_overwrite = П&ерезаписать
extract.dlg.oa_skip = П&ропустить
extract.dlg.oa_rename = Переи&меновать
extract.dlg.oa_append = До&бавить
extract.dlg.move_files = &Удалить файлы из архива после распаковки
extract.dlg.password = П&ароль:
extract.dlg.separate_dir = Распаковать архив в &отдельный каталог
extract.dlg.delete_archive = У&далить архив после распаковки
extract.dlg.open_dir = Пере&йти в целевой каталог после распаковки
extract.dlg.save_params = Со&хранить настройки
extract.dlg.params_saved = Настройки сохранены

# Update dialog
update.dlg.title = Обновление архива
update.dlg.title.create = Создание архива
update.dlg.profile = &Профиль:
update.dlg.save_profile = Сохранить
update.dlg.delete_profile = Удалить
update.dlg.arc_path = Пу&ть:
update.dlg.arc_path_eval = Пок&азать
update.dlg.append_ext = &Добавить расширение
update.dlg.arc_type = Формат архива:
update.dlg.arc_type.other = П&рочие:
update.dlg.level = &Уровень сжатия:
update.dlg.method = &Метод:
update.dlg.solid = &Непрерывный
update.dlg.advanced = Прод&винутые параметры:
update.dlg.encrypt = &Шифрование файлов
update.dlg.show_password = Показать пароль
update.dlg.password = Паро&ль:
update.dlg.password2 = Проверка пароля:
update.dlg.encrypt_header = Шифрование оглавления
update.dlg.create_sfx = &Создать самоизвлекающийся архив
update.dlg.sfx_options = Параметры SFX
update.dlg.enable_volumes = Порезать на куск&и
update.dlg.volume_size = Размер куска:
update.dlg.move_files = Удалит&ь файлы после архивации
update.dlg.open_shared = Архивировать файлы, &открытые для записи
update.dlg.ignore_errors = Пропус&кать файлы с ошибками
update.dlg.oa = Если файл существует:
update.dlg.oa_ask = &Спросить
update.dlg.oa_overwrite = &Перезаписать
update.dlg.oa_skip = П&ропустить
update.dlg.enable_filter = Использовать &фильтр
update.dlg.wrong_arc_type = Необходимо выбрать формат архива
update.dlg.wrong_level = Необходимо выбрать уровень сжатия
update.dlg.wrong_method = Необходимо выбрать метод сжатия
update.dlg.passwords_dont_match = Введенные пароли не совпадают
update.dlg.password_is_empty = Необходимо указать пароль
update.dlg.wrong_volume_size = Неверно указан размер куска
update.dlg.confirm_overwrite = Целевой архив уже существует. Хотите перезаписать его?
update.dlg.input_profile_name = Введите название профиля
update.dlg.confirm_profile_delete = Хотите удалить текущий профиль?
update.dlg.sfx_no_ppmd = Выбранный SFX модуль не поддерживает сжатие методом PPMD
update.dlg.sfx_no_encrypt = Выбранный SFX модуль не поддерживает шифрование данных
update.dlg.save_params = Сохранить настройки
update.dlg.params_saved = Настройки сохранены

# Archive types
compression.archive.7z = &7z
compression.archive.zip = &zip

# Compression levels
compression.level.store = Store
compression.level.fastest = Fastest
compression.level.fast = Fast
compression.level.normal = Normal
compression.level.maximum = Maximum
compression.level.ultra = Ultra

# Compression methods
compression.method.lzma = LZMA
compression.method.lzma2 = LZMA2
compression.method.ppmd = PPMD

# Delete files
delete.dlg.confirm = Вы хотите удалить выбранные файлы?

# Create directory
create_dir.dlg.name = Введите имя каталога:

# Test archive
test.ok = Тест архива завершен успешно
test.menu = Протестировать архив

# Password dialog
password.title = Ввод пароля
password.password = Пароль:

# Overwrite dialog
overwrite.dlg.title = Перезапись файла
overwrite.dlg.question = Целевой файл уже существует. Заменить?
overwrite.dlg.source = Источник:
overwrite.dlg.destination = Цель:
overwrite.dlg.newer = (более новый)
overwrite.dlg.all = &Запомнить выбор
overwrite.dlg.overwrite = П&ерезаписать
overwrite.dlg.skip = П&ропустить
overwrite.dlg.rename = Переи&меновать
overwrite.dlg.append = До&бавить

# Error retry dialog
button.retry = &Повтор
button.ignore = &Игнорировать
button.ignore_all = Игнорировать &все

# Log dialog
log.title = Список ошибок
log.info = Операция завершена с ошибками. Хотите просмотреть список?
log.close = &Закрыть
log.show = &Показать список

# Settings dialog
settings.dlg.handle_create = Обрабатывать команду создания архива (Shift-F&1)
settings.dlg.handle_commands = Обрабатывать прочие архивные команды (Ctrl-PgDn, Shift-F&2)
settings.dlg.own_panel_view_mode = Использовать собственный ре&жим работы панели
settings.dlg.use_include_masks = Маски &включения:
settings.dlg.use_exclude_masks = Маски &исключения:
settings.dlg.pgdn_masks = Н&астройки масок влияют на Ctrl-PgDn
settings.dlg.generate_masks = &Сгенерировать маски
settings.dlg.default_masks = С&тандартные маски
settings.dlg.use_enabled_formats = В&ключить форматы архивов:
settings.dlg.use_disabled_formats = &Отключить форматы архивов:
settings.dlg.pgdn_formats = &Настройки форматов влияют на Ctrl-PgDn
settings.dlg.lib_info = Информация о библиотеке &7z
settings.dlg.lib_not_found = Библиотека 7z не найдена
settings.dlg.lib_version = Версия:
settings.dlg.lib_formats = Форматы
settings.dlg.lib_masks = Маски
settings.dlg.active = Активно
settings.dlg.edit = Редактировать

# Attributes dialog
attr.dlg.title = Атрибуты

# SFX options dialog
sfx_options.dlg.title = Параметры SFX
sfx_options.dlg.profile = Использовать про&филь:
sfx_options.dlg.module = &Модуль:
sfx_options.dlg.replace_icon = Заменить &иконку
sfx_options.dlg.icon_path = &Путь к ICO файлу:
sfx_options.dlg.replace_version = Заменить информацию о &версии
sfx_options.dlg.ver_info.version = В&ерсия:
sfx_options.dlg.ver_info.comments = &Комментарии:
sfx_options.dlg.ver_info.company_name = На&звание компании:
sfx_options.dlg.ver_info.file_description = &Описание файла:
sfx_options.dlg.ver_info.legal_copyright = Копи&райт:
sfx_options.dlg.ver_info.product_name = Название про&дукта:
sfx_options.dlg.append_install_config = Параметры и&нсталлятора
sfx_options.dlg.install_config.title = З&аголовок:
sfx_options.dlg.install_config.begin_prompt = Вопро&с:
sfx_options.dlg.install_config.progress = Индика&тор распаковки:
sfx_options.dlg.install_config.run_program = Зап&устить программу из архива:
sfx_options.dlg.install_config.directory = Ката&лог с программой:
sfx_options.dlg.install_config.execute_file = Запустит&ь произвольный файл:
sfx_options.dlg.install_config.execute_parameters = Параметр&ы запуска:
sfx_options.dlg.wrong_module = Необходимо указать SFX модуль
sfx_options.dlg.invalid_icon_path = ICO файл не найден

# SFX module description
sfx_descr.7z = Стандартный
sfx_descr.7zcon = Стандартный консольный
sfx_descr.7zs = Инсталлятор (статический)
sfx_descr.7zs2 = Инсталлятор упрощенный
sfx_descr.7zs2con = Инсталлятор упрощенный консольный
sfx_descr.7zsd = Инсталлятор
sfx_descr.unknown = Неизвестный

# Plugin menu
menu.open = &Открыть архив
menu.detect = О&ткрыть (список форматов)
menu.create = &Создать архив
menu.extract = &Распаковать архив(ы)
menu.test = &Проверить архив(ы)
menu.sfx_convert = Прео&бразовать архив в SFX

# Progress messages
progress.open = Чтение архива...
progress.extract = Извлечение файлов...
progress.create_dirs = Создание каталогов...
progress.set_attr = Установка атрибутов...
progress.test = Тестирование архива...
progress.update = Обновление архива...
progress.create = Создание архива...
progress.delete_files = Удаление файлов...
progress.scan_dirs = Сканирование каталогов...
progress.interrupt = Хотите прервать текущую операцию?
progress.paused = Пауза
progress.low_priority = Фон
progress.sfx_convert = Преобразование в SFX...

# Error messages
error.extract.unsupported_method = Неподдерживаемый метод сжатия
error.extract.data_error = Ошибка в данных
error.extract.data_error.encrypted = Неправильный пароль либо ошибка в данных
error.extract.crc_error = Ошибка при проверке контрольной суммы
error.extract.crc_error.encrypted = Неправильный пароль либо ошибка при проверке контрольной суммы
error.extract.unknown = Неизвестная ошибка распаковки
error.update.error = Ошибка обновления
error.update.unknown = Неизвестная ошибка обновления
error.no_format = Формат архива не найден
error.not_updatable = Изменение архива невозможно
error.not_archive = Файл не распознан как архив
error.sfx_convert = Архив нельзя преобразовать в SFX архив

# Generic buttons
button.ok = OK
button.cancel = Отмена

# Size and speed suffixes
suffix.size.b = Б
suffix.size.kb = КБ
suffix.size.mb = МБ
suffix.size.gb = ГБ
suffix.size.tb = ТБ
suffix.speed.b = Б/с
suffix.speed.kb = КБ/с
suffix.speed.mb = МБ/с
suffix.speed.gb = ГБ/с
suffix.speed.tb = ТБ/с

# Property names
kpid.Path = Путь
kpid.Name = Имя
kpid.Extension = Расширение
kpid.IsDir = Папка
kpid.Size = Размер
kpid.PackSize = Сжатый
kpid.Attrib = Атрибуты
kpid.CTime = Создан
kpid.ATime = Открыт
kpid.MTime = Изменен
kpid.Solid = Непрерывный
kpid.Commented = Комментарий
kpid.Encrypted = Зашифрован
kpid.SplitBefore = Разбит До
kpid.SplitAfter = Разбит После
kpid.DictionarySize = Словарь
kpid.CRC = CRC
kpid.Type = Тип
kpid.IsAnti = Анти
kpid.Method = Метод
kpid.HostOS = Система
kpid.FileSystem = Файловая Система
kpid.User = Пользователь
kpid.Group = Группа
kpid.Block = Блок
kpid.Comment = Комментарий
kpid.Position = Позиция
kpid.Prefix = Префикс
kpid.NumSubDirs = Папок
kpid.NumSubFiles = Файлов
kpid.UnpackVer = Версия
kpid.Volume = Том
kpid.IsVolume = Многотомный
kpid.Offset = Смещение
kpid.Links = Ссылок
kpid.NumBlocks = Блоков
kpid.NumVolumes = Томов
kpid.TimeType = Тип Времени
kpid.Bit64 = 64-bit
kpid.BigEndian = Big-endian
kpid.Cpu = Процессор
kpid.PhySize = Физический Размер
kpid.HeadersSize = Размер Заголовков
kpid.Checksum = Контр. Сумма
kpid.Characts = Характеристики
kpid.Va = Виртуальный Адрес
kpid.Id = ID
kpid.ShortName = Короткое имя
kpid.CreatorApp = Программа
kpid.SectorSize = Размер сектора
kpid.PosixAttrib = Права
kpid.Link = Ссылка
kpid.TotalSize = Емкость
kpid.FreeSpace = Свободно
kpid.ClusterSize = Размер кластера
kpid.VolumeName = Метка
kpid.LocalName = Локальное Имя
kpid.Provider = Провайдер

# Property values
property.true = Да
property.false = Нет
property.compression_ratio = Степень сжатия
property.total_size = Размер данных
property.total_packed_size = Упакованный размер
property.file_count = Число файлов
property.dir_count = Число каталогов
