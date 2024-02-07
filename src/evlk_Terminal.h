#ifndef _EVLK_TERMINAL_H_
#define _EVLK_TERMINAL_H_

/************************************************************
 * @file evlk_Terminal.h
 * @brief 终端模拟库
 * @author AEva
 * @version 0.1.0
 * @date 2024.01
 * ***********************************************************/

#include "Arduino.h"
#include "Print.h"

#ifndef TERMINAL_COLORS
#define TERMINAL_COLORS
#define T565_BLACK 0X0
#define T565_RED 0XF800
#define T565_GREEN 0X400
#define T565_YELLOW 0XFFE0
#define T565_BLUE 0X1F
#define T565_MAGENTA 0XF81F
#define T565_CYAN 0X7FF
#define T565_WHITE 0XFFFF
#define T_default_fontColor T565_WHITE
#define T_default_backgroundColor T565_BLACK
#endif

namespace _EVLK_TERMINAL_
{
    class Terminal : public Print
    {
    public:
        struct font
        {
            char c = '\0';
            uint16_t color = T_default_fontColor;
            uint16_t bgcolor = T_default_backgroundColor;
        };

    private:
#define _cmdTempBufferLen 16
        char cmdtemp[_cmdTempBufferLen] = "";
        uint8_t cmdlock = 0; // 0-未标识
                             // 1-头检测('[')
                             // 2-尾检测

        font *log;    // 主记录数组
        font *size;   // 容量
        font *end;    // 结尾标识
        font *focus;  // 焦点视窗
        font *cursor; // 光标
        uint8_t cursor_Save_Row = 1;
        uint8_t cursor_Save_Column = 1;
        bool cursor_Hide = false;
        font *stand; // 基准位置

        uint8_t width; // 窗口宽度

        uint16_t color = T_default_fontColor;         // 字体颜色
        uint16_t bgcolor = T_default_backgroundColor; // 背景颜色

        char *printLog; // 打印记录
        font *window;   // 打印窗口

    private:
        // 以下参数的含义
        // head:一般与p一起出现，指对于p所在行的行头，用于减少运算量，可用getColumn()算出
        // stand:当需要算head时作为基准头
        // force:当函数可能会增加记录个数时，指定是否强制增加
        bool replace(font *p, font f);
        bool insert(font *p, font f, bool force = false);                           // 尾插,force指定是否挤出最后一个字符
        bool insertFollow(font *p, char c, bool force = false);                     // 尾插，但是会跟随前一个字符的样式
        bool remove(font *p, size_t num = 1);                                       // 删除字符
        bool remove(font *p, font *e);                                              // 删除字符
        bool removeFormat(font *p, size_t num, font *head);                         //! 从某行删去几个字符，并填充补偿以保持排版
        bool clear(font *p, size_t num);                                            // 替换字符成' '
        bool clearFormat(font *p, size_t num, font *head);                          // 替换字符成' ',保持排版
        bool clearFormatR(font *p, size_t num, font *head);                         // 向前替换字符成' ',保持排版
        font *lineDown(font *head);                                                 // 获取下一行的行头,最后一行的下一行为NULL
        const font *lineDown(const font *head);                                     // 安全获取下一行的行头
        font *lineUp(font *head, font *stand, size_t num = 1);                      // 获取上一行的行头
        font *lineEnd(font *head);                                                  // 获取此行的行尾
        uint8_t getColumn(font *p, font *&head);                                    // 获取节点的纵坐标/找到行头，需要指定head作为初始迭代值
        uint8_t getRow(font *p, font *head);                                        // 获取节点的横坐标
        bool getColumn(uint8_t column, size_t &owe, font *&head);                   // 获取节点的纵坐标偏差值，需要指定head作为stand(初始迭代值)
        bool getRow(uint8_t row, uint8_t &owe, uint8_t &pos, font *head);           // 获取节点的横坐标偏差值
        font *getPos(uint8_t row, uint8_t column, font *stand, bool force = false); //! 根据横纵坐标获取节点
        uint8_t rowFillNum(font *head);                                             // 单行需要填充'\n'所需要的空字符数
        bool cursorPop();                                                           // 光标前移
        bool endPop(bool force = false);                                            // 结尾前移
        bool cursorPush(bool force = false);                                        // 光标后移
        bool endPush();                                                             // 结尾后移
        uint16_t colorCode(uint8_t code);                                           // 色码
        void cmdParser(char c);                                                     // 控制符解析

    public:
        using Print::write;
        size_t write(uint8_t) override;
        /**
         * @param width 指定窗口宽度
         * @param LogLen 存储的最大字符数
         * @param Log 初始化字符数组，当字符串长度大于LogLen时，以LogLen大小以Log长度为准
         */
        Terminal(uint8_t width, size_t LogLen, const char *Log = "");
        ~Terminal();

