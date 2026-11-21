// PrayerBar: время намазов на панели задач Windows 10
// Языки: English / العربية / Русский. Город: авто (по IP) или поиск вручную.
// Уведомления, автозапуск, расчёт Аср. Лёгкий: фоновый режим, сброс памяти, без лишних библиотек.
//
// Иконка лежит в res/app.rc (+ app.ico); для MinGW: windres res/app.rc -O coff -o res/app.res
// Сборка (MinGW):
//   g++ PrayerBar.cpp -o PrayerBar.exe -Os -s -mwindows -static -lwinhttp -lgdi32 -luser32 -lshell32 -ladvapi32 -lbcrypt
// Сборка (Visual Studio, Developer Command Prompt):
//   cl /utf-8 /EHsc /O1 PrayerBar.cpp /link /SUBSYSTEM:WINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <bcrypt.h>
#include <cstdint>
#include <ctime>
using namespace std;

#ifdef _MSC_VER
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "bcrypt.lib")
#endif

#ifndef PROCESS_MODE_BACKGROUND_BEGIN
#define PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
#endif

// ---------------------------------------------------------------- языки
enum Str {
    S_SETTINGS, S_LANGUAGE, S_LOCATION, S_AUTO, S_MANUAL, S_FIND, S_FOUND, S_NOTFOUND,
    S_SEARCHING, S_ASR, S_STD, S_HANAFI, S_NOTIF, S_NOTIFON, S_AT, S_LEAD, S_REMIND, S_AUTORUN,
    S_SAVE, S_REFRESH, S_EXIT, S_OFFLINE, S_RETRY, S_LOADING, S_NOTE_NOW, S_NOTE_BEFORE, S_HM, S_TAB_MAIN, S_TAB_TIMES, S_ADJ, S_ADJ_HINT, S_RESET, S_ON, S_OFF, S_TOAST_NOW, S_TOAST_SOON, S_TEST, S_RAMADAN, S_TOAST_JUMA, S_TOAST_IFTAR, S_TOAST_FAST, S_TAB_IQAMA, S_IQ_ON, S_IQ_HINT, S_IQAMA, S_TAB_LOOK, S_W_FONT, S_W_OPAC, S_W_POS, S_W_ALIGN, S_W_LEFT, S_W_CENTER, S_W_RIGHT, S_W_COLOR, S_UPD_CHECK, S_UPD_AUTO, S_UPD_TO, S_UPD_AVT, S_UPD_AVB, S_UPD_LATEST, S_UPD_FAIL, S_UPD_OK, S_COUNT
};

static const wchar_t* STR[3][S_COUNT] = {
    {L"Settings", L"Language", L"Location", L"Auto", L"Manual", L"Find",
     L"Found: %ls", L"City not found", L"Searching…", L"Asr calculation",
     L"Standard", L"Hanafi", L"Notifications", L"Notify me about prayers",
     L"On time", L"%d min", L"Remind before", L"Start with Windows", L"Save", L"Refresh", L"Exit",
     L"No connection", L"Retrying…", L"Loading…", L"%ls — time to pray", L"%ls in %d min", L"%dh %02dm",
     L"General", L"Times", L"Adjust times manually", L"Shift each prayer by some minutes", L"Reset", L"On", L"Off", L"Time to pray", L"In %d min",
     L"Test", L"Ramadan, day %d", L"Friday prayer", L"Time to break the fast", L"Fasting begins",
     L"Iqama", L"Show countdown to iqama", L"Minutes from adhan to iqama", L"Iqama",
     L"Look", L"Font size", L"Opacity", L"Distance from tray", L"Text alignment", L"Left", L"Center", L"Right", L"Text color",
     L"Check for updates", L"Auto-update", L"Update to %ls", L"Update available", L"Version %ls is available", L"You have the latest version", L"Update failed", L"Updated to version %ls"},
    {L"الإعدادات", L"اللغة", L"الموقع", L"تلقائي", L"يدوي", L"بحث",
     L"تم العثور: %ls", L"لم يتم العثور على المدينة", L"جارٍ البحث…", L"حساب العصر",
     L"الجمهور", L"الحنفي", L"الإشعارات", L"تنبيهي بأوقات الصلاة",
     L"في الوقت", L"%d د", L"التذكير قبل", L"التشغيل مع ويندوز", L"حفظ", L"تحديث", L"خروج",
     L"لا يوجد اتصال", L"إعادة المحاولة…", L"جارٍ التحميل…", L"حان وقت صلاة %ls", L"%ls بعد %d دقيقة", L"%d س %02d د",
     L"عام", L"الأوقات", L"تعديل الأوقات يدوياً", L"إزاحة كل صلاة بعدد من الدقائق", L"إعادة ضبط", L"تشغيل", L"إيقاف", L"حان وقت الصلاة", L"بعد %d دقيقة",
     L"اختبار", L"رمضان، اليوم %d", L"صلاة الجمعة", L"حان وقت الإفطار", L"بدء الصيام",
     L"الإقامة", L"عدّاد الإقامة", L"الدقائق بين الأذان والإقامة", L"الإقامة",
     L"المظهر", L"حجم الخط", L"الشفافية", L"المسافة من الأيقونات", L"محاذاة النص", L"يسار", L"وسط", L"يمين", L"لون النص",
     L"التحقق من التحديثات", L"التحديث التلقائي", L"التحديث إلى %ls", L"تحديث متاح", L"الإصدار %ls متاح", L"لديك أحدث إصدار", L"فشل التحديث", L"تم التحديث إلى الإصدار %ls"},
    {L"Настройки", L"Язык", L"Город", L"Авто", L"Вручную", L"Найти",
     L"Найдено: %ls", L"Город не найден", L"Поиск…", L"Расчёт Аср",
     L"Стандарт", L"Ханафитский", L"Уведомления", L"Уведомлять о намазах",
     L"Вовремя", L"%d мин", L"Напомнить за", L"Запускать вместе с Windows", L"Сохранить", L"Обновить", L"Выход",
     L"Нет сети", L"Пробую снова…", L"Загрузка…", L"%ls — время намаза", L"%ls через %d мин", L"%dч %02dм",
     L"Общее", L"Время", L"Ручная коррекция времени", L"Сдвиг каждого намаза в минутах", L"Сбросить", L"Вкл.", L"Выкл.", L"Время намаза", L"Через %d мин",
     L"Проверить", L"Рамадан, день %d", L"Пятничный намаз", L"Время разговения", L"Начало поста",
     L"Икама", L"Показывать отсчёт до икамы", L"Минут от азана до икамы", L"Икама",
     L"Вид", L"Размер шрифта", L"Прозрачность", L"Отступ от трея", L"Выравнивание", L"Слева", L"По центру", L"Справа", L"Цвет текста",
     L"Проверить обновления", L"Автообновление", L"Обновить до %ls", L"Доступно обновление", L"Вышла версия %ls", L"У вас последняя версия", L"Не удалось обновить", L"Обновлено до версии %ls"}};

static const wchar_t* PN[3][6] = {
    {L"Fajr", L"Sunrise", L"Dhuhr", L"Asr", L"Maghrib", L"Isha"},
    {L"الفجر", L"الشروق", L"الظهر", L"العصر", L"المغرب", L"العشاء"},
    {L"Фаджр", L"Восход", L"Зухр", L"Аср", L"Магриб", L"Иша"}};
static const char* PKEY[6] = {"Fajr", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"};
static const wchar_t* APP_VERSION = L"1.0.0";  // версия программы (теги релизов на GitHub: v1.0.0, v1.1.0 ...)
static const int LEADS[5] = {0, 5, 10, 15, 30};
static const int IQI[5] = {0, 2, 3, 4, 5};  // намазы, у которых есть икама

// ---------------------------------------------------------------- данные
struct Cfg {
    int lang = 0;  // 0 en, 1 ar, 2 ru
    bool autoCity = true;
    wstring city, cityName;
    double lat = 0, lon = 0;
    int school = 0;
    bool notify = false;
    int leadIdx = 0;
    bool autorun = false;
    int wFont = 9, wOpacity = 100, wOffset = 0, wAlign = 1, wColor = 0;  // вид виджета
    bool iqOn = true;                   // показывать отсчёт до икамы
    int iq[6] = {25, 0, 20, 20, 10, 20};  // минут от азана до икамы
    int off[6] = {0, 0, 0, 0, 0, 0};  // ручная поправка к времени каждого намаза, минуты
};
struct Res {
    bool ok = false;
    wstring name;
    long off = 0;
    int sec[6] = {0, 0, 0, 0, 0, 0};
    long long day = 0;
    int hm = 0, hd = 0;  // месяц и день по хиджре (месяц 9 = Рамадан)
};
struct Job { HWND h; Cfg c; };

static Cfg g_cfg;
static Res g_res;
static wstring g_l1, g_l2;
static HWND g_main = 0, g_ctl = 0, g_set = 0;
static HINSTANCE g_hi = 0;
static bool g_loading = false, g_again = false;
static int g_tick = 0, g_dpi = 96;
static long long g_notified = -1;
static HFONT g_font = 0, g_ui = 0, g_bold = 0, g_hdr = 0, g_small = 0;
static NOTIFYICONDATAW g_nid;
static UINT g_taskbarMsg;
static HICON g_icon = 0, g_iconBig = 0, g_iconToast = 0;
static HWINEVENTHOOK g_hook = 0;

#define SC(x) MulDiv((x), g_dpi, 96)
#define T(i) STR[g_cfg.lang][i]
static bool RTL() { return g_cfg.lang == 1; }

// Цвета окна настроек: стиль Windows 10 (светлая/тёмная тема и акцентный цвет берутся из системы)
static COLORREF CBG, CROW, CCARD, CCARDH, CTXT, CDIM, CDIS, CACC, CACCH, CACCT, CERR, COFF, CFIELD, CBORD, CHEAD;
static bool g_dark = true, g_w11 = false;
static COLORREF Lighten(COLORREF c, int p) {
    return RGB(GetRValue(c) + (255 - GetRValue(c)) * p / 100, GetGValue(c) + (255 - GetGValue(c)) * p / 100,
               GetBValue(c) + (255 - GetBValue(c)) * p / 100);
}
static void DetectOS() {  // Windows 11 = сборка 22000 и выше
    typedef LONG(WINAPI * RtlGetVer)(OSVERSIONINFOW*);
    OSVERSIONINFOW vi = {};
    vi.dwOSVersionInfoSize = sizeof vi;
    HMODULE nt = GetModuleHandleW(L"ntdll.dll");
    RtlGetVer f = nt ? (RtlGetVer)GetProcAddress(nt, "RtlGetVersion") : 0;
    if (f && f(&vi) == 0) g_w11 = vi.dwMajorVersion >= 10 && vi.dwBuildNumber >= 22000;
}
static void ApplyTheme() {
    DWORD v = 1, sz = sizeof v;
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, 0, &v, &sz);
    g_dark = (v == 0);
    DWORD ac = 0;
    sz = sizeof ac;
    COLORREF acc = RGB(0, 120, 215);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, 0, &ac, &sz) == ERROR_SUCCESS)
        acc = RGB(ac & 255, (ac >> 8) & 255, (ac >> 16) & 255);
    CACC = acc;
    CACCH = Lighten(acc, 18);
    if (g_dark) {
        CBG = RGB(31, 31, 31); CROW = RGB(43, 43, 43); CCARD = RGB(51, 51, 51); CCARDH = RGB(76, 76, 76);
        CTXT = RGB(255, 255, 255); CDIM = RGB(170, 170, 170); CDIS = RGB(110, 110, 110);
        CACCT = Lighten(acc, 35); CERR = RGB(255, 153, 164); COFF = RGB(170, 170, 170);
        CFIELD = RGB(51, 51, 51); CBORD = RGB(122, 122, 122);
    } else {
        CBG = RGB(255, 255, 255); CROW = RGB(243, 243, 243); CCARD = RGB(230, 230, 230); CCARDH = RGB(204, 204, 204);
        CTXT = RGB(0, 0, 0); CDIM = RGB(102, 102, 102); CDIS = RGB(160, 160, 160);
        CACCT = acc; CERR = RGB(196, 43, 28); COFF = RGB(102, 102, 102);
        CFIELD = RGB(255, 255, 255); CBORD = RGB(122, 122, 122);
    }
    CHEAD = g_dark ? RGB(20, 20, 22) : RGB(243, 243, 243);
    if (g_w11) CFIELD = CCARD;
}

// ---------------------------------------------------------------- утилиты
static wstring ToW(const string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0);
    wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}
