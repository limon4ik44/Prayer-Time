#!/bin/bash
set -e
cd "$(dirname "$0")"
APP="Sujood.app"
rm -rf "$APP" build-arm64 build-x86_64
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
for A in arm64 x86_64; do
  swiftc -O -swift-version 5 -target "$A-apple-macos13.0" Data.swift Engine.swift Windows.swift UI.swift main.swift -o "build-$A"
done
lipo -create build-arm64 build-x86_64 -output "$APP/Contents/MacOS/Sujood"
rm -f build-arm64 build-x86_64
cp Info.plist "$APP/Contents/Info.plist"
cp icons/icon*.png "$APP/Contents/Resources/"
iconutil -c icns AppIcon.iconset -o "$APP/Contents/Resources/AppIcon.icns"
codesign --force --deep --sign - "$APP"
ditto -c -k --sequesterRsrc --keepParent "$APP" Sujood-mac.zip
echo "Done: $APP and Sujood-mac.zip"
