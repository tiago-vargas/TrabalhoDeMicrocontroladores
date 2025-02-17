#include <Arduino.h>
#include <Stepper.h>

const int trigPin = 12;
const int echoPin = 14;
const int wakeUpPin = 23;

long duration;
int distance;

const int stepsPerRevolution = 2048;  

#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);


void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(wakeUpPin, OUTPUT);
  digitalWrite(wakeUpPin, LOW);

  delay(2000);

  myStepper.setSpeed(5);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.0344 / 2;

  if (distance < 20) {
    Serial.println("Movimento Detectado! Acordando o ESP32-CAM...");
    myStepper.step(stepsPerRevolution);
    delay(1000);
    digitalWrite(wakeUpPin, HIGH);
    delay(2000);
    digitalWrite(wakeUpPin, LOW);
    Serial.println("desligou...");
  }

  delay(500);
}
