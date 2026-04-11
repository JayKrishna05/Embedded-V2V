#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUZZER_PIN 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "ESP32_V2V";
const char* password = "12345678";

WebServer server(80);

// ---------- STRUCT ----------
struct SystemData {
  float speed;
  float heading;
  float distance;
  float ttc;
  int rssi;
  bool danger;
};

SystemData data = {0, 0, 1, 99, -90, false};

// ---------- MUTEX ----------
SemaphoreHandle_t dataMutex;

// ---------- RSSI Calibration ----------
int A = -58;
float n = 2.5;

// ---------- Kalman ----------
float kalman_d = 1;
float P = 1;
float Q = 0.08;
float R = 3;

// ---------- FUNCTIONS ----------
float getDistance(int rssi) {
  float d = pow(10.0, (A - rssi) / (10.0 * n));
  if (d < 0.2) d = 0.2;
  return d;
}
float predict_distance(float rssi) { //ml random forest regression based prediction
    float a = 0.009403;
    float b = 1.0079;
    float c = 27.6746;

    float distance = a * rssi * rssi + b * rssi + c;

    // Safety clamp
    if (distance < 0.2) distance = 0.2;
    if (distance > 20.0) distance = 20.0;

    return distance;
}
int getRSSI() {
  wifi_sta_list_t list;
  if (esp_wifi_ap_get_sta_list(&list) == ESP_OK && list.num > 0) {
    return list.sta[0].rssi;
  }
  return -90;
}

// ---------- HTTP ----------
void handleUpdate() {
  if (server.hasArg("speed") && server.hasArg("heading")) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    data.speed = server.arg("speed").toFloat();
    data.heading = server.arg("heading").toFloat();
    xSemaphoreGive(dataMutex);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing data");
  }
}

// ---------- RADAR ----------
void drawRadar(float x, float y, bool danger) {
  int cx = 96, cy = 32;

  display.drawCircle(cx, cy, 10, WHITE);
  display.drawCircle(cx, cy, 20, WHITE);

  display.drawLine(cx-20, cy, cx+20, cy, WHITE);
  display.drawLine(cx, cy-20, cx, cy+20, WHITE);

  display.fillCircle(cx, cy, 2, WHITE);

  int px = cx + (int)(x * 20);
  int py = cy - (int)(y * 20);

  px = constrain(px, 0, 127);
  py = constrain(py, 0, 63);

  if (danger)
    display.fillCircle(px, py, 4, WHITE);
  else
    display.fillCircle(px, py, 2, WHITE);
}

// ---------- TASK: HTTP ----------
void taskHTTP(void *pvParameters) {
  while (1) {
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ---------- TASK: PROCESS ----------
void taskProcessing(void *pvParameters) {
  while (1) {

    int rssi = getRSSI();
    //float d = getDistance(rssi); local model distance calculation
    float d=predict_distance(rssi);

    // Kalman Filter
    P += Q;
    float K = P / (P + R);
    kalman_d = kalman_d + K * (d - kalman_d);
    P = (1 - K) * P;

    float local_speed, local_heading;

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    local_speed = data.speed;
    local_heading = data.heading;
    xSemaphoreGive(dataMutex);

    float rad = local_heading * PI / 180.0;
    float closing_speed = local_speed * cos(rad);

    float local_ttc = 99;
    if (closing_speed > 0.05)
      local_ttc = kalman_d / closing_speed;

    if (isnan(local_ttc) || isinf(local_ttc) || local_ttc < 0)
      local_ttc = 99;

    bool local_danger = false;
    if (local_ttc < 1.2) local_danger = true;
    else if (local_ttc > 2.5) local_danger = false;

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    data.distance = kalman_d;
    data.ttc = local_ttc;
    data.rssi = rssi;
    data.danger = local_danger;
    xSemaphoreGive(dataMutex);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ---------- TASK: DISPLAY ----------
void taskDisplay(void *pvParameters) {
  while (1) {

    SystemData local;

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    local = data;
    xSemaphoreGive(dataMutex);

    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("R:");
    display.println(local.rssi);

    display.print("D:");
    display.println(local.distance, 1);

    display.print("H:");
    display.println(local.heading, 0);

    display.print("S:");
    display.println(local.speed, 1);

    display.setCursor(0, 40);

    if (local.ttc < 1.5) display.print("DANGER ");
    else if (local.ttc < 3.0) display.print("CAUTION ");
    else display.print("SAFE ");

    display.print(local.ttc, 1);

    drawRadar(0, local.distance / 2, local.danger);

    display.display();

    vTaskDelay(120 / portTICK_PERIOD_MS);
  }
}

// ---------- TASK: BUZZER ----------
void taskBuzzer(void *pvParameters) {
  bool state = false;

  while (1) {

    bool dg;

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    dg = data.danger;
    xSemaphoreGive(dataMutex);

    if (dg) {
      state = !state;
      digitalWrite(BUZZER_PIN, state);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.softAP(ssid, password);
  server.on("/update", handleUpdate);
  server.begin();

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);

  dataMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(taskHTTP, "HTTP", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskProcessing, "PROCESS", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(taskDisplay, "DISPLAY", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskBuzzer, "BUZZER", 2048, NULL, 2, NULL, 1);
}

// ---------- LOOP ----------
void loop() {
  // Empty - RTOS handles everything
}