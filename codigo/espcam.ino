#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <base64.h>
#include "esp_camera.h"
#include "esp_sleep.h"

const char* ssid = "MotoG42";
const char* password = "senha123";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "Esp32Cam";  /

WiFiClient espClient;
PubSubClient client(espClient);

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const int wakeUpPin = 13;    
const int flashPin = 4;      

void configInit(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera initialized successfully");
  client.setBufferSize(16384);

  client.setServer(mqtt_server, mqtt_port);
  connectMQTT();
}

void setup() {
  Serial.begin(115200);
  pinMode(wakeUpPin, INPUT_PULLDOWN);
  pinMode(flashPin, OUTPUT);

  if (digitalRead(wakeUpPin) == HIGH) {
    Serial.println("Acordando do Deep Sleep...");
    digitalWrite(flashPin, HIGH);  
    delay(4000); 
    digitalWrite(flashPin, LOW);   

    delay(1000); 

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    configInit();
  } else {
    Serial.println("Entrando em Deep Sleep...");
  }

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);  
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32CAM_Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void sendPhoto() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  String base64Image = base64::encode(fb->buf, fb->len);
  Serial.println("Base64 Image Length: " + String(base64Image.length())); 

  int fragmentSize = 13500;
  Serial.println("Sending image in fragments...");
  
  for (int i = 0; i < base64Image.length(); i += fragmentSize) {
    String chunk = "data:image/jpeg;base64," + base64Image.substring(i, i + fragmentSize);
    if (client.connected()) { 
      client.publish(mqtt_topic, chunk.c_str());
      delay(85); 
    } else {
      Serial.println("Not connected to MQTT. Reconnecting...");
      connectMQTT(); 
    }
  }

  Serial.println("Imagem enviada em partes.");
  esp_camera_fb_return(fb); 
  delay(50);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();  
  }

  client.loop();  

  sendPhoto();

  Serial.println("Entrando em Deep Sleep...");
  esp_deep_sleep_start();
}
