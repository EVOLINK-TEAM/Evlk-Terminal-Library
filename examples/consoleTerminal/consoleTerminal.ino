#include "evlk_ConsoleTerminal_565.h"
#include "def_ANSIctl.h"
using namespace _EVLK_TERMINAL_;

#include "Arduino.h"

const size_t col = 10; // col
const size_t row = 6;  // row
const size_t log_len = 100;
const size_t style_len = 20;
ConsoleTerminal_565 terminal(Serial, col, row, log_len, style_len);

void setup()
{
    Serial.begin(19200);
    Serial.print(def_vt100_clean); // 清屏

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
    Serial.print(def_vt100_clean); // 清屏
    Serial.print(def_vt100_cup(String(1), String(1)));
    Serial.print(terminal.Begin());
}

void loop(){};