static string ToU(const wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), 0, 0, 0, 0);
    string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, 0, 0);
    return s;
}
static wstring UrlEnc(const wstring& w) {
    string u = ToU(w);
    wstring o;
    for (unsigned char c : u) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') o += (wchar_t)c;
        else { wchar_t b[8]; _snwprintf(b, 8, L"%%%02X", c); o += b; }
    }
    return o;
}
static string HttpGet(const wstring& host, const wstring& path, bool tls) {
    string out;
    HINTERNET s = WinHttpOpen(L"PrayerBar/3.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 0, 0, 0);
    if (!s) return out;
    WinHttpSetTimeouts(s, 5000, 5000, 5000, 5000);
    HINTERNET c = WinHttpConnect(s, host.c_str(), tls ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
    HINTERNET r = c ? WinHttpOpenRequest(c, L"GET", path.c_str(), 0, 0, 0, tls ? WINHTTP_FLAG_SECURE : 0) : 0;
    if (r && WinHttpSendRequest(r, 0, 0, 0, 0, 0, 0) && WinHttpReceiveResponse(r, 0)) {
        char buf[4096]; DWORD n;
        while (WinHttpReadData(r, buf, sizeof buf, &n) && n) out.append(buf, n);
    }
    if (r) WinHttpCloseHandle(r);
    if (c) WinHttpCloseHandle(c);
    WinHttpCloseHandle(s);
    return out;
}
static string StrVal(const string& j, const string& key) {
    string k = "\"" + key + "\":\"";
    size_t p = j.find(k);
    if (p == string::npos) return "";
    p += k.size();
    size_t e = j.find('"', p);
    return e == string::npos ? "" : j.substr(p, e - p);
}
static double NumVal(const string& j, const string& key) {
    size_t p = j.find("\"" + key + "\":");
    return p == string::npos ? 0 : atof(j.c_str() + p + key.size() + 3);
}
static long LocalOffset() {
    TIME_ZONE_INFORMATION z;
    DWORD r = GetTimeZoneInformation(&z);
    long bias = z.Bias + (r == TIME_ZONE_ID_DAYLIGHT ? z.DaylightBias : z.StandardBias);
    return -bias * 60;
}
static wstring Fmt(int s) {
    s = ((s % 86400) + 86400) % 86400;
    wchar_t b[16];
    _snwprintf(b, 16, L"%02d:%02d", s / 3600, (s / 60) % 60);
    return b;
}
static void TrimMem() { SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1); }
static int Clamp(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }
static HFONT MkFont(int pt, int weight, DWORD quality = CLEARTYPE_QUALITY) {
    return CreateFont(-MulDiv(pt, g_dpi, 72), 0, 0, 0, weight, 0, 0, 0, g_cfg.lang == 1 ? ARABIC_CHARSET : DEFAULT_CHARSET, 0, 0, quality, 0, L"Segoe UI");
}

// ---------------------------------------------------------------- настройки (ini в UTF-16)
static bool GetAutorun() {
    HKEY k; bool r = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &k) == ERROR_SUCCESS) {
        r = RegQueryValueExW(k, L"PrayerBar", 0, 0, 0, 0) == ERROR_SUCCESS;
        RegCloseKey(k);
    }
    return r;
}
static void SetAutorun(bool on) {
    HKEY k;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &k) != ERROR_SUCCESS) return;
    if (on) {
        wchar_t p[MAX_PATH + 3];
        DWORD n = GetModuleFileNameW(0, p + 1, MAX_PATH);
        p[0] = L'"'; p[n + 1] = L'"'; p[n + 2] = 0;
        RegSetValueExW(k, L"PrayerBar", 0, REG_SZ, (const BYTE*)p, (n + 3) * sizeof(wchar_t));
    } else RegDeleteValueW(k, L"PrayerBar");
    RegCloseKey(k);
}
static const wchar_t* RK = L"Software\\PrayerBar";
static wstring GetS(const wchar_t* k, const wchar_t* d) {
    wchar_t b[256];
    DWORD sz = sizeof b;
    if (RegGetValueW(HKEY_CURRENT_USER, RK, k, RRF_RT_REG_SZ, 0, b, &sz) == ERROR_SUCCESS) return b;
    return d;
}
static int GetI(const wchar_t* k, int d) {
    DWORD v = 0, sz = sizeof v;
    if (RegGetValueW(HKEY_CURRENT_USER, RK, k, RRF_RT_REG_DWORD, 0, &v, &sz) == ERROR_SUCCESS) return (int)v;
    return d;
}
static void PutS(const wchar_t* k, const wstring& v) {
    HKEY h;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, RK, 0, 0, 0, KEY_WRITE, 0, &h, 0) != ERROR_SUCCESS) return;
    RegSetValueExW(h, k, 0, REG_SZ, (const BYTE*)v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(h);
}
static void PutI(const wchar_t* k, int v) {
    HKEY h;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, RK, 0, 0, 0, KEY_WRITE, 0, &h, 0) != ERROR_SUCCESS) return;
    DWORD d = (DWORD)v;
    RegSetValueExW(h, k, 0, REG_DWORD, (const BYTE*)&d, sizeof d);
    RegCloseKey(h);
}
static void LoadCfg() {
    LANGID ll = GetUserDefaultUILanguage() & 0x3FF;
    int dl = (ll == LANG_RUSSIAN) ? 2 : (ll == LANG_ARABIC) ? 1 : 0;
    g_cfg.lang = Clamp(GetI(L"Language", dl), 0, 2);
    g_cfg.autoCity = GetI(L"AutoLocation", 1) != 0;
    g_cfg.city = GetS(L"City", L"");
    g_cfg.cityName = GetS(L"CityName", L"");
    g_cfg.lat = _wtof(GetS(L"Latitude", L"0").c_str());
    g_cfg.lon = _wtof(GetS(L"Longitude", L"0").c_str());
    g_cfg.school = Clamp(GetI(L"AsrSchool", 0), 0, 1);
    g_cfg.notify = GetI(L"Notifications", 0) != 0;
    g_cfg.leadIdx = Clamp(GetI(L"NotifyLead", 0), 0, 4);
    for (int i = 0; i < 6; i++) {
        wchar_t k[16];
        _snwprintf(k, 16, L"Offset%d", i);
        g_cfg.off[i] = Clamp(GetI(k, 0), -60, 60);
    }
    g_cfg.wFont = Clamp(GetI(L"WidgetFont", 9), 8, 13);
    g_cfg.wOpacity = Clamp(GetI(L"WidgetOpacity", 100), 30, 100);
    g_cfg.wOffset = Clamp(GetI(L"WidgetOffset", 0), 0, 800);
    g_cfg.wAlign = Clamp(GetI(L"WidgetAlign", 1), 0, 2);
    g_cfg.wColor = Clamp(GetI(L"WidgetColor", 0), 0, 6);
    g_cfg.iqOn = GetI(L"IqamaOn", 1) != 0;
    for (int i = 0; i < 6; i++) {
        wchar_t k[16];
        _snwprintf(k, 16, L"Iqama%d", i);
        g_cfg.iq[i] = Clamp(GetI(k, g_cfg.iq[i]), 0, 90);
    }
    g_cfg.autorun = GetAutorun();
}
static void SaveCfg() {
    PutI(L"Language", g_cfg.lang); PutI(L"AutoLocation", g_cfg.autoCity); PutS(L"City", g_cfg.city);
    PutS(L"CityName", g_cfg.cityName);
    wchar_t b[48];
    _snwprintf(b, 48, L"%.5f", g_cfg.lat); PutS(L"Latitude", b);
    _snwprintf(b, 48, L"%.5f", g_cfg.lon); PutS(L"Longitude", b);
    for (int i = 0; i < 6; i++) {
        wchar_t k[16];
        _snwprintf(k, 16, L"Offset%d", i);
        PutI(k, g_cfg.off[i]);
    }
    PutI(L"WidgetFont", g_cfg.wFont); PutI(L"WidgetOpacity", g_cfg.wOpacity); PutI(L"WidgetOffset", g_cfg.wOffset);
    PutI(L"WidgetAlign", g_cfg.wAlign); PutI(L"WidgetColor", g_cfg.wColor);
    PutI(L"IqamaOn", g_cfg.iqOn);
    for (int i = 0; i < 6; i++) {
        wchar_t k[16];
        _snwprintf(k, 16, L"Iqama%d", i);
        PutI(k, g_cfg.iq[i]);
    }
    PutI(L"AsrSchool", g_cfg.school); PutI(L"Notifications", g_cfg.notify); PutI(L"NotifyLead", g_cfg.leadIdx);
}

// ---------------------------------------------------------------- сеть
static bool Geocode(const wstring& q, int lang, wstring& name, wstring& country, double& lat, double& lon) {
    static const wchar_t* lc[3] = {L"en", L"ar", L"ru"};
    wstring path = L"/v1/search?count=1&format=json&language=" + wstring(lc[lang]) + L"&name=" + UrlEnc(q);
    string j = HttpGet(L"geocoding-api.open-meteo.com", path, true);
    if (j.find("\"results\"") == string::npos) return false;
    string n = StrVal(j, "name");
    if (n.empty()) return false;
    name = ToW(n);
    country = ToW(StrVal(j, "country"));
    lat = NumVal(j, "latitude");
    lon = NumVal(j, "longitude");
    return true;
}

static DWORD WINAPI LoadThread(LPVOID p) {
    Job* J = (Job*)p;
    Res* R = new Res();
    double lat = J->c.lat, lon = J->c.lon;
    wstring name = J->c.cityName;
    bool have = true;
    if (J->c.autoCity) {
        string g = HttpGet(L"ip-api.com", wstring(L"/json/?fields=status,city,lat,lon&lang=") + (J->c.lang == 2 ? L"ru" : L"en"), false);
        if (g.find("\"success\"") != string::npos) { lat = NumVal(g, "lat"); lon = NumVal(g, "lon"); name = ToW(StrVal(g, "city")); }
        else have = false;
    } else if (lat == 0 && lon == 0) have = false;

    if (have) {
        wchar_t path[300];
        _snwprintf(path, 300, L"/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m&timezone=auto", lat, lon);
        string o = HttpGet(L"api.open-meteo.com", path, true);
        R->off = o.find("utc_offset_seconds") != string::npos ? (long)NumVal(o, "utc_offset_seconds") : LocalOffset();

        long long t = (long long)time(0) + R->off;
        R->day = t / 86400;
        time_t tt = (time_t)t;
        tm* g = gmtime(&tt);
        _snwprintf(path, 300, L"/v1/timings/%02d-%02d-%04d?latitude=%.4f&longitude=%.4f&school=%d",
                   g->tm_mday, g->tm_mon + 1, g->tm_year + 1900, lat, lon, J->c.school);
        string body = HttpGet(L"api.aladhan.com", path, true);
        bool good = !body.empty();
        for (int i = 0; i < 6 && good; i++) {
            int h, m;
            if (sscanf(StrVal(body, PKEY[i]).c_str(), "%d:%d", &h, &m) != 2) good = false;
            else R->sec[i] = h * 3600 + m * 60;
        }
        R->ok = good;
        size_t hp = body.find("\"hijri\"");
        if (hp != string::npos) {
            size_t q = body.find("\"day\":\"", hp);
            if (q != string::npos) R->hd = atoi(body.c_str() + q + 7);
            q = body.find("\"month\":{\"number\":", hp);
            if (q != string::npos) R->hm = atoi(body.c_str() + q + 18);
        }
        R->name = name;
    }
    PostMessage(J->h, WM_APP + 1, 0, (LPARAM)R);
    delete J;
    return 0;
}
static void StartLoad() {
    if (g_loading) { g_again = true; return; }
    g_loading = true;
    Job* j = new Job{g_ctl, g_cfg};
    HANDLE h = CreateThread(0, 0, LoadThread, j, 0, 0);
    if (h) CloseHandle(h);
    else { g_loading = false; delete j; }
}

