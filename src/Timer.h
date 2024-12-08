#pragma once
#include <Arduino.h>
#include <esp32-hal-timer.h>
#include <esp_timer.h>
typedef void (*Func)();
class Timer
{
public:
    int id;
    hw_timer_t *timer = NULL;
    Timer(int _id)
    {
        id = _id;
        timer = timerBegin(id, 100, true);
    }

    static inline void periodic_callback(int miliseconds, void (*func)())
    {

        // Khởi tạo timer
        esp_timer_create_args_t timerArgs = {
            .callback = [](void *args)
            {
                Func func = Func(args);
                func();
            },
            .arg = (void *)func,
            .name = "my_timer"};
        esp_timer_handle_t myTimer;
        esp_timer_create(&timerArgs, &myTimer);
        // Đặt thời gian theo mili giây
        esp_timer_start_periodic(myTimer, miliseconds * 1000);
    };


};