#include <oled.h>

OLED::OLED(int pin_sda, int pin_scl, int width, int height) {
    this->pin_sda = pin_sda;
    this->pin_scl = pin_scl;
    this->width = width;
    this->height = height;
}

void OLED::begin() {
    DEBUG_SER_PRINTLN("Initializing OLED module...");
    disp_wire = new TwoWire(0);
    disp_wire->begin(pin_sda, pin_scl);
    display = new Adafruit_SSD1306(width, height, disp_wire, -1);

    if(!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DEBUG_SER_PRINTLN(F("SSD1306 allocation failed"));
        sysHalt();
    }
    DEBUG_SER_PRINTLN("OLED module initialized.");
}

void OLED::clear(int color, int cursor_x, int cursor_y) {
    display->clearDisplay();
    display->setCursor(cursor_x, cursor_y);
    display->display();
    display->setTextColor(color);
}

void OLED::print(const char *str, int color, int cursor_x, int cursor_y, int text_size) {
    display->setCursor(cursor_x, cursor_y);
    display->setTextColor(color);
    display->setTextSize(text_size);
    display->println(str);
    display->display();
}