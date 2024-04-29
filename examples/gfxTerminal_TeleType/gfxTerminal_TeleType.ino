#include "evlk_GFXTerminal_565.h"
#include "Arduino.h"

using namespace _EVLK_TERMINAL_;

const size_t width = 128; // px
const size_t height = 64; // px
const size_t log_len = 2000;
const size_t style_len = 400;
GFXTerminal_565 terminal(width, height, log_len, style_len, u8g2_font_4x6_tf);

/**
 * - 方向键:移动光标
 * - 退格键:删除一个字符
 * - enter键:换行
 * - shift+0(')'):切换颜色修改模式(前景色/背景色)
 * - 数字键(1~8):切换颜色(1:black,2:red,3:green,4:yellow,5:blue,6:pink,7:cyan,8:white)
 * - 数字键(0):切换成默认样式(与'颜色修改模式'无关)
 * - '='键:窗口下滚
 * - '+'键:窗口上滚
 * - 其余字母正常打印
 */

void setup()
{
    Serial.begin(19200);

    Stream *Io = &Serial;
    char c = '\0';
    static bool bg = false;
    while (1)
    {
        if (Io->available())
        {
            c = (char)Io->read();
            if (c == '\0' || c == 0xE0 || c == -32)
            {
                c = (char)Io->read();
                switch (c)
                {
                case 72: // up
                    terminal.move(0, 1);
                    break;
                case 80: // down
                    terminal.move(1, 1);
                    break;
                case 75: // left
                    terminal.move(3, 1);
                    break;
                case 77: // right
                    terminal.move(2, 1);
                    break;
                default:
                    break;
                }
            }
            else if (c == '\b')
            {
                terminal.move(3, 1);
                terminal.remove(1, 1);
            }
            else if (c == '\r')
                terminal.write('\n');

            else if (c == ')')
                bg = !bg;
            else if (c == '0')
                terminal.Pencil().sgr0();
            else if ('1' <= c && c <= '8')
                if (bg)
                    terminal.Pencil().bgColor_8(c - '1');
                else
                    terminal.Pencil().color_8(c - '1');

            else if (c == '=')
                terminal.Focus(1);
            else if (c == '-')
                terminal.Focus(0);

            else
                terminal.write(c);
        }
        terminal.flush();
    }
}

void loop() {}