// ---------------------------------------------------------------- время намазов
static void CityNow(long long& day, int& sec) {
    long long t = (long long)time(0) + (g_res.ok ? g_res.off : LocalOffset());
    day = t / 86400;
    sec = (int)(t % 86400);
}
static const wchar_t* JN[3] = {L"Jumu'ah", L"الجمعة", L"Джума"};
static const wchar_t* IFTAR[3] = {L"Iftar", L"الإفطار", L"Ифтар"};
static const wchar_t* SUHOOR[3] = {L"Suhoor ends", L"الإمساك", L"Конец сухура"};
static bool IsFriday() {  // 1 января 1970 был четвергом
    long long d; int c;
    CityNow(d, c);
    return ((d + 4) % 7) == 5;
}
static bool IsRamadan() { return g_res.ok && g_res.hm == 9; }
static const wchar_t* PNm(int i) {
    if (i == 2 && IsFriday()) return JN[g_cfg.lang];
    if (IsRamadan()) { if (i == 4) return IFTAR[g_cfg.lang]; if (i == 0) return SUHOOR[g_cfg.lang]; }
    return PN[g_cfg.lang][i];
}
static int PT(int i) { return g_res.sec[i] + g_cfg.off[i] * 60; }
static void NextPrayer(int cur, int& idx, int& tgt) {
    static const int o[5] = {0, 2, 3, 4, 5};
    for (int k = 0; k < 5; k++)
        if (PT(o[k]) > cur) { idx = o[k]; tgt = PT(o[k]); return; }
    idx = 0; tgt = PT(0) + 86400;
}
// Иконка в трее (полумесяц), рисуется программно со сглаживанием
static HICON MakeIcon(int n) {
    if (n < 16) n = 16;
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = n;
    bi.bmiHeader.biHeight = -n;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    HDC dc = GetDC(0);
    uint32_t* px = 0;
    HBITMAP color = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&px, 0, 0);
    HBITMAP mask = CreateBitmap(n, n, 1, 1, 0);
    ReleaseDC(0, dc);
    if (!color || !px || !mask) {
        if (color) DeleteObject(color);
        if (mask) DeleteObject(mask);
        return 0;
    }
    float c = n / 2.0f, R = n * 0.48f;
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++) {
            float fx = x + 0.5f - c, fy = y + 0.5f - c;
            float aBg = 0.5f - (sqrtf(fx * fx + fy * fy) - R);
            if (aBg < 0) aBg = 0;
            if (aBg > 1) aBg = 1;
            float ax = fx + n * 0.04f, ay = fy;
            float bx = fx - n * 0.09f, by = fy + n * 0.05f;
            float dA = sqrtf(ax * ax + ay * ay) - n * 0.27f;
            float dB = sqrtf(bx * bx + by * by) - n * 0.22f;
            float sd = dA > -dB ? dA : -dB;
            float aC = 0.5f - sd;
            if (aC < 0) aC = 0;
            if (aC > 1) aC = 1;
            float r = 30 + (255 - 30) * aC, g = 160 + (255 - 160) * aC, b = 110 + (255 - 110) * aC;
            uint32_t a = (uint32_t)(aBg * 255.0f + 0.5f);
            px[y * n + x] = (a << 24) | ((uint32_t)(r * aBg) << 16) | ((uint32_t)(g * aBg) << 8) | (uint32_t)(b * aBg);
        }
    ICONINFO ii = {};
    ii.fIcon = TRUE;
    ii.hbmMask = mask;
    ii.hbmColor = color;
    HICON ic = CreateIconIndirect(&ii);
    DeleteObject(color);
    DeleteObject(mask);
    return ic;
}
static void AddTray(HWND h) {
    ZeroMemory(&g_nid, sizeof g_nid);
    g_nid.cbSize = sizeof g_nid;
    g_nid.hWnd = h;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_APP + 2;
    g_nid.hIcon = g_icon ? g_icon : LoadIcon(0, IDI_APPLICATION);
    lstrcpynW(g_nid.szTip, (wstring(L"PrayerBar ") + APP_VERSION).c_str(), 128);
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}
static bool ShowToast(int idx, int lead);
static void Notify(int i, int lead) {
    if (ShowToast(i, lead)) return;
    wchar_t msg[200];
    if (lead == 0) _snwprintf(msg, 200, T(S_NOTE_NOW), PNm(i));
    else _snwprintf(msg, 200, T(S_NOTE_BEFORE), PNm(i), lead);
    NOTIFYICONDATAW n = g_nid;
    n.uFlags = NIF_INFO;
    n.dwInfoFlags = NIIF_INFO;
    lstrcpynW(n.szInfoTitle, L"PrayerBar", 64);
    lstrcpynW(n.szInfo, msg, 256);
    Shell_NotifyIconW(NIM_MODIFY, &n);
}
static void CheckNotify() {
    if (!g_cfg.notify || !g_res.ok) return;
    long long day; int cur;
    CityNow(day, cur);
    static const int o[5] = {0, 2, 3, 4, 5};
    int lead = LEADS[g_cfg.leadIdx];
    for (int k = 0; k < 5; k++) {
        int i = o[k], target = PT(i) - lead * 60;
        if (cur >= target && cur < target + 90) {
            long long key = day * 10 + i;
            if (g_notified != key) { g_notified = key; Notify(i, lead); }
        }
    }
}
// Обновляет две строки виджета; true, если текст изменился (тогда нужна перерисовка).
static bool UpdateLines() {
    wstring a, b;
    if (!g_res.ok) {
        a = g_loading ? T(S_LOADING) : T(S_OFFLINE);
        b = g_loading ? L"" : T(S_RETRY);
    } else {
        long long day; int cur, idx, tgt;
        CityNow(day, cur);
        int iqIdx = -1, iqEnd = 0;  // азан уже был, икама ещё не наступила
        if (g_cfg.iqOn)
            for (int k = 0; k < 5; k++) {
                int p = IQI[k], s = PT(p), e = s + g_cfg.iq[p] * 60;
                if (g_cfg.iq[p] > 0 && !(p == 2 && IsFriday()) && cur >= s && cur < e) { iqIdx = p; iqEnd = e; }
            }
        wchar_t c[48];
        if (iqIdx >= 0) {
            int d = iqEnd - cur;
            if (d >= 3600) _snwprintf(c, 48, T(S_HM), d / 3600, (d / 60) % 60);
            else _snwprintf(c, 48, L"%02d:%02d", d / 60, d % 60);
            a = wstring(T(S_IQAMA)) + L" " + c;
            b = wstring(PNm(iqIdx)) + L" " + Fmt(PT(iqIdx));
        } else {
            NextPrayer(cur, idx, tgt);
            int d = tgt - cur;
            if (d >= 3600) _snwprintf(c, 48, T(S_HM), d / 3600, (d / 60) % 60);
            else _snwprintf(c, 48, L"%02d:%02d", d / 60, d % 60);
            a = c;
            b = wstring(PNm(idx)) + L" " + Fmt(tgt);
        }
    }
    bool ch = a != g_l1 || b != g_l2;
    g_l1 = a; g_l2 = b;
    return ch;
}

// ---------------------------------------------------------------- виджет на панели задач (прозрачный: цвет панели не искажается)
static bool g_light = false;
static int g_wx = -32000, g_wy = 0, g_ww = 0, g_wh = 0;
static bool g_child = false, g_forcePopup = false;

static void ReadTheme() {
    DWORD v = 0, pv = 0, sz = sizeof(DWORD);
    const wchar_t* key = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    RegGetValueW(HKEY_CURRENT_USER, key, L"SystemUsesLightTheme", RRF_RT_REG_DWORD, 0, &v, &sz);
    sz = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, key, L"ColorPrevalence", RRF_RT_REG_DWORD, 0, &pv, &sz);
    g_light = (v == 1 && pv == 0);
}
static int IMax(int a, int b) { return a > b ? a : b; }
static COLORREF WCol(int i) {  // цвет текста виджета: 0 = по теме Windows
    static const COLORREF pal[7] = {0, RGB(255, 255, 255), RGB(46, 204, 142), RGB(64, 196, 255), RGB(255, 205, 86),
                                    RGB(255, 120, 180), RGB(170, 130, 255)};
    if (i <= 0 || i > 6) return g_light ? RGB(20, 20, 20) : RGB(255, 255, 255);
    return pal[i];
}
static void ApplyWidgetFont() {
    HFONT w = g_font;
    g_font = MkFont(g_cfg.wFont, FW_NORMAL, ANTIALIASED_QUALITY);
    if (w) DeleteObject(w);
}
static int IMin(int a, int b) { return a < b ? a : b; }

// Виджет встраивается дочерним окном В панель задач: панель общая для всех рабочих столов,
// поэтому он не пропадает при их смене, а в полноэкранном режиме скрывается вместе с панелью.
static void AttachToTray(HWND tray) {
    if (g_forcePopup || !g_main) return;
    if (GetParent(g_main) == tray) { g_child = true; return; }
    LONG_PTR st = GetWindowLongPtr(g_main, GWL_STYLE);
    SetWindowLongPtr(g_main, GWL_STYLE, (st & ~(LONG_PTR)WS_POPUP) | WS_CHILD);
    SetParent(g_main, tray);
    g_child = GetParent(g_main) == tray;
    if (!g_child) SetWindowLongPtr(g_main, GWL_STYLE, st);
    g_wx = -32000;
}
static void DetachFromTray() {  // запасной вариант: обычное окно поверх всех
    g_forcePopup = true;
    g_child = false;
    ShowWindow(g_main, SW_HIDE);
    SetParent(g_main, 0);
    LONG_PTR st = GetWindowLongPtr(g_main, GWL_STYLE);
    SetWindowLongPtr(g_main, GWL_STYLE, (st & ~(LONG_PTR)WS_CHILD) | WS_POPUP);
    g_wx = -32000;
}
static void EnsureMain() {
    if (g_main && IsWindow(g_main)) return;
    g_main = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, L"PrayerBar", L"", WS_POPUP,
                            0, 0, SC(230), SC(40), 0, 0, g_hi, 0);
    g_child = false;
    g_wx = -32000;
}

