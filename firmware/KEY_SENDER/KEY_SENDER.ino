#include <M5StickC.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h>

// ============================================================
// Scroll Lock
// ============================================================

// ESP32-BLE-Keyboard does not provide a predefined constant for Scroll Lock.
// 0xCF corresponds to KEY_SCROLL_LOCK in the Arduino Keyboard convention.
const uint8_t KEY_SCROLL_LOCK_CUSTOM = 0xCF;

// ============================================================
// WiFi
// ============================================================

#include "secrets.h"

// ============================================================
// BLE Keyboard
// ============================================================

BleKeyboard bleKeyboard(
  "M5StickC KEY SENDER",
  "M5StickC",
  100
);

// ============================================================
// Web server
// ============================================================

WebServer server(80);

// ============================================================
// Settings
// ============================================================

bool sendingEnabled = false;

// Default: 4 minutes
unsigned long sendInterval = 4UL * 60UL * 1000UL;

unsigned long lastSendTime = 0;
unsigned long keyCount = 0;

// Default key
String keyCombination = "SCROLLLOCKX2";

// How long a key or combination is held down
const unsigned long KEY_HOLD_MS = 60;

// ============================================================
// Display cache
// ============================================================

bool firstDisplayUpdate = true;

bool lastDisplaySending = false;
bool lastDisplayBle = false;
bool lastDisplayWifi = false;

unsigned long lastDisplayCount = 0;
unsigned long lastDisplayInterval = 0;

String lastDisplayCombo = "";

// ============================================================
// Function declarations
// ============================================================

void updateDisplay(bool force = false);

// ============================================================
// HTML
// ============================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="sv">

<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>KEY SENDER</title>

<style>

body {
    font-family: Arial, sans-serif;
    background: #181818;
    color: #eee;
    margin: 0;
    padding: 20px;
}

.container {
    max-width: 520px;
    margin: auto;
}

.card {
    background: #282828;
    padding: 20px;
    border-radius: 12px;
    margin-bottom: 20px;
}

h1, h2 {
    margin-top: 0;
}

.status {
    font-size: 22px;
    font-weight: bold;
}

.running {
    color: #57d957;
}

.stopped {
    color: #ff6060;
}

.connected {
    color: #57d957;
}

.disconnected {
    color: #ffaa44;
}

button {
    width: 100%;
    border: 0;
    padding: 15px;
    margin-top: 10px;
    font-size: 18px;
    border-radius: 8px;
    cursor: pointer;
}

.start {
    background: #32a852;
    color: white;
}

.stop {
    background: #d64545;
    color: white;
}

.send {
    background: #3979d6;
    color: white;
}

input {
    width: 100%;
    box-sizing: border-box;
    font-size: 18px;
    padding: 10px;
    border-radius: 6px;
    border: 0;
    margin-top: 8px;
    margin-bottom: 10px;
}

.small {
    color: #aaa;
    font-size: 14px;
    line-height: 1.5;
}

.current {
    background: #202020;
    padding: 10px;
    border-radius: 6px;
}

</style>
</head>

<body>

<div class="container">

<div class="card">

<h1>KEY SENDER</h1>

<p>
Sändning:
<span id="sending" class="status">...</span>
</p>

<p>
Bluetooth:
<span id="ble">...</span>
</p>

<p>
WiFi:
<span id="wifi">...</span>
</p>

<p>
Antal sändningar:
<strong id="count">0</strong>
</p>

<p class="current">
Aktuell kombination:
<strong id="currentCombo">...</strong>
</p>

<button class="start" onclick="startSending()">
STARTA
</button>

<button class="stop" onclick="stopSending()">
STOPPA
</button>

<button class="send" onclick="toggleSending()">
TOGGLE
</button>

<button class="send" onclick="sendOnce()">
SKICKA NU
</button>

</div>

<div class="card">

<h2>Tangentkombination</h2>

<input
    type="text"
    id="combo"
    value="SCROLLLOCKX2"
    placeholder="CTRL+ALT+3"
>

<p class="small">
Exempel:<br>
SCROLLLOCKX2<br>
SCROLLLOCK<br>
CTRL<br>
SHIFT<br>
CTRL+ALT+3<br>
WIN+L<br>
CTRL+SHIFT+ESC<br>
ALT+F4<br>
CTRL+ALT+DELETE
</p>

</div>

<div class="card">

<h2>Intervall</h2>

<label>Minuter</label>

<input
    type="number"
    id="minutes"
    min="0"
    value="4"
