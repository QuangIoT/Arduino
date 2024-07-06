#include <Arduino.h>
#include <QTRSensors.h>
// 2pin (1000 : speed)
//  maxsd: 120  ; 160  ;
//  sd   : 100  ; 150  ;
//  kp   : 0.15 ; 0.1625 ;
//  ki   :      ; 00005;
//  kd   : 3.4  ; 1.855  ;

// 3pin
//  maxsd: 90  ;
//  sd   : 90  ;
//  kp   : 0.1425 ; 0.1275
//  ki   : 0.0;
//  kd   : 3.8375  ;3.8455
#define NUM_SENSORS 5                                           // number of sensors used
#define NUM_SAMPLES_PER_SENSOR 5                                // average 5 analog samples per sensor reading
#define EMITTER_PIN QTRNoEmitterPin

QTRSensors qtra;                                                // khởi tạo thanh cảm biến line
uint16_t sensorValues[NUM_SENSORS];
uint16_t meanV = 500;

const int trig = 11;                                            // chân trig của HC-SR04
const int echo = 10;                                            // chân echo của HC-SR04
uint16_t distance;                                              // biến lưu khoảng cách
void Tinh_khoangcach();
// Motor Driver Properties
uint8_t max_speed = 160;
uint8_t base_speed = 150;
uint8_t turn_speed = 140;
uint8_t min_speed = 0;
// PID Properties
const double KP = 0.1625;
const double KI = 0.00005;
const double KD = 1.855;
int error;
double lastError = 0;
double sumError = 0;
const int GOAL = 2000;
// Status
boolean start = false;

// chân điều khiển l298n
#define IN1 7
#define IN2 6
#define IN3 5
#define IN4 4
void Motor1_Tien(int speed)
{
  digitalWrite(IN1, LOW);                                       // chân này không có PWM
  analogWrite(IN2, speed);
}
void Motor1_Lui(int speed)
{
  digitalWrite(IN1, HIGH);                                      // chân này không có PWM
  analogWrite(IN2, 255 - speed);
}
void Motor2_Tien(int speed)
{
  analogWrite(IN3, 255 - speed);
  digitalWrite(IN4, HIGH);                                      // chân này không có PWM
}
void Motor2_Lui(int speed)
{
  analogWrite(IN3, speed);
  digitalWrite(IN4, LOW);                                        // chân này không có PWM
}

void calibrateLineSensor()
{
  digitalWrite(13, HIGH);                                        // bật đèn led báo chế độ
  for (int i = 0; i < 50; i++)
  {
    Motor1_Tien(100);
    Motor2_Lui(100);
    // if (i < 50)
    // {
    //   Motor1_Tien(100);
    //   Motor2_Lui(100);
    // }
    // else
    // {
    //   Motor1_Lui(100);
    //   Motor2_Tien(100);
    // }
    qtra.calibrate();
    delay(20);
  }
  Motor1_Tien(0);
  Motor2_Tien(0);
  delay(500);
  for (int i = 0; i < 50; i++)
  {
    Motor1_Lui(100);
    Motor2_Tien(100);
    qtra.calibrate();
    delay(20);
  }
  Motor1_Tien(0);
  Motor2_Tien(0);
  delay(500);
  digitalWrite(13, LOW);                                         // turn off Arduino's LED to indicate we are through with calibration
}

void setup()
{
  Serial.begin(9600);                                            // giao tiếp Serial với baudrate 9600
  qtra.setTypeAnalog();
  qtra.setSamplesPerSensor(NUM_SAMPLES_PER_SENSOR);
  qtra.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4}, NUM_SENSORS);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  delay(2000);
  calibrateLineSensor();
  delay(3000);
}
uint16_t position;
// uint8_t min, max;
void loop()
{
  Tinh_khoangcach();
  delay(20);
  Serial.println(distance);

  if (distance <= 15) //(2pin)
  {
    Motor1_Tien(0);
    Motor2_Tien(0);
    delay(700);

    Motor1_Tien(120);
    delay(400);
    
    Motor2_Tien(120);
    delay(700);
    
    Motor1_Tien(0);
    delay(400);
    
    Motor1_Tien(120);
    delay(300);
    
    Motor1_Tien(0);
    delay(300);
    
    Motor1_Tien(120);
    delay(300);
    
    Motor2_Tien(0);
    delay(500);
    
    Motor1_Tien(0);
    return;
  }
  // Get line position
  position = qtra.readLineBlack(sensorValues);
  // Compute error from line
  if (position == 500)
  { // phải
    Motor1_Tien(turn_speed);
    Motor2_Tien(min_speed);
    // delay(100);
    return;
  }
  else if (position == 3500)
  { // trái
    Motor1_Tien(min_speed);
    Motor2_Tien(turn_speed);
    // delay(100);
    return;
  }
  if (sensorValues[0] > 50 && sensorValues[1] > 50 && sensorValues[2] > 50 && sensorValues[3] > 50 && sensorValues[4] > 50)
  {
    error = lastError;
  }
  else
  {
    error = GOAL - position;
  }

  // Compute motor adjustment
  int adjustment = KP * error + KD * (error - lastError) + KI * (sumError);

  // Store error for next increment
  lastError = error;
  sumError += error;
  // Adjust motors
  Motor1_Tien(constrain(base_speed + adjustment, min_speed, max_speed));
  Motor2_Tien(constrain(base_speed - adjustment, min_speed, max_speed));
}

void Tinh_khoangcach()
{
  unsigned long duration;                                      // biến đo thời gian
  digitalWrite(trig, 0);
  delayMicroseconds(2);
  digitalWrite(trig, 1);
  delayMicroseconds(5);                                        // xung có độ dài 5 microSeconds
  digitalWrite(trig, 0);

  /* Tính toán thời gian */
  duration = pulseIn(echo, HIGH);                              // Đo độ rộng xung HIGH ở chân echo.

  if (duration > 23500) // Ngoại lệ
  {
    return;
  }

  distance = int(duration / 2 / 29.412);                       // Tính khoảng cách đến vật.
  delayMicroseconds(45);
  
}