// ADXL345 Library reference: https://github.com/adafruit/Adafruit_Sensor and https://learn.adafruit.com/adxl345-digital-accelerometer/library-reference
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial Bluetooth(8, 7); // RX | TX
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
const int bt_state_pin = 4;
const int gnd_light_pin = 12;
const int red_light_pin = 10;
const int green_light_pin = 11;
const int blue_light_pin = 9;

const float tilt_threshold = 6.5;
const float autpilot_threshold = 18;
const float stop_threshold = 2.5;

float correction_x = 0;
float correction_y = 0;
float correction_z = 0;

bool in_motion = false;
bool in_autopilot = false;

void setup()
{
    Bluetooth.begin(38400);
    pinMode(bt_state_pin, INPUT);
    pinMode(gnd_light_pin, OUTPUT);
    pinMode(red_light_pin, OUTPUT);
    pinMode(blue_light_pin, OUTPUT);
    pinMode(green_light_pin, OUTPUT);
    digitalWrite(gnd_light_pin, LOW);

    RGB_colour(0, 0, 255);
    while (!digitalRead(bt_state_pin)) continue; // wait till bluetooth is active

    if (!accel.begin())
    {
        while (1) continue;
    }

    calibrate();
}

void loop()
{
    sensors_event_t event;
    accel.getEvent(&event); // Populates event with the data

    float accel_x = event.acceleration.x - correction_x;
    float accel_y = event.acceleration.y - correction_y;

    // Autopilot trigger
    if (abs(accel_y) > autpilot_threshold)
    {
        // Forward jerk
        if (accel_y > 0 && !in_autopilot)
        {
            Bluetooth.write("a");
            in_autopilot = true;
            in_motion = true;
            autopilot_enable_led();
        }
        // Backward jerk
        else if (accel_y < 0 && in_autopilot)
        {
            Bluetooth.write("x");
            in_autopilot = false;
            in_motion = false;
            autopilot_disable_led();
        }
    }

    // Backward/Forward trigger
    else if (abs(accel_y) > tilt_threshold && !in_motion)
    {
        if (accel_y > 0)
        {
            Bluetooth.write("f");
            movement();
        }
        else
        {
            Bluetooth.write("b");
            movement();
        }
    }

    // Right/Left trigger
    else if (abs(accel_x) > tilt_threshold && !in_motion)
    {
        if (accel_x < 0)
        {
            Bluetooth.write("r");
            movement();
        }
        else
        {
            Bluetooth.write("l");
            movement();
        }
    }

    // Stop trigger
    // Issues stop signal if we are in motion with our hands neutral and autopilot disabled
    else if (in_motion && abs(accel_x) < stop_threshold && abs(accel_y) < stop_threshold && !in_autopilot)
    {
        RGB_colour(0, 0, 0);
        // Sends stop signal three times as it is prone to loss of control
        for (int i = 0; i < 3; i++)
        {
            Bluetooth.write("s");
            delay(15);
        }
        in_motion = false;
    }
}

void calibrate()
{
    RGB_colour(255, 0, 0);
    float value_x = 0;
    float value_y = 0;
    float value_z = 0;

    // Collect data over a period of 5 seconds
    for (int index = 0; index < 100; index++)
    {
        sensors_event_t event;
        accel.getEvent(&event);

        value_x += event.acceleration.x;
        value_y += event.acceleration.y;
        value_z += event.acceleration.z;
        delay(50);
    }

    // Get mean readings
    correction_x = value_x / 100;
    correction_y = value_y / 100;
    correction_z = (value_z / 100) - 9.8; // Force of gravity is 9.8 m/s^2
    Serial.println("Calibration complete.");
    RGB_colour(0, 0, 0);
    delay(10);
}

void RGB_colour(int red_light_value, int green_light_value, int blue_light_value)
{
    analogWrite(red_light_pin, red_light_value);
    analogWrite(green_light_pin, green_light_value);
    analogWrite(blue_light_pin, blue_light_value);
}

void movement()
{
    in_motion = true;
    RGB_colour(0, 255, 0);
}

void autopilot_enable_led()
{
    RGB_colour(0, 255, 0);
    delay(300);
    RGB_colour(255, 0, 0);
    delay(300);
    RGB_colour(0, 0, 255);
    delay(300);
    RGB_colour(0, 255, 255);
}

void autopilot_disable_led()
{
    RGB_colour(0, 0, 255);
    delay(300);
    RGB_colour(255, 0, 0);
    delay(300);
    RGB_colour(0, 255, 0);
    delay(300);
    RGB_colour(0, 0, 0);
}