>

<label>Sekunder</label>

<input
    type="number"
    id="seconds"
    min="0"
    max="59"
    value="0"
>

<p class="small" id="intervalInfo">
Nuvarande intervall: 4 min 0 sek
</p>

<button onclick="saveSettings()">
SPARA INSTÄLLNINGAR
</button>

</div>

</div>

<script>

async function api(url, options = {}) {
    const response = await fetch(url, options);
    const data = await response.json();

    if (!response.ok) {
        throw new Error(data.error || "API error");
    }

    return data;
}

async function startSending() {
    try {
        await api('/api/start', { method: 'POST' });
        updateStatus();
    }
    catch(e) {
        alert(e.message);
    }
}

async function stopSending() {
    try {
        await api('/api/stop', { method: 'POST' });
        updateStatus();
    }
    catch(e) {
        alert(e.message);
    }
}

async function toggleSending() {
    try {
        await api('/api/toggle', { method: 'POST' });
        updateStatus();
    }
    catch(e) {
        alert(e.message);
    }
}

async function sendOnce() {
    try {
        await api('/api/send', { method: 'POST' });
        updateStatus();
    }
    catch(e) {
        alert(e.message);
    }
}

async function saveSettings() {

    const combo =
        document.getElementById('combo').value.trim();

    const minutes =
        parseInt(document.getElementById('minutes').value) || 0;

    const seconds =
        parseInt(document.getElementById('seconds').value) || 0;

    if(combo.length === 0) {
        alert("Ange en tangentkombination.");
        return;
    }

    if (
        minutes < 0 ||
        seconds < 0 ||
        seconds > 59
    ) {
        alert("Sekunder måste vara mellan 0 och 59.");
        return;
    }

    if (
        minutes === 0 &&
        seconds === 0
    ) {
        alert("Intervallet måste vara minst 1 sekund.");
        return;
    }

    const params =
        new URLSearchParams();

    params.set('combo', combo);
    params.set('minutes', minutes);
    params.set('seconds', seconds);

    try {
        await api(
            '/api/settings?' + params.toString(),
            {
                method: 'POST'
            }
        );

        updateStatus();

        alert("Inställningarna är sparade.");
    }
    catch(e) {
        alert(e.message);
    }
}

async function updateStatus() {

    try {

        const status =
            await api('/api/status');

        const sending =
            document.getElementById('sending');

        if(status.sending) {
            sending.textContent = "AKTIV";
            sending.className = "status running";
        }
        else {
            sending.textContent = "STOPPAD";
            sending.className = "status stopped";
        }

        const ble =
            document.getElementById('ble');

        if(status.bluetooth) {
            ble.textContent = "Ansluten";
            ble.className = "connected";
        }
        else {
            ble.textContent = "Inte ansluten";
            ble.className = "disconnected";
        }

        const wifi =
            document.getElementById('wifi');

        if(status.wifi) {
            wifi.textContent = "Ansluten";
            wifi.className = "connected";
        }
        else {
            wifi.textContent = "Inte ansluten";
            wifi.className = "disconnected";
        }

        document.getElementById('count')
            .textContent = status.count;

        document.getElementById('currentCombo')
            .textContent = status.combo;

        document.getElementById('intervalInfo')
            .textContent =
            "Nuvarande intervall: " +
            status.minutes +
            " min " +
            status.seconds +
            " sek";
    }
    catch(e) {
        console.log(e);
    }
}

updateStatus();

setInterval(
    updateStatus,
    1000
);

</script>

</body>
</html>
)rawliteral";

// ============================================================
// Interval
// ============================================================

unsigned long getIntervalMinutes()
{
    return
        (sendInterval / 1000UL) / 60UL;
}

unsigned long getIntervalSeconds()
{
    return
        (sendInterval / 1000UL) % 60UL;
}

// ============================================================
// Modifier keys
// ============================================================

