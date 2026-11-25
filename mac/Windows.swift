import Cocoa

final class MonthWindow: NSObject, NSTableViewDataSource, NSTableViewDelegate {
    static let shared = MonthWindow()
    var window: NSWindow?
    let table = NSTableView()
    let titleLabel = NSTextField(labelWithString: "")
    var prevBtn = NSButton()
    var nextBtn = NSButton()
    var year = 0
    var month = 0
    var baseY = 0
    var baseM = 0
    var rows: [[Int]] = []
    var cache = [Int: [[Int]]]()
    var todayY = 0
    var todayM = 0
    var todayD = 0
    let range = 6

    func offset() -> Int { (year - baseY) * 12 + (month - baseM) }

    func show() {
        let e = Engine.shared
        let n = e.now()
        year = n.y
        month = n.m
        baseY = n.y
        baseM = n.m
        todayY = n.y
        todayM = n.m
        todayD = n.d
        cache.removeAll()
        rows = []
        if let w = window { w.close() }
        let vis = NSScreen.main?.visibleFrame.height ?? 800
        let h = min(CGFloat(700), vis - 40)
        let w = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 580, height: h),
                         styleMask: [.titled, .closable, .miniaturizable], backing: .buffered, defer: false)
        w.title = tr("schedTitle")
        w.isReleasedWhenClosed = false
        let content = NSView(frame: NSRect(x: 0, y: 0, width: 580, height: h))
        prevBtn = NSButton(title: "‹", target: self, action: #selector(prevTapped))
        prevBtn.frame = NSRect(x: 16, y: h - 40, width: 44, height: 28)
        nextBtn = NSButton(title: "›", target: self, action: #selector(nextTapped))
        nextBtn.frame = NSRect(x: 580 - 60, y: h - 40, width: 44, height: 28)
        titleLabel.frame = NSRect(x: 70, y: h - 38, width: 440, height: 24)
        titleLabel.alignment = .center
        titleLabel.font = NSFont.boldSystemFont(ofSize: 16)
        let scroll = NSScrollView(frame: NSRect(x: 0, y: 0, width: 580, height: h - 52))
        scroll.hasVerticalScroller = true
        for col in table.tableColumns { table.removeTableColumn(col) }
        let dc = NSTableColumn(identifier: NSUserInterfaceItemIdentifier("d"))
        dc.title = tr("day")
        dc.width = 110
        table.addTableColumn(dc)
        for i in 0..<6 {
            let c = NSTableColumn(identifier: NSUserInterfaceItemIdentifier("\(i)"))
            c.title = PN[Prefs.lang][i]
            c.width = 76
            table.addTableColumn(c)
        }
        table.dataSource = self
        table.delegate = self
        table.rowHeight = 20
        table.usesAlternatingRowBackgroundColors = true
        table.allowsSelection = false
        scroll.documentView = table
        content.addSubview(prevBtn)
        content.addSubview(nextBtn)
        content.addSubview(titleLabel)
        content.addSubview(scroll)
        w.contentView = content
        w.center()
        window = w
        NSApp.activate(ignoringOtherApps: true)
        w.makeKeyAndOrderFront(nil)
        load()
        prefetch(Array(1...range).flatMap { [$0, -$0] })
    }

    func ym(_ off: Int) -> (Int, Int) {
        let total = baseY * 12 + (baseM - 1) + off
        return (total / 12, total % 12 + 1)
    }

    func fetchMonth(_ y: Int, _ m: Int, _ done: @escaping ([[Int]]?) -> Void) {
        let e = Engine.shared
        guard e.base != nil else { done(nil); return }
        let u = "https://api.aladhan.com/v1/calendar/\(y)/\(m)?latitude=\(e.lat)&longitude=\(e.lon)&school=\(Prefs.school)"
        getJSON(u) { j in
            guard let o = j as? [String: Any], let arr = o["data"] as? [[String: Any]] else { done(nil); return }
            var out: [[Int]] = []
            for d in arr {
                guard let t = d["timings"] as? [String: Any] else { done(nil); return }
                var r = [Int]()
                for k in KEYS {
                    guard let s = t[k] as? String, let v = parseHM(s) else { done(nil); return }
                    r.append(v)
                }
                out.append(r)
            }
            done(out)
        }
    }

    func load() {
        titleLabel.stringValue = "\(MONTHS[Prefs.lang][month - 1]) \(year)"
        let o = offset()
        prevBtn.isEnabled = o > -range
        nextBtn.isEnabled = o < range
        let key = year * 100 + month
        if let r = cache[key] {
            rows = r
            table.reloadData()
            return
        }
        rows = []
        table.reloadData()
        let y = year, m = month
        fetchMonth(y, m) { r in
            guard let r = r else { return }
            self.cache[y * 100 + m] = r
            if y == self.year && m == self.month {
                self.rows = r
                self.table.reloadData()
            }
        }
    }

    func prefetch(_ list: [Int]) {
        guard let first = list.first else { return }
        let rest = Array(list.dropFirst())
        let (y, m) = ym(first)
        if cache[y * 100 + m] != nil || window == nil {
            prefetch(rest)
            return
        }
        fetchMonth(y, m) { r in
            if let r = r {
                self.cache[y * 100 + m] = r
                if y == self.year && m == self.month {
                    self.rows = r
                    self.table.reloadData()
                }
            }
            self.prefetch(rest)
        }
    }

    func step(_ d: Int) {
        let o = offset() + d
        if o < -range || o > range { return }
        month += d
        if month < 1 { month = 12; year -= 1 }
        if month > 12 { month = 1; year += 1 }
        load()
    }

    @objc func prevTapped() { step(-1) }
    @objc func nextTapped() { step(1) }

    func numberOfRows(in tableView: NSTableView) -> Int { rows.count }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        guard let id = tableColumn?.identifier.rawValue, row < rows.count else { return nil }
        let tf = NSTextField(labelWithString: "")
        let isToday = year == todayY && month == todayM && row + 1 == todayD
        if id == "d" {
            var comps = DateComponents()
            comps.year = year
            comps.month = month
            comps.day = row + 1
            comps.hour = 12
            var wd = 0
            if let dt = Calendar(identifier: .gregorian).date(from: comps) {
                wd = Calendar(identifier: .gregorian).component(.weekday, from: dt) - 1
            }
            tf.stringValue = "\(row + 1)  \(WEEKDAYS[Prefs.lang][wd])"
            tf.alignment = .left
            tf.font = (isToday || wd == 5) ? NSFont.boldSystemFont(ofSize: 13) : NSFont.systemFont(ofSize: 13)
            if wd == 5 { tf.textColor = .systemGreen }
        } else if let i = Int(id) {
            tf.stringValue = fmtTime(rows[row][i] + Prefs.off[i] * 60)
            tf.alignment = .center
            tf.font = NSFont.monospacedDigitSystemFont(ofSize: 13, weight: isToday ? .bold : .regular)
        }
        if isToday { tf.textColor = .controlAccentColor }
        return tf
    }
}
