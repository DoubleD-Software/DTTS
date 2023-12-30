#include <oled.h>

/**
 * Initialize class variables during class creation when constructor gets called.
 * In order to initialize the OLED screen, begin() has to be called.
 * @param pin_sda The SDA pin the screen is connected to.
 * @param pin_scl The SCL pin the screen is connected to.
 * @param width The width of the screen in pixels.
 * @param height The height of the screen in pixels.
*/
OLED::OLED(int pin_sda, int pin_scl, int width, int height) {
    this->pin_sda = pin_sda;
    this->pin_scl = pin_scl;
    this->width = width;
    this->height = height;
}

/**
 * Initialize the OLED screen. This function has to be called before any other function of this class.
 * In case of an error, the system will halt, with an error message on the serial.
*/
void OLED::begin() {
    DEBUG_SER_PRINTLN("Initializing OLED module...");
    disp_wire = new TwoWire(0); // Allocate memory for the wire object.
    disp_wire->begin(pin_sda, pin_scl); // Initialize the wire object with the given pins.
    display = new Adafruit_SSD1306(width, height, disp_wire, -1); // Allocate memory for the display object of type Adafruit_SSD1306.

    if(!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Initialize the display object with the given parameters.
        DEBUG_SER_PRINTLN(F("SSD1306 allocation failed"));
        sysHalt();
    }
    DEBUG_SER_PRINTLN("OLED module initialized.");
}

/**
 * Clear the screen with the given color and set the cursor to the given position.
 * @param color The color to clear the screen with.
 * @param cursor_x The x position of the cursor.
 * @param cursor_y The y position of the cursor.
*/
void OLED::clear(int color, int cursor_x, int cursor_y) {
    display->clearDisplay();
    display->setCursor(cursor_x, cursor_y);
    display->display();
    display->setTextColor(color);
}

/**
 * Print the given string to the screen with the given text size and color.
 * @param str The string to print.
 * @param text_size The text size to print the string with.
 * @param color The color to print the string with.
*/
void OLED::print(const char *str, int text_size, int color) {
    display->setTextColor(color);
    display->setTextSize(text_size);
    display->print(str);
    display->display();
}

/**
 * Set the cursor to the given position.
 * @param cursor_x The x position of the cursor.
 * @param cursor_y The y position of the cursor.
*/
void OLED::setCursor(int cursor_x, int cursor_y) {
    display->setCursor(cursor_x, cursor_y);
}