#include <AFMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(A2, A3); // RX | TX
AF_DCMotor left_motor(4);
AF_DCMotor right_motor(2);
Servo servo;

const int left_led_red = 13;
const int left_led_green = 2;
const int right_led_red = A0;
const int right_led_green = A1;
const int trig_pin = A4;
const int echo_pin = A5;
const bool debug = false;

int turn_delay = 300;
int u_turn_delay = 520;
int detection_range = 40;
bool autopilot_enabled = false;

void setup()
{
    Bluetooth.begin(38400);
    servo.attach(9);
    servo.write(90);

    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
    pinMode(left_led_red, OUTPUT);
    pinMode(left_led_green, OUTPUT);
    pinMode(right_led_red, OUTPUT);
    pinMode(right_led_green, OUTPUT);
    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
}

void loop()
{
    if (Bluetooth.available())
    {
        switch (Bluetooth.read())
        {
        case 'f':
            move_forward();
            break;
        case 'b':
            move_backward();
            break;
        case 'l':
            move_left();
            break;
        case 'r':
            move_right();
            break;
        case 's':
            stop();
            break;
        case 'a':
            stop();
            delay(1000);
            autopilot_enabled = true;
            break;
        case 'x':
            stop();
            autopilot_enabled = false;
        }
    }

    if (autopilot_enabled)
    {
        autopilot();
    }
}

void move_left()
{
    right_motor.run(FORWARD);
    left_motor.run(BACKWARD);

    digitalWrite(left_led_green, HIGH);
    digitalWrite(right_led_red, HIGH);
}

void move_right()
{
    left_motor.run(FORWARD);
    right_motor.run(BACKWARD);

    digitalWrite(right_led_green, HIGH);
    digitalWrite(left_led_red, HIGH);
}

void move_forward()
{
    right_motor.run(FORWARD);
    left_motor.run(FORWARD);
    digitalWrite(left_led_green, HIGH);
    digitalWrite(right_led_green, HIGH);
}

void move_backward()
{
    right_motor.run(BACKWARD);
    left_motor.run(BACKWARD);

    digitalWrite(left_led_red, HIGH);
    digitalWrite(right_led_red, HIGH);
}

void stop()
{
    left_motor.run(RELEASE);
    right_motor.run(RELEASE);

    digitalWrite(left_led_green, LOW);
    digitalWrite(right_led_green, LOW);
    digitalWrite(left_led_red, LOW);
    digitalWrite(right_led_red, LOW);
}

void autopilot()
{
    bool left_clear = false;
    bool right_clear = false;

    if (!path_clear())
    {
        stop();

        // scan left
        servo.write(180);
        delay(340);
        if (path_clear())
            left_clear = true;
        delay(10);

        // scan right
        servo.write(0);
        delay(480);
        if (path_clear())
            right_clear = true;
        servo.write(0);
        delay(10);

        // reset
        servo.write(90);

        if (left_clear)
        {
            move_left();
            delay(turn_delay);
            stop();
        }
        else if (right_clear)
        {
            move_right();
            delay(turn_delay);
            stop();
        }
        else // U turn
        {
            move_left();
            delay(u_turn_delay);
            stop();
        }
    }

    else
    {
        move_forward();
    }
}

bool path_clear()
{
    long duration;
    int distance;

    digitalWrite(trig_pin, LOW); // Makes trigger pin low incase it was not already
    delayMicroseconds(2);
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);

    duration = pulseIn(echo_pin, HIGH); // Reads the echo_pin, returns the sound wave travel time in microseconds
    distance = duration / 2 * 0.034;    // speed of sound * duration / 2 (back and forth)
    if (distance > detection_range)
        return true;
    else
        return false;
}
