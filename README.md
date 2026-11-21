<div align="center">

# 🌙 PrayerBar

**Prayer times right on your Windows taskbar**
**Время намазов прямо на панели задач Windows**

[English](#english) · [Русский](#русский)

</div>

---

## English

A tiny, lightweight widget that lives on the Windows taskbar and always shows how long is left until the next prayer.

```
 02:45        <- time until the Adhan
 Asr 15:12    <- prayer and its time
```

### Features

- **Taskbar widget**: countdown to the next Adhan on top, the prayer name and time below. When the Adhan arrives, it switches to a countdown to the **Iqama**, then goes back to the next prayer.
- **Location**: detected automatically, or choose any city by name.
- **Prayer times** for the chosen place, with Asr calculation (Standard / Hanafi).
- **Manual adjustment** of every prayer time (±60 minutes) and of the Iqama delay for each prayer.
- **Jumu'ah**: on Fridays Dhuhr is shown as Jumu'ah.
- **Ramadan mode**: Iftar and end of Suhoor, plus the Ramadan day in the menu.
- **Notifications**: native Windows notifications at prayer time or 5 / 10 / 15 / 30 minutes before. A test button is included.
- **Languages**: English, العربية (RTL), Русский.
- **Look**: font size, color, opacity, alignment and distance from the tray, with a live preview.
- **Windows 10 and Windows 11** styles: flat on Windows 10, rounded on Windows 11. Follows the system light/dark theme and accent color.
- **Hides in fullscreen** (games, video) together with the taskbar, and stays visible across virtual desktops.
- **Light**: one small `.exe`, no installer, no extra runtime, low memory use.
- **Auto-update** from GitHub Releases (optional).
- All settings are stored in the registry: `HKEY_CURRENT_USER\Software\PrayerBar`.

### Install

1. Download `PrayerBar.exe` from the [latest release](../../releases/latest).
2. Put it in any folder you own (for example `Documents\PrayerBar`) and run it.
3. Click the widget → **Settings** to choose language, city, notifications and more.
4. Enable **Start with Windows** in the settings if you want it to start automatically.

> Windows SmartScreen may warn about an unsigned app. Click **More info → Run anyway**.
> Keep the file in a folder where you can write, otherwise auto-update cannot replace it.

### Usage

| Action | What happens |
| --- | --- |
| Click the widget (or tray icon) | Opens the menu with today's prayers, update options, **Settings**, **Refresh**, **Exit** |
| Settings → **General** | Language, location, Asr method, notifications, autostart |
| Settings → **Times** | Shift any prayer by ±60 minutes |
| Settings → **Iqama** | Minutes from Adhan to Iqama for each prayer |
| Settings → **Look** | Font, color, opacity, alignment, distance from the tray |

### Privacy and data sources

PrayerBar has no account, no analytics and no ads. It talks to these services only:

| Service | Used for | What is sent |
| --- | --- | --- |
| [ip-api.com](https://ip-api.com) | Automatic location | Your IP address (the service sees it, as any website does) |
| [open-meteo.com](https://open-meteo.com) | City search and time zone | The city name or coordinates |
| [aladhan.com](https://aladhan.com) | Prayer times and Hijri date | Coordinates and date |
| GitHub | Update check (optional) | A standard request to the public Releases API |

Choose a city manually in the settings if you do not want automatic location by IP.

### Updates

The app checks GitHub Releases at start and once a day. It downloads **only** `PrayerBar.exe` from this repository over HTTPS, verifies its SHA-256 checksum, replaces itself and restarts. Nothing else is downloaded or installed. Turn it off in the widget menu (**Auto-update**).

### Build from source

Requires a C++17 compiler for Windows (MinGW-w64 or MSVC).

```bash
# 1) compile the icon resource (MinGW)
windres res/app.rc -O coff -o res/app.res

# 2) build
g++ PrayerBar.cpp res/app.res -o PrayerBar.exe -std=c++17 -Os -s -mwindows -static \
    -lwinhttp -lgdi32 -luser32 -lshell32 -ladvapi32 -lbcrypt
```

MSVC: `cl /utf-8 /EHsc /O1 PrayerBar.cpp /link /SUBSYSTEM:WINDOWS`

### Publishing a release (for maintainers)

1. Raise `APP_VERSION` in `PrayerBar.cpp` and build.
2. Create a GitHub release with a tag like `v1.1.0`.
3. Attach the file named exactly **`PrayerBar.exe`**.

Set your repository once in `UPD_REPO` (format `owner/name`) or in the registry value `UpdateRepo`.

### Notes

- Prayer times come from an online calculation service, so small differences from your local mosque are normal. Use **Settings → Times** and **Iqama** to match them.
- Not affiliated with any religious authority. Please verify times with your local mosque if in doubt.

---

## Русский

Маленький лёгкий виджет на панели задач Windows, который всегда показывает, сколько осталось до ближайшего намаза.

```
 02:45        <- время до азана
 Аср 15:12    <- намаз и его время
```

### Возможности

- **Виджет на панели задач**: сверху отсчёт до азана, снизу название намаза и его время. Когда азан наступил, показывается отсчёт до **икамы**, после неё виджет возвращается к следующему намазу.
- **Местоположение**: определяется автоматически или выбирается любой город по названию.
- **Время намазов** для выбранного места, расчёт Аср (стандартный / ханафитский).
- **Ручная коррекция** времени каждого намаза (±60 минут) и задержки до икамы для каждого намаза.
- **Джума**: по пятницам Зухр отображается как Джума.
- **Режим Рамадана**: ифтар и конец сухура, день Рамадана в меню.
- **Уведомления**: системные уведомления Windows в момент намаза или за 5 / 10 / 15 / 30 минут. Есть кнопка проверки.
- **Языки**: English, العربية (справа налево), Русский.
- **Вид**: размер шрифта, цвет, прозрачность, выравнивание и отступ от трея с живым предпросмотром.
- **Стили Windows 10 и Windows 11**: плоский для Windows 10, скруглённый для Windows 11. Учитывает светлую и тёмную тему и акцентный цвет системы.
- **Скрывается в полноэкранном режиме** (игры, видео) вместе с панелью задач и не пропадает при смене рабочих столов.
- **Лёгкий**: один небольшой `.exe`, без установщика и дополнительных библиотек, мало памяти.
- **Автообновление** из GitHub Releases (по желанию).
- Все настройки хранятся в реестре: `HKEY_CURRENT_USER\Software\PrayerBar`.

### Установка

1. Скачай `PrayerBar.exe` из [последнего релиза](../../releases/latest).
2. Положи его в любую свою папку (например, `Документы\PrayerBar`) и запусти.
3. Нажми на виджет, затем **Настройки**: язык, город, уведомления и остальное.
4. Включи **Запускать вместе с Windows**, если хочешь автозапуск.

> SmartScreen может предупредить о неподписанной программе. Нажми **Подробнее → Выполнить в любом случае**.
> Держи файл в папке, куда можно писать, иначе автообновление не сможет его заменить.

### Использование

| Действие | Что происходит |
| --- | --- |
| Клик по виджету (или иконке в трее) | Меню: намазы на сегодня, обновления, **Настройки**, **Обновить**, **Выход** |
| Настройки → **Общее** | Язык, местоположение, расчёт Аср, уведомления, автозапуск |
| Настройки → **Время** | Сдвиг любого намаза на ±60 минут |
| Настройки → **Икама** | Минуты от азана до икамы для каждого намаза |
| Настройки → **Вид** | Шрифт, цвет, прозрачность, выравнивание, отступ от трея |

### Конфиденциальность и источники данных

В PrayerBar нет аккаунтов, аналитики и рекламы. Программа обращается только к этим сервисам:

| Сервис | Для чего | Что отправляется |
| --- | --- | --- |
| [ip-api.com](https://ip-api.com) | Автоопределение города | Твой IP-адрес (сервис видит его, как любой сайт) |
| [open-meteo.com](https://open-meteo.com) | Поиск города и часовой пояс | Название города или координаты |
| [aladhan.com](https://aladhan.com) | Время намазов и дата по хиджре | Координаты и дата |
| GitHub | Проверка обновлений (по желанию) | Обычный запрос к публичному API релизов |

Если не хочешь определять город по IP, выбери его вручную в настройках.

### Обновления

Программа проверяет GitHub Releases при запуске и раз в сутки. Она скачивает **только** `PrayerBar.exe` из этого репозитория по HTTPS, проверяет контрольную сумму SHA-256, заменяет себя и перезапускается. Больше ничего не скачивается и не устанавливается. Отключить можно в меню виджета (**Автообновление**).

### Сборка из исходников

Нужен компилятор C++17 для Windows (MinGW-w64 или MSVC).

```bash
# 1) ресурс с иконкой (MinGW)
windres res/app.rc -O coff -o res/app.res

# 2) сборка
g++ PrayerBar.cpp res/app.res -o PrayerBar.exe -std=c++17 -Os -s -mwindows -static \
    -lwinhttp -lgdi32 -luser32 -lshell32 -ladvapi32 -lbcrypt
```

MSVC: `cl /utf-8 /EHsc /O1 PrayerBar.cpp /link /SUBSYSTEM:WINDOWS`

### Как выпустить релиз (для автора)

1. Увеличь `APP_VERSION` в `PrayerBar.cpp` и собери программу.
2. Создай релиз на GitHub с тегом вида `v1.1.0`.
3. Приложи файл с именем строго **`PrayerBar.exe`**.

Репозиторий один раз задаётся в `UPD_REPO` (формат `логин/название`) или в реестре, значение `UpdateRepo`.

### Примечания

- Время намазов считает онлайн-сервис, поэтому небольшие отличия от твоей мечети нормальны. Подстрой их в **Настройки → Время** и **Икама**.
- Проект не связан ни с какой религиозной организацией. Если есть сомнения, сверяйся с расписанием своей мечети.