static bool Layout(HWND h) {
    HWND tray = FindWindow(L"Shell_TrayWnd", 0);
    if (!tray) return false;
    AttachToTray(tray);
    RECT tr, nr;
    GetWindowRect(tray, &tr);
    HDC dc = GetDC(0);
    HGDIOBJ old = SelectObject(dc, g_font);
    SIZE a = {0, 0}, b = {0, 0};
    GetTextExtentPoint32W(dc, g_l1.c_str(), (int)g_l1.size(), &a);
    GetTextExtentPoint32W(dc, g_l2.c_str(), (int)g_l2.size(), &b);
    SelectObject(dc, old);
    ReleaseDC(0, dc);
    int w = IMin(IMax(IMax((int)a.cx, (int)b.cx) + SC(28), SC(110)), SC(320));
    int hg = IMin(tr.bottom - tr.top, SC(60));
    HWND notify = FindWindowEx(tray, 0, L"TrayNotifyWnd", 0);
    int x = tr.right - w - SC(250) - SC(g_cfg.wOffset);
    if (notify && GetWindowRect(notify, &nr) && nr.right - nr.left > 0 && nr.left > tr.left + w) x = nr.left - w - SC(g_cfg.wOffset);
    if (x < tr.left) x = tr.left;
    bool ch = x != g_wx || tr.top != g_wy || w != g_ww || hg != g_wh;
    if (ch) {
        g_wx = x; g_wy = tr.top; g_ww = w; g_wh = hg;
        if (g_child) SetWindowPos(h, HWND_TOP, x - tr.left, 0, w, hg, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
        else SetWindowPos(h, HWND_TOPMOST, x, tr.top, w, hg, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    return ch;
}
static void Raise() {
    if (g_main) SetWindowPos(g_main, g_child ? HWND_TOP : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}
// Текст рисуется белым на чёрном, яркость пикселя становится прозрачностью:
// фона у виджета нет вообще, поэтому он всегда совпадает с панелью задач.
static void RenderMain() {
    if (!g_main || g_ww <= 0 || g_wh <= 0) return;
    int W = g_ww, H = g_wh;
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    HDC scr = GetDC(0);
    HDC mem = CreateCompatibleDC(scr);
    uint32_t* px = 0;
    HBITMAP bm = CreateDIBSection(scr, &bi, DIB_RGB_COLORS, (void**)&px, 0, 0);
    if (!bm || !px) {
        if (bm) DeleteObject(bm);
        DeleteDC(mem);
        ReleaseDC(0, scr);
        return;
    }
    HGDIOBJ old = SelectObject(mem, bm);
    memset(px, 0, (size_t)W * H * 4);
    SetBkMode(mem, TRANSPARENT);
    SelectObject(mem, g_font);
    SetTextColor(mem, RGB(255, 255, 255));
    UINT rtl = g_cfg.lang == 1 ? DT_RTLREADING : 0;
    UINT hal = g_cfg.wAlign == 0 ? DT_LEFT : (g_cfg.wAlign == 2 ? DT_RIGHT : DT_CENTER);
    RECT r1 = {SC(8), 0, W - SC(8), H / 2 + 2}, r2 = {SC(8), H / 2 - 1, W - SC(8), H};
    DrawTextW(mem, g_l1.c_str(), -1, &r1, hal | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | rtl);
    DrawTextW(mem, g_l2.c_str(), -1, &r2, hal | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | rtl);
    GdiFlush();
    COLORREF c = WCol(g_cfg.wColor);
    uint32_t m1 = (uint32_t)(g_cfg.wOpacity * 256 / 100), m2 = m1 * 80 / 100;  // нижняя строка чуть бледнее
    for (int y = 0; y < H; y++) {
        uint32_t mm = y < H / 2 ? m1 : m2;
        uint32_t cr = GetRValue(c), cg = GetGValue(c), cb = GetBValue(c);
        for (int x = 0; x < W; x++) {
            uint32_t p = px[y * W + x];
            uint32_t a = IMax(IMax((int)((p >> 16) & 255), (int)((p >> 8) & 255)), (int)(p & 255));
            a = a * mm / 256;
            if (a == 0) { px[y * W + x] = 0x01000000; continue; }  // почти невидимый, но кликабельный
            px[y * W + x] = (a << 24) | ((cr * a / 255) << 16) | ((cg * a / 255) << 8) | (cb * a / 255);
        }
    }
    POINT pd = {g_wx, g_wy}, ps = {0, 0};
    SIZE sz = {W, H};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    BOOL okU = UpdateLayeredWindow(g_main, scr, g_child ? 0 : &pd, &sz, mem, &ps, 0, &bf, ULW_ALPHA);
    if (!okU && g_child) DetachFromTray();
    SelectObject(mem, old);
    DeleteObject(bm);
    DeleteDC(mem);
    ReleaseDC(0, scr);
}
static bool g_hidden = false;
static bool ShouldHide() {
    HWND tray = FindWindow(L"Shell_TrayWnd", 0);
    if (!tray || !IsWindowVisible(tray)) return true;
    HMONITOR mon = MonitorFromWindow(tray, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {};
    mi.cbSize = sizeof mi;
    GetMonitorInfo(mon, &mi);
    RECT tr, is;
    GetWindowRect(tray, &tr);
    long full = (long)(tr.right - tr.left) * (tr.bottom - tr.top);
    if (full > 0) {  // панель с автоскрытием уехала за край экрана
        long vis = 0;
        if (IntersectRect(&is, &tr, &mi.rcMonitor)) vis = (long)(is.right - is.left) * (is.bottom - is.top);
        if (vis * 100 < full * 60) return true;
    }
    HWND fg = GetForegroundWindow();  // окно во весь монитор (игра, видео) — панели не видно
    if (fg && fg != tray && fg != g_main && fg != g_set && IsWindowVisible(fg) && !IsIconic(fg)) {
        wchar_t cls[64] = L"";
        GetClassNameW(fg, cls, 64);
        if (wcscmp(cls, L"Progman") && wcscmp(cls, L"WorkerW") && wcscmp(cls, L"Shell_TrayWnd") &&
            wcscmp(cls, L"Shell_SecondaryTrayWnd")) {
            RECT r;
            GetWindowRect(fg, &r);
            if (MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST) == mon && r.left <= mi.rcMonitor.left &&
                r.top <= mi.rcMonitor.top && r.right >= mi.rcMonitor.right && r.bottom >= mi.rcMonitor.bottom)
                return true;
        }
    }
    QUERY_USER_NOTIFICATION_STATE s;
    if (SUCCEEDED(SHQueryUserNotificationState(&s)) && (s == QUNS_RUNNING_D3D_FULL_SCREEN || s == QUNS_PRESENTATION_MODE))
        return true;
    return false;
}
static void RefreshMain() {
    UpdateLines();
    if (g_hidden) return;
    EnsureMain();
    if (!g_main) return;
    Layout(g_main);
    RenderMain();
}
static void SyncVisibility() {
    bool hide = g_child ? false : ShouldHide();
    if (hide != g_hidden) {
        g_hidden = hide;
        if (hide && g_main) ShowWindow(g_main, SW_HIDE);
        else { g_wx = -32000; RefreshMain(); }
    }
    if (!g_hidden) Raise();
}

// ---------------------------------------------------------------- окно настроек (всё рисуется вручную)
enum { ID_LANG = 10, ID_MODE = 20, ID_FIND = 30, ID_ASR = 40, ID_NOTIF = 50, ID_LEAD = 60, ID_AUTORUN = 70, ID_SAVE = 80, ID_TEST = 85, ID_IQON = 87, ID_TAB = 90, ID_RESET = 95, ID_OFF = 100, ID_IQ = 120, ID_WS = 140, ID_WAL = 150, ID_WC = 160, ID_WRESET = 170 };
enum Kind { K_CARD, K_LABEL, K_CAP, K_SEG, K_BTN, K_PRIMARY, K_TOGGLE, K_CHIP, K_STATUS, K_STEP, K_NAME, K_VAL, K_FIELD, K_TAB, K_SWATCH, K_PREVIEW };
struct Item { int id; int kind; RECT r; wstring t; bool on; bool en; };

static const int WIN_W = 420, WIN_H = 584;
static Cfg s_f;
static wstring s_status, s_pendQuery, s_pendName;
static bool s_err = false, s_pendOk = false;
static double s_pendLat = 0, s_pendLon = 0;
static HWND s_edit = 0;
static HBRUSH s_cardBr = 0;
static int s_hover = 0, s_page = 0, s_rep = 0;
static bool s_tracking = false;

static RECT MR(int x, int y, int w, int h) {
    int x0 = RTL() ? WIN_W - x - w : x;
    RECT r = {SC(x0), SC(y), SC(x0 + w), SC(y + h)};
    return r;
}
static void Build10(vector<Item>& v) {
    auto add = [&](int id, int k, int x, int y, int w, int h, const wstring& t, bool on, bool en) {
        Item it = {id, k, MR(x, y, w, h), t, on, en};
        v.push_back(it);
    };
    auto seg = [&](int base, int y, const vector<wstring>& lb, int sel, bool en) {
        int n = (int)lb.size(), sw = (372 - 2 * (n - 1)) / n;
        for (int i = 0; i < n; i++) add(base + i, K_SEG, 24 + i * (sw + 2), y, sw, 32, lb[i], i == sel, en);
    };
    add(ID_TAB, K_TAB, 24, 46, 92, 28, T(S_TAB_MAIN), s_page == 0, true);
    add(ID_TAB + 1, K_TAB, 116, 46, 92, 28, T(S_TAB_TIMES), s_page == 1, true);
    add(ID_TAB + 2, K_TAB, 208, 46, 92, 28, T(S_TAB_IQAMA), s_page == 2, true);
    add(ID_TAB + 3, K_TAB, 300, 46, 92, 28, T(S_TAB_LOOK), s_page == 3, true);
    if (s_page == 0) {
        bool man = !s_f.autoCity;
        add(0, K_LABEL, 24, 88, 372, 16, T(S_LANGUAGE), false, true);
        seg(ID_LANG, 110, {L"English", L"العربية", L"Русский"}, s_f.lang, true);

        add(0, K_LABEL, 24, 156, 372, 16, T(S_LOCATION), false, true);
        seg(ID_MODE, 178, {T(S_AUTO), T(S_MANUAL)}, man ? 1 : 0, true);
        add(0, K_FIELD, 24, 220, 262, 34, L"", false, man);
        add(ID_FIND, K_BTN, 294, 220, 102, 34, T(S_FIND), false, man);
        add(0, K_STATUS, 24, 258, 372, 16, s_status, false, true);

        add(0, K_LABEL, 24, 288, 372, 16, T(S_ASR), false, true);
        seg(ID_ASR, 310, {T(S_STD), T(S_HANAFI)}, s_f.school, true);

        add(0, K_LABEL, 24, 356, 372, 16, T(S_NOTIF), false, true);
        add(ID_TEST, K_BTN, 296, 349, 100, 24, T(S_TEST), false, true);
        add(ID_NOTIF, K_TOGGLE, 24, 378, 372, 32, T(S_NOTIFON), s_f.notify, true);
        add(0, K_CAP, 24, 416, 372, 16, T(S_REMIND), false, true);
        for (int i = 0; i < 5; i++) {
            wchar_t b[32];
            if (i == 0) lstrcpynW(b, T(S_AT), 32); else _snwprintf(b, 32, T(S_LEAD), LEADS[i]);
            add(ID_LEAD + i, K_CHIP, 24 + i * 75, 436, 71, 30, b, i == s_f.leadIdx, s_f.notify);
        }
        add(ID_AUTORUN, K_TOGGLE, 24, 476, 372, 32, T(S_AUTORUN), s_f.autorun, true);
    } else if (s_page == 1) {
        add(0, K_LABEL, 24, 88, 372, 16, T(S_ADJ), false, true);
        add(0, K_CAP, 24, 108, 372, 16, T(S_ADJ_HINT), false, true);
        for (int i = 0; i < 6; i++) {
            int y = 132 + i * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 90, 38, PN[s_f.lang][i], false, true);
            add(0, K_CAP, 118, y, 70, 38, g_res.ok ? Fmt(g_res.sec[i] + s_f.off[i] * 60) : wstring(L"--:--"), false, true);
            add(ID_OFF + i * 2, K_STEP, 262, y + 5, 32, 28, L"−", false, true);
            wchar_t vb[32];
            if (s_f.off[i] == 0) lstrcpynW(vb, L"0", 32); else _snwprintf(vb, 32, L"%+d", s_f.off[i]);
            add(0, K_VAL, 298, y, 54, 38, vb, s_f.off[i] != 0, true);
            add(ID_OFF + i * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, true);
        }
        add(ID_RESET, K_BTN, 24, 404, 372, 34, T(S_RESET), false, true);
    } else if (s_page == 2) {
        add(ID_IQON, K_TOGGLE, 24, 88, 372, 34, T(S_IQ_ON), s_f.iqOn, true);
        add(0, K_CAP, 24, 130, 372, 16, T(S_IQ_HINT), false, true);
        for (int k = 0; k < 5; k++) {
            int i = IQI[k], y = 154 + k * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 120, 38, PN[s_f.lang][i], false, true);
            add(ID_IQ + k * 2, K_STEP, 232, y + 5, 32, 28, L"\u2212", false, s_f.iqOn);
            wchar_t vb[32];
            _snwprintf(vb, 32, T(S_LEAD), s_f.iq[i]);
            add(0, K_VAL, 266, y, 86, 38, vb, s_f.iq[i] > 0, true);
            add(ID_IQ + k * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, s_f.iqOn);
        }
    } else {
        const wchar_t* nm[3] = {T(S_W_FONT), T(S_W_OPAC), T(S_W_POS)};
        int val[3] = {s_f.wFont, s_f.wOpacity, s_f.wOffset};
        for (int k = 0; k < 3; k++) {
            int y = 88 + k * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 150, 38, nm[k], false, true);
            add(ID_WS + k * 2, K_STEP, 232, y + 5, 32, 28, L"\u2212", false, true);
            wchar_t vb[32];
            _snwprintf(vb, 32, k == 0 ? L"%d pt" : (k == 1 ? L"%d%%" : L"%d px"), val[k]);
            add(0, K_VAL, 266, y, 86, 38, vb, true, true);
            add(ID_WS + k * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, true);
        }
        add(0, K_LABEL, 24, 228, 372, 16, T(S_W_ALIGN), false, true);
        seg(ID_WAL, 250, {T(S_W_LEFT), T(S_W_CENTER), T(S_W_RIGHT)}, s_f.wAlign, true);
        add(0, K_LABEL, 24, 298, 372, 16, T(S_W_COLOR), false, true);
        for (int i = 0; i < 7; i++) add(ID_WC + i, K_SWATCH, 24 + i * 56, 320, 36, 36, i == 0 ? L"A" : L"", i == s_f.wColor, true);
        add(0, K_PREVIEW, 24, 376, 372, 56, L"", false, true);
        add(ID_WRESET, K_BTN, 24, 444, 372, 34, T(S_RESET), false, true);
    }
    add(ID_SAVE, K_PRIMARY, 24, 524, 372, 40, T(S_SAVE), false, true);
}
// --- сглаженная отрисовка: расстояние до фигуры (SDF) даёт плавные края без "лесенки"
static uint32_t* s_px = 0;
static int s_W = 0, s_H = 0;

static inline void Bl(uint32_t* d, COLORREF c, float a) {
    int ia = (int)(a * 256.0f + 0.5f);
    if (ia <= 0) return;
    if (ia > 256) ia = 256;
    uint32_t v = *d;
    int dr = (v >> 16) & 255, dg = (v >> 8) & 255, db = v & 255;
    dr += ((GetRValue(c) - dr) * ia) >> 8;
    dg += ((GetGValue(c) - dg) * ia) >> 8;
    db += ((GetBValue(c) - db) * ia) >> 8;
    *d = ((uint32_t)dr << 16) | ((uint32_t)dg << 8) | (uint32_t)db;
}
static inline float sdRR(float px, float py, float hx, float hy, float r) {
    float qx = fabsf(px) - (hx - r), qy = fabsf(py) - (hy - r);
    float ax = qx > 0 ? qx : 0, ay = qy > 0 ? qy : 0;
    float m = qx > qy ? qx : qy;
    if (m > 0) m = 0;
    return sqrtf(ax * ax + ay * ay) + m - r;
}
static void PxFill(const RECT& r, COLORREF c) {
    int x0 = IMax(0, (int)r.left), x1 = IMin(s_W, (int)r.right), y0 = IMax(0, (int)r.top), y1 = IMin(s_H, (int)r.bottom);
    uint32_t v = ((uint32_t)GetRValue(c) << 16) | ((uint32_t)GetGValue(c) << 8) | GetBValue(c);
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) s_px[y * s_W + x] = v;
}
static void RRpx(const RECT& r, float rad, COLORREF c) {
    float hx = (r.right - r.left) / 2.0f, hy = (r.bottom - r.top) / 2.0f;
    float cx = r.left + hx, cy = r.top + hy;
    if (rad > hy) rad = hy;
    if (rad > hx) rad = hx;
    int x0 = IMax(0, (int)r.left - 1), x1 = IMin(s_W, (int)r.right + 1);
    int y0 = IMax(0, (int)r.top - 1), y1 = IMin(s_H, (int)r.bottom + 1);
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            float a = 0.5f - sdRR(x + 0.5f - cx, y + 0.5f - cy, hx, hy, rad);
            if (a <= 0) continue;
            if (a > 1) a = 1;
            Bl(&s_px[y * s_W + x], c, a);
        }
}
static void RR(const RECT& r, int rad, COLORREF c) { RRpx(r, (float)SC(rad), c); }
static void Circle(float cx, float cy, float rad, COLORREF c) {
    int x0 = IMax(0, (int)(cx - rad) - 1), x1 = IMin(s_W, (int)(cx + rad) + 2);
    int y0 = IMax(0, (int)(cy - rad) - 1), y1 = IMin(s_H, (int)(cy + rad) + 2);
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
            float a = 0.5f - (sqrtf(dx * dx + dy * dy) - rad);
            if (a <= 0) continue;
            if (a > 1) a = 1;
            Bl(&s_px[y * s_W + x], c, a);
        }
}
static UINT AL() { return RTL() ? DT_RIGHT : DT_LEFT; }
static void Txt(HDC dc, const wstring& s, RECT r, HFONT f, COLORREF c, UINT fl) {
    SelectObject(dc, f);
    SetTextColor(dc, c);
    DrawTextW(dc, s.c_str(), -1, &r, fl | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS | (RTL() ? DT_RTLREADING : 0));
}
static RECT Inset(RECT r, int d) {
    r.left += d; r.top += d; r.right -= d; r.bottom -= d;
    return r;
}
static COLORREF Mix(COLORREF a, COLORREF b, int pct) {
    return RGB((GetRValue(a) * pct + GetRValue(b) * (100 - pct)) / 100, (GetGValue(a) * pct + GetGValue(b) * (100 - pct)) / 100,
               (GetBValue(a) * pct + GetBValue(b) * (100 - pct)) / 100);
}
static COLORREF PreviewBar() { return g_light ? RGB(238, 238, 238) : RGB(30, 30, 30); }
static void SwatchShape(const Item& it, bool hov) {
    float cx = (it.r.left + it.r.right) / 2.0f, cy = (it.r.top + it.r.bottom) / 2.0f;
    int idx = it.id - ID_WC;
    if (it.on) Circle(cx, cy, (float)SC(18), CACC);
    else if (hov) Circle(cx, cy, (float)SC(18), CCARDH);
    if (it.on || hov) Circle(cx, cy, (float)SC(16), CBG);
    Circle(cx, cy, (float)SC(13), idx == 0 ? CCARD : WCol(idx));
}
static void PreviewText(HDC dc, const RECT& r) {
    HFONT f = MkFont(s_f.wFont, FW_NORMAL, ANTIALIASED_QUALITY);
    COLORREF bar = PreviewBar(), base = WCol(s_f.wColor);
    COLORREF c1 = Mix(base, bar, s_f.wOpacity), c2 = Mix(base, bar, s_f.wOpacity * 80 / 100);
    UINT al = s_f.wAlign == 0 ? DT_LEFT : (s_f.wAlign == 2 ? DT_RIGHT : DT_CENTER);
    int mid = (r.top + r.bottom) / 2;
    RECT r1 = {r.left + SC(12), r.top, r.right - SC(12), mid + SC(2)}, r2 = {r.left + SC(12), mid - SC(1), r.right - SC(12), r.bottom};
    Txt(dc, L"02:45", r1, f, c1, al);
    Txt(dc, wstring(PN[s_f.lang][3]) + L" 15:12", r2, f, c2, al);
    DeleteObject(f);
}
static void ToggleGeo(const RECT& r, RECT& tk, RECT& tr, RECT& st) {
    int sw = SC(40), sh = SC(20);
    int sx = RTL() ? r.left : r.right - sw;
    int sy = (r.top + r.bottom - sh) / 2;
    tk.left = sx; tk.top = sy; tk.right = sx + sw; tk.bottom = sy + sh;
    tr = r; st = r;
    if (RTL()) { tr.left = sx + sw + SC(56); st.left = sx + sw + SC(8); st.right = sx + sw + SC(52); }
    else { tr.right = sx - SC(56); st.left = sx - SC(52); st.right = sx - SC(8); }
}
static void ShapeItem10(const Item& it, bool hov) {
    const RECT& r = it.r;
    switch (it.kind) {
    case K_CARD: PxFill(r, CROW); break;
    case K_SWATCH: SwatchShape(it, hov); break;
    case K_PREVIEW: PxFill(r, PreviewBar()); break;
    case K_FIELD:
        PxFill(r, it.en ? CBORD : CCARDH);
        PxFill(Inset(r, SC(2)), it.en ? CFIELD : CCARD);
        break;
    case K_SEG: case K_CHIP:
        if (it.on && it.en) PxFill(r, hov ? CACCH : CACC);
        else PxFill(r, hov && it.en ? CCARDH : CCARD);
        break;
    case K_BTN: PxFill(r, hov && it.en ? CCARDH : CCARD); break;
    case K_PRIMARY: PxFill(r, hov ? CACCH : CACC); break;
    case K_STEP: PxFill(r, hov ? CACC : CCARDH); break;
    case K_TAB:
        if (it.on) {
            HDC d = GetDC(0);
            HGDIOBJ o = SelectObject(d, g_bold);
            SIZE tz = {0, 0};
            GetTextExtentPoint32W(d, it.t.c_str(), (int)it.t.size(), &tz);
            SelectObject(d, o);
            ReleaseDC(0, d);
            RECT u = {0, r.bottom - SC(3), 0, r.bottom - SC(1)};
            if (RTL()) { u.right = r.right - SC(4); u.left = u.right - tz.cx; }
            else { u.left = r.left + SC(4); u.right = u.left + tz.cx; }
            PxFill(u, CACC);
        }
        break;
    case K_TOGGLE: {
        RECT tk, tr, st;
        ToggleGeo(r, tk, tr, st);
        int sh = tk.bottom - tk.top;
        float rad = sh / 2.0f, cy = tk.top + rad;
        if (it.on) {
            RRpx(tk, rad, hov ? CACCH : CACC);
            Circle((float)(tk.right - SC(10)), cy, (float)SC(5), RGB(255, 255, 255));
        } else {
            RRpx(tk, rad, hov ? CTXT : COFF);
            RRpx(Inset(tk, SC(2)), rad - SC(2), CBG);
            Circle((float)(tk.left + SC(10)), cy, (float)SC(5), hov ? CTXT : COFF);
        }
        break;
    }
    }
}
static void TextItem10(HDC dc, const Item& it, bool hov) {
    const RECT& r = it.r;
    switch (it.kind) {
    case K_LABEL: Txt(dc, it.t, r, g_bold, CTXT, AL()); break;
    case K_CAP: Txt(dc, it.t, r, g_small, CDIM, AL()); break;
    case K_STATUS: Txt(dc, it.t, r, g_small, s_err ? CERR : CDIM, AL()); break;
    case K_SEG:
        if (it.on && it.en) Txt(dc, it.t, r, g_bold, RGB(255, 255, 255), DT_CENTER);
        else Txt(dc, it.t, r, g_ui, it.en ? CTXT : CDIS, DT_CENTER);
        break;
    case K_CHIP:
        if (it.on && it.en) Txt(dc, it.t, r, g_small, RGB(255, 255, 255), DT_CENTER);
        else Txt(dc, it.t, r, g_small, it.en ? CTXT : CDIS, DT_CENTER);
        break;
    case K_BTN: Txt(dc, it.t, r, g_ui, it.en ? CTXT : CDIS, DT_CENTER); break;
    case K_PRIMARY: Txt(dc, it.t, r, g_bold, RGB(255, 255, 255), DT_CENTER); break;
    case K_STEP: Txt(dc, it.t, r, g_bold, hov ? RGB(255, 255, 255) : CTXT, DT_CENTER); break;
    case K_NAME: Txt(dc, it.t, r, g_ui, CTXT, AL()); break;
    case K_SWATCH: if (!it.t.empty()) Txt(dc, it.t, r, g_bold, CTXT, DT_CENTER); break;
    case K_PREVIEW: PreviewText(dc, r); break;
    case K_VAL: Txt(dc, it.t, r, g_bold, it.on ? CACCT : CDIM, DT_CENTER); break;
    case K_TAB: {
        RECT t = r;
        if (RTL()) t.right -= SC(4); else t.left += SC(4);
        Txt(dc, it.t, t, it.on ? g_bold : g_ui, it.on ? CTXT : CDIM, AL());
        break;
    }
    case K_TOGGLE: {
        RECT tk, tr, st;
        ToggleGeo(r, tk, tr, st);
        Txt(dc, it.t, tr, g_ui, CTXT, AL());
        Txt(dc, it.on ? T(S_ON) : T(S_OFF), st, g_small, CDIM, RTL() ? DT_LEFT : DT_RIGHT);
        break;
    }
    }
}
// ---- Windows 11: скруглённый стиль (карточки, "таблетки", круглые переключатели)
static void Build11(vector<Item>& v) {
    auto add = [&](int id, int k, int x, int y, int w, int h, const wstring& t, bool on, bool en) {
        Item it = {id, k, MR(x, y, w, h), t, on, en};
        v.push_back(it);
    };
    auto seg = [&](int base, int y, const vector<wstring>& lb, int sel, bool en) {
        add(0, K_CARD, 24, y, 372, 34, L"", false, true);
        int n = (int)lb.size(), sw = 364 / n;
        for (int i = 0; i < n; i++) add(base + i, K_SEG, 28 + i * sw, y + 3, sw, 28, lb[i], i == sel, en);
    };
    add(0, K_CARD, 130, 19, 266, 34, L"", false, true);
    add(ID_TAB, K_SEG, 134, 22, 65, 28, T(S_TAB_MAIN), s_page == 0, true);
    add(ID_TAB + 1, K_SEG, 199, 22, 65, 28, T(S_TAB_TIMES), s_page == 1, true);
    add(ID_TAB + 2, K_SEG, 264, 22, 65, 28, T(S_TAB_IQAMA), s_page == 2, true);
    add(ID_TAB + 3, K_SEG, 329, 22, 65, 28, T(S_TAB_LOOK), s_page == 3, true);
    if (s_page == 0) {
        bool man = !s_f.autoCity;
        add(0, K_LABEL, 24, 88, 372, 16, T(S_LANGUAGE), false, true);
        seg(ID_LANG, 108, {L"English", L"العربية", L"Русский"}, s_f.lang, true);

        add(0, K_LABEL, 24, 156, 372, 16, T(S_LOCATION), false, true);
        seg(ID_MODE, 176, {T(S_AUTO), T(S_MANUAL)}, man ? 1 : 0, true);
        add(0, K_CARD, 24, 218, 262, 36, L"", false, true);
        add(ID_FIND, K_BTN, 296, 218, 100, 36, T(S_FIND), false, man);
        add(0, K_STATUS, 24, 258, 372, 16, s_status, false, true);

        add(0, K_LABEL, 24, 288, 372, 16, T(S_ASR), false, true);
        seg(ID_ASR, 308, {T(S_STD), T(S_HANAFI)}, s_f.school, true);

        add(0, K_LABEL, 24, 356, 372, 16, T(S_NOTIF), false, true);
        add(ID_TEST, K_BTN, 296, 349, 100, 24, T(S_TEST), false, true);
        add(ID_NOTIF, K_TOGGLE, 24, 376, 372, 34, T(S_NOTIFON), s_f.notify, true);
        add(0, K_CAP, 24, 416, 372, 16, T(S_REMIND), false, true);
        for (int i = 0; i < 5; i++) {
            wchar_t b[32];
            if (i == 0) lstrcpynW(b, T(S_AT), 32); else _snwprintf(b, 32, T(S_LEAD), LEADS[i]);
            add(ID_LEAD + i, K_CHIP, 24 + i * 76, 434, 68, 30, b, i == s_f.leadIdx, s_f.notify);
        }
        add(ID_AUTORUN, K_TOGGLE, 24, 476, 372, 34, T(S_AUTORUN), s_f.autorun, true);
    } else if (s_page == 1) {
        add(0, K_LABEL, 24, 88, 372, 16, T(S_ADJ), false, true);
        add(0, K_CAP, 24, 108, 372, 16, T(S_ADJ_HINT), false, true);
        for (int i = 0; i < 6; i++) {
            int y = 132 + i * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 90, 38, PN[s_f.lang][i], false, true);
            add(0, K_CAP, 118, y, 70, 38, g_res.ok ? Fmt(g_res.sec[i] + s_f.off[i] * 60) : wstring(L"--:--"), false, true);
            add(ID_OFF + i * 2, K_STEP, 262, y + 5, 32, 28, L"−", false, true);
            wchar_t vb[32];
            if (s_f.off[i] == 0) lstrcpynW(vb, L"0", 32); else _snwprintf(vb, 32, L"%+d", s_f.off[i]);
            add(0, K_VAL, 298, y, 54, 38, vb, s_f.off[i] != 0, true);
            add(ID_OFF + i * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, true);
        }
        add(ID_RESET, K_BTN, 24, 404, 372, 36, T(S_RESET), false, true);
    } else if (s_page == 2) {
        add(ID_IQON, K_TOGGLE, 24, 88, 372, 34, T(S_IQ_ON), s_f.iqOn, true);
        add(0, K_CAP, 24, 130, 372, 16, T(S_IQ_HINT), false, true);
        for (int k = 0; k < 5; k++) {
            int i = IQI[k], y = 154 + k * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 120, 38, PN[s_f.lang][i], false, true);
            add(ID_IQ + k * 2, K_STEP, 232, y + 5, 32, 28, L"\u2212", false, s_f.iqOn);
            wchar_t vb[32];
            _snwprintf(vb, 32, T(S_LEAD), s_f.iq[i]);
            add(0, K_VAL, 266, y, 86, 38, vb, s_f.iq[i] > 0, true);
            add(ID_IQ + k * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, s_f.iqOn);
        }
    } else {
        const wchar_t* nm[3] = {T(S_W_FONT), T(S_W_OPAC), T(S_W_POS)};
        int val[3] = {s_f.wFont, s_f.wOpacity, s_f.wOffset};
        for (int k = 0; k < 3; k++) {
            int y = 88 + k * 44;
            add(0, K_CARD, 24, y, 372, 38, L"", false, true);
            add(0, K_NAME, 38, y, 150, 38, nm[k], false, true);
            add(ID_WS + k * 2, K_STEP, 232, y + 5, 32, 28, L"\u2212", false, true);
            wchar_t vb[32];
            _snwprintf(vb, 32, k == 0 ? L"%d pt" : (k == 1 ? L"%d%%" : L"%d px"), val[k]);
            add(0, K_VAL, 266, y, 86, 38, vb, true, true);
            add(ID_WS + k * 2 + 1, K_STEP, 356, y + 5, 32, 28, L"+", false, true);
        }
        add(0, K_LABEL, 24, 228, 372, 16, T(S_W_ALIGN), false, true);
        seg(ID_WAL, 250, {T(S_W_LEFT), T(S_W_CENTER), T(S_W_RIGHT)}, s_f.wAlign, true);
        add(0, K_LABEL, 24, 298, 372, 16, T(S_W_COLOR), false, true);
        for (int i = 0; i < 7; i++) add(ID_WC + i, K_SWATCH, 24 + i * 56, 320, 36, 36, i == 0 ? L"A" : L"", i == s_f.wColor, true);
        add(0, K_PREVIEW, 24, 376, 372, 56, L"", false, true);
        add(ID_WRESET, K_BTN, 24, 444, 372, 34, T(S_RESET), false, true);
    }
    add(ID_SAVE, K_PRIMARY, 24, 524, 372, 40, T(S_SAVE), false, true);
}
static void ToggleGeo11(const RECT& r, RECT& tk, RECT& tr) {
    int sw = SC(40), sh = SC(20), pad = SC(14), gap = SC(8);
    int sx = RTL() ? r.left + pad : r.right - pad - sw;
    int sy = (r.top + r.bottom - sh) / 2;
    tk.left = sx; tk.top = sy; tk.right = sx + sw; tk.bottom = sy + sh;
    tr = r;
    if (RTL()) { tr.left = r.left + pad + sw + gap; tr.right = r.right - pad; }
    else { tr.left = r.left + pad; tr.right = r.right - pad - sw - gap; }
}
static void ShapeItem11(const Item& it, bool hov) {
    const RECT& r = it.r;
    switch (it.kind) {
    case K_CARD: RR(r, 10, CCARD); break;
    case K_SWATCH: SwatchShape(it, hov); break;
    case K_PREVIEW: RR(r, 10, PreviewBar()); break;
    case K_SEG:
        if (it.on) RR(r, 7, hov ? CACCH : CACC);
        else if (hov && it.en) RR(r, 7, CCARDH);
        break;
    case K_CHIP: RR(r, 8, it.on && it.en ? (hov ? CACCH : CACC) : (hov && it.en ? CCARDH : CCARD)); break;
    case K_BTN: RR(r, 10, hov && it.en ? CCARDH : CCARD); break;
    case K_PRIMARY: RR(r, 10, hov ? CACCH : CACC); break;
    case K_STEP: RR(r, 8, hov ? CACC : CCARDH); break;
    case K_TOGGLE: {
        RR(r, 10, hov ? CCARDH : CCARD);
        RECT tk, tr;
        ToggleGeo11(r, tk, tr);
        int sh = tk.bottom - tk.top, sw = tk.right - tk.left;
        COLORREF off = g_dark ? RGB(95, 95, 104) : RGB(160, 160, 166);
        RRpx(tk, sh / 2.0f, it.on ? CACC : off);
        int kd = sh - SC(6);
        float kx = it.on ? (float)(tk.left + sw - kd - SC(3)) : (float)(tk.left + SC(3));
        Circle(kx + kd / 2.0f, tk.top + sh / 2.0f, kd / 2.0f, RGB(255, 255, 255));
        break;
    }
    }
}
static void TextItem11(HDC dc, const Item& it, bool hov) {
    const RECT& r = it.r;
    switch (it.kind) {
    case K_LABEL: Txt(dc, it.t, r, g_bold, CACCT, AL()); break;
    case K_CAP: Txt(dc, it.t, r, g_small, CDIM, AL()); break;
    case K_STATUS: Txt(dc, it.t, r, g_small, s_err ? CERR : CDIM, AL()); break;
    case K_SEG:
        if (it.on) Txt(dc, it.t, r, g_bold, RGB(255, 255, 255), DT_CENTER);
        else Txt(dc, it.t, r, g_ui, it.en ? CTXT : CDIS, DT_CENTER);
        break;
    case K_CHIP:
        if (it.on && it.en) Txt(dc, it.t, r, g_small, RGB(255, 255, 255), DT_CENTER);
        else Txt(dc, it.t, r, g_small, it.en ? CTXT : CDIS, DT_CENTER);
        break;
    case K_BTN: Txt(dc, it.t, r, g_bold, it.en ? CACCT : CDIS, DT_CENTER); break;
    case K_PRIMARY: Txt(dc, it.t, r, g_bold, RGB(255, 255, 255), DT_CENTER); break;
    case K_STEP: Txt(dc, it.t, r, g_bold, hov ? RGB(255, 255, 255) : CTXT, DT_CENTER); break;
    case K_NAME: Txt(dc, it.t, r, g_ui, CTXT, AL()); break;
    case K_SWATCH: if (!it.t.empty()) Txt(dc, it.t, r, g_bold, CTXT, DT_CENTER); break;
    case K_PREVIEW: PreviewText(dc, r); break;
    case K_VAL: Txt(dc, it.t, r, g_bold, it.on ? CACCT : CDIM, DT_CENTER); break;
    case K_TOGGLE: {
        RECT tk, tr;
        ToggleGeo11(r, tk, tr);
        Txt(dc, it.t, tr, g_ui, CTXT, AL());
        break;
    }
    }
}
// ---- выбор стиля по версии Windows
static void Build(vector<Item>& v) { if (g_w11) Build11(v); else Build10(v); }
static void ShapeItem(const Item& it, bool hov) { if (g_w11) ShapeItem11(it, hov); else ShapeItem10(it, hov); }
static void TextItem(HDC dc, const Item& it, bool hov) { if (g_w11) TextItem11(dc, it, hov); else TextItem10(dc, it, hov); }
static void PaintSettings(HWND hw) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hw, &ps);
    RECT rc; GetClientRect(hw, &rc);
    int W = rc.right, H = rc.bottom;
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    uint32_t* px = 0;
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bm = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&px, 0, 0);
    if (!bm || !px) {
        if (bm) DeleteObject(bm);
        DeleteDC(mem);
        EndPaint(hw, &ps);
        return;
    }
    HGDIOBJ old = SelectObject(mem, bm);
    s_px = px; s_W = W; s_H = H;
    PxFill(rc, CBG);
    if (g_w11) {
        RECT hd = rc; hd.bottom = SC(72);
        PxFill(hd, CHEAD);
        RECT ln = hd; ln.top = hd.bottom - SC(2);
        PxFill(ln, CACC);
    }
    vector<Item> v;
    Build(v);
    for (const Item& it : v) ShapeItem(it, it.id != 0 && it.id == s_hover);
    s_px = 0;
    SetBkMode(mem, TRANSPARENT);
    Txt(mem, T(S_SETTINGS), g_w11 ? MR(24, 0, 372, 70) : MR(24, 6, 372, 40), g_hdr, CTXT, AL());
    for (const Item& it : v) TextItem(mem, it, it.id != 0 && it.id == s_hover);
    BitBlt(dc, 0, 0, W, H, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bm);
    DeleteDC(mem);
    EndPaint(hw, &ps);
}
static int Hit(int x, int y) {
    vector<Item> v;
    Build(v);
    POINT p = {x, y};
    for (const Item& it : v)
        if (it.id && it.en && PtInRect(&it.r, p)) return it.id;
    return 0;
}
static void MakeEdit(HWND hw) {
    wchar_t b[128];
    if (s_edit) { GetWindowTextW(s_edit, b, 128); DestroyWindow(s_edit); }
    else lstrcpynW(b, ((s_f.autoCity && s_f.city.empty()) ? g_res.name : s_f.city).c_str(), 128);
    RECT r = g_w11 ? MR(36, 226, 238, 20) : MR(32, 227, 246, 20);
    DWORD st = WS_CHILD | (s_page == 0 ? WS_VISIBLE : 0) | ES_AUTOHSCROLL | (RTL() ? ES_RIGHT : 0);
    s_edit = CreateWindowEx(RTL() ? WS_EX_RTLREADING : 0, L"EDIT", b, st, r.left, r.top, r.right - r.left, r.bottom - r.top,
                            hw, (HMENU)1, GetModuleHandle(0), 0);
    SendMessage(s_edit, WM_SETFONT, (WPARAM)g_ui, TRUE);
    EnableWindow(s_edit, !s_f.autoCity);
}
static void ShowFound(const wstring& full) {
    wchar_t m[300];
    _snwprintf(m, 300, T(S_FOUND), full.c_str());
    s_status = m;
    s_err = false;
}
static bool DoFind(HWND hw) {
    wchar_t b[128];
    GetWindowTextW(s_edit, b, 128);
    wstring q = b;
    if (q.empty()) { s_status = T(S_NOTFOUND); s_err = true; InvalidateRect(hw, 0, FALSE); return false; }
    s_status = T(S_SEARCHING);
    s_err = false;
    InvalidateRect(hw, 0, FALSE);
    UpdateWindow(hw);
    SetCursor(LoadCursor(0, IDC_WAIT));
    wstring nm, cn; double la = 0, lo = 0;
    bool ok = Geocode(q, s_f.lang, nm, cn, la, lo);
    SetCursor(LoadCursor(0, IDC_ARROW));
    if (!ok) {
        s_pendOk = false; s_status = T(S_NOTFOUND); s_err = true;
        InvalidateRect(hw, 0, FALSE);
        return false;
    }
    s_pendOk = true; s_pendQuery = q; s_pendName = nm; s_pendLat = la; s_pendLon = lo;
    ShowFound(cn.empty() ? nm : nm + L", " + cn);
    InvalidateRect(hw, 0, FALSE);
    return true;
}
static void DoSave(HWND hw) {
    Cfg c = s_f;
    wchar_t b[128];
    GetWindowTextW(s_edit, b, 128);
    c.city = b;
    if (!c.autoCity) {
        bool usePend = s_pendOk && s_pendQuery == c.city;
        bool keep = !usePend && c.city == g_cfg.city && (g_cfg.lat != 0 || g_cfg.lon != 0) && !g_cfg.cityName.empty();
        if (!usePend && !keep) {
            if (!DoFind(hw)) return;
            usePend = true;
        }
        if (usePend) { c.lat = s_pendLat; c.lon = s_pendLon; c.cityName = s_pendName; }
    }
    g_cfg = c;
    SaveCfg();
    ApplyWidgetFont();
    g_wx = -32000;
    SetAutorun(c.autorun);
    g_res.ok = false;
    g_notified = -1;
    DestroyWindow(hw);
    StartLoad();
    UpdateLines();
    RefreshMain();
}
static void RebuildFonts() {
    HFONT old[4] = {g_ui, g_bold, g_hdr, g_small};
    g_ui = MkFont(10, FW_NORMAL);
    g_bold = MkFont(10, FW_SEMIBOLD);
    g_small = MkFont(9, FW_NORMAL);
    g_hdr = g_w11 ? MkFont(17, FW_SEMIBOLD) : MkFont(20, g_cfg.lang == 1 ? FW_NORMAL : FW_LIGHT);
    if (s_edit) SendMessage(s_edit, WM_SETFONT, (WPARAM)g_ui, TRUE);
    for (HFONT f : old) if (f) DeleteObject(f);
    HFONT w = g_font;
    g_font = MkFont(g_cfg.wFont, FW_NORMAL, ANTIALIASED_QUALITY);
    if (w) DeleteObject(w);
}
static void Click(HWND hw, int id) {
    if (id >= ID_LANG && id < ID_LANG + 3) {
        s_f.lang = id - ID_LANG;
        g_cfg.lang = s_f.lang;
        PutI(L"Language", g_cfg.lang);
        RebuildFonts();
        SetWindowTextW(hw, T(S_SETTINGS));
        MakeEdit(hw);
        UpdateLines();
        RefreshMain();
    } else if (id == ID_MODE || id == ID_MODE + 1) {
        s_f.autoCity = id == ID_MODE;
        EnableWindow(s_edit, !s_f.autoCity);
    } else if (id == ID_FIND) {
        DoFind(hw);
    } else if (id == ID_ASR || id == ID_ASR + 1) {
        s_f.school = id - ID_ASR;
    } else if (id == ID_NOTIF) {
        s_f.notify = !s_f.notify;
    } else if (id >= ID_LEAD && id < ID_LEAD + 5) {
        s_f.leadIdx = id - ID_LEAD;
    } else if (id == ID_TEST) {
        long long day; int cur, idx = 2, tg = 0;
        CityNow(day, cur);
        if (g_res.ok) NextPrayer(cur, idx, tg);
        ShowToast(idx, LEADS[s_f.leadIdx]);
    } else if (id == ID_AUTORUN) {
        s_f.autorun = !s_f.autorun;
    } else if (id == ID_IQON) {
        s_f.iqOn = !s_f.iqOn;
    } else if (id >= ID_IQ && id < ID_IQ + 10) {
        int i = IQI[(id - ID_IQ) / 2], d = ((id - ID_IQ) % 2) ? 1 : -1;
        s_f.iq[i] = Clamp(s_f.iq[i] + d, 0, 90);
    } else if (id >= ID_WS && id < ID_WS + 6) {
        int k = (id - ID_WS) / 2, d = ((id - ID_WS) % 2) ? 1 : -1;
        if (k == 0) s_f.wFont = Clamp(s_f.wFont + d, 8, 13);
        else if (k == 1) s_f.wOpacity = Clamp(s_f.wOpacity + d * 5, 30, 100);
        else s_f.wOffset = Clamp(s_f.wOffset + d * 10, 0, 800);
    } else if (id >= ID_WAL && id < ID_WAL + 3) {
        s_f.wAlign = id - ID_WAL;
    } else if (id >= ID_WC && id < ID_WC + 7) {
        s_f.wColor = id - ID_WC;
    } else if (id == ID_WRESET) {
        s_f.wFont = 9; s_f.wOpacity = 100; s_f.wOffset = 0; s_f.wAlign = 1; s_f.wColor = 0;
    } else if (id >= ID_TAB && id <= ID_TAB + 3) {
        s_page = id - ID_TAB;
        if (s_edit) ShowWindow(s_edit, s_page == 0 ? SW_SHOW : SW_HIDE);
    } else if (id >= ID_OFF && id < ID_OFF + 12) {
        int i = (id - ID_OFF) / 2, d = ((id - ID_OFF) % 2) ? 1 : -1;
        s_f.off[i] = Clamp(s_f.off[i] + d, -60, 60);
    } else if (id == ID_RESET) {
        for (int i = 0; i < 6; i++) s_f.off[i] = 0;
    } else if (id == ID_SAVE) {
        DoSave(hw);
        return;
    }
    InvalidateRect(hw, 0, FALSE);
}
static void DarkTitle(HWND hw) {
    typedef HRESULT(WINAPI * DwmSet)(HWND, DWORD, LPCVOID, DWORD);
    HMODULE d = LoadLibraryW(L"dwmapi.dll");
    if (!d) return;
    DwmSet f = (DwmSet)GetProcAddress(d, "DwmSetWindowAttribute");
    if (f) {
        BOOL v = g_dark ? TRUE : FALSE;
        if (FAILED(f(hw, 20, &v, sizeof v))) f(hw, 19, &v, sizeof v);
    }
    FreeLibrary(d);
}
static LRESULT CALLBACK SetProc(HWND hw, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_CREATE: DarkTitle(hw); MakeEdit(hw); return 0;
    case WM_PAINT: PaintSettings(hw); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_MOUSEMOVE: {
        int id = Hit((short)LOWORD(l), (short)HIWORD(l));
        if (!s_tracking) {
            TRACKMOUSEEVENT t = {sizeof t, TME_LEAVE, hw, 0};
            TrackMouseEvent(&t);
            s_tracking = true;
        }
        if (id != s_hover) { s_hover = id; InvalidateRect(hw, 0, FALSE); }
        return 0;
    }
    case WM_MOUSELEAVE:
        s_tracking = false;
        if (s_hover) { s_hover = 0; InvalidateRect(hw, 0, FALSE); }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(l) == HTCLIENT && s_hover) { SetCursor(LoadCursor(0, IDC_HAND)); return TRUE; }
        break;
    case WM_LBUTTONDOWN: {
        int id = Hit((short)LOWORD(l), (short)HIWORD(l));
        if (id) {
            Click(hw, id);
            if ((id >= ID_OFF && id < ID_OFF + 12) || (id >= ID_IQ && id < ID_IQ + 10) || (id >= ID_WS && id < ID_WS + 6)) { s_rep = id; SetCapture(hw); SetTimer(hw, 7, 400, 0); }  // удержание = повтор
        }
        return 0;
    }
    case WM_LBUTTONUP: case WM_CAPTURECHANGED:
        if (s_rep) { s_rep = 0; KillTimer(hw, 7); if (m == WM_LBUTTONUP) ReleaseCapture(); }
        return 0;
    case WM_TIMER:
        if (w == 7 && s_rep) { Click(hw, s_rep); SetTimer(hw, 7, 70, 0); }
        return 0;
    case WM_CTLCOLOREDIT: case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)w;
        SetBkColor(dc, CFIELD);
        SetTextColor(dc, IsWindowEnabled((HWND)l) ? CTXT : RGB(110, 110, 118));
        return (LRESULT)s_cardBr;
    }
    case WM_CLOSE: DestroyWindow(hw); return 0;
    case WM_DESTROY:
        if (hw == g_set) g_set = 0;
        s_edit = 0; s_hover = 0; s_tracking = false;
        DeleteObject(g_ui); DeleteObject(g_bold); DeleteObject(g_hdr); DeleteObject(g_small); DeleteObject(s_cardBr);
        g_ui = g_bold = g_hdr = g_small = 0; s_cardBr = 0;
        TrimMem();
        return 0;
    }
    return DefWindowProc(hw, m, w, l);
}
static void OpenSettings() {
    if (g_set && IsWindow(g_set)) { ShowWindow(g_set, SW_SHOW); SetForegroundWindow(g_set); return; }
    ApplyTheme();
    s_f = g_cfg;
    s_page = 0;
    s_rep = 0;
    s_pendOk = false;
    s_err = false;
    s_status = L"";
    if (!s_f.autoCity && !g_cfg.cityName.empty()) ShowFound(g_cfg.cityName);
    RebuildFonts();
    s_cardBr = CreateSolidBrush(CFIELD);
    DWORD st = WS_CAPTION | WS_SYSMENU;
    RECT r = {0, 0, SC(WIN_W), SC(WIN_H)};
    AdjustWindowRectEx(&r, st, FALSE, 0);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2, y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    if (y < 0) y = 0;
    g_set = CreateWindowEx(0, L"PrayerBarSettings", T(S_SETTINGS), st | WS_VISIBLE, x, y, w, h, 0, 0, GetModuleHandle(0), 0);
    if (g_set) SetForegroundWindow(g_set);
}

