#include "evlk_Terminal.h"
#include "evlk_Terminal_font565.h"
#include "def_vt100.h"
#include "evlk_miniGraph.h"

#include "Arduino.h"
namespace _EVLK_TERMINAL_
{
    class ConsoleTerminal_565 : public Terminal,
                                public Print
    {
    private:
        uint16_t BG = 10857;
        Print &realTerminal;
        _EVLK_TERMINAL_::fontFactory565 *factory;

        _EVLK_TERMINAL_::fontFactory565 &newfact();

    public:
        ConsoleTerminal_565(Print &realTerminal, size_t width, size_t height, size_t Log_Len, size_t Style_Len);
        ~ConsoleTerminal_565();
        void drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg);
        void drawLine(const char *p, size_t row);
        void display();
        size_t write(uint8_t c) override;
        size_t write(const uint8_t *buffer, size_t size) override;
        using Print ::write;
    };
}