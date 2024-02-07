#include "evlk_Terminal.h"
#include "def_ANSIctl.h"
using namespace _EVLK_TERMINAL_;

#include "HardwareSerial.h"
#include "Arduino.h"

// 向控制台打印字符
void consoleDrawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
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
// 打印一行字符
void consoleDisplayTerminal_line(const Terminal::font *lineData, uint8_t width, size_t column)
{
    for (uint8_t row = 1; row <= width; row++)
    {
        Terminal::font f = *(lineData + row - 1);
        /* if (f.c == ' ') //* debug
             f.c = '.';*/
        if (f.c == '\n')
            f.c = ' ';
        if (f.c == '\0')
            f.c = ' ';
        consoleDrawChar(row, column, f.c, f.color, f.bgcolor);
    }
}
// 打印height行字符
void consoleDisplayTerminal(Terminal &t, uint8_t height)
{
    const Terminal::font *head = NULL;
    uint8_t width = t.Width();
    for (uint8_t column = 1; column <= height; column++)
    {
        const Terminal::font *idx = t.getWindow(head);
        if (!idx)
            return;
        consoleDisplayTerminal_line(idx, width, column);
    }
}

int main()
{
    Serial.begin(19200);
    Serial.print(def_ansiec_clean); // 清屏

    const char *str = "abcdefg\nhijklmn\nopqrst\nuvwxzy";
    Terminal terminal(5, 50, str);
    consoleDisplayTerminal(terminal, 30);

    terminal.print(123); // 基本打印
    terminal.charStyle(T565_RED, T565_BLACK);
    terminal.print(456);
    consoleDisplayTerminal(terminal, 30);
    delay(1000);

    terminal.focusDown(3); // 页面滚动
    terminal.focusUp(1);
    Serial.print(def_ansiec_clean);
    consoleDisplayTerminal(terminal, 30);
    delay(1000);

    // 光标移动
    terminal.cursorDirect(0, 2, true); // 向上两行
    terminal.write('!');
    terminal.cursorDirect(3, 1);       // 向左一列
    terminal.cursorDirect(0, 1, true); // 向上一行
    terminal.write('@');
    terminal.cursorPos(3, 5); // 移动到3列5行
    terminal.write('#');
    consoleDisplayTerminal(terminal, 30);
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
    consoleDisplayTerminal(terminal, 30);
    delay(1000);

    // 打印记录
    Serial.print(terminal.getLog());
}