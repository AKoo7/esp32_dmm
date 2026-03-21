# 📟 ESP32 Digital Multimeter (DMM) Web Reader

Turn your **serial-output digital multimeter** into a **live WiFi-enabled dashboard** using an ESP32.

This project reads measurement data from a DMM (e.g., UT33D+), decodes it, and serves a **real-time web interface** showing:

* Mode (Voltage, Current, Resistance, etc.)
* Range (e.g., 200V, 20KΩ)
* Live reading (with units)

---

## 🚀 Features

* 📡 Reads DMM data via UART (HardwareSerial)
* 🌐 Hosts a live web dashboard on ESP32
* 🔄 Auto-refresh every second
* 🌙 Dark mode UI with digital-style display
* 🔍 Supports multiple measurement modes:

  * Voltage (AC/DC)
  * Current (mA / A)
  * Resistance (Ω, KΩ, MΩ)
  * Diode mode

---

## 🧰 Hardware Requirements

* ESP32 board
* Digital Multimeter with serial output (e.g., **UNI-T UT33D+**)
* Connecting wires
* Optional: level shifter (depending on DMM output)

---

## 🔌 Wiring

| ESP32 Pin    | DMM |
| ------------ | --- |
| GPIO16 (RX2) | TX  |
| GPIO17 (TX2) | RX  |

> ⚠️ Ensure voltage compatibility before connecting!

---

## ⚙️ Configuration

Update your WiFi credentials in the code:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_PASSWORD";
```

---

## ▶️ How It Works

1. ESP32 reads serial data from the DMM at **2400 baud**
2. Incoming bytes are buffered and split into frames
3. Frames are decoded into:

   * Raw value
   * Mode code
4. Mode is mapped using a lookup table (`MODE_TABLE`)
5. Values are scaled and formatted
6. Data is displayed on a web page

---

## 🌐 Web Interface

Once connected, open your browser and go to:

```
http://<ESP32_IP_ADDRESS>/
```

### UI Features:

* Dark mode theme 🌙
* Large green digital-style reading
* Auto-refresh every second

---

## 📊 Example Output

```
Mode: V_DC
Range: 20V
Reading: 12.345 V
```

---

## 🧠 Mode Mapping

The project uses a lookup table:

```cpp
struct ModeEntry {
  uint16_t code;
  ModeInfo info;
};
```

Each incoming code is mapped to:

* Mode (V, A, R, etc.)
* Range
* Scale factor
* Unit

---

## ⚠️ Notes

* `32767` is treated as **OL (Overload)**
* UTF-8 encoding is enabled for proper symbols like `Ω`
* Not all DMMs use the same protocol—this is tailored for UT33D+

---

## 🛠️ Future Improvements

* 📈 Graph plotting (real-time charts)
* 📱 Mobile-friendly UI
* 🔌 MQTT / Home Assistant integration
* 💾 Data logging (SD card or cloud)
* ⚡ AJAX (no page refresh)

---

## 📄 License

MIT License — feel free to use and modify.

---

## 🙌 Acknowledgements

* Inspired by reverse-engineering of DMM serial protocols
* ESP32 community & Arduino ecosystem ❤️
* images taken from - https://lygte-info.dk/review/DMMUNI-T%20UT33D%2B%20UK.html

---

## 📬 Contributing

Pull requests are welcome!
If you find a new mode/code, feel free to add it to `MODE_TABLE`.

---
