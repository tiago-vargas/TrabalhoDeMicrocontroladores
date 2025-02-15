#include <Arduino.h>

float travel_distance_in_cm(long);

constexpr int trigger = 5;
constexpr int echo = 18;

void setup()
{
	Serial.begin(9600);

	pinMode(trigger, OUTPUT);
	pinMode(echo, INPUT);
}

void loop()
{
	digitalWrite(trigger, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigger, LOW);

	const long travel_time = pulseIn(echo, HIGH) /* us */;
	const float distance = travel_distance_in_cm(travel_time) / 2;

	Serial.print("Distance: ");
	Serial.print(distance);
	Serial.println(" cm");

	delay(1000);
}

float travel_distance_in_cm(const long travel_time_us)
{
	// Sound speed = 343 m/s = 34300 cm/s = 0.0343 cm/us
	constexpr float sound_speed = 0.0343 /* cm/us */;
	const float travel_distance = travel_time_us * sound_speed;
	return travel_distance;
}
