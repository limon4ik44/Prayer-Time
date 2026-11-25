import Cocoa
import UserNotifications

struct NowInfo {
    var sod: Int
    var weekday: Int
    var key: String
    var y: Int
    var m: Int
    var d: Int
}

final class Engine {
    static let shared = Engine()
    var base: [Int]? = nil
    var tz = TimeZone.current
    var hijriMonth = 0
    var hijriDay = 0
    var lat = 0.0
    var lon = 0.0
    var cityLabel = ""
    var dayKey = ""
    var fetching = false
    var fresh = false
    var ticks = 0
    var fired = Set<String>()
    var statusItem: NSStatusItem?
    var lastBadge = ""
    var dockView: DockView? = nil
    var onStatus: ((String) -> Void)? = nil

    func cal() -> Calendar {
        var c = Calendar(identifier: .gregorian)
        c.timeZone = tz
        return c
    }

    func now() -> NowInfo {
        let p = cal().dateComponents([.year, .month, .day, .hour, .minute, .second, .weekday], from: Date())
        let sod = (p.hour ?? 0) * 3600 + (p.minute ?? 0) * 60 + (p.second ?? 0)
        let key = String(format: "%04d-%02d-%02d", p.year ?? 0, p.month ?? 0, p.day ?? 0)
        return NowInfo(sod: sod, weekday: p.weekday ?? 1, key: key, y: p.year ?? 0, m: p.month ?? 0, d: p.day ?? 0)
    }

    func times() -> [Int] {
        let o = Prefs.off
        let b = base ?? [Int](repeating: 0, count: 6)
        return (0..<6).map { b[$0] + o[$0] * 60 }
    }

    func isRamadan() -> Bool { hijriMonth == 9 }

    func name(_ i: Int, weekday: Int) -> String {
        let l = Prefs.lang
        if i == 2 && weekday == 6 { return JUMAH[l] }
        if hijriMonth == 9 {
            if i == 4 { return IFTAR[l] }
            if i == 0 { return SUHOOR[l] }
        }
        return PN[l][i]
    }

    func resolveLocation(_ done: @escaping (Double?, Double?, String) -> Void) {
        if Prefs.auto {
            getJSON("http://ip-api.com/json/?fields=status,city,lat,lon") { j in
                if let o = j as? [String: Any], (o["status"] as? String) == "success",
                   let la = o["lat"] as? Double, let lo = o["lon"] as? Double {
                    done(la, lo, (o["city"] as? String) ?? "")
                } else {
                    done(nil, nil, "")
                }
            }
        } else if Prefs.lat != 0 || Prefs.lon != 0 {
            done(Prefs.lat, Prefs.lon, Prefs.cityName)
        } else {
            done(nil, nil, "")
        }
    }

    func reload() {
        if fetching { return }
        fetching = true
        resolveLocation { la, lo, nm in
            guard let la = la, let lo = lo else {
                self.fetching = false
                self.tick()
                return
            }
            let u = "https://api.open-meteo.com/v1/forecast?latitude=\(la)&longitude=\(lo)&current=temperature_2m&timezone=auto"
            getJSON(u) { j in
                var zone = TimeZone.current
                if let o = j as? [String: Any], let id = o["timezone"] as? String, let z = TimeZone(identifier: id) { zone = z }
                self.fetchTimings(la, lo, nm, zone)
            }
        }
    }

