# PetBot-

PetBot- is a hybrid Android + ESP32 robot control project with a browser-based AI avatar UI, Bluetooth control, optional ground-station telemetry, and object detection overlays.

## What’s included

- Android app (WebView UI, Bluetooth, TTS, voice input)
- Web-based avatar UI (assets)
- ESP32 firmware for robot control
- Optional websocket ground-station and telemetry tools

## Quick start

Start here for the fastest path:

- [START_HERE.md](START_HERE.md)
- [QUICK_START.md](QUICK_START.md)

## Android app setup

Follow the detailed guide:

- [BUILD_ANDROID_APP.md](BUILD_ANDROID_APP.md)

At a high level:

1. Open the project in Android Studio.
2. Set your Android SDK path in local.properties (not committed).
3. Sync Gradle and run the app on a device.

## ESP32 firmware setup

Pick the firmware that matches your build and follow the guide:

- [ESP32_CAR_SETUP.md](ESP32_CAR_SETUP.md)
- [ESP32_CAR_UPLOAD_GUIDE.md](ESP32_CAR_UPLOAD_GUIDE.md)
- [ESP32_INTEGRATION_GUIDE.md](ESP32_INTEGRATION_GUIDE.md)

If you are using the PetBot build specifically:

- [ESP32_PetBot/WIRING_GUIDE.md](ESP32_PetBot/WIRING_GUIDE.md)

## How the AI avatar works

- The Android app loads the avatar UI from app assets.
- Wake word and speech input are handled in the UI.
- If an AI key is configured in the settings panel, the UI calls the AI endpoint and speaks the response.
- Settings are stored locally on-device (localStorage in the WebView) and are not committed to git.

## API keys (important)

- Do not commit API keys to the repo.
- Enter your own key in the settings panel at runtime.
- Keys are stored locally on your device and never written to the repository.

If you previously exposed a key, revoke it immediately and issue a new one.

## Optional components

- Ground station telemetry and websocket helpers are in the root:
  - [ground_station.py](ground_station.py)
  - [websocket-server.js](websocket-server.js)
- The avatar UI assets are in:
  - [app/src/main/assets](app/src/main/assets)

## Docs and guides

- [RUN_INSTRUCTIONS.md](RUN_INSTRUCTIONS.md)
- [INSTALL_ON_PHONE.md](INSTALL_ON_PHONE.md)
- [BLUETOOTH_GUIDE.md](BLUETOOTH_GUIDE.md)
- [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)

## Security note

This repo is configured to ignore build outputs, IDE files, and local properties. Keep secrets out of source control and rotate any keys that were ever committed.
