#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHTesp.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Thông tin Firebase
#define FIREBASE_HOST "https://detectgas-43a29-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "iYJddeE6KayFNR4wqkKCCHHU213j6sGfwSZjBUMj"

FirebaseData firebaseData;
FirebaseJson json;

// Định nghĩa các chân kết nối
DHTesp dht;
const int dhtPin = D1; // ESP8266 D1
const int mqPin = A0;
const int fan = D5;    // ESP8266 D5
const int coi = D4;    // ESP8266 D4
const int led = D3;    // ESP8266 D3

LiquidCrystal_I2C lcd(0x27, 16, 2); // Đặt địa chỉ LCD là 0x27 cho một hiển thị 16 ký tự và 2 dòng

void setup() {
  pinMode(fan, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(coi, OUTPUT);

  Serial.begin(9600);

  WiFiManager wm;
  // Bắt đầu quản lý Wi-Fi
  wm.autoConnect("AP-Name", "password");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Đã kết nối với Firebase");

  dht.setup(dhtPin, DHTesp::DHT11);

  Wire.begin(D7, D6); // SDA D1 / SCL D2
  lcd.init();         // Khởi tạo LCD
  lcd.clear();
  lcd.backlight();
}

void displayDataOnLCD(String line1, String line2) {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "                ");
  lcd.setCursor(0, 1);
  lcd.print(line2 + "                ");
}

void readAndSendDHT11Data() {
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    Serial.println("Nhiệt độ: " + String(temperature) + " °C");
    Serial.println("Độ ẩm: " + String(humidity) + " %");

    Firebase.setFloat(firebaseData, "/DHT11/temp", temperature);
    Firebase.setFloat(firebaseData, "/DHT11/humi", humidity);

    displayDataOnLCD("Temp: " + String(temperature) + " C", "Humi: " + String(humidity) + " %");
  } else {
    Serial.println("Không thể đọc từ cảm biến DHT11!");
  }

  delay(1000); // Thời gian chờ giữa các lần đọc và gửi dữ liệu
}

void readAndSendMQ4Data() {
  int mqValue = analogRead(mqPin);
  Firebase.setInt(firebaseData, "/methan", mqValue);

  Serial.println("Methane: " + String(mqValue));

  if (mqValue > 800) {
    displayDataOnLCD("Food spoilage        ", "Spoil           ");
    digitalWrite(coi, HIGH); // Giả sử coi là còi báo động
    digitalWrite(led, HIGH);
  } else {
    // displayDataOnLCD("Không phát hiện Gas", "Quạt TẮT"); 
    digitalWrite(coi, LOW);
    digitalWrite(led, LOW);
  }
}

void controlFan() {
  Firebase.getFloat(firebaseData, "/temp");
  float temp = firebaseData.floatData();
  Serial.println("temp from Firebase: " + String(temp));

  float temperature = dht.getTemperature();
  Serial.println("Current temperature: " + String(temperature));

  if (temperature > temp){
    digitalWrite(fan, HIGH);
  }else{
    digitalWrite(fan, LOW);
  }
}


void loop() {
  readAndSendDHT11Data();
  readAndSendMQ4Data();
  controlFan();
}
