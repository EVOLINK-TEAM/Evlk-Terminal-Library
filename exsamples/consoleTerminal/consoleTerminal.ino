#include "evlk_Terminal.h"
#include "evlk_Terminal_font565.h"
#include "def_ANSIctl.h"
using namespace _EVLK_TERMINAL_;

#include "Arduino.h"

class ConsoleTerminal_565 : public Terminal,
                            public Print
{
private:
    uint16_t BG = 10857;
    Print &realTerminal;

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
        realTerminal.print(def_ansiec_cur_pos(String(itoa(row, num, 10)), String(itoa(column, num, 10)))); // TODO
        realTerminal.print("\033[38;2;" + convert565ToANSI(color) + 'm');
        realTerminal.print("\033[48;2;" + convert565ToANSI(bg) + 'm');
        realTerminal.print(char(c));
        realTerminal.print(def_ansiec_normal);
    }
    void drawLine(const char *p, size_t column)
    {
        const char *h = p;
        const char *e = end(h);
        if (!e)
            return;
        while (p <= e)
        {
            char c = *p;
            const font565 *s = dynamic_cast<const font565 *>(style(p));
            /* if (c == ' ') //* debug
                 c = '.';*/
            if (c == '\n')
                c = '\\';
            if (c == '\0')
                c = '*';
            if (p != Cursor())
                drawChar(p - h + 1, column, c, s->color, s->BGopacity ? s->bgcolor : BG);
            else
                drawChar(p - h + 1, column, c, BG, s->color);
            p++;
        }
    }
    void display()
    {
        realTerminal.print(def_ansiec_clean);
#define displayOffset 0;
        static size_t off = 0;
        const char *b = Focus();
        int16_t Height = Terminal::Height();
        for (size_t col = 1; col <= Height; col++)
        {
            drawLine(b, col + off);
            const char *s = down(b);
            if (!s)
                break;
            b = s;
        }
        off += displayOffset;
        delay(1000);
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
    ConsoleTerminal_565(Print &realTerminal, fontFactory &factory, size_t width, size_t height, size_t Log_Len, size_t Style_Len)
        : Terminal(factory, width, height, Log_Len, Style_Len),
          realTerminal(realTerminal) {}
};

fontFactory565 factory;
ConsoleTerminal_565 terminal(Serial, factory, 10, 6, 100, 20);

void setup()
{
    Serial.begin(19200);
    Serial.print(def_ansiec_clean); // 清屏

    terminal.print("abcdefghijklmnopqrstuvwxzy");
    terminal.display();

    terminal.print(123);            // 基本输入
    terminal.display();             // 打印
    terminal.Pencil().color_8(6);   // color = cyan
    terminal.Pencil().bgColor_8(0); // background color = black
    terminal.print(456);
    terminal.display(); // 打印

    terminal.Focus(1, 3); // 页面滚动(down)
    terminal.display();   //  打印
    terminal.Focus(0, 1); // up
    terminal.display();   //  打印

    // 光标移动
    terminal.move(0, 1); // 向上一行
    terminal.display();  // 打印
    terminal.write('!');
    terminal.display();  // 在write()后，focus会自动调整位置
    terminal.move(3, 1); // 向左一列
    terminal.move(0, 1); // 向上一行
    terminal.display();  // 打印
    terminal.write('@');
    terminal.display(); // 打印
    terminal.cup(1, 3); // 移动到1行3列
    terminal.write('#');
    terminal.display(); // 打印

    // 屏幕控制
    terminal.cup(1, 4);
    terminal.display(); // 打印
    terminal.clear(1);  // 清除光标右边的全部字符
    terminal.display(); // 打印
    terminal.cup(3, 4);
    terminal.display(); // 打印
    terminal.clear(5);  // 清除光标左上角的字符
    terminal.display(); // 打印
    terminal.cup(1, 1);
    terminal.display();    // 打印
    terminal.remove(0, 2); // 删除光标下边两行字符
    terminal.display();    // 打印
    terminal.remove(1, 2); // 删除光标右边两个字符
    terminal.display();    // 打印
    delay(1000);

    // 打印记录
    Serial.print(def_ansiec_clean); // 清屏
    Serial.print(def_ansiec_cur_pos(String(1), String(1)));
    Serial.print(terminal.Begin());
}

void loop(){};