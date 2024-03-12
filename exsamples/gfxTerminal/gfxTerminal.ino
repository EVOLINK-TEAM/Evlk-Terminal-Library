#include "Adafruit_GFX.h"
#include "U8g2_for_Adafruit_GFX.h"

#include "evlk_Terminal.h"
#include "evlk_Terminal_font565.h"
#include "evlk_miniGraph.h"
using namespace _EVLK_TERMINAL_;
using namespace _EVLK_MINIGRAPH_;

#include "Arduino.h"

class GFXTerminal_565 : public Terminal,
                        public Print
{
private:
    const size_t width;
    const size_t height;
    int16_t font_width;
    int16_t font_height;
    fontFactory565 *factory;

    fontFactory565 &newfact()
    {
        factory = new fontFactory565();
        return *factory;
    }

public:
    U8G2_FOR_ADAFRUIT_GFX u8g2;
    GFXcanvas16 gfx;
    evlk_miniGraph driver;

public:
    GFXTerminal_565(size_t width, size_t height, size_t logLen, size_t styleLen, const uint8_t *u8g2_font = u8g2_font_4x6_tr)
        : gfx(width, height), driver(width, height),
          width(width), height(height),
          Terminal(0, logLen, newfact(), styleLen)
    {
        font565 *f = dynamic_cast<font565 *>(factory->createFont());
        u8g2.begin(gfx);
        setFont(u8g2_font);
        u8g2.setFontMode(1);
    }
    ~GFXTerminal_565()
    {
        delete factory;
    }
    void setFont(const uint8_t *u8g2_font)
    {
        u8g2.setFont(u8g2_font);
        font_width = u8g2.getUTF8Width(" ");
        font_height = u8g2.getFontAscent() - u8g2.getFontDescent();
        Terminal::resize(width / font_width);
    }
    void drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
    {
        if (row < 1 || column < 1)
            return;

        int16_t cursor_x = (row - 1) * font_width;
        int16_t cursor_y = column * font_height;
        int16_t descent = u8g2.getFontDescent();

        u8g2.setForegroundColor(color);
        // u8g2.setBackgroundColor(bg);
        gfx.fillRect(cursor_x, cursor_y - font_height - descent, font_width, font_height, bg);
        u8g2.setCursor(cursor_x, cursor_y);
        u8g2.write((uint8_t)c);
    }
    void drawLine(const char *p, size_t column)
    {
        if (p < getBegin() || p >= getEnd())
            return;

        const char *h = p;
        const char *e = DOWN(h) == NULL ? getEnd() - 1 : DOWN(h) - 1;
        while (p <= e)
        {
            char c = *p;
            const font565 *s = dynamic_cast<const font565 *>(style(p));
            /* if (c == ' ') //* debug
                 c = '.';*/
            if (c == '\n')
                c = ' ';
            if (c == '\0')
                c = ' ';
            if (p != getCursor())
                drawChar(p - h + 1, column, c, s->color, s->bgcolor);
            else
                drawChar(p - h + 1, column, c, s->bgcolor, s->color);
            p++;
        }
    }
    void clearDisplay()
    {
        gfx.fillScreen(T565_BLACK);
    }
    void display()
    {
#define displayOffset 0;
        static size_t off = 0;
        const char *b = getFocus();
        int16_t Height = height / font_height;
        for (size_t col = 1; col <= Height; col++)
        {
            drawLine(b, col + off);
            const char *s = DOWN(b);
            if (!s)
                break;
            b = s;
        }
        driver.display(gfx.getBuffer());
        off += displayOffset;
    }
    size_t write(uint8_t c) override
    {
        return Terminal::write(c);
    };
    size_t write(const uint8_t *buffer, size_t size) override
    {
        return Terminal::write(buffer, size);
    }
    using Print ::write;
};

const size_t width = 128; // px
const size_t height = 64; // px
const size_t log_len = 200;
const size_t style_len = 40;
GFXTerminal_565 terminal(width, height, log_len, style_len, u8g2_font_4x6_tr);

void setup()
{
    terminal.write("\033[31ma\033[32mb\033[33mc\033[34md\033[35me\033[36mf\033[37m123456");
    terminal.display();
}

void loop()
{
    terminal.driver.handleEvents();
}