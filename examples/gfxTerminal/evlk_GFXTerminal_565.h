#include "evlk_Terminal.h"
#include "evlk_Terminal_font565.h"
#include "evlk_miniGraph.h"

#include "Print.h"
#include "Adafruit_GFX.h"
#include "U8g2_for_Adafruit_GFX.h"

namespace _EVLK_TERMINAL_
{
    class GFXTerminal_565 : public _EVLK_TERMINAL_::Terminal,
                            public Print
    {
    private:
        const size_t width;
        const size_t height;
        int16_t font_width;
        int16_t font_height;
        _EVLK_TERMINAL_::fontFactory565 *factory;
        uint16_t BG;

        _EVLK_TERMINAL_::fontFactory565 &newfact();

    public:
        U8G2_FOR_ADAFRUIT_GFX u8g2;
        GFXcanvas16 gfx;
        _EVLK_MINIGRAPH_::evlk_miniGraph driver;

    public:
        GFXTerminal_565(size_t width, size_t height, size_t logLen, size_t styleLen, const uint8_t *u8g2_font = u8g2_font_4x6_tr, uint8_t scale = 3);
        ~GFXTerminal_565();
        void setFont(const uint8_t *u8g2_font);
        void drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg);
        void drawLine(const char *p, size_t column);
        void clearDisplay();
        void display();
        size_t write(uint8_t c) override;
        size_t write(const uint8_t *buffer, size_t size) override;
        void flush() override;
        using Print ::write;
    };
}