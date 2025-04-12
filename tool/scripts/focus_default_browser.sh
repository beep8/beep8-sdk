#!/bin/bash

os=$(uname)
browser=""

if [[ "$os" == "Darwin" ]]; then
    browser=$(defaults read com.apple.LaunchServices/com.apple.launchservices.secure \
    LSHandlers | grep -B 1 -e 'LSHandlerURLScheme = http;' -e 'LSHandlerURLScheme = https;' \
    | grep LSHandlerRoleAll | awk -F ' = ' '{print $2}' | tr -d '";')
    case "$browser" in
        *"com.apple.Safari"*)
            osascript -e 'tell application "Safari" to activate'
            ;;
        *"com.google.chrome"*)
            osascript -e 'tell application "Google Chrome" to activate'
            ;;
        *"org.mozilla.firefox"*)
            osascript -e 'tell application "Firefox" to activate'
            ;;
        *"com.microsoft.edgemac"*)
            osascript -e 'tell application "Microsoft Edge" to activate'
            ;;
        *"com.brave.Browser"*)
            osascript -e 'tell application "Brave Browser" to activate'
            ;;
        *"com.operasoftware.Opera"*)
            osascript -e 'tell application "Opera" to activate'
            ;;
        *)
            echo "Unsupported or unknown default browser: $browser"
            ;;
    esac
elif [[ "$os" == "Linux" ]]; then
    browser=$(xdg-settings get default-web-browser)
    case "$browser" in
        *"firefox.desktop"*)
            firefox &
            ;;
        *"chromium.desktop"*)
            chromium-browser &
            ;;
        *"google-chrome.desktop"*)
            google-chrome &
            ;;
        *)
            echo "Unsupported or unknown default browser: $browser"
            ;;
    esac
elif [[ "$os" == "MINGW"* ]]; then
    browser=$(cmd.exe /c "REG QUERY HKEY_CLASSES_ROOT\HTTP\shell\open\command /ve" | awk -F'=' '{print $2}' | tr -d '\r')
    browser="${browser//%1/}"
    browser="${browser//\"/}"
    case "$browser" in
        *"iexplore.exe"*)
            start iexplore.exe
            ;;
        *"chrome.exe"*)
            start chrome.exe
            ;;
        *"firefox.exe"*)
            start firefox.exe
            ;;
        *)
            echo "Unsupported or unknown default browser: $browser"
            ;;
    esac
elif [[ "$os" == "Linux"* ]]; then
    browser=$(xdg-settings get default-web-browser)
    case "$browser" in
        *"firefox.desktop"*)
            firefox &
            ;;
        *"chromium.desktop"*)
            chromium-browser &
            ;;
        *"google-chrome.desktop"*)
            google-chrome &
            ;;
        *)
            echo "Unsupported or unknown default browser: $browser"
            ;;
    esac
else
    echo "This script only works on macOS, Linux or Windows (with or without WSL)."
fi
