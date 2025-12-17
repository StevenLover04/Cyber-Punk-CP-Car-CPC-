#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// REPLACE this with the SLAVE ESP32 MAC address
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure to send data
// MUST match the receiver exactly
typedef struct struct_message {
  int x_angle;
  int y_angle;
  int z_angle;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Send Success" : "Send Fail");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (mpu.begin() != 0) {
    Serial.println("MPU6050 connection failed");
    while (true);
  }

  Serial.println("Calibrating MPU6050...");
  delay(1000);
  mpu.calcOffsets();
  Serial.println("Calibration complete");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  mpu.update();

  myData.x_angle = (int)mpu.getAngleX();
  myData.y_angle = (int)mpu.getAngleY();
  myData.z_angle = (int)mpu.getAngleZ();

  esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));

  delay(1000);
}




-----------




#include <esp_now.h>
#include <WiFi.h>

// MUST match the sender exactly
typedef struct struct_message {
  int x_angle;
  int y_angle;
  int z_angle;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  Serial.print("X: ");
  Serial.print(myData.x_angle);
  Serial.print("  Y: ");
  Serial.print(myData.y_angle);
  Serial.print("  Z: ");
  Serial.println(myData.z_angle);


  // At this point were just making if cases for when the 
  // robot moves left right up down and whatever

  if (myData.x_angle > 45) {
    Serial.println("X angle greater than 45");
    // put your action here
    // Figure out the pin number and what servo were running
    // https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/
  }

  delay(200);
}
