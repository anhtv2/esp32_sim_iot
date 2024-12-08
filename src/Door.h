#pragma once
#include <ESP32Servo.h>

#include <Arduino.h>
namespace door
{

    Servo my_servo;
    // int pin;
    int current_angle = 0;
    inline void setup(int _pin)
    {
        // pin = _pin;
        my_servo.attach(_pin);
    }
    inline void change_angle(int angle)
    {
        my_servo.write(angle);
    }
    inline int get_angle()
    {
        // Get the current position of the servo
        int servoPosition = my_servo.read();

        // Map the servo position to the angle range
        return map(servoPosition, 0, 180, 0, 180);
    }
}