#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <debug.h>
#include <fatal.h>
#include <error_codes.h>

class OLED {
    public:
        OLED(int pin_sda, int pin_scl, int width, int height);
        void begin();
        void clear(int color = WHITE, int cursor_x = 0, int cursor_y = 0);
        void print(const char *str, int text_size, int color = WHITE);
        void setCursor(int cursor_x, int cursor_y);

    private:
        int pin_sda;
        int pin_scl;
        int width;
        int height;
        TwoWire *disp_wire;
        Adafruit_SSD1306 *display;
};

#endif