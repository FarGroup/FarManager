|[English](README.md)|[Polski](README-PL.md)|
|-|-|

[![Header][logo-img]][logo-url]

### Far Manager
| | AppVeyor | Azure |
|-|-|-|
| VS | [![AppVeyor][VS-AppVeyor-img]][VS-AppVeyor-url] | [![Azure Pipelines][VS-Azure-img]][VS-Azure-url] |
| GCC | [![AppVeyor][GCC-AppVeyor-img]][GCC-AppVeyor-url] | TBD |
| Clang | [![AppVeyor][Clang-AppVeyor-img]][Clang-AppVeyor-url] | TBD |


### Far Manager — что это?
Far Manager — консольный файловый менеджер для операционных систем семейства Windows. Программа предоставляет удобный интерфейс пользователя для работы с файловыми системами (реальными и эмулированными) и файлами:
* просматривать файлы и каталоги;
* редактировать, копировать и переименовывать файлы;
* и многое другое.

### Такой, каким вы хотите его видеть
Far Manager имеет многоязычный, легко настраиваемый интерфейс. Простую навигацию по файловой системе обеспечивают цветовое выделение и группы сортировки файлов.

### Под ваши задачи
Функциональность Far Manager существенно расширяется за счет внешних подключаемых DLL-модулей — плагинов (этому способствует набор специальных интерфейсов — [Plugins API](https://api.farmanager.com/)). Например, работа с архивами, FTP-клиент, временная панель и просмотр сети реализованы с помощью плагинов, включенных в стандартную поставку Far Manager.


#### Форум поддержки
https://enforum.farmanager.com/<br/>
https://forum.farmanager.com/

#### Баг трекер
https://bugs.farmanager.com/

#### Рассылка для разработчиков (англоязычная)
https://groups.google.com/group/fardeven<br/>
<fardeven@googlegroups.com>

#### Рассылка для разработчиков (русскоязычная)
https://groups.google.com/group/fardev<br/>
<fardev@googlegroups.com>

#### Рассылка коммитов
https://groups.google.com/group/farcommits<br/>
<farcommits@googlegroups.com>

#### Исходный код
https://github.com/FarGroup/FarManager

[logo-img]: ./logo.svg
[logo-url]: https://www.farmanager.com
[VS-AppVeyor-img]: https://ci.appveyor.com/api/projects/status/6pca73evwo3oxvr9?svg=true
[VS-AppVeyor-url]: https://ci.appveyor.com/project/FarGroup/farmanager/history
[GCC-AppVeyor-img]: https://ci.appveyor.com/api/projects/status/k7ln3edp8nt5aoay?svg=true
[GCC-AppVeyor-url]: https://ci.appveyor.com/project/FarGroup/farmanager-5lhsj/history
[Clang-AppVeyor-img]: https://ci.appveyor.com/api/projects/status/pvwnc6gc5tjlpmti?svg=true
[Clang-AppVeyor-url]: https://ci.appveyor.com/project/FarGroup/farmanager-tgu1s/history
[VS-Azure-img]: https://img.shields.io/azure-devops/build/FarGroup/66d0ddcf-a098-4b98-9470-1c90632c4ba3/1.svg?logo=azuredevops
[VS-Azure-url]: https://dev.azure.com/FarGroup/FarManager/_build?definitionId=1
