#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#define LCD_COLS 16
#define LCD_ROWS 2
#define I2C_ADDR 0x27
namespace lcd
{
    LiquidCrystal_I2C my_lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

    inline void setup()
    {
        my_lcd.init();
        my_lcd.backlight();
    }
    inline void print(String message, int row = 0, int col = 0)
    {
        my_lcd.setCursor(col, row);
        my_lcd.print(message);
    }
    inline void clear()
    {
        return my_lcd.clear();
    }
};