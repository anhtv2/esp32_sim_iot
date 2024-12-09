#pragma once

#include <Arduino.h>
#include <DHTesp.h>
#define DHTTYPE DHTesp::DHT22
namespace sensor
{
    DHTesp dht;
    int pir_pin = 0;

    inline void setup_dht(int _pin)
    {
        return dht.setup(_pin, DHTTYPE);
    }
    inline float get_temperature()
    {
        return dht.getTemperature();
    }
    inline float get_humidity()
    {
        return dht.getHumidity();
    }
    inline bool check_motion()
    {
        return digitalRead(pir_pin);
    }

    inline void setup_pir(int _pin, void (*func)() = nullptr)
    {
        pir_pin = _pin;
        pinMode(_pin, INPUT);
        if (func != nullptr)
        {
            attachInterrupt(digitalPinToInterrupt(_pin), func, CHANGE);
        }
    }
};