bool isModifier(
    String token,
    uint8_t &key
)
{
    token.trim();
    token.toUpperCase();

    if (
        token == "CTRL" ||
        token == "CONTROL" ||
        token == "LCTRL"
    ) {
        key = KEY_LEFT_CTRL;
        return true;
    }

    if (
        token == "SHIFT" ||
        token == "LSHIFT"
    ) {
        key = KEY_LEFT_SHIFT;
        return true;
    }

    if (
        token == "ALT" ||
        token == "LALT"
    ) {
        key = KEY_LEFT_ALT;
        return true;
    }

    if (
        token == "WIN" ||
        token == "WINDOWS" ||
        token == "GUI" ||
        token == "META"
    ) {
        key = KEY_LEFT_GUI;
        return true;
    }

    if (token == "RCTRL") {
        key = KEY_RIGHT_CTRL;
        return true;
    }

    if (token == "RSHIFT") {
        key = KEY_RIGHT_SHIFT;
        return true;
    }

    if (
        token == "RALT" ||
        token == "ALTGR"
    ) {
        key = KEY_RIGHT_ALT;
        return true;
    }

    if (token == "RWIN") {
        key = KEY_RIGHT_GUI;
        return true;
    }

    return false;
}

// ============================================================
// Special keys
// ============================================================

uint8_t getSpecialKey(
    String token
)
{
    token.trim();
    token.toUpperCase();

    if (token == "F1")  return KEY_F1;
    if (token == "F2")  return KEY_F2;
    if (token == "F3")  return KEY_F3;
    if (token == "F4")  return KEY_F4;
    if (token == "F5")  return KEY_F5;
    if (token == "F6")  return KEY_F6;
    if (token == "F7")  return KEY_F7;
    if (token == "F8")  return KEY_F8;
    if (token == "F9")  return KEY_F9;
    if (token == "F10") return KEY_F10;
    if (token == "F11") return KEY_F11;
    if (token == "F12") return KEY_F12;
    if (token == "F13") return KEY_F13;
    if (token == "F14") return KEY_F14;
    if (token == "F15") return KEY_F15;
    if (token == "F16") return KEY_F16;
    if (token == "F17") return KEY_F17;
    if (token == "F18") return KEY_F18;
    if (token == "F19") return KEY_F19;
    if (token == "F20") return KEY_F20;
    if (token == "F21") return KEY_F21;
    if (token == "F22") return KEY_F22;
    if (token == "F23") return KEY_F23;
    if (token == "F24") return KEY_F24;

    if (
        token == "ESC" ||
        token == "ESCAPE"
    )
        return KEY_ESC;

    if (
        token == "ENTER" ||
        token == "RETURN"
    )
        return KEY_RETURN;

    if (token == "TAB")
        return KEY_TAB;

    if (
        token == "SPACE" ||
        token == "SPACEBAR"
    )
        return ' ';

    if (
        token == "DELETE" ||
        token == "DEL"
    )
        return KEY_DELETE;

    if (
        token == "BACKSPACE" ||
        token == "BS"
    )
        return KEY_BACKSPACE;

    if (token == "HOME")
        return KEY_HOME;

    if (token == "END")
        return KEY_END;

    if (
        token == "PAGEUP" ||
        token == "PGUP"
    )
        return KEY_PAGE_UP;

    if (
        token == "PAGEDOWN" ||
        token == "PGDN"
    )
        return KEY_PAGE_DOWN;

    if (token == "LEFT")
        return KEY_LEFT_ARROW;

    if (token == "RIGHT")
        return KEY_RIGHT_ARROW;

    if (token == "UP")
        return KEY_UP_ARROW;

    if (token == "DOWN")
        return KEY_DOWN_ARROW;

    if (
        token == "SCROLLLOCK" ||
        token == "SCROLL_LOCK" ||
        token == "SCROLL"
    )
        return KEY_SCROLL_LOCK_CUSTOM;

    return 0;
}

// ============================================================
// Validate combination
// ============================================================

bool validateCombination(
    String combo
)
{
    combo.trim();
    combo.toUpperCase();

    if (
        combo.length() == 0
    )
        return false;

    if (
        combo == "SCROLLLOCKX2" ||
        combo == "SCROLLX2"
    ) {
        return true;
    }

    int start = 0;

    while (
        start < combo.length()
    ) {

        int plus =
            combo.indexOf(
                '+',
                start
            );

        String token;

        if (plus == -1) {
            token =
                combo.substring(start);
        }
        else {
            token =
                combo.substring(
                    start,
                    plus
                );
        }

        token.trim();

        if (
            token.length() == 0
        )
            return false;

        uint8_t modifier;

        if (
            isModifier(
                token,
                modifier
            )
        ) {
            // OK
        }
        else {

            uint8_t special =
                getSpecialKey(token);

            if (
                special != 0
            ) {
                // OK
            }
            else {

                if (
                    token.length() != 1
                ) {
                    return false;
                }
            }
        }

        if (
            plus == -1
        )
            break;

        start =
            plus + 1;
    }

    return true;
}

