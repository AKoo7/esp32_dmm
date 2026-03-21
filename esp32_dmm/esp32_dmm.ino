#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "xxxx";
const char* password = "xxxx";
WebServer server(80);

String currentMode1 = "?";
String currentRange = "?";
String currentUnit = "";
String currentReading = "N/A";

HardwareSerial DMM(2);

#define RXD2 16
#define TXD2 17
#define BAUD 2400

struct ModeInfo {
  const char* mode;
  const char* range;
  float scale;
  const char* unit;
};

struct ModeEntry {
  uint16_t code;
  ModeInfo info;
};

ModeEntry MODE_TABLE[] = {
  {205, {"R", "200MΩ", 10, "Ω"}},
  {394, {"R", "20MΩ", 100, "Ω"}},
  {397, {"R", "200KΩ", 10, "Ω"}},
  {398, {"R", "20KΩ", 100, "Ω"}},
  {396, {"R", "2KΩ", 1, "Ω"}},
  {412, {"R", "200Ω", 10, "Ω"}},
  {410, {"D", "Diode", 0, ""}},
  {34,  {"A", "2000mA", 1000, "A"}},
  {33,  {"A", "20mA", 1000, "A"}},
  {26,  {"A", "200mA", 1000, "A"}},
  {32,  {"A", "10A", 1, "A"}},
  {37,  {"V_AC", "200V", 10, "V"}},
  {19,  {"V_AC", "600V", 10, "V"}},
  {20,  {"V_DC", "600V", 10, "V"}},
  {21,  {"V_DC", "200V", 10, "V"}},
  {54,  {"V_DC", "20V", 100, "V"}},
  {271, {"V_DC", "2000mV", 1000, "V"}},
  {48,  {"V_DC", "200mV", 1000, "V"}},
};

ModeInfo currentMode = {"?", "?", 0, ""};

uint8_t buffer[256];
int buf_len = 0;

bool findMode(uint16_t code, ModeInfo &info) {
  for (auto &entry : MODE_TABLE) {
    if (entry.code == code) {
      info = entry.info;
      return true;
    }
  }
  return false;
}

void handleRoot() {
String html = "<!DOCTYPE html><html><head>"
              "<meta charset='UTF-8'>"
              "<meta http-equiv='refresh' content='1'>"
              "<title>DMM ESP32</title>"
              "<style>"
              "body { background-color: #121212; color: #e0e0e0; font-family: Arial; text-align: center; margin-top: 40px; }"
              "h2 { color: #ffffff; }"
              "p { font-size: 18px; }"
              ".reading { font-size: 60px; font-weight: bold; "
              "color: #00ff00; background-color: #000000; "
              "display: inline-block; padding: 15px 25px; border-radius: 8px; "
              "font-family: 'Courier New', monospace; letter-spacing: 2px; }"
              "</style>"
              "</head><body>"
              "<h2>ESP32 DMM Reader</h2>"
              "<p>Mode: " + String(currentMode1) + "</p>"
              "<p>Range: " + String(currentRange) + "</p>"
              "<p class='reading'>" + String(currentReading) + " " + String(currentUnit) + "</p>"
              "</body></html>";

server.send(200, "text/html", html);
}

void processFrame(uint8_t *frame, int len) {
  if (len < 6) return;

  uint8_t device = frame[2];
  uint8_t msg = frame[3];

  int payload_len = len - 4;

  if (payload_len % 2 != 0) return;

  int values_count = payload_len / 2;
  uint16_t values[16];

  for (int i = 0; i < values_count; i++) {
    values[i] = (frame[4 + i * 2] << 8) | frame[5 + i * 2];
  }

  if (values_count < 3) return;

  uint16_t raw_value = values[1];
  uint16_t code = values[2];

  ModeInfo info;
  if (findMode(code, info)) {
    currentMode = info;
  }

  String reading;

  if (raw_value == 32767) {
    reading = "OL";
  } else if (currentMode.scale > 0) {
    float value = raw_value / currentMode.scale;
    reading = String(value, 3);
  } else {
    reading = "N/A";
  }

  Serial.print("Mode=");
  Serial.print(currentMode.mode);
  Serial.print(" Range=");
  Serial.print(currentMode.range);
  Serial.print(" Reading=");
  Serial.print(reading);
  Serial.print(" ");
  Serial.println(currentMode.unit);
  currentMode1 = currentMode.mode;
  currentRange = currentMode.range;
  currentUnit = currentMode.unit;
  currentReading = reading;
}

void splitFrames() {
  int start = 0;

  while (true) {
    int idx = -1;

    for (int i = start; i < buf_len - 1; i++) {
      if (buffer[i] == 0xAB && buffer[i + 1] == 0xCD) {
        idx = i;
        break;
      }
    }

    if (idx == -1) break;

    int next_idx = -1;
    for (int i = idx + 2; i < buf_len - 1; i++) {
      if (buffer[i] == 0xAB && buffer[i + 1] == 0xCD) {
        next_idx = i;
        break;
      }
    }

    int frame_len;
    if (next_idx == -1) {
      break;
    } else {
      frame_len = next_idx - idx;
    }

    processFrame(&buffer[idx], frame_len);
    start = next_idx;
  }

  if (start > 0) {
    memmove(buffer, buffer + start, buf_len - start);
    buf_len -= start;
  }
}

void setup() {
  Serial.begin(115200);
  DMM.begin(BAUD, SERIAL_8N1, RXD2, TXD2);

  Serial.println("UT33D+ ESP32 Decoder Started");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.begin();

}

void loop() {
  server.handleClient();
  while (DMM.available()) {
    uint8_t b = DMM.read();
    if (buf_len < sizeof(buffer)) {
      buffer[buf_len++] = b;
    }

  }

  if (buf_len > 4) {
    splitFrames();
  }
}