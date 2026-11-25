import SwiftUI
import UserNotifications
import ServiceManagement

func iconImage(_ i: Int) -> NSImage {
    if let p = Bundle.main.path(forResource: "icon\(i + 1)", ofType: "png"), let img = NSImage(contentsOfFile: p) { return img }
    return NSImage()
}

func requestAuth() {
    UNUserNotificationCenter.current().requestAuthorization(options: [.alert, .sound]) { _, _ in }
}

final class SettingsModel: ObservableObject {
    @Published var lang: Int { didSet { Prefs.lang = lang; Engine.shared.tick() } }
    @Published var auto: Bool { didSet { Prefs.auto = auto; Engine.shared.resetData() } }
    @Published var cityText: String
    @Published var status: String
    @Published var school: Int { didSet { Prefs.school = school; Engine.shared.resetData() } }
    @Published var notify: Bool { didSet { Prefs.notify = notify; if notify { requestAuth() } } }
    @Published var lead: Int { didSet { Prefs.lead = lead } }
    @Published var fmt12: Bool { didSet { Prefs.fmt12 = fmt12; Engine.shared.tick() } }
    @Published var dock: Bool { didSet { Prefs.dock = dock; Engine.shared.applyDock() } }
    @Published var icon: Int { didSet { Prefs.icon = icon; Engine.shared.applyIcon() } }
    @Published var launch: Bool { didSet { setLaunch(launch) } }
    @Published var iqOn: Bool { didSet { Prefs.iqOn = iqOn; Engine.shared.tick() } }
    @Published var iq: [Int] { didSet { Prefs.iq = iq; Engine.shared.tick() } }
    @Published var off: [Int] { didSet { Prefs.off = off; Engine.shared.tick() } }

    init() {
        lang = Prefs.lang
        auto = Prefs.auto
        cityText = Prefs.cityName
        status = Prefs.auto ? "" : Prefs.cityName
        school = Prefs.school
        notify = Prefs.notify
        lead = Prefs.lead
        fmt12 = Prefs.fmt12
        dock = Prefs.dock
        icon = Prefs.icon
        var l = false
        if #available(macOS 13.0, *) { l = SMAppService.mainApp.status == .enabled }
        launch = l
        iqOn = Prefs.iqOn
        iq = Prefs.iq
        off = Prefs.off
    }

    func setLaunch(_ on: Bool) {
        if #available(macOS 13.0, *) {
            do {
                if on { try SMAppService.mainApp.register() } else { try SMAppService.mainApp.unregister() }
            } catch {}
        }
    }

    func find() {
        let q = cityText.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty, let enc = q.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) else { return }
        let lc = ["en", "ar", "ru"][Prefs.lang]
        getJSON("https://geocoding-api.open-meteo.com/v1/search?name=\(enc)&count=1&language=\(lc)") { [weak self] j in
            guard let self = self else { return }
            if let o = j as? [String: Any], let r = (o["results"] as? [[String: Any]])?.first,
               let la = r["latitude"] as? Double, let lo = r["longitude"] as? Double {
                let nm = (r["name"] as? String) ?? q
                let co = (r["country"] as? String) ?? ""
                Prefs.lat = la
                Prefs.lon = lo
                Prefs.cityName = nm
                self.status = String(format: tr("found"), co.isEmpty ? nm : "\(nm), \(co)")
                self.auto = false
            } else {
                self.status = tr("notFound")
            }
        }
    }

    func offText(_ v: Int) -> String {
        v == 0 ? "0" : String(format: "%+d %@", v, tr("min"))
    }
}

struct SettingsView: View {
    @ObservedObject var m: SettingsModel

    var body: some View {
        TabView {
            general.tabItem { Label(tr("tabGeneral"), systemImage: "gearshape") }
            times.tabItem { Label(tr("tabTimes"), systemImage: "clock") }
            iqama.tabItem { Label(tr("tabIqama"), systemImage: "timer") }
        }
        .frame(width: 520, height: 640)
        .environment(\.layoutDirection, m.lang == 1 ? .rightToLeft : .leftToRight)
        .id(m.lang)
    }

