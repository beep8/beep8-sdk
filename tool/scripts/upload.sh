#!/bin/bash
# ==== BEEP-8 ROM Image Upload & Browser Launch Script ====
# This script uploads the pre-built BEEP-8 ROM image to beep8.org and launches it in the browser.

# Default value for ports
http_port=443
api_port=8082

RESET="\033[0m"
BLUE="\033[34m"
CYAN="\033[36m"
YELLOW="\033[33m"

# If arugements are provided, use them to set the port values.
if [ ! -z "$2" ]; then
  http_port="$2"
fi

if [ ! -z "$3" ]; then
  api_port="$3"
fi

if [ -z "$1" ]; then
  echo "Usage: $0 <*.b8>"
  exit 1
fi

if [ ! -e "$1" ]; then
  echo "Error: File '$1' not found."
  exit 1
fi

if [[ "$1" != /* ]]; then
  fullpath=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
else
  fullpath="$1"
fi

if echo "$fullpath" | LC_ALL=C grep -q '[^ -~]'; then
  echo "Error: Variable contains non-ASCII characters."
  echo "\033[36mCopying the BEEP-8 SDK to the path containing only ASCII characters will resolve this issue."
  exit 1
fi

# Validate the file size
filesize=$(wc -c <"$fullpath")
if [ $filesize -gt 4194304 ]; then
  echo "Error: File size exceeds 4MB."
  exit 1
fi

# Detect OS
os=$(uname)
converted=$(echo "$fullpath" | sed 's/\//_/g')

# Upload the file and get the response
response=$(curl --retry 2 --max-time 15 -X POST -H "Content-Type: application/octet-stream" \
  -H "Content-Disposition: attachment; filename=$converted" \
  --data-binary "@$1" \
  https://beep8.org:${api_port}/api/upload)

url="https://beep8.org:${http_port}/b8/beep8.html?b8rom=${response}&vterm0=1&wdt=1&api_port=${api_port}"
url2=$url

case "$os" in
  "Darwin")
    BROWSER_NAME="Google Chrome"
    ;;
  "Linux")
    if grep -qEi "(Microsoft|WSL)" /proc/version &> /dev/null ; then
      # Windows WSL
      BROWSER_NAME='cmd.exe /C start ""'
      url="${url//&/^&}"

    elif command -v google-chrome &> /dev/null; then
      # Linux / google-chrome
      BROWSER_NAME="google-chrome"

    elif command -v firefox &> /dev/null; then
      # Linux / Firefox
      BROWSER_NAME="firefox"

    else
      echo "No compatible browser found."
    fi
    ;;
  "CYGWIN_NT"*|"MINGW"*|"MSYS_NT"*)
    # Windows DOS
    BROWSER_NAME="/cygdrive/c/Program Files/Google/Chrome/Application/chrome.exe"
    ;;
  *)
    echo "Unsupported OS: $os"
    ;;
esac

if [[ "$response" =~ ^[0-9a-f]{32}\.b8 ]]; then
  echo -e "$YELLOW"
  echo "By manually copying and pasting the URL"
  echo -e "$CYAN $url2"
  echo -e "$YELLOW into your web browser, you can run the built program."
  echo "It is executable on any web browser, whether on iPhone, Android, Mac, Windows PC, or Linux PC."
  echo -e "$RESET"

  case "$os" in
    "Darwin")
      open -a "$BROWSER_NAME" "$url"
      ;;
    "Linux")
      full_command="$BROWSER_NAME \"${url}"\"
      eval "$full_command"

      "$BROWSER_NAME" "${url}" &> /dev/null &
      ;;
    *)
      echo "Unsupported OS: $os"
      ;;
  esac

elif [[ "$response" == *"already"* ]]; then
  echo -e "$CYAN"
  echo "Hint:"
  echo "If the browser does not update automatically, please manually press the browser's refresh button."
  echo -e "$RESET"
  case "$os" in
    "Darwin")
      osascript -e 'tell application "Google Chrome" to activate'
      ;;
    "Linux")
      if grep -qEi "(Microsoft|WSL)" /proc/version &> /dev/null ; then
        echo "reboot"

      elif command -v google-chrome &> /dev/null; then
        echo "reboot google-chrome"

      elif command -v firefox &> /dev/null; then
        echo "reboot firefox"

      else
        echo "No compatible browser found."
      fi
      ;;
    "CYGWIN_NT"*|"MINGW"*|"MSYS_NT"*)
      # Windows DOS
      echo "reboot"
      ;;
    *)
      echo "Unsupported OS: $os"
      ;;
  esac
else
  echo "Unexpected response: $response"
fi
