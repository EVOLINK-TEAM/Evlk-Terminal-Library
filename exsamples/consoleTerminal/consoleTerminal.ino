#include "evlk_Terminal.h"
#include "evlk_Terminal_font565.h"
#include "def_ANSIctl.h"
using namespace _EVLK_TERMINAL_;

#include "Arduino.h"

class ConsoleTerminal_565 : public Terminal,
                            public Print
{
public:
    void drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
    {
        auto convert565ToANSI = [](uint16_t color565) -> String
        {
            int red = (color565 >> 11) & 0x1F;
            int green = ((color565 >> 5) & 0x3F);
            int blue = color565 & 0x1F;

            int red8 = (red * 255) / 31;
            int green8 = (green * 255) / 63;
            int blue8 = (blue * 255) / 31;

            char num[4];
            return String(itoa(red8, num, 10)) + ';' + String(itoa(green8, num, 10)) + ';' + String(itoa(blue8, num, 10));
        };
        char num[4];
        Serial.print(def_ansiec_cur_pos(String(itoa(row, num, 10)), String(itoa(column, num, 10)))); // TODO
        Serial.print("\033[38;2;" + convert565ToANSI(color) + 'm');
        Serial.print("\033[48;2;" + convert565ToANSI(bg) + 'm');
        Serial.print(char(c));
        Serial.print(def_ansiec_normal);
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
    void display(uint8_t Height)
    {
#define displayOffset 0;
        static size_t off = 0;
        const char *b = getFocus();
        for (size_t col = 1; col <= Height; col++)
        {
            drawLine(b, col + off);
            const char *s = DOWN(b);
            if (!s)
                break;
            b = s;
        }
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
    ConsoleTerminal_565(uint8_t width, size_t Log_Len, fontFactory &factory, size_t Style_Len)
        : Terminal(width, Log_Len, factory, Style_Len) {}
};

fontFactory565 factory;
ConsoleTerminal_565 terminal(5, 100, factory, 20);

void setup()
{
    Serial.begin(19200);
    Serial.print(def_ansiec_clean); // 清屏

    terminal.print("abcdefg\nhijklmn\nopqrst\nuvwxzy");
    terminal.display(30);

    terminal.print(123); // 基本打印
    font565 f;
    f.color_8(6);   // color = cyan
    f.bgColor_8(0); // background color = black
    terminal.charStyle(f);
    terminal.print(456);
    terminal.display(30);
    delay(1000);

    terminal.focusDown(3); // 页面滚动
    terminal.focusUp(1);
    Serial.print(def_ansiec_clean);
    terminal.display(30);
    delay(1000);

    // 光标移动
    terminal.cursorDirect(0, 2, true); // 向上两行
    terminal.write('!');
    terminal.cursorDirect(3, 1);       // 向左一列
    terminal.cursorDirect(0, 1, true); // 向上一行
    terminal.write('@');
    terminal.cursorPos(3, 5); // 移动到3列5行
    terminal.write('#');
    terminal.display(30);
    delay(1000);

    // 屏幕控制
    terminal.focusUp(2);
    terminal.cursorPos(4, 1);
    terminal.clearDirect(0, -1); // 清除光标右边的全部字符
    terminal.cursorPos(2, 3);
    terminal.clearScreen(1); // 清除光标左上角的字符
    terminal.cursorPos(2, 5);
    terminal.removeDirect(0, 2); // 删除光标下边两行字符
    terminal.removeDirect(1, 2); // 删除光标右边两个字符
    terminal.display(30);
    delay(1000);

    // 打印记录
    Serial.print(terminal.getBegin());
}

void loop(){};