    var general: some View {
        Form {
            Section(tr("language")) {
                Picker("", selection: $m.lang) {
                    Text("English").tag(0)
                    Text("العربية").tag(1)
                    Text("Русский").tag(2)
                }
                .pickerStyle(.segmented)
                .labelsHidden()
            }
            Section(tr("location")) {
                Picker("", selection: $m.auto) {
                    Text(tr("auto")).tag(true)
                    Text(tr("manual")).tag(false)
                }
                .pickerStyle(.segmented)
                .labelsHidden()
                if !m.auto {
                    HStack {
                        TextField(tr("city"), text: $m.cityText)
                            .onSubmit { m.find() }
                        Button(tr("find")) { m.find() }
                    }
                    if !m.status.isEmpty {
                        Text(m.status).foregroundColor(.secondary)
                    }
                }
                Picker(tr("asr"), selection: $m.school) {
                    Text(tr("standard")).tag(0)
                    Text(tr("hanafi")).tag(1)
                }
                .pickerStyle(.segmented)
            }
            Section(tr("notify")) {
                Toggle(tr("notify"), isOn: $m.notify)
                Picker(tr("remind"), selection: $m.lead) {
                    ForEach(0..<LEADS.count, id: \.self) { i in
                        Text(LEADS[i] == 0 ? tr("atTime") : String(format: tr("before"), LEADS[i])).tag(i)
                    }
                }
                .disabled(!m.notify)
                Button(tr("test")) {
                    requestAuth()
                    Engine.shared.sendTest()
                }
            }
            Section {
                Toggle(tr("fmt12"), isOn: $m.fmt12)
                Toggle(tr("autostart"), isOn: $m.launch)
                Toggle(tr("dock"), isOn: $m.dock)
            }
            Section(tr("icon")) {
                HStack(spacing: 10) {
                    ForEach(0..<6, id: \.self) { i in
                        Button {
                            m.icon = i
                        } label: {
                            Image(nsImage: iconImage(i))
                                .resizable()
                                .frame(width: 44, height: 44)
                                .padding(5)
                                .background(
                                    RoundedRectangle(cornerRadius: 12)
                                        .stroke(m.icon == i ? Color.accentColor : Color.clear, lineWidth: 3)
                                )
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
        }
        .formStyle(.grouped)
    }

    var times: some View {
        Form {
            Section(tr("timesHint")) {
                ForEach(0..<6, id: \.self) { i in
                    HStack {
                        Text(PN[m.lang][i])
                        Spacer()
                        Text(timeText(i))
                            .monospacedDigit()
                            .foregroundColor(.secondary)
                        Stepper(value: offBinding(i), in: -60...60) {
                            Text(m.offText(m.off[i]))
                                .frame(width: 70, alignment: .trailing)
                        }
                    }
                }
                Button(tr("reset")) { m.off = [0, 0, 0, 0, 0, 0] }
            }
        }
        .formStyle(.grouped)
    }

    func timeText(_ i: Int) -> String {
        if let b = Engine.shared.base { return fmtTime(b[i] + m.off[i] * 60) }
        return "--:--"
    }

    func offBinding(_ i: Int) -> Binding<Int> {
        Binding(get: { m.off[i] }, set: { v in
            var a = m.off
            a[i] = v
            m.off = a
        })
    }

    var iqama: some View {
        Form {
            Section {
                Toggle(tr("iqamaOn"), isOn: $m.iqOn)
            }
            Section(tr("iqamaHint")) {
                ForEach([0, 2, 3, 4, 5], id: \.self) { i in
                    HStack {
                        Text(PN[m.lang][i])
                        Spacer()
                        Stepper(value: iqBinding(i), in: 0...90) {
                            Text("\(m.iq[i]) \(tr("min"))")
                                .frame(width: 70, alignment: .trailing)
                        }
                    }
                }
            }
            .disabled(!m.iqOn)
        }
        .formStyle(.grouped)
    }

    func iqBinding(_ i: Int) -> Binding<Int> {
        Binding(get: { m.iq[i] }, set: { v in
            var a = m.iq
            a[i] = v
            m.iq = a
        })
    }
}

struct TodayView: View {
    var openSchedule: () -> Void
    var openSettings: () -> Void

    var body: some View {
        TimelineView(.periodic(from: Date(), by: 1)) { _ in
            content()
        }
        .frame(width: 360)
        .environment(\.layoutDirection, Prefs.lang == 1 ? .rightToLeft : .leftToRight)
    }

    func content() -> some View {
        let e = Engine.shared
        let s = e.summary()
        let t = e.times()
        let wd = e.now().weekday
        return VStack(spacing: 14) {
            VStack(spacing: 2) {
                Text(e.cityLabel.isEmpty ? " " : e.cityLabel)
                    .font(.subheadline)
                    .foregroundColor(.secondary)
                if let s = s {
                    Text(s.iqama ? "\(IQAMA_W[Prefs.lang])" : s.name)
                        .font(.title3.weight(.semibold))
                    Text(fmtCount(s.rem))
                        .font(.system(size: 46, weight: .semibold, design: .rounded))
                        .monospacedDigit()
                    Text(s.iqama ? "\(s.name) · \(fmtTime(s.time))" : fmtTime(s.time))
                        .foregroundColor(.secondary)
                } else {
                    Text(e.fetching ? tr("loading") : tr("offline"))
                        .font(.title3)
                        .padding(.vertical, 20)
                }
            }
            .padding(.top, 6)
            VStack(spacing: 4) {
                ForEach(0..<6, id: \.self) { i in
                    let on = s?.idx == i
                    HStack {
                        Text(e.name(i, weekday: wd))
                        Spacer()
                        Text(e.base == nil ? "--:--" : fmtTime(t[i]))
                            .monospacedDigit()
                    }
                    .font(.system(size: 14, weight: on ? .semibold : .regular))
                    .foregroundColor(on ? .white : .primary)
                    .padding(.horizontal, 12)
                    .padding(.vertical, 7)
                    .background(
                        RoundedRectangle(cornerRadius: 10)
                            .fill(on ? Color.accentColor : Color.primary.opacity(0.06))
                    )
                }
            }
            HStack {
                Button(tr("schedule").replacingOccurrences(of: "…", with: "")) { openSchedule() }
                Spacer()
                Button(tr("settings").replacingOccurrences(of: "…", with: "")) { openSettings() }
            }
        }
        .padding(20)
    }
}

final class SettingsWindow {
    static let shared = SettingsWindow()
    var window: NSWindow?

    func show() {
        if let w = window {
            w.delegate = nil
            w.close()
            window = nil
        }
        let w = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 520, height: 640),
                         styleMask: [.titled, .closable], backing: .buffered, defer: false)
        w.title = "Sujood"
        w.isReleasedWhenClosed = false
        w.contentViewController = NSHostingController(rootView: SettingsView(m: SettingsModel()))
        w.center()
        window = w
        NSApp.activate(ignoringOtherApps: true)
        w.makeKeyAndOrderFront(nil)
    }
}

final class TodayWindow {
    static let shared = TodayWindow()
    var window: NSWindow?

    func show() {
        if window == nil {
            let w = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 360, height: 520),
                             styleMask: [.titled, .closable], backing: .buffered, defer: false)
            w.title = "Sujood"
            w.isReleasedWhenClosed = false
            w.contentViewController = NSHostingController(rootView: TodayView(
                openSchedule: { MonthWindow.shared.show() },
                openSettings: { SettingsWindow.shared.show() }))
            w.center()
            window = w
        }
        NSApp.activate(ignoringOtherApps: true)
        window?.makeKeyAndOrderFront(nil)
    }
}
