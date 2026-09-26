/***************************************************
  Project : Restart POE and Access Point
  Board   : ESP8266 (NodeMCU / Wemos D1 mini) + W5500 Ethernet
  Control : Blynk (Web/Mobile)
***************************************************/

#define BLYNK_TEMPLATE_ID   "TMPL6dOVuMV0g"
#define BLYNK_TEMPLATE_NAME "Restart POE and Access Point"
#define BLYNK_AUTH_TOKEN    "hw_b1TrtzAEWHgVqqqjjLRsl2hreu8NP"

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>

// CS pin Ethernet W5500 (sesuaikan dengan wiring)
#define WIZ_CS D4

// Pin relay (aktif LOW)
#define RELAY1 D1   // GPIO5 → Relay 1 (POE)
#define RELAY2 D2   // GPIO4 → Relay 2 (Access Point)

BlynkTimer timer;

// Konfigurasi Static IP (fallback)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
IPAddress ip(192, 168, 1, 200);
IPAddress dns(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

int reconnectAttempts = 0;

// =============================
// 🔄 Fungsi cek koneksi rutin
// =============================
void checkConnection() {
  if (!Blynk.connected()) {
    Serial.println("[WARN] Blynk terputus, mencoba reconnect...");
    if (Blynk.connect(5000)) {
      Serial.println("[OK] Reconnected to Blynk.");
      reconnectAttempts = 0;
      Blynk.syncAll(); // Sinkron ulang status tombol setelah reconnect
    } else {
      Serial.println("[FAIL] Reconnect gagal.");
      reconnectAttempts++;
      if (reconnectAttempts >= 6) {
        Serial.println("[CRIT] Restart ESP karena gagal reconnect 6x.");
        ESP.restart();
      }
    }
  }
}

// =============================
// ⚙️ SETUP
// =============================
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // Relay default OFF (aktif LOW)
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  Ethernet.init(WIZ_CS);
  Serial.println("\n[INIT] Mencoba DHCP...");

  if (Ethernet.begin(mac) == 0) {
    Serial.println("[WARN] DHCP gagal, gunakan Static IP.");
    Ethernet.begin(mac, ip, dns, gateway, subnet);
  }

  Serial.print("[NET] IP address: ");
  Serial.println(Ethernet.localIP());

  // Konfigurasi token tanpa auto-connect (lebih stabil)
  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println("[BLYNK] Mencoba koneksi ke server...");
  if (Blynk.connect(5000)) {
    Serial.println("[OK] Blynk connected.");
    Blynk.syncAll();  // Sinkron status awal tombol
  } else {
    Serial.println("[FAIL] Tidak dapat connect ke Blynk (akan dicoba otomatis).");
  }

  // Jalankan pengecekan koneksi tiap 10 detik
  timer.setInterval(10000L, checkConnection);
}

// =============================
// 🧠 Fungsi kontrol dari Blynk
// =============================

// Tombol V0 → Relay 1 (POE)
BLYNK_WRITE(V0) {
  int value = param.asInt();
  digitalWrite(RELAY1, value ? LOW : HIGH); // aktif LOW
  Serial.print("[RELAY1] POE: ");
  Serial.println(value ? "ON" : "OFF");
}

// Tombol V1 → Relay 2 (Access Point)
BLYNK_WRITE(V1) {
  int value = param.asInt();
  digitalWrite(RELAY2, value ? LOW : HIGH);
  Serial.print("[RELAY2] Access Point: ");
  Serial.println(value ? "ON" : "OFF");
}

// =============================
// 🔁 LOOP
// =============================
void loop() {
  Blynk.run();
  timer.run();
}
