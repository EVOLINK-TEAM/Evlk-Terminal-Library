#include "evlk_GFXTerminal_565.h"
namespace _EVLK_TERMINAL_
{
    _EVLK_TERMINAL_::fontFactory565 &GFXTerminal_565::newfact()
    {
        factory = new _EVLK_TERMINAL_::fontFactory565();
        return *factory;
    }

    GFXTerminal_565::GFXTerminal_565(size_t width, size_t height, size_t logLen, size_t styleLen, const uint8_t *u8g2_font, uint8_t scale)
        : gfx(width, height),
          driver(width, height, scale),
          width(width), height(height),
          Terminal(newfact(), 0, 0, logLen, styleLen)
    {
        _EVLK_TERMINAL_::font565 *f = dynamic_cast<_EVLK_TERMINAL_::font565 *>(factory->createFont());
        setFont(u8g2_font);
        u8g2.setFontMode(1);

        BG = 10857;
    }
    GFXTerminal_565::~GFXTerminal_565() { delete factory; }
    void GFXTerminal_565::setFont(const uint8_t *u8g2_font)
    {
        u8g2.begin(gfx);
        u8g2.setFont(u8g2_font);
        font_width = u8g2.getUTF8Width(" ");
        font_height = u8g2.getFontAscent() - u8g2.getFontDescent();
        resize(width / font_width, height / font_height);
    }
    void GFXTerminal_565::drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
    {
        if (row < 1 || column < 1)
            return;

        int16_t cursor_x = (column - 1) * font_width;
        int16_t cursor_y = row * font_height;
        int16_t descent = u8g2.getFontDescent();

        u8g2.setForegroundColor(color);
        // u8g2.setBackgroundColor(bg);
        gfx.fillRect(cursor_x, cursor_y - font_height - descent, font_width, font_height, bg);
        u8g2.setCursor(cursor_x, cursor_y);
        u8g2.write((uint8_t)c);
    }
    void GFXTerminal_565::drawLine(const char *p, size_t row)
    {
        const char *h = p;
        const char *e = end(h);
        if (!e)
            return;
        while (p <= e)
        {
            char c = *p;
            const _EVLK_TERMINAL_::font565 *s = dynamic_cast<const _EVLK_TERMINAL_::font565 *>(style(p));
            /* if (c == ' ') //* debug
                 c = '.';*/
            if (c == '\n')
                c = '\\';
            if (c == '\0')
                c = '*';
            if (p != Cursor())
                drawChar(row, p - h + 1, c, s->color, s->BGopacity ? s->bgcolor : BG);
            else
                drawChar(row, p - h + 1, c, BG, s->color);
            p++;
        }
    }
    void GFXTerminal_565::clearDisplay() { gfx.fillScreen(BG); }
    void GFXTerminal_565::display()
    {
#define displayOffset 0;
        static size_t off = 0;
        const char *b = Focus();
        int16_t Height = Terminal::Height();
        for (size_t row = 1; row <= Height; row++)
        {
            drawLine(b, row + off);
            const char *s = down(b);
            if (!s)
                break;
            b = s;
        }
        driver.display(gfx.getBuffer());
        off += displayOffset;
    }
    size_t GFXTerminal_565::write(uint8_t c) { return Terminal::write(c); };
    size_t GFXTerminal_565::write(const uint8_t *buffer, size_t size) { return Terminal::write(buffer, size); }
    void GFXTerminal_565::flush()
    {
        driver.handleEvents();
        clearDisplay();
        display();
    }
}