    func fetchTimings(_ la: Double, _ lo: Double, _ nm: String, _ zone: TimeZone) {
        let f = DateFormatter()
        f.locale = Locale(identifier: "en_US_POSIX")
        f.timeZone = zone
        f.dateFormat = "dd-MM-yyyy"
        let ds = f.string(from: Date())
        let u = "https://api.aladhan.com/v1/timings/\(ds)?latitude=\(la)&longitude=\(lo)&school=\(Prefs.school)"
        getJSON(u) { j in
            self.fetching = false
            guard let o = j as? [String: Any], let data = o["data"] as? [String: Any],
                  let t = data["timings"] as? [String: Any] else {
                self.tick()
                return
            }
            var s = [Int]()
            for k in KEYS {
                guard let str = t[k] as? String, let v = parseHM(str) else {
                    self.tick()
                    return
                }
                s.append(v)
            }
            if let dt = data["date"] as? [String: Any], let h = dt["hijri"] as? [String: Any] {
                if let m = h["month"] as? [String: Any], let n = m["number"] as? Int { self.hijriMonth = n }
                if let dd = h["day"] as? String, let n = Int(dd) { self.hijriDay = n }
            }
            self.base = s
            self.tz = zone
            self.lat = la
            self.lon = lo
            self.cityLabel = nm
            self.fresh = true
            self.dayKey = self.now().key
            self.saveCache()
            self.tick()
        }
    }

    func saveCache() {
        guard let b = base else { return }
        let c: [String: Any] = ["base": b, "tz": tz.identifier, "hm": hijriMonth, "hd": hijriDay,
                                "lat": lat, "lon": lon, "name": cityLabel, "key": dayKey,
                                "auto": Prefs.auto, "school": Prefs.school]
        UserDefaults.standard.set(c, forKey: "cache")
    }

    func loadCache() {
        guard let c = UserDefaults.standard.dictionary(forKey: "cache"),
              let b = c["base"] as? [Int], b.count == 6,
              let id = c["tz"] as? String, let z = TimeZone(identifier: id),
              (c["auto"] as? Bool) == Prefs.auto, (c["school"] as? Int) == Prefs.school else { return }
        let la = (c["lat"] as? Double) ?? 0
        let lo = (c["lon"] as? Double) ?? 0
        if !Prefs.auto && (abs(la - Prefs.lat) > 0.001 || abs(lo - Prefs.lon) > 0.001) { return }
        tz = z
        if (c["key"] as? String) != now().key { return }
        base = b
        hijriMonth = (c["hm"] as? Int) ?? 0
        hijriDay = (c["hd"] as? Int) ?? 0
        lat = la
        lon = lo
        cityLabel = (c["name"] as? String) ?? ""
        dayKey = now().key
    }

    func resetData() {
        base = nil
        fresh = false
        UserDefaults.standard.removeObject(forKey: "cache")
        tick()
        reload()
    }

    struct Summary {
        var idx: Int
        var name: String
        var time: Int
        var rem: Int
        var iqama: Bool
    }

    func summary() -> Summary? {
        guard base != nil else { return nil }
        let n = now()
        let t = times()
        let q = Prefs.iq
        if Prefs.iqOn {
            for i in [0, 2, 3, 4, 5] where q[i] > 0 && n.sod >= t[i] && n.sod < t[i] + q[i] * 60 {
                return Summary(idx: i, name: name(i, weekday: n.weekday), time: t[i], rem: t[i] + q[i] * 60 - n.sod, iqama: true)
            }
        }
        for i in [0, 2, 3, 4, 5] where t[i] > n.sod {
            return Summary(idx: i, name: name(i, weekday: n.weekday), time: t[i], rem: t[i] - n.sod, iqama: false)
        }
        return Summary(idx: 0, name: name(0, weekday: n.weekday % 7 + 1), time: t[0], rem: t[0] + 86400 - n.sod, iqama: false)
    }

    func tick() {
        ticks += 1
        guard let btn = statusItem?.button else { return }
        btn.font = NSFont.monospacedDigitSystemFont(ofSize: 13, weight: .regular)
        guard let s = summary() else {
            btn.title = fetching ? tr("loading") : tr("offline")
            updateDock("")
            retry()
            return
        }
        let n = now()
        if n.key != dayKey {
            dayKey = n.key
            fresh = false
            reload()
        }
        let head = "\(s.name) \(fmtTime(s.time))"
        if s.iqama {
            btn.title = "\(head) · \(IQAMA_W[Prefs.lang]) \(fmtCount(s.rem))"
        } else {
            btn.title = "\(head) · \(fmtCount(s.rem))"
        }
        updateDock(head)
        checkNotify(n, times())
        retry()
    }