        /**
         * @brief 返回当前设定的宽度
         */
        uint8_t Width();

        /**
         * @brief 返回存储的字符数组的长度
         */
        size_t Length();

        /**
         * @brief 返回当前的最大行数
         * @param stand 指定开始的节点指针，如果为NULL则从头节点开始
         */
        size_t Height(font *stand = NULL);

        /**
         * @brief 重新设定宽度
         * @attention 可能会导致光标偏移或排版混乱
         */
        bool resize(uint8_t width);

        /**
         * @brief ANSI转义控制符解析
         * @param 控制码，以"\033["开头
         */
        bool cmdParser(const char *str);

        /**
         * @brief 以字符串的形式返回存储记录
         */
        const char *getLog();

        //! ****************** look this
        /**
         * @brief 获取一行的存储记录，可用于打印终端内容
         * @param head 存储记录的某个行头节点
         * @return 以复制的形式返回该行的行头指针
         * @note 1.返回的数组宽度应当严格等于当前行宽度,若内容不足行宽,则剩余部分用'\0'填充。 \n
         * @note 2.因未开放存储记录的数组及其指针，所以无法以常规方式获取行头节点。当head设置为NULL时，则head被解析为focus节点(页面左上角)。
         * @note \n 当每调用一次getWindow()后，head的值自动移到下一行的行头节点。
         * @note \n 当head已为最后一行时，再次调用getWindow()后head被置于尾节点(不显示)，此后head不会自动移动，并返回NULL。
         * @note \n 详细用法请见用例。
         */
        const font *getWindow(const font *&head);

        //**************************************************************** 光标控制

        /**
         * @brief 设置光标位置
         * @param row 列
         * @param column 行
         * @param force 当需要填充空字符时是否填充
         */
        bool cursorPos(uint8_t row, uint8_t column, bool force = false);

        /**
         * @brief 光标上下左右移动
         * @param direct 0:向上 - 1:向下 - 2:向右 - 3:向左
         * @param num 移动的行列数
         * @param force 当需要填充空字符时是否填充
         * @note force在direct为3时无效
         */
        bool cursorDirect(uint8_t direct, uint8_t num, bool force = false);

        /**
         * @brief 光标上下移动N行的第一列
         * @param direct 0:向下 - 1:向上
         * @param num 移动的行数
         * @param force 当需要填充空字符时是否填充
         * @note force在direct为1时无效
         */
        bool cursorDirectHead(uint8_t direct, uint8_t num, bool force = false);

        /**
         * @brief 光标移动到当前行的指定列
         * @param row 指定列数
         * @param force 当需要填充空字符时是否填充
         */
        bool cursorRow(uint8_t row, bool force = false);

        /** @brief 获取光标所在位置*/
        bool cursorGet(uint8_t &row, uint8_t &column);

        /** @brief 保存光标所在位置*/
        bool cursorSave();

        /** @brief 取出保存的光标位置*/
        bool cursorUse();

        /**
         * @brief 光标隐藏
         * @param enable true:隐藏 - false:显示
         */
        bool cursorHide(bool enable = false);

        //**************************************************************** 屏幕控制

        /**
         * @brief 清空光标左右的字符，光标位置不变
         * @param direct 0:向右(不含光标) - 1:向左 - 2:整行
         * @param num 清空个数
         */
        bool clearDirect(uint8_t direct, uint8_t num);

        /**
         * @brief 清空屏幕
         * @param direct 0:光标右下屏(不含光标行列) - 1:光标左上屏 - 2:整屏(光标移至左上角)
         */
        bool clearScreen(uint8_t direct);

        /**
         * @brief 删除字符
         * @param direct 0:删除光标之下n行，剩下行往上移 - 1:删除光标右边n个字符，剩下部分左移
         * @param num 删除的行数/个数
         */
        bool removeDirect(uint8_t direct, uint8_t num);

        /**
         * @brief 在光标处插入n个字符
         * @param c 字符
         * @param num 插入的个数
         * @param force 当插入个数溢出时，是否将最后的字符移出
         */
        bool Insert(char c, uint8_t num, bool force = false);

        /**
         * @brief 页面向上滚动
         * @param num 滚动的行数
         * @attention 单行时间复杂度较高，不宜频繁使用
         */
        bool focusUp(uint8_t num = 1);

        /**
         * @brief 页面向下滚动
         * @param num 滚动的行数
         */
        bool focusDown(uint8_t num = 1);

        //**************************************************************** 字体控制

        /**
         * @brief 设置字体颜色，输入565颜色
         * @param color 前景色
         * @param bgColor 背景色
         */
        bool charStyle(uint16_t color = T_default_fontColor, uint16_t bgColor = T_default_backgroundColor);

        /** @brief 当前颜色翻转*/
        bool charColorReverse();
    };
}
#endif