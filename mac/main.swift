import Cocoa
import UserNotifications

func isNewer(_ tag: String, _ cur: String) -> Bool {
    func nums(_ s: String) -> [Int] {
        let t = s.hasPrefix("v") || s.hasPrefix("V") ? String(s.dropFirst()) : s
        return t.split(separator: ".").map { Int($0.filter { $0.isNumber }) ?? 0 }
    }
    let a = nums(tag), b = nums(cur)
    for i in 0..<max(a.count, b.count) {
        let x = i < a.count ? a[i] : 0
        let y = i < b.count ? b[i] : 0
        if x != y { return x > y }
    }
    return false
}

final class AppDelegate: NSObject, NSApplicationDelegate, NSMenuDelegate, UNUserNotificationCenterDelegate {
    var latestTag = ""
    var latestURL = ""

    func applicationDidFinishLaunching(_ n: Notification) {
        NSApp.setActivationPolicy(Prefs.dock ? .regular : .accessory)
        Engine.shared.applyIcon()
        let item = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        let menu = NSMenu()
        menu.delegate = self
        menu.autoenablesItems = false
        item.menu = menu
        Engine.shared.statusItem = item
        UNUserNotificationCenter.current().delegate = self
        UNUserNotificationCenter.current().requestAuthorization(options: [.alert, .sound]) { _, _ in }
        Engine.shared.loadCache()
        Engine.shared.tick()
        Engine.shared.reload()
        let t = Timer(timeInterval: 1, repeats: true) { _ in Engine.shared.tick() }
        RunLoop.main.add(t, forMode: .common)
        NSWorkspace.shared.notificationCenter.addObserver(forName: NSWorkspace.didWakeNotification, object: nil, queue: .main) { _ in
            Engine.shared.fresh = false
            Engine.shared.reload()
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 20) { self.checkUpdate(manual: false) }
        let u = Timer(timeInterval: 6 * 3600, repeats: true) { _ in self.checkUpdate(manual: false) }
        RunLoop.main.add(u, forMode: .common)
    }

    func userNotificationCenter(_ center: UNUserNotificationCenter, willPresent notification: UNNotification,
                                withCompletionHandler completionHandler: @escaping (UNNotificationPresentationOptions) -> Void) {
        completionHandler([.banner, .sound])
    }

    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        TodayWindow.shared.show()
        return true
    }

    func menuNeedsUpdate(_ menu: NSMenu) {
        menu.removeAllItems()
        let e = Engine.shared
        let n = e.now()
        func add(_ title: String, _ sel: Selector?, enabled: Bool = true) -> NSMenuItem {
            let it = NSMenuItem(title: title, action: sel, keyEquivalent: "")
            it.target = self
            it.isEnabled = enabled
            menu.addItem(it)
            return it
        }
        if e.isRamadan() {
            _ = add(String(format: tr("ramadan"), e.hijriDay), nil, enabled: false)
            menu.addItem(NSMenuItem.separator())
        }
        let t = e.times()
        var nx = -1
        for i in [0, 2, 3, 4, 5] where t[i] > n.sod {
            nx = i
            break
        }
        if nx < 0 { nx = 0 }
        for i in 0..<6 {
            let tm = e.base == nil ? "--:--" : fmtTime(t[i])
            let it = add("\(e.name(i, weekday: n.weekday))   \(tm)", nil, enabled: true)
            it.state = (e.base != nil && i == nx) ? .on : .off
        }
        menu.addItem(NSMenuItem.separator())
        _ = add(tr("open"), #selector(openToday))
        _ = add(tr("schedule"), #selector(openSchedule))
        _ = add(tr("settings"), #selector(openSettings))
        menu.addItem(NSMenuItem.separator())
        if !latestTag.isEmpty && isNewer(latestTag, appVersion) {
            _ = add(String(format: tr("updAvail"), latestTag), #selector(openRelease))
        }
        _ = add(tr("updCheck"), #selector(manualCheck))
        _ = add(tr("refresh"), #selector(refresh))
        menu.addItem(NSMenuItem.separator())
        _ = add(tr("quit"), #selector(quit))
    }

    @objc func openToday() { TodayWindow.shared.show() }
    @objc func openSchedule() { MonthWindow.shared.show() }
    @objc func openSettings() { SettingsWindow.shared.show() }
    @objc func refresh() { Engine.shared.fresh = false; Engine.shared.reload() }
    @objc func quit() { NSApp.terminate(nil) }
    @objc func manualCheck() { checkUpdate(manual: true) }
    @objc func openRelease() {
        if let u = URL(string: latestURL.isEmpty ? "https://github.com/\(updateRepo)/releases/latest" : latestURL) {
            NSWorkspace.shared.open(u)
        }
    }

    func checkUpdate(manual: Bool) {
        getJSON("https://api.github.com/repos/\(updateRepo)/releases/latest") { j in
            guard let o = j as? [String: Any], let tag = o["tag_name"] as? String else {
                if manual { self.alert(tr("updFail")) }
                return
            }
            self.latestTag = tag
            self.latestURL = (o["html_url"] as? String) ?? ""
            if manual {
                if isNewer(tag, appVersion) {
                    self.alert(String(format: tr("updAvail"), tag))
                    self.openRelease()
                } else {
                    self.alert(tr("updLatest") + " (" + appVersion + ")")
                }
            }
        }
    }

    func alert(_ text: String) {
        NSApp.activate(ignoringOtherApps: true)
        let a = NSAlert()
        a.messageText = "Sujood"
        a.informativeText = text
        a.addButton(withTitle: tr("ok"))
        a.runModal()
    }
}

let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate
app.run()