// ============================================================
// Scroll Lock x2
// ============================================================

bool sendScrollLockX2()
{
    if (
        !bleKeyboard.isConnected()
    ) {
        return false;
    }

    // First Scroll Lock
    bleKeyboard.press(
        KEY_SCROLL_LOCK_CUSTOM
    );

    delay(50);

    bleKeyboard.release(
        KEY_SCROLL_LOCK_CUSTOM
    );

    delay(120);

    // Second Scroll Lock
    bleKeyboard.press(
        KEY_SCROLL_LOCK_CUSTOM
    );

    delay(50);

    bleKeyboard.release(
        KEY_SCROLL_LOCK_CUSTOM
    );

    bleKeyboard.releaseAll();

    keyCount++;

    Serial.print(
        "Sent: SCROLLLOCKX2 Count: "
    );

    Serial.println(
        keyCount
    );

    updateDisplay();

    return true;
}

// ============================================================
// Send key combination
// ============================================================

bool sendKeyCombination()
{
    if (
        !bleKeyboard.isConnected()
    ) {
        return false;
    }

    String comboUpper =
        keyCombination;

    comboUpper.trim();
    comboUpper.toUpperCase();

    if (
        comboUpper == "SCROLLLOCKX2" ||
        comboUpper == "SCROLLX2"
    ) {
        return sendScrollLockX2();
    }

    int start = 0;

    while (
        start < keyCombination.length()
    ) {

        int plus =
            keyCombination.indexOf(
                '+',
                start
            );

        String token;

        if (plus == -1) {
            token =
                keyCombination.substring(start);
        }
        else {
            token =
                keyCombination.substring(
                    start,
                    plus
                );
        }

        token.trim();
        token.toUpperCase();

        uint8_t modifier;

        if (
            isModifier(
                token,
                modifier
            )
        ) {

            bleKeyboard.press(
                modifier
            );

        }
        else {

            uint8_t special =
                getSpecialKey(token);

            if (
                special != 0
            ) {

                bleKeyboard.press(
                    special
                );

            }
            else if (
                token.length() == 1
            ) {

                char c =
                    token[0];

                if (
                    c >= 'A' &&
                    c <= 'Z'
                ) {
                    c =
                        c - 'A' + 'a';
                }

                bleKeyboard.press(
                    c
                );
            }
        }

        if (
            plus == -1
        )
            break;

        start =
            plus + 1;
    }

    delay(
        KEY_HOLD_MS
    );

    bleKeyboard.releaseAll();

    keyCount++;

    Serial.print(
        "Sent: "
    );

    Serial.print(
        keyCombination
    );

    Serial.print(
        " Count: "
    );

    Serial.println(
        keyCount
    );

    updateDisplay();

    return true;
}

// ============================================================
// Start
// ============================================================

bool startSending()
{
    if (
        !bleKeyboard.isConnected()
    ) {
        return false;
    }

    // Send IMMEDIATELY
    if (
        !sendKeyCombination()
    ) {
        return false;
    }

    // Then start the full interval
    lastSendTime =
        millis();

    sendingEnabled =
        true;

    Serial.println(
        "Sending STARTED"
    );

    updateDisplay(true);

    return true;
}

// ============================================================
// Stop
// ============================================================

void stopSending()
{
    sendingEnabled =
        false;

    if (
        bleKeyboard.isConnected()
    ) {
        bleKeyboard.releaseAll();
    }

    Serial.println(
        "Sending STOPPED"
    );

    updateDisplay(true);
}

// ============================================================
// Toggle
// ============================================================

bool toggleSending()
{
    if (
        sendingEnabled
    ) {

        stopSending();

        return true;
    }

    return startSending();
}

// ============================================================
// Button A
// ============================================================

void handleButtonA()
{
    toggleSending();
}

// ============================================================
// JSON escape
// ============================================================

String jsonEscape(
    String value
)
{
    value.replace(
        "\\",
        "\\\\"
    );

    value.replace(
        "\"",
        "\\\""
    );

    return value;
}

// ============================================================
// JSON status
// ============================================================

