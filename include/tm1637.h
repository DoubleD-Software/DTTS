#ifndef NUMDISP_H
#define NUMDISP_H

#include <Arduino.h>
#include <debug.h>

/***************
   ---
  |   |
   ---
  |   |
   ---    .


      -01-
  20 |    | 02
      -40-
  10 |    | 04
      -08-    .80

*/

#define TM1637_ADDR_AUTO           0x40
#define TM1637_CMD_SET_ADDR        0xC0
#define TM1637_CMD_DISPLAY         0x88
#define DISPLAY_DOT                0x80

class TM1637 {
    public:
        TM1637(int clk_pin, int dio_pin, int digits = 6);
        void begin();
        void setBrightness(int brightness);
        void displayRawBuffer(uint8_t *buffer);
        void displayNumBuffer(uint8_t *buffer);
        void displayString(const char *str);

    private:
        void display();
        uint8_t writeByte(uint8_t data);
        void writeSync(int pin, int value);
        void start();
        void stop();
        
        int clk_pin;
        int dio_pin;
        int digits;
        int brightness = 10;
        uint8_t *digit_order;
        uint8_t *digit_buffer;
        uint8_t *dec_mapping;
        uint8_t *ascii_mapping;
};

#endif