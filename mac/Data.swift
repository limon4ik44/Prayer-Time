import Cocoa
import UserNotifications
import ServiceManagement

let appVersion = "1.0.0"
let updateRepo = "limon4ik44/Prayer-Time"

let PN: [[String]] = [
    ["Fajr", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"],
    ["الفجر", "الشروق", "الظهر", "العصر", "المغرب", "العشاء"],
    ["Фаджр", "Восход", "Зухр", "Аср", "Магриб", "Иша"]]
let JUMAH = ["Jumu'ah", "الجمعة", "Джума"]
let IFTAR = ["Iftar", "الإفطار", "Ифтар"]
let SUHOOR = ["Suhoor ends", "الإمساك", "Конец сухура"]
let IQAMA_W = ["Iqama", "الإقامة", "Икама"]
let MONTHS: [[String]] = [
    ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
    ["يناير", "فبراير", "مارس", "أبريل", "مايو", "يونيو", "يوليو", "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر"],
    ["Январь", "Февраль", "Март", "Апрель", "Май", "Июнь", "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"]]
let WEEKDAYS: [[String]] = [
    ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"],
    ["الأحد", "الاثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت"],
    ["Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"]]
let KEYS = ["Fajr", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"]
let LEADS = [0, 5, 10, 15, 30]

let STR: [String: [String]] = [
    "settings": ["Settings…", "الإعدادات…", "Настройки…"],
    "schedule": ["Monthly schedule…", "جدول الشهر…", "Расписание на месяц…"],
    "quit": ["Quit", "خروج", "Выход"],
    "refresh": ["Refresh", "تحديث", "Обновить"],
    "updCheck": ["Check for updates", "التحقق من التحديثات", "Проверить обновления"],
    "updAvail": ["Update available: %@", "تحديث متوفر: %@", "Доступно обновление: %@"],
    "updLatest": ["You have the latest version", "لديك أحدث إصدار", "У вас последняя версия"],
    "updFail": ["Could not check for updates", "تعذر التحقق من التحديثات", "Не удалось проверить обновления"],
    "ramadan": ["Ramadan, day %d", "رمضان، اليوم %d", "Рамадан, день %d"],
    "loading": ["Loading…", "جار التحميل…", "Загрузка…"],
    "offline": ["No data", "لا توجد بيانات", "Нет данных"],
    "tabGeneral": ["General", "عام", "Общее"],
    "tabTimes": ["Times", "الأوقات", "Время"],
    "tabIqama": ["Iqama", "الإقامة", "Икама"],
    "language": ["Language", "اللغة", "Язык"],
    "location": ["Location", "الموقع", "Местоположение"],
    "auto": ["Automatic", "تلقائي", "Авто"],
    "manual": ["Manual", "يدوي", "Вручную"],
    "city": ["City", "المدينة", "Город"],
    "find": ["Find", "بحث", "Найти"],
    "found": ["Found: %@", "تم العثور: %@", "Найдено: %@"],
    "notFound": ["City not found", "المدينة غير موجودة", "Город не найден"],
    "asr": ["Asr calculation", "حساب العصر", "Расчёт Аср"],
    "standard": ["Standard", "قياسي", "Стандарт"],
    "hanafi": ["Hanafi", "حنفي", "Ханафитский"],
    "notify": ["Notifications", "الإشعارات", "Уведомления"],
    "atTime": ["At prayer time", "عند الأذان", "В момент азана"],
    "before": ["%d min before", "قبل %d دقيقة", "За %d мин"],
    "test": ["Send test notification", "إرسال إشعار تجريبي", "Проверить уведомление"],
    "fmt12": ["12-hour time", "نظام 12 ساعة", "12-часовой формат"],
    "autostart": ["Launch at login", "التشغيل عند بدء النظام", "Запускать при входе"],
    "dock": ["Show next prayer in the Dock", "عرض الصلاة القادمة في Dock", "Показывать ближайший намаз в Dock"],
    "icon": ["Icon", "الأيقونة", "Значок"],
    "iqamaOn": ["Show countdown to iqama", "عرض العد التنازلي للإقامة", "Показывать отсчёт до икамы"],
    "iqamaHint": ["Minutes from adhan to iqama", "الدقائق من الأذان إلى الإقامة", "Минуты от азана до икамы"],
    "timesHint": ["Shift each prayer by some minutes", "إزاحة كل صلاة بعدد من الدقائق", "Сдвиг каждого намаза в минутах"],
    "reset": ["Reset", "إعادة", "Сбросить"],
    "min": ["min", "د", "мин"],
    "schedTitle": ["Monthly schedule", "جدول الشهر", "Расписание на месяц"],
    "day": ["Day", "اليوم", "День"],
    "toastNow": ["Time to pray", "حان وقت الصلاة", "Время намаза"],
    "toastSoon": ["In %d min", "بعد %d دقيقة", "Через %d мин"],
    "toastIftar": ["Time to break the fast", "حان وقت الإفطار", "Время ифтара"],
    "toastFast": ["Fasting begins", "بدأ الإمساك", "Начало поста"],
    "ok": ["OK", "حسنًا", "OK"],
    "open": ["Open", "فتح", "Открыть"],
    "remind": ["Remind", "التذكير", "Напомнить"],
    "next": ["Next prayer", "الصلاة القادمة", "Следующий намаз"],
    "iqamaIn": ["Iqama in", "الإقامة بعد", "До икамы"],
    "untilAdhan": ["Until adhan", "حتى الأذان", "До азана"],
]
func tr(_ k: String) -> String { STR[k]?[Prefs.lang] ?? k }

enum Prefs {
    static let d = UserDefaults.standard
    static func defaultLang() -> Int {
        let l = Locale.preferredLanguages.first ?? "en"
        if l.hasPrefix("ru") { return 2 }
        if l.hasPrefix("ar") { return 1 }
        return 0
    }
    static var lang: Int {
        get { (d.object(forKey: "lang") as? Int) ?? defaultLang() }
        set { d.set(newValue, forKey: "lang") }
    }
    static var auto: Bool {
        get { (d.object(forKey: "auto") as? Bool) ?? true }
        set { d.set(newValue, forKey: "auto") }
    }
    static var cityName: String {
        get { d.string(forKey: "cityName") ?? "" }
        set { d.set(newValue, forKey: "cityName") }
    }
    static var lat: Double {
        get { d.double(forKey: "lat") }
        set { d.set(newValue, forKey: "lat") }
    }
    static var lon: Double {
        get { d.double(forKey: "lon") }
        set { d.set(newValue, forKey: "lon") }
    }
    static var school: Int {
        get { d.integer(forKey: "school") }
        set { d.set(newValue, forKey: "school") }
    }
    static var notify: Bool {
        get { (d.object(forKey: "notify") as? Bool) ?? true }
        set { d.set(newValue, forKey: "notify") }
    }
    static var lead: Int {
        get { min(max(d.integer(forKey: "lead"), 0), LEADS.count - 1) }
        set { d.set(newValue, forKey: "lead") }
    }
    static var iqOn: Bool {
        get { (d.object(forKey: "iqOn") as? Bool) ?? true }
        set { d.set(newValue, forKey: "iqOn") }
    }
    static var iq: [Int] {
        get {
            if let a = d.array(forKey: "iq") as? [Int], a.count == 6 { return a }
            return [25, 0, 20, 20, 10, 20]
        }
        set { d.set(newValue, forKey: "iq") }
    }
    static var off: [Int] {
        get {
            if let a = d.array(forKey: "off") as? [Int], a.count == 6 { return a }
            return [0, 0, 0, 0, 0, 0]
        }
        set { d.set(newValue, forKey: "off") }
    }
    static var fmt12: Bool {
        get { d.bool(forKey: "fmt12") }
        set { d.set(newValue, forKey: "fmt12") }
    }
    static var dock: Bool {
        get { d.bool(forKey: "dock") }
        set { d.set(newValue, forKey: "dock") }
    }
    static var icon: Int {
        get { min(max(d.integer(forKey: "icon"), 0), 5) }
        set { d.set(newValue, forKey: "icon") }
    }
}

func getJSON(_ s: String, _ done: @escaping (Any?) -> Void) {
    guard let url = URL(string: s) else { done(nil); return }
    var req = URLRequest(url: url)
    req.timeoutInterval = 8
    req.setValue("application/json", forHTTPHeaderField: "Accept")
    URLSession.shared.dataTask(with: req) { data, _, _ in
        var obj: Any? = nil
        if let data = data { obj = try? JSONSerialization.jsonObject(with: data) }
        DispatchQueue.main.async { done(obj) }
    }.resume()
}

func parseHM(_ s: String) -> Int? {
    let p = s.prefix(5).split(separator: ":")
    guard p.count == 2, let h = Int(p[0]), let m = Int(p[1]) else { return nil }
    return h * 3600 + m * 60
}

func fmtTime(_ sec: Int) -> String {
    let s = ((sec % 86400) + 86400) % 86400
    let h = s / 3600, m = (s / 60) % 60
    if Prefs.fmt12 {
        let ap = [["AM", "PM"], ["ص", "م"], ["AM", "PM"]][Prefs.lang]
        var h12 = h % 12
        if h12 == 0 { h12 = 12 }
        return String(format: "%d:%02d %@", h12, m, ap[h >= 12 ? 1 : 0])
    }
    return String(format: "%02d:%02d", h, m)
}

func fmtCount(_ s: Int) -> String {
    let v = max(s, 0)
    if v >= 3600 { return String(format: "%d:%02d:%02d", v / 3600, (v / 60) % 60, v % 60) }
    return String(format: "%02d:%02d", v / 60, v % 60)
}

