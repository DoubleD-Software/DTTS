#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <debug.h>
#include <fatal.h>

class OLED {
    public:
        OLED(int pin_sda, int pin_scl, int width, int height);
        void begin();
        void clear(int color = WHITE, int cursor_x = 0, int cursor_y = 0);
        void print(const char *str, int color = WHITE, int cursor_x = 0, int cursor_y = 0, int text_size = 2);

    private:
        int pin_sda;
        int pin_scl;
        int width;
        int height;
        TwoWire *disp_wire;
        Adafruit_SSD1306 *display;
};

#endif