String getStatusJson()
{
    String json = "{";

    json += "\"sending\":";

    json +=
        sendingEnabled
        ? "true"
        : "false";

    json += ",";

    json += "\"bluetooth\":";

    json +=
        bleKeyboard.isConnected()
        ? "true"
        : "false";

    json += ",";

    json += "\"wifi\":";

    json +=
        WiFi.status() == WL_CONNECTED
        ? "true"
        : "false";

    json += ",";

    json += "\"minutes\":";

    json +=
        String(
            getIntervalMinutes()
        );

    json += ",";

    json += "\"seconds\":";

    json +=
        String(
            getIntervalSeconds()
        );

    json += ",";

    json += "\"combo\":\"";

    json +=
        jsonEscape(
            keyCombination
        );

    json += "\"";

    json += ",";

    json += "\"count\":";

    json +=
        String(
            keyCount
        );

    json += ",";

    json += "\"ip\":\"";

    json +=
        WiFi.localIP().toString();

    json += "\"";

    json += "}";

    return json;
}

// ============================================================
// Display
// ============================================================

void updateDisplay(
    bool force
)
{
    bool wifiConnected =
        WiFi.status() == WL_CONNECTED;

    bool bleConnected =
        bleKeyboard.isConnected();

    if (
        !force &&
        !firstDisplayUpdate &&
        lastDisplaySending == sendingEnabled &&
        lastDisplayBle == bleConnected &&
        lastDisplayWifi == wifiConnected &&
        lastDisplayCount == keyCount &&
        lastDisplayInterval == sendInterval &&
        lastDisplayCombo == keyCombination
    ) {
        return;
    }

    firstDisplayUpdate =
        false;

    lastDisplaySending =
        sendingEnabled;

    lastDisplayBle =
        bleConnected;

    lastDisplayWifi =
        wifiConnected;

    lastDisplayCount =
        keyCount;

    lastDisplayInterval =
        sendInterval;

    lastDisplayCombo =
        keyCombination;

    M5.Lcd.fillScreen(
        BLACK
    );

    // Heading
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(
        CYAN,
        BLACK
    );

    M5.Lcd.setCursor(
        17,
        2
    );

    M5.Lcd.print(
        "KEY SENDER"
    );

    // WiFi
    M5.Lcd.setTextSize(1);

    M5.Lcd.setCursor(
        3,
        20
    );

    M5.Lcd.setTextColor(
        wifiConnected
        ? GREEN
        : RED,
        BLACK
    );

    M5.Lcd.print(
        wifiConnected
        ? "WiFi:OK"
        : "WiFi:--"
    );

    // BLE
    M5.Lcd.setCursor(
        95,
        20
    );

    M5.Lcd.setTextColor(
        bleConnected
        ? GREEN
        : ORANGE,
        BLACK
    );

    M5.Lcd.print(
        bleConnected
        ? "BLE:OK"
        : "BLE:--"
    );

    // Status
    M5.Lcd.setTextSize(2);

    M5.Lcd.setCursor(
        37,
        32
    );

    M5.Lcd.setTextColor(
        sendingEnabled
        ? GREEN
        : RED,
        BLACK
    );

    M5.Lcd.print(
        sendingEnabled
        ? "RUNNING"
        : "STOPPED"
    );

    // Combination
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(
        WHITE,
        BLACK
    );

    M5.Lcd.setCursor(
        3,
        52
    );

    String displayCombo =
        keyCombination;

    if (
        displayCombo.length() > 20
    ) {
        displayCombo =
            displayCombo.substring(
                0,
                20
            );
    }

    M5.Lcd.print(
        displayCombo
    );

    // Interval
    M5.Lcd.setCursor(
        3,
        64
    );

    M5.Lcd.printf(
        "%02lu:%02lu",
        getIntervalMinutes(),
        getIntervalSeconds()
    );

    // Counter
    M5.Lcd.setCursor(
        105,
        64
    );

    M5.Lcd.printf(
        "#%lu",
        keyCount
    );

    // IP
    M5.Lcd.setCursor(
        3,
        73
    );

    if (
        wifiConnected
    ) {
        M5.Lcd.print(
            WiFi.localIP()
        );
    }
}

// ============================================================
// Web server
// ============================================================

