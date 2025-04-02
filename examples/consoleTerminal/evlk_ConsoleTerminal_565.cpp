#include "evlk_ConsoleTerminal_565.h"

namespace _EVLK_TERMINAL_
{
    ConsoleTerminal_565::ConsoleTerminal_565(Print &realTerminal, size_t width, size_t height, size_t Log_Len, size_t Style_Len)
        : Terminal(newfact(), width, height, Log_Len, Style_Len),
          realTerminal(realTerminal) {}
    ConsoleTerminal_565::~ConsoleTerminal_565() { delete factory; }
    _EVLK_TERMINAL_::fontFactory565 &ConsoleTerminal_565::newfact()
    {
        factory = new _EVLK_TERMINAL_::fontFactory565();
        return *factory;
    }
    void ConsoleTerminal_565::drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
    {
        auto convert565ToANSI = [](uint16_t color565) -> String
        {
            int red = (color565 >> 11) & 0x1F;
            int green = ((color565 >> 5) & 0x3F);
            int blue = color565 & 0x1F;

            int red8 = (red * 255) / 31;
            int green8 = (green * 255) / 63;
            int blue8 = (blue * 255) / 31;

            return (String)red8 + ';' + (String)green8 + ';' + (String)blue8;
        };
        realTerminal.print((String)def_vt100_cup((String)row, (String)column)); // TODO
        realTerminal.print("\033[38;2;" + convert565ToANSI(color) + 'm');
        realTerminal.print("\033[48;2;" + convert565ToANSI(bg) + 'm');
        realTerminal.print(char(c));
        realTerminal.print(def_vt100_srg0);
    }
    void ConsoleTerminal_565::drawLine(const char *p, size_t row)
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
    void ConsoleTerminal_565::display()
    {
        realTerminal.print(def_vt100_clean);
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
        off += displayOffset;
        delay(1000);
    }
    size_t ConsoleTerminal_565::write(uint8_t c) { return Terminal::write(c); };
    size_t ConsoleTerminal_565::write(const uint8_t *buffer, size_t size) { return Terminal::write(buffer, size); }
}