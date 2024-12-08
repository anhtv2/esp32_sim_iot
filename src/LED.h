#pragma once
#include <Arduino.h>
class LED
{
public:
    int pin;
    LED()
    {
    }
    inline void set_pin(int _pin)
    {
        pin = _pin;
        pinMode(pin, OUTPUT);
    }
    LED(int _pin)
    {
        pin = _pin;
        pinMode(pin, OUTPUT);
    }

    inline void turn_on()
    {
        digitalWrite(pin, LOW);
    }
    inline void turn_off()
    {
        digitalWrite(pin, HIGH);
    }
    inline int getState()
    {
        return digitalRead(pin);
    }
    inline void turn(int state)
    {
        // HIGH : 1
        //  LOW : 0
        digitalWrite(pin, state);
    }
    inline void invert_state()
    {
        turn(!getState());
    }
};