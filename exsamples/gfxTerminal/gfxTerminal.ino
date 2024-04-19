#include "evlk_GFXTerminal_565.h"
#include "Arduino.h"

using namespace _EVLK_TERMINAL_;

const size_t width = 128; // px
const size_t height = 64; // px
const size_t log_len = 2000;
const size_t style_len = 400;
GFXTerminal_565 terminal(width, height, log_len, style_len, u8g2_font_4x6_tf);

void setup()
{
    terminal.clearDisplay();
    terminal.write("\033[31ma\033[32mb\033[33mc\033[34md\033[35me\033[36mf\033[37m123456");
    terminal.display();
}

void loop()
{
    terminal.driver.handleEvents();
}