    func retry() {
        if fetching { return }
        let every = (fresh || ticks > 300) ? 30 : 5
        if ticks % every == 0 && (!fresh || base == nil) { reload() }
    }

    func updateDock(_ s: String) {
        let tile = NSApp.dockTile
        if Prefs.dock {
            if dockView == nil { dockView = DockView(frame: NSRect(x: 0, y: 0, width: 128, height: 128)) }
            if tile.contentView !== dockView { tile.contentView = dockView }
            if s != lastBadge {
                dockView?.text = s
                lastBadge = s
                tile.display()
            }
        } else if tile.contentView != nil {
            tile.contentView = nil
            tile.display()
            lastBadge = ""
        }
    }

    func applyDock() {
        NSApp.setActivationPolicy(Prefs.dock ? .regular : .accessory)
        applyIcon()
        lastBadge = "?"
        tick()
    }

    func applyIcon() {
        if let p = Bundle.main.path(forResource: "icon\(Prefs.icon + 1)", ofType: "png"), let img = NSImage(contentsOfFile: p) {
            NSApp.applicationIconImage = img
        }
    }

    func checkNotify(_ n: NowInfo, _ t: [Int]) {
        guard Prefs.notify else { return }
        let lead = LEADS[Prefs.lead]
        for i in [0, 2, 3, 4, 5] {
            let trig = t[i] - lead * 60
            if n.sod >= trig && n.sod < trig + 60 {
                let k = "\(n.key)-\(i)-\(lead)"
                if fired.contains(k) { continue }
                fired.insert(k)
                send(i, lead, n.weekday, t[i])
            }
        }
    }

    func send(_ i: Int, _ lead: Int, _ weekday: Int, _ time: Int) {
        let c = UNMutableNotificationContent()
        c.title = "\(name(i, weekday: weekday)) · \(fmtTime(time))"
        if lead > 0 {
            c.body = String(format: tr("toastSoon"), lead)
        } else if hijriMonth == 9 && i == 4 {
            c.body = tr("toastIftar")
        } else if hijriMonth == 9 && i == 0 {
            c.body = tr("toastFast")
        } else {
            c.body = tr("toastNow")
        }
        c.sound = .default
        let r = UNNotificationRequest(identifier: UUID().uuidString, content: c, trigger: nil)
        UNUserNotificationCenter.current().add(r, withCompletionHandler: nil)
    }

    func sendTest() {
        let n = now()
        let t = times()
        send(3, 0, n.weekday, t[3])
    }
}

final class DockView: NSView {
    var text = ""
    override func draw(_ dirtyRect: NSRect) {
        if let img = NSApp.applicationIconImage {
            img.draw(in: bounds, from: .zero, operation: .sourceOver, fraction: 1)
        }
        if text.isEmpty { return }
        let h = bounds.height * 0.30
        let pill = NSRect(x: bounds.width * 0.06, y: bounds.height * 0.03, width: bounds.width * 0.88, height: h)
        NSColor(calibratedWhite: 0, alpha: 0.78).setFill()
        NSBezierPath(roundedRect: pill, xRadius: h / 2, yRadius: h / 2).fill()
        var size = h * 0.55
        var str = NSAttributedString()
        repeat {
            let attrs: [NSAttributedString.Key: Any] = [.font: NSFont.systemFont(ofSize: size, weight: .semibold), .foregroundColor: NSColor.white]
            str = NSAttributedString(string: text, attributes: attrs)
            size -= 1
        } while str.size().width > pill.width - 16 && size > 6
        let sz = str.size()
        str.draw(at: NSPoint(x: pill.midX - sz.width / 2, y: pill.midY - sz.height / 2))
    }
}
