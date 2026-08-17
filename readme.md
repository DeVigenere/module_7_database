Система обработки сообщений с сохранением в БД



Обоснование выбора СУБД

Выбрана SQLite по следующим причинам:



1.Встраиваемость - не требуется отдельный сервер, БД работает внутри приложения

2.Простота развертывания - один файл, нет необходимости в настройке

3.Легковесность - минимальное потребление ресурсов



Схема базы данных:



Поле Тип Описание

id | INTEGER PRIMARY KEY | Уникальный идентификатор (суррогатный ключ)

source\_service | TEXT | Источник сообщения (calculator)

timestamp\_utc | DATETIME | Время отправки (из контракта)

payload | TEXT | Полезные данные

received\_at | DATETIME | Время приёма сервером (автоматически)

status | TEXT | Статус обработки ('received', 'processed')

schema\_version | INTEGER | Версия схемы (для совместимости)

processed | BOOLEAN | Флаг обработки



Индексы:

idx\_source\_service - для фильтрации по источнику

idx\_timestamp - для сортировки по времени

idx\_status - для выборки по статусу



Установка и запуск:

Требования

Windows

CMake

SQLite 3



Сборка, запуск, проверка:

1. cmd в папке проекта
2. mkdir build \&\& cd build
3. cmake ..
4. cmake --build . --config Release
5. cd Release

6\. открываем новый cmd
7. cd \*путь, где к Release\*
8. на любом cmd сначала запускаем receiver.exe
9. calculator\_sender.exe
10. после завершения в папке Release появится message.db

11\. можем открыть в SQLite гитхаб проекта: https://inloop.github.io/sqlite-viewer/