// ---------------------------------------------------------------- системное уведомление Windows
// Показывается обычным окошком Windows 10/11 (и попадает в центр уведомлений), со звуком и иконкой приложения.
static bool ShowToast(int idx, int lead) {
    wchar_t title[96], body[96];
    _snwprintf(title, 96, L"%ls  ·  %ls", PNm(idx), Fmt(PT(idx)).c_str());
    if (lead != 0) _snwprintf(body, 96, T(S_TOAST_SOON), lead);
    else if (idx == 2 && IsFriday()) lstrcpynW(body, T(S_TOAST_JUMA), 96);
    else if (idx == 4 && IsRamadan()) lstrcpynW(body, T(S_TOAST_IFTAR), 96);
    else if (idx == 0 && IsRamadan()) lstrcpynW(body, T(S_TOAST_FAST), 96);
    else lstrcpynW(body, T(S_TOAST_NOW), 96);
    NOTIFYICONDATAW n = g_nid;
    n.uFlags = NIF_INFO;
    n.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | NIIF_RESPECT_QUIET_TIME;
    n.hBalloonIcon = g_iconToast ? g_iconToast : (g_iconBig ? g_iconBig : g_icon);
    lstrcpynW(n.szInfoTitle, title, 64);
    lstrcpynW(n.szInfo, body, 256);
    return Shell_NotifyIconW(NIM_MODIFY, &n) != 0;
}

