#include <tm1637.h>

/**
 * Create a new TM1637 module instance.
 * @param clk_pin The CLK pin the module is connected to.
 * @param dio_pin The DIO pin the module is connected to.
 * @param digits The amount of digits the module has (usually 4 or 6).
 */
TM1637::TM1637(int clk_pin, int dio_pin, int digits) {
    this->clk_pin = clk_pin;
    this->dio_pin = dio_pin;
    this->digits = digits;
}

/**
 * Initialize the TM1637 module and required variables.
 */
void TM1637::begin() {
    DEBUG_SER_PRINTLN("Initializing TM1637 module...");
    digit_order = new uint8_t[digits];
    digit_buffer = new uint8_t[digits];
    dec_mapping = new uint8_t[16];
    ascii_mapping = new uint8_t[256];
    memset(ascii_mapping + 0x80, 0, 0x80);

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

    // ASCII to 7-segment mapping
    uint8_t tmp[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Control characters
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x00, 0x86, 0x22, 0x7F, 0x6D, 0x52, 0x7D, 0x02, //   !"# $%&'
        0x39, 0x0F, 0x7F, 0x46, 0x80, 0x40, 0x80, 0x52, //  ()*+ ,-./
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, //  0123 4567
        0x7F, 0x6F, 0x09, 0x89, 0x58, 0x48, 0x4C, 0xD3, //  89:; <=>?
        0x77, 0x5F, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, //  @ABC DEFG
        0x74, 0x06, 0x0E, 0x75, 0x38, 0x37, 0x54, 0x5C, //  HIJK LMNO
        0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x9C, //  PQRS TUVW
        0x76, 0x6E, 0x5B, 0x39, 0x52, 0x0F, 0x23, 0x08, //  XYZ[ /]^_
        0x02, 0x5F, 0x7C, 0x58, 0x5E, 0x79, 0x71, 0x3D, //  `abc defg
        0x74, 0x06, 0x0E, 0x75, 0x38, 0x37, 0x54, 0x5C, //  hijk lmno
        0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x9C, //  pqrs tuvw
        0x76, 0x6E, 0x5B, 0x39, 0x30, 0x0F, 0x40, 0x00, //  xyz{ |}~

        // Extended character set is filled with zeros
    };

    memcpy(ascii_mapping, tmp, sizeof(tmp));

    if (digits == 6) {
        digit_order[0] = 2;
        digit_order[1] = 1;
        digit_order[2] = 0;
        digit_order[3] = 5;
        digit_order[4] = 4;
        digit_order[5] = 3;
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

/**
 * Set the brightness of the display.
 * @param brightness The brightness to set.
 */
void TM1637::setBrightness(int brightness) {
    this->brightness = brightness;
}

/**
 * Display a raw buffer on the display.
 * @param buffer The buffer to display.
 */
void TM1637::displayRawBuffer(uint8_t *buffer) {
    memcpy(digit_buffer, buffer, digits);
    display();
}

/**
 * Display a number buffer on the display.
 * @param buffer The buffer to display.
 */
void TM1637::displayNumBuffer(uint8_t *buffer) {
    for (int i = 0; i < digits; i++) {
        digit_buffer[i] = dec_mapping[0x7F & buffer[i]]; // Map the number to the 7 segments
        digit_buffer[i] |= buffer[i] & DISPLAY_DOT;
    }
    display();
}

/**
 * Display a string on the display. Inline dots are added by using a comma (,), while standalone
 * dots are added by using a period (.).
 * @param str The string to display.
 */
void TM1637::displayString(const char *str) {
    memset(digit_buffer, 0, digits);
    int digit = -1; // The current digit (not the current character, since a comma can be used to add a dot, which isn't its own digit)
    for (int i = 0; str[i] != 0 && digit <= digits; i++) {
        if ((uint8_t) str[i] == 0x2C) { // Check if the character is a comma
            if (digit != -1 && digit < digits) { // Don't add the dot while the digit is -1 (no digit has been set yet) or when the maximum amount of digits has been reached
                digit_buffer[digit] |= DISPLAY_DOT;
            }
            continue;
        }
        digit++; // Increase the amount of digits and stop if the maximum amount of digits has been reached
        if (digit < digits) { // Don't add the character while the maximum amount of digits has been reached
            digit_buffer[digit] = ascii_mapping[(0x7F & str[i])]; // Map the character to the 7 segments
            digit_buffer[digit] |= str[i] & DISPLAY_DOT;
        }
    }
    display();
}

/**
 * Display the display buffer on the display.
 */
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

/**
 * Write a byte to the module and read a byte back.
 * @param data The byte to write.
 * @return The ACK or NAK bit.
 */
uint8_t TM1637::writeByte(uint8_t data) {
    for (uint8_t i = 8; i > 0; i--) { // Shift out the bits of the byte; LSB first
        writeSync(clk_pin, LOW); // Pull the CLK pin low in order to signal the module that a bit is being sent
        writeSync(dio_pin, data & 0x01); // Write the bit to the DIO pin
        writeSync(clk_pin, HIGH); // Set the CLK pin high again in order to signal the module that the bit has been sent
        data >>= 1; // Shift the next bit into the LSB position
    }

    writeSync(clk_pin, LOW); // Send a high bit to the module in order to signal that the transmission is over
    writeSync(dio_pin, HIGH);
    writeSync(clk_pin, HIGH);

    pinMode(dio_pin, INPUT); // Change the DIO pin to an input in order to read the ACK bit
    delayMicroseconds(10); // Wait for the module to pull the DIO pin low in order to signal that the transmission was successful
    uint8_t rv = digitalRead(dio_pin); // Read the response bit

    pinMode(dio_pin, OUTPUT); // Change the DIO pin back to an output
    digitalWrite(dio_pin, LOW); // Pull the DIO pin low in order to signal the module that the ACK bit has been read
    delayMicroseconds(10); // Wait in case the function is called again immediately
    return rv;
}

/**
 * Write a value to a pin and wait a specific amount of clock cycles.
 * @param pin The pin to write to.
 * @param value The value to write.
 */
void TM1637::writeSync(int pin, int value) {
    digitalWrite(pin, value);
    volatile uint16_t i = 25; // Needs to be volatile, otherwise the compiler will optimize it away
    while (i--); // Delay for a specific number of clock cycles
}

/**
 * Start the transmission to the module according to the datasheet.
 */
void TM1637::start() {
    writeSync(clk_pin, HIGH);
    writeSync(dio_pin, HIGH);
    writeSync(dio_pin, LOW);
    writeSync(clk_pin, LOW);
}

/**
 * Stop the transmission to the module according to the datasheet.
 */
void TM1637::stop() {
    writeSync(clk_pin, LOW);
    writeSync(dio_pin, LOW);
    writeSync(clk_pin, HIGH);
    writeSync(dio_pin, HIGH);
}