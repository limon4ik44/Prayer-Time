<div align="center">

# 🌙 PrayerBar

**Prayer times right on your Windows taskbar** - 
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
- **Auto-update**.
- All settings are stored in the registry: `HKEY_CURRENT_USER\Software\PrayerBar`.

### Install

1. Download `PrayerBar.exe` from the [latest release](../../releases/latest).
2. Put it in any folder you own (for example `Documents\PrayerBar`) and run it.

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
| GitHub | Update check | A standard request to the public Releases API |

Choose a city manually in the settings if you do not want automatic location by IP.
> [!CAUTION]
> **Important!** The program is completely open source. Make in C++. There are no viruses in it.

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
- **Лёгкий**: один небольшой `.exe`, без установщика и дополнительных библиотек, маленькое использование памяти.
- **Автообновление**.
- Все настройки хранятся в реестре: `HKEY_CURRENT_USER\Software\PrayerBar`.

### Установка

1. Скачай `PrayerBar.exe` из [последнего релиза](../../releases/latest).
2. Положи его в любую свою папку (например, `Документы\PrayerBar`) и запусти.

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
| GitHub | Проверка обновлений | Обычный запрос к публичному API релизов |

Если не хочешь определять город по IP, выбери его вручную в настройках.
> [!CAUTION]
> **Важно!** Программа полностью с открытым исходным кодом. Сделана на C++. Никаких вирусов здесь нет.