void setupWebServer()
{
    // Web interface
    server.on(
        "/",
        HTTP_GET,
        []()
        {
            server.send_P(
                200,
                "text/html",
                INDEX_HTML
            );
        }
    );

    // Full status
    server.on(
        "/api/status",
        HTTP_GET,
        []()
        {
            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // Simple Companion status
    server.on(
        "/api/running",
        HTTP_GET,
        []()
        {
            server.send(
                200,
                "text/plain",
                sendingEnabled
                    ? "1"
                    : "0"
            );
        }
    );

    // Start
    server.on(
        "/api/start",
        HTTP_POST,
        []()
        {
            if (
                !startSending()
            ) {

                server.send(
                    503,
                    "application/json",
                    "{\"error\":\"Bluetooth keyboard not connected\"}"
                );

                return;
            }

            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // Stop
    server.on(
        "/api/stop",
        HTTP_POST,
        []()
        {
            stopSending();

            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // Toggle
    server.on(
        "/api/toggle",
        HTTP_POST,
        []()
        {
            if (
                !toggleSending()
            ) {

                server.send(
                    503,
                    "application/json",
                    "{\"error\":\"Bluetooth keyboard not connected\"}"
                );

                return;
            }

            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // Send now
    server.on(
        "/api/send",
        HTTP_POST,
        []()
        {
            if (
                !sendKeyCombination()
            ) {

                server.send(
                    503,
                    "application/json",
                    "{\"error\":\"Bluetooth keyboard not connected\"}"
                );

                return;
            }

            if (
                sendingEnabled
            ) {
                lastSendTime =
                    millis();
            }

            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // Settings
    server.on(
        "/api/settings",
        HTTP_POST,
        []()
        {
            if (
                !server.hasArg("combo") ||
                !server.hasArg("minutes") ||
                !server.hasArg("seconds")
            ) {

                server.send(
                    400,
                    "application/json",
                    "{\"error\":\"Missing parameters\"}"
                );

                return;
            }

            String newCombo =
                server.arg("combo");

            newCombo.trim();
            newCombo.toUpperCase();

            if (
                !validateCombination(
                    newCombo
                )
            ) {

                server.send(
                    400,
                    "application/json",
                    "{\"error\":\"Invalid key combination\"}"
                );

                return;
            }

            long minutes =
                server.arg(
                    "minutes"
                ).toInt();

            long seconds =
                server.arg(
                    "seconds"
                ).toInt();

            if (
                minutes < 0 ||
                seconds < 0 ||
                seconds > 59
            ) {

                server.send(
                    400,
                    "application/json",
                    "{\"error\":\"Invalid interval\"}"
                );

                return;
            }

            unsigned long totalSeconds =
                ((unsigned long)minutes * 60UL) +
                (unsigned long)seconds;

            if (
                totalSeconds == 0
            ) {

                server.send(
                    400,
                    "application/json",
                    "{\"error\":\"Minimum interval is 1 second\"}"
                );

                return;
            }

            keyCombination =
                newCombo;

            sendInterval =
                totalSeconds * 1000UL;

            if (
                sendingEnabled
            ) {
                lastSendTime =
                    millis();
            }

            updateDisplay(true);

            server.send(
                200,
                "application/json",
                getStatusJson()
            );
        }
    );

    // 404
    server.onNotFound(
        []()
        {
            server.send(
                404,
                "application/json",
                "{\"error\":\"Not found\"}"
            );
        }
    );

    server.begin();
}

// ============================================================
// WiFi
// ============================================================

void connectWiFi()
{
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(
        YELLOW,
        BLACK
    );

    M5.Lcd.setCursor(
        3,
        30
    );

    M5.Lcd.print(
        "Connecting WiFi..."
    );

    WiFi.mode(
        WIFI_STA
    );

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    Serial.print(
        "Connecting WiFi"
    );

    while (
        WiFi.status()
        != WL_CONNECTED
    ) {

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.print(
        "IP: "
    );

    Serial.println(
        WiFi.localIP()
    );
}

// ============================================================
// Setup
// ============================================================

void setup()
{
    M5.begin();

    Serial.begin(
        115200
    );

    delay(500);

    M5.Lcd.setRotation(3);

    M5.Lcd.fillScreen(
        BLACK
    );

    connectWiFi();

    Serial.println(
        "Starting BLE keyboard..."
    );

    bleKeyboard.begin();

    setupWebServer();

    updateDisplay(true);

    Serial.println(
        "KEY SENDER ready"
    );

    Serial.print(
        "Web: http://"
    );

    Serial.println(
        WiFi.localIP()
    );
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    M5.update();

    // Knapp A:
    // stopped -> send immediately and start
    // running -> stop
    if (
        M5.BtnA.wasPressed()
    ) {
        handleButtonA();
    }

    server.handleClient();

    // Automatic sending
    if (
        sendingEnabled &&
        bleKeyboard.isConnected()
    ) {

        unsigned long now =
            millis();

        if (
            now - lastSendTime
            >= sendInterval
        ) {

            sendKeyCombination();

            lastSendTime =
                millis();
        }
    }

    updateDisplay();

    delay(5);
}