// ---------------------------------------------------------------- обновления
// Берётся только один файл PrayerBar.exe из релиза твоего репозитория на GitHub (по HTTPS),
// проверяется контрольная сумма SHA-256, никакие другие программы не скачиваются.
static const wchar_t* UPD_REPO = L"OWNER/REPO";  // имя репозитория вида логин/название (можно задать в реестре: UpdateRepo)
static wstring g_repo;
static bool g_updAuto = true, g_updBusy = false, g_updAvail = false;
static wstring g_updVer, g_updPage;
struct UpdReq { bool manual, install; };
struct UpdRes { int kind = 0; bool manual = false, install = false; wstring ver, page; };  // 0 ошибка, 1 свежая, 2 есть новая, 3 скачана и проверена
static bool UpdEnabled() { return !g_repo.empty() && g_repo != L"OWNER/REPO"; }

static void Balloon(const wchar_t* title, const wchar_t* body) {
    NOTIFYICONDATAW n = g_nid;
    n.uFlags = NIF_INFO;
    n.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | NIIF_RESPECT_QUIET_TIME;
    n.hBalloonIcon = g_iconToast ? g_iconToast : (g_iconBig ? g_iconBig : g_icon);
    lstrcpynW(n.szInfoTitle, title, 64);
    lstrcpynW(n.szInfo, body, 256);
    Shell_NotifyIconW(NIM_MODIFY, &n);
}
static wstring ExeDir() {
    wchar_t p[MAX_PATH];
    DWORD n = GetModuleFileNameW(0, p, MAX_PATH);
    wstring s(p, n);
    size_t k = s.find_last_of(L"\\/");
    return k == wstring::npos ? L"" : s.substr(0, k + 1);
}
static void CleanupUpdateFiles() {
    wstring d = ExeDir();
    DeleteFileW((d + L"PrayerBar.old.exe").c_str());
    DeleteFileW((d + L"PrayerBar.new.exe").c_str());
}
static bool HttpGetBin(const wstring& url, string& out, size_t maxSize, bool api) {
    wchar_t host[256], path[2048];
    URL_COMPONENTS uc = {};
    uc.dwStructSize = sizeof uc;
    uc.lpszHostName = host; uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path; uc.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc) || uc.nScheme != INTERNET_SCHEME_HTTPS) return false;  // только https
    HINTERNET s = WinHttpOpen(L"PrayerBar-Updater", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 0, 0, 0);
    if (!s) return false;
    WinHttpSetTimeouts(s, 10000, 10000, 30000, 30000);
    HINTERNET c = WinHttpConnect(s, host, uc.nPort, 0);
    HINTERNET r = c ? WinHttpOpenRequest(c, L"GET", path, 0, 0, 0, WINHTTP_FLAG_SECURE) : 0;
    bool ok = false;
    if (r) {
        const wchar_t* hdr = L"Accept: application/vnd.github+json\r\n";
        bool sent = api ? WinHttpSendRequest(r, hdr, (DWORD)-1L, 0, 0, 0, 0) : WinHttpSendRequest(r, 0, 0, 0, 0, 0, 0);
        if (sent && WinHttpReceiveResponse(r, 0)) {
            DWORD code = 0, sz = sizeof code;
            WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, 0, &code, &sz, 0);
            if (code == 200) {
                ok = true;
                char buf[16384];
                DWORD n;
                while (WinHttpReadData(r, buf, sizeof buf, &n) && n) {
                    out.append(buf, n);
                    if (out.size() > maxSize) { ok = false; break; }
                }
            }
        }
    }
    if (r) WinHttpCloseHandle(r);
    if (c) WinHttpCloseHandle(c);
    WinHttpCloseHandle(s);
    return ok;
}
static string Sha256Hex(const string& d) {
    BCRYPT_ALG_HANDLE ha = 0;
    BCRYPT_HASH_HANDLE hh = 0;
    UCHAR h[32];
    string hex;
    if (BCryptOpenAlgorithmProvider(&ha, BCRYPT_SHA256_ALGORITHM, 0, 0) == 0) {
        if (BCryptCreateHash(ha, &hh, 0, 0, 0, 0, 0) == 0) {
            if (BCryptHashData(hh, (PUCHAR)d.data(), (ULONG)d.size(), 0) == 0 && BCryptFinishHash(hh, h, 32, 0) == 0) {
                char b[3];
                for (int i = 0; i < 32; i++) { snprintf(b, 3, "%02x", h[i]); hex += b; }
            }
            BCryptDestroyHash(hh);
        }
        BCryptCloseAlgorithmProvider(ha, 0);
    }
    return hex;
}
static bool VerNewer(const string& tag, const wchar_t* cur) {
    int a[3] = {0, 0, 0}, b[3] = {0, 0, 0};
    const char* p = tag.c_str();
    if (*p == 'v' || *p == 'V') p++;
    sscanf(p, "%d.%d.%d", &a[0], &a[1], &a[2]);
    string cs = ToU(cur);
    sscanf(cs.c_str(), "%d.%d.%d", &b[0], &b[1], &b[2]);
    for (int i = 0; i < 3; i++)
        if (a[i] != b[i]) return a[i] > b[i];
    return false;
}
static DWORD WINAPI UpdThread(LPVOID p) {
    UpdReq q = *(UpdReq*)p;
    delete (UpdReq*)p;
    UpdRes* R = new UpdRes();
    R->manual = q.manual;
    R->install = q.install;
    string j;
    if (HttpGetBin(L"https://api.github.com/repos/" + g_repo + L"/releases/latest", j, 1 << 20, true)) {
        string tag = StrVal(j, "tag_name");
        R->kind = 1;
        if (!tag.empty() && VerNewer(tag, APP_VERSION)) {
            R->kind = 2;
            R->ver = ToW((tag[0] == 'v' || tag[0] == 'V') ? tag.substr(1) : tag);
            size_t hp = j.find("\"html_url\":\"");
            R->page = hp != string::npos ? ToW(j.substr(hp + 12, j.find('"', hp + 12) - hp - 12)) : (L"https://github.com/" + g_repo + L"/releases/latest");
            string url, sha;
            size_t np = j.find("\"name\":\"PrayerBar.exe\"");
            if (np != string::npos) {
                size_t up = j.find("\"browser_download_url\":\"", np);
                if (up != string::npos) {
                    string seg = j.substr(np, up - np);
                    up += 24;
                    url = j.substr(up, j.find('"', up) - up);
                    size_t dp = seg.find("\"digest\":\"sha256:");
                    if (dp != string::npos) sha = seg.substr(dp + 17, 64);
                }
            }
            if (q.install && !url.empty() && sha.size() == 64) {
                string bin;
                bool good = HttpGetBin(ToW(url), bin, 30u << 20, false) && bin.size() > 50000 && bin[0] == 'M' && bin[1] == 'Z' && Sha256Hex(bin) == sha;
                R->kind = 0;
                if (good) {
                    HANDLE f = CreateFileW((ExeDir() + L"PrayerBar.new.exe").c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
                    if (f != INVALID_HANDLE_VALUE) {
                        DWORD w = 0;
                        bool wr = WriteFile(f, bin.data(), (DWORD)bin.size(), &w, 0) && w == bin.size();
                        CloseHandle(f);
                        if (wr) R->kind = 3;
                    }
                }
            }
        }
    }
    PostMessage(g_ctl, WM_APP + 4, 0, (LPARAM)R);
    return 0;
}
static void StartUpdate(bool manual, bool install) {
    if (!UpdEnabled() || g_updBusy) return;
    g_updBusy = true;
    wchar_t t[32];
    _snwprintf(t, 32, L"%lld", (long long)time(0));
    PutS(L"UpdateLast", t);
    UpdReq* q = new UpdReq{manual, install};
    HANDLE h = CreateThread(0, 0, UpdThread, q, 0, 0);
    if (h) CloseHandle(h);
    else { g_updBusy = false; delete q; }
}
static void AutoCheck() {
    if (!UpdEnabled()) return;
    long long last = _wtoi64(GetS(L"UpdateLast", L"0").c_str());
    if ((long long)time(0) - last < 6 * 3600) return;
    StartUpdate(false, g_updAuto);
}
// Запущенный exe можно переименовать, но нельзя перезаписать: старый уходит в .old, новый встаёт на его место.
static bool ApplyUpdate() {
    wchar_t exe[MAX_PATH];
    DWORD n = GetModuleFileNameW(0, exe, MAX_PATH);
    wstring cur(exe, n), dir = ExeDir(), neu = dir + L"PrayerBar.new.exe", old = dir + L"PrayerBar.old.exe";
    DeleteFileW(old.c_str());
    if (!MoveFileExW(cur.c_str(), old.c_str(), 0)) return false;
    if (!MoveFileExW(neu.c_str(), cur.c_str(), 0)) { MoveFileExW(old.c_str(), cur.c_str(), 0); return false; }
    STARTUPINFOW si = {};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi = {};
    wstring cmd = L"\"" + cur + L"\" --updated";
    if (!CreateProcessW(cur.c_str(), &cmd[0], 0, 0, FALSE, 0, 0, dir.c_str(), &si, &pi)) {
        MoveFileExW(cur.c_str(), neu.c_str(), 0);
        MoveFileExW(old.c_str(), cur.c_str(), 0);
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

// ---------------------------------------------------------------- главное окно
static void ShowMenu(HWND h) {
    HMENU m = CreatePopupMenu();
    long long day; int cur, nx = -1, tg = 0;
    CityNow(day, cur);
    if (g_res.ok) NextPrayer(cur, nx, tg);
    if (IsRamadan()) {
        wchar_t rb[64];
        _snwprintf(rb, 64, T(S_RAMADAN), g_res.hd);
        AppendMenu(m, MF_STRING | MF_GRAYED, 0, rb);
        AppendMenu(m, MF_SEPARATOR, 0, 0);
    }
    for (int i = 0; i < 6; i++) {
        wstring s = wstring(PNm(i)) + L"\t" + (g_res.ok ? Fmt(PT(i)) : wstring(L"--:--"));
        AppendMenu(m, MF_STRING | (i == nx ? MF_CHECKED : 0), 100 + i, s.c_str());
    }
    AppendMenu(m, MF_SEPARATOR, 0, 0);
    if (UpdEnabled()) {
        if (g_updAvail) {
            wchar_t ub[96];
            _snwprintf(ub, 96, T(S_UPD_TO), g_updVer.c_str());
            AppendMenu(m, MF_STRING, 6, ub);
        }
        AppendMenu(m, MF_STRING, 4, T(S_UPD_CHECK));
        AppendMenu(m, MF_STRING | (g_updAuto ? MF_CHECKED : 0), 5, T(S_UPD_AUTO));
        AppendMenu(m, MF_SEPARATOR, 0, 0);
    }
    AppendMenu(m, MF_STRING, 1, T(S_SETTINGS));
    AppendMenu(m, MF_STRING, 2, T(S_REFRESH));
    AppendMenu(m, MF_STRING, 3, T(S_EXIT));
    POINT pt; GetCursorPos(&pt);
    SetForegroundWindow(h);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_RIGHTALIGN | (g_cfg.lang == 1 ? TPM_LAYOUTRTL : 0), pt.x, pt.y, 0, h, 0);
    DestroyMenu(m);
    if (c == 1) OpenSettings();
    if (c == 4) StartUpdate(true, g_updAuto);
    if (c == 5) { g_updAuto = !g_updAuto; PutI(L"UpdateAuto", g_updAuto); }
    if (c == 6) StartUpdate(true, true);
    if (c == 2) StartLoad();
    if (c == 3) DestroyWindow(h);
}
static void CALLBACK WinEv(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD) {
    if (g_ctl) PostMessage(g_ctl, WM_APP + 3, 0, 0);
}
static LRESULT CALLBACK ViewProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_ERASEBKGND: return 1;
    case WM_LBUTTONUP: case WM_RBUTTONUP: ShowMenu(g_ctl); return 0;
    case WM_DESTROY: if (h == g_main) g_main = 0; return 0;  // например, при перезапуске проводника
    }
    return DefWindowProc(h, m, w, l);
}
static LRESULT CALLBACK CtlProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (g_taskbarMsg && m == g_taskbarMsg) { AddTray(h); g_wx = -32000; RefreshMain(); return 0; }
    switch (m) {
    case WM_TIMER: {
        g_tick++;
        if (g_tick == 20 || g_tick % 86400 == 20) AutoCheck();
        if (g_tick == 40) CleanupUpdateFiles();
        long long day; int cur;
        CityNow(day, cur);
        bool need = !g_res.ok || day != g_res.day;
        if (need && !g_loading && g_tick % 30 == 0) StartLoad();
        EnsureMain();
        SyncVisibility();
        bool ch = false;
        if (g_tick % 10 == 0) { bool was = g_light; ReadTheme(); if (was != g_light) ch = true; }
        CheckNotify();
        if (UpdateLines()) ch = true;
        if (!g_hidden && g_main) {
            if (g_tick % 2 == 0 || ch) { if (Layout(g_main)) ch = true; }
            if (ch) RenderMain();
        }
        if (g_tick % 120 == 0) TrimMem();
        return 0;
    }
    case WM_APP + 1: {
        Res* R = (Res*)l;
        g_loading = false;
        if (R->ok) g_res = *R;
        delete R;
        if (g_again) { g_again = false; StartLoad(); }
        RefreshMain();
        TrimMem();
        return 0;
    }
    case WM_APP + 4: {
        UpdRes* R = (UpdRes*)l;
        g_updBusy = false;
        wchar_t b[200];
        if (R->kind == 3) {
            if (ApplyUpdate()) { delete R; DestroyWindow(h); return 0; }
            Balloon(T(S_UPD_FAIL), g_repo.c_str());
        } else if (R->kind == 2) {
            g_updAvail = true; g_updVer = R->ver; g_updPage = R->page;
            if (R->install) ShellExecuteW(0, L"open", R->page.c_str(), 0, 0, SW_SHOWNORMAL);  // без контрольной суммы сами не ставим
            else { _snwprintf(b, 200, T(S_UPD_AVB), R->ver.c_str()); Balloon(T(S_UPD_AVT), b); }
        } else if (R->kind == 1) {
            g_updAvail = false;
            if (R->manual) Balloon(T(S_UPD_LATEST), (wstring(L"PrayerBar ") + APP_VERSION).c_str());
        } else if (R->manual) Balloon(T(S_UPD_FAIL), g_repo.c_str());
        delete R;
        return 0;
    }
    case WM_APP + 3: SyncVisibility(); return 0;
    case WM_DISPLAYCHANGE: case WM_SETTINGCHANGE: case WM_POWERBROADCAST:
        g_wx = -32000;
        ReadTheme();
        RefreshMain();
        return m == WM_POWERBROADCAST ? TRUE : 0;
    case WM_APP + 2:
        if (LOWORD(l) == WM_LBUTTONUP || LOWORD(l) == WM_RBUTTONUP) ShowMenu(h);
        return 0;
    case WM_DESTROY:
        if (g_main) DestroyWindow(g_main);
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        if (g_hook) UnhookWinEvent(g_hook);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR cmd, int) {
    bool justUpdated = cmd && wcsstr(cmd, L"--updated");
    HANDLE mtx = CreateMutexW(0, TRUE, L"PrayerBarSingleInstance");
    for (int i = 0; i < 50 && justUpdated && GetLastError() == ERROR_ALREADY_EXISTS; i++) {  // ждём, пока закроется старая копия
        if (mtx) CloseHandle(mtx);
        Sleep(200);
        mtx = CreateMutexW(0, TRUE, L"PrayerBarSingleInstance");
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
    SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);  // низкий приоритет CPU/диска/памяти
    {  // тот же режим DPI, что у проводника: иначе Windows не даст встроить окно в панель задач
        typedef BOOL(WINAPI * SetCtx)(HANDLE);
        SetCtx f = (SetCtx)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext");
        if (!(f && f((HANDLE)-4))) SetProcessDPIAware();
    }
    DetectOS();
    g_hi = hi;
    HDC d = GetDC(0);
    g_dpi = GetDeviceCaps(d, LOGPIXELSY);
    ReleaseDC(0, d);
    LoadCfg();
    g_repo = GetS(L"UpdateRepo", UPD_REPO);
    g_updAuto = GetI(L"UpdateAuto", 1) != 0;
    ReadTheme();
    g_font = MkFont(g_cfg.wFont, FW_NORMAL, ANTIALIASED_QUALITY);
    g_icon = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    g_iconBig = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    g_iconToast = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);  // крупная иконка для уведомлений
    if (!g_icon) g_icon = MakeIcon(GetSystemMetrics(SM_CXSMICON));

    WNDCLASS wc = {};
    wc.lpfnWndProc = ViewProc;
    wc.hInstance = hi;
    wc.lpszClassName = L"PrayerBar";
    wc.hCursor = LoadCursor(0, IDC_HAND);
    RegisterClass(&wc);
    WNDCLASS cc = {};
    cc.lpfnWndProc = CtlProc;
    cc.hInstance = hi;
    cc.lpszClassName = L"PrayerBarCtl";
    RegisterClass(&cc);
    WNDCLASS sc = {};
    sc.lpfnWndProc = SetProc;
    sc.hInstance = hi;
    sc.lpszClassName = L"PrayerBarSettings";
    sc.hCursor = LoadCursor(0, IDC_ARROW);
    sc.hIcon = g_iconBig ? g_iconBig : (g_icon ? g_icon : LoadIcon(0, IDI_APPLICATION));
    RegisterClass(&sc);

    g_taskbarMsg = RegisterWindowMessage(L"TaskbarCreated");
    g_ctl = CreateWindowEx(WS_EX_TOOLWINDOW, L"PrayerBarCtl", L"", WS_POPUP, 0, 0, 0, 0, 0, 0, hi, 0);
    AddTray(g_ctl);
    if (justUpdated) {
        wchar_t ub[96];
        _snwprintf(ub, 96, T(S_UPD_OK), APP_VERSION);
        Balloon(ub, L"PrayerBar");
    }
    RefreshMain();
    g_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0, WinEv, 0, 0, WINEVENT_OUTOFCONTEXT);
    StartLoad();
    SetTimer(g_ctl, 1, 1000, 0);
    TrimMem();

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (mtx) CloseHandle(mtx);
    return 0;
}
