#include <tm1637.h>

TM1637::TM1637(int clk_pin, int dio_pin, int digits) {
    this->clk_pin = clk_pin;
    this->dio_pin = dio_pin;
    this->digits = digits;
}

void TM1637::begin() {
    DEBUG_SER_PRINTLN("Initializing TM1637 module...");
    digit_order = new uint8_t[digits];
    digit_buffer = new uint8_t[digits];
    dec_mapping = new uint8_t[16];
    ascii_mapping = new uint8_t[64];

    dec_mapping[0] = 0x3F;
    dec_mapping[1] = 0x06;
    dec_mapping[2] = 0x5B;
    dec_mapping[3] = 0x4F;
    dec_mapping[4] = 0x66;
    dec_mapping[5] = 0x6D;
    dec_mapping[6] = 0x7D;
    dec_mapping[7] = 0x07;
    dec_mapping[8] = 0x7F;
    dec_mapping[9] = 0x6F;

    uint8_t tmp[] = {
        0x00, 0x86, 0x22, 0x7f, 0x6d, 0x52, 0x7d, 0x02, //   !"# $%&'
        0x39, 0x0f, 0x7f, 0x46, 0x80, 0x40, 0x80, 0x52, //  ()*+ ,-./
        0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, //  0123 4567
        0x7f, 0x6f, 0x09, 0x89, 0x58, 0x48, 0x4c, 0xD3, //  89:; <=>?
        0x77, 0x5f, 0x7c, 0x39, 0x5E, 0x79, 0x71, 0x3d, //  @aBC DEFG
        0x74, 0x06, 0x0E, 0x75, 0x38, 0x37, 0x54, 0x5c, //  HIJK LMNO
        0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x9c, //  PQRS TUVW
        0x76, 0x6E, 0x5B, 0x39, 0x52, 0x0F, 0x23, 0x08  //  XYZ[ /]^_
    };

    memcpy(ascii_mapping, tmp, 64);

    if (digits == 6) {
        digit_order[0] = 3;
        digit_order[1] = 4;
        digit_order[2] = 5;
        digit_order[3] = 0;
        digit_order[4] = 1;
        digit_order[5] = 2;
    } else {
        digit_order[0] = 3;
        digit_order[1] = 2;
        digit_order[2] = 1;
        digit_order[3] = 0;
    }

    pinMode(clk_pin, OUTPUT);
    pinMode(dio_pin, OUTPUT);
    digitalWrite(clk_pin, HIGH);
    digitalWrite(dio_pin, HIGH);

    DEBUG_SER_PRINTLN("TM1637 module initialized.");
}

void TM1637::setBrightness(int brightness) {
    this->brightness = brightness;
}

void TM1637::displayRawBuffer(uint8_t *buffer) {
    memcpy(digit_buffer, buffer, digits);
    display();
}

void TM1637::displayNumBuffer(uint8_t *buffer) {
    for (int i = 0; i < digits; i++) {
        digit_buffer[i] = dec_mapping[0x7F & buffer[i]];
        digit_buffer[i] |= buffer[i] & DISPLAY_DOT;
    }
    display();
}

void TM1637::displayString(const char *str) {
    for (int i = 0; i < digits; i++) {
        if (((0x7F & str[i]) - 0x20) < 0 || ((0x7F & str[i]) - 0x20) > 63) {
            digit_buffer[i] = str[i] & DISPLAY_DOT;
            continue;
        }
        digit_buffer[i] = ascii_mapping[(0x7F & str[i]) - 0x20];
        digit_buffer[i] |= str[i] & DISPLAY_DOT;
    }
    display();
}

void TM1637::display() {
    start();
    writeByte(TM1637_ADDR_AUTO);
    stop();
    start();
    writeByte(TM1637_CMD_SET_ADDR);

    for (int i = 0; i < digits; i++) {
        int n = digit_order[i];
        writeByte(digit_buffer[n]);
    }

    stop();
    start();
    writeByte(TM1637_CMD_DISPLAY | brightness);
    stop();
}

uint8_t TM1637::writeByte(uint8_t data) {
    for (uint8_t i = 8; i > 0; i--) {
        writeSync(clk_pin, LOW);
        writeSync(dio_pin, data & 0x01);
        writeSync(clk_pin, HIGH);
        data >>= 1;
    }

    writeSync(clk_pin, LOW);
    writeSync(dio_pin, HIGH);
    writeSync(clk_pin, HIGH);

    pinMode(dio_pin, INPUT);
    delayMicroseconds(10);
    uint8_t rv = digitalRead(dio_pin);

    pinMode(dio_pin, OUTPUT);
    digitalWrite(dio_pin, LOW);
    delayMicroseconds(10);
    return rv;
}

void TM1637::writeSync(int pin, int value) {
    digitalWrite(pin, value);
    nanoDelay(21);
}

void TM1637::start() {
    writeSync(clk_pin, HIGH);
    writeSync(dio_pin, HIGH);
    writeSync(dio_pin, LOW);
    writeSync(clk_pin, LOW);
}

void TM1637::stop() {
    writeSync(clk_pin, LOW);
    writeSync(dio_pin, LOW);
    writeSync(clk_pin, HIGH);
    writeSync(dio_pin, HIGH);
}

void TM1637::nanoDelay(uint16_t n)
{
  volatile uint16_t i = n;
  while (i--);
}