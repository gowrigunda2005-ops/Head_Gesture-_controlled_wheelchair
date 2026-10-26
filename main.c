#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// 🔸 NRF24L01 Setup
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";
char command = 'S';

// 🔸 BTS7960 Pins (Left & Right)
#define RPWM_L 5
#define LPWM_L 6
#define RPWM_R 3
#define LPWM_R 4

// 🔸 Sensors & Feedback
#define TRIG 7
#define ECHO 8
#define IR_PIN 2
#define ESP_STOP A0
#define BUZZER A1

long duration;
int distance;

void setup() {
  Serial.begin(9600);

  // Motor Pins
  pinMode(RPWM_L, OUTPUT);
  pinMode(LPWM_L, OUTPUT);
  pinMode(RPWM_R, OUTPUT);
  pinMode(LPWM_R, OUTPUT);

  // Ultrasonic Pins
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Safety Pins
  pinMode(IR_PIN, INPUT);
  pinMode(ESP_STOP, INPUT_PULLUP); // LOW = Normal, HIGH = Triggered Stop

  pinMode(BUZZER, OUTPUT);

  // NRF24 Setup
  if (!radio.begin()) {
    Serial.println("NRF24 Hardware not responding!");
  }
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  Serial.println("🚗 FINAL SYSTEM READY...");
}

void loop() {
  // 🔹 1. Get Ultrasonic Distance
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 30000); // 30ms timeout
  distance = duration * 0.034 / 2;
  
  // Handle out of range or error readings
  if (duration == 0 || distance > 400) distance = 400;

  int irState = digitalRead(IR_PIN);
  int espState = digitalRead(ESP_STOP);

  // 🔴 SAFETY CHECK 1: ULTRASONIC OBSTACLE
  // Stops car if object is closer than 20cm
  if (distance > 0 && distance < 20) {
    stopCar();
    tone(BUZZER, 1000);
    Serial.print("🚧 OBSTACLE DETECTED! Dist: ");
    Serial.println(distance);
    return; // Exit loop early - ignore NRF commands
  } else {
    noTone(BUZZER);
  }

  // 🔴 SAFETY CHECK 2: IR / ESP EXTERNAL STOP
  if (irState != LOW || espState != LOW) {
    stopCar();
    Serial.println("🛑 SAFETY SENSORS TRIGGERED (IR/ESP)");
    return; // Exit loop early
  }

  // 🔹 2. Receive NRF Command
  if (radio.available()) {
    radio.read(&command, sizeof(command));
    Serial.print("New Command: ");
    Serial.println(command);
  }

  // 🚗 3. Execute Movement
  switch (command) {
    case 'F': forward();  break;
    case 'B': backward(); break;
    case 'L': left();     break;
    case 'R': right();    break;
    default:  stopCar();  break;
  }

  // Small delay for stability
  delay(50);
}

// 🔸 MOTOR CONTROL FUNCTIONS

void forward() {
  analogWrite(RPWM_L, 200);
  analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 200);
  analogWrite(LPWM_R, 0);
}

void backward() {
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, 200);
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, 200);
}

void left() {
  // Pivot turn or sharp left
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, 150);
  analogWrite(RPWM_R, 150);
  analogWrite(LPWM_R, 0);
}

void right() {
  // Pivot turn or sharp right
  analogWrite(RPWM_L, 150);
  analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, 150);
}

void stopCar() {
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, 0);
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, 0);
}