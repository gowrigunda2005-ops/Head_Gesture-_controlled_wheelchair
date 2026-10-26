#include <Wire.h>
#include <MPU6050.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

MPU6050 mpu;
RF24 radio(9, 10);

const byte address[6] = "00001";

int16_t ax, ay, az;

// Smoothed values
float ax_avg = 0;
float ay_avg = 0;

char command = 'S';

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  mpu.initialize();

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  Serial.println("🖐 Gesture Control Ready...");
}

void loop() {

  // 🔹 Read MPU
  mpu.getAcceleration(&ax, &ay, &az);

  // 🔹 Smooth values (filter)
  ax_avg = ax_avg * 0.7 + ax * 0.3;
  ay_avg = ay_avg * 0.7 + ay * 0.3;

  // 🔹 Dead zone (no movement area)
  if (abs(ax_avg) < 3000 && abs(ay_avg) < 3000) {
    command = 'S';
  }
  else {
    // 🔥 Direction control
    if (ay_avg > 7000) command = 'F';       // Forward
    else if (ay_avg < -7000) command = 'B'; // Backward
    else if (ax_avg > 7000) command = 'R';  // Right
    else if (ax_avg < -7000) command = 'L'; // Left
    else command = 'S';
  }

  // 🔹 Send data
  radio.write(&command, sizeof(command));

  // 🔹 Debug output
  Serial.print("AX: "); Serial.print(ax_avg);
  Serial.print(" | AY: "); Serial.print(ay_avg);
  Serial.print(" | CMD: "); Serial.println(command);

  delay(100);
}