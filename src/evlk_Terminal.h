#ifndef _EVLK_TERMINAL_H_
#define _EVLK_TERMINAL_H_

/************************************************************
 * @file evlk_Terminal.h
 * @brief 终端模拟库
 * @author AEva
 * @version 0.1.0
 * @date 2024.01
 * ***********************************************************/

#include <cstddef>
#include "simpleCntr.h"
#include "evlk_Terminal_fontImpl.h"

namespace _EVLK_TERMINAL_
{
    class Terminal //: public Print
    {
    private:
        struct fontI // style与log对应的索引器，应包含一个font实例
        {
            size_t size;
            font *style;
            bool destory;
            fontI(bool des = true);
            ~fontI();
            fontI &operator=(const fontI &);
        };

    private:
        typedef const fontI *fp;
        typedef const char *lp;

    private:
        fp style(lp, lp &, fp &); // 样式查找器
        fp style_q(lp);           // 快速样式查找器
        fp style_q(lp, lp &h);    // 快速样式查找器
        void style_q_r();         // 左移快速样式查找器
        fp style_resize(fp, size_t);
        fp style_new(fp, size_t, font &); // 应接在styles.insert以后，否则内存不会被释放

        /**
         * @brief 获取一段节点占用的样式状态
         * @param b begin
         * @param n num
         * @return h 返回头样式的占用情况
         * @return m 返回中间样式的占用情况
         * @return e 返回尾样式的占用情况
         * @return se 返回尾样式指针
         * @note b+n未超出头结点时：
         * @note h = n, m = 0, e = 头结点剩余数, se = sh
         */
        void style_diff(lp b, size_t n, size_t &h, size_t &m, size_t &e, fp &se);

        fontFactory *const styleFactory;
        simpleCntr<fontI> styles;
        font *pencil;

        bool style_check(); //! test

    private:
#define _cmdTempBufferLen 16
        char cmdtemp[_cmdTempBufferLen] = "";
        uint8_t cmdlock = 0; // 0-未标识
                             // 1-头检测('[')
                             // 2-尾检测

        simpleCntr<char> log;

        uint8_t width; // 窗口宽度
        lp stand;      // 基准位置
        lp focus;      // 焦点视窗
        lp cursor;     // 光标

        uint8_t cursor_Save_Row = 1;
        uint8_t cursor_Save_Column = 1;
        bool cursor_Hide = false;

        // 快速搜寻样式，适用于连续查找，相当于存档点不需要从头开始计算
        //(只能被style_q调用)
        lp style_serch_lp = NULL;
        fp style_serch_fp = NULL;
        // 快速搜寻行头
        lp line_serch_lp = NULL;
        size_t line_serch_num = 0;
        size_t line_serch_col = 0;

    private:
        // 以下参数的含义
        // head:一般与p一起出现，指对于p所在行的行头，用于减少运算量，可用getColumn()算出
        // stand:当需要算head时作为基准头
        // force:当函数可能会增加记录个数时，指定是否强制增加
        bool isIn(lp p);
        bool isIn(lp p, lp h);
        lp insert(lp p, size_t n, font &s, char f);   // 前插,分配空间
        lp insert_s(lp p, size_t n, font &s, char f); // 保持光标、焦点等位置不变
        lp remove(lp p, size_t n);                    // 删除字符
        lp remove_s(lp p, size_t n);                  // 保持光标、焦点等位置不变
        lp remove_f(lp p, size_t n, lp head);         //! 从某行删去几个字符，并填充补偿以保持排版
        lp replace(lp p, size_t n, font &s, char f);

        bool clear(lp p, size_t num);                                         // 替换字符成' '
        bool clear_f(lp p, size_t num, lp head);                              // 替换字符成' ',保持排版
        bool clear_fR(lp p, size_t num, lp head);                             // 向前替换字符成' ',保持排版
        lp lineDown(lp head);                                                 // 获取下一行的行头,最后一行的下一行为NULL
        lp lineUp(lp head, lp stand, size_t num = 1);                         // 获取上一行的行头
        lp lineEnd(lp head);                                                  // 获取此行的行尾
        uint8_t getColumn(lp p, lp &head);                                    // 获取节点的纵坐标/找到行头，需要指定head作为初始迭代值
        uint8_t getRow(lp p, lp head);                                        // 获取节点的横坐标,从1开始
        bool getColumn(uint8_t column, size_t &owe, lp &head);                // 获取节点的纵坐标偏差值，需要指定head作为stand(初始迭代值)
        bool getRow(uint8_t row, uint8_t &owe, uint8_t &pos, lp head);        // 获取节点的横坐标偏差值
        lp getPos(uint8_t row, uint8_t column, lp stand, bool force = false); //! 根据横纵坐标获取节点
        uint8_t rowFillNum(lp head);                                          // 单行需要填充'\n'所需要的空字符数
        void cmdParser(char c);                                               // 控制符解析

    public:
        size_t write(uint8_t);
        size_t write(const uint8_t *buffer, size_t size);

        /**
         * @param width 指定窗口宽度
         * @param LogLen 存储的最大字符数
         * @param Log 初始化字符数组，当字符串长度大于LogLen时，以LogLen大小以Log长度为准
         */
        Terminal(uint8_t width, size_t Log_Len, fontFactory &factory, size_t Style_Len);
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
        size_t Height(lp stand = NULL);

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

        //**************************************************************** 字体控制

        /**
         * @brief 设置字体画笔
         * @param pencil 画笔对象
         */
        bool charStyle(_EVLK_TERMINAL_::font &pencil);

        //************************************************************** 打印接口
        /**
         * @brief 焦点上行
         * @param num 滚动的行数
         * @attention 单行时间复杂度较高，不宜频繁使用
         */
        bool focusUp(uint8_t num = 1);
        /**
         * @brief 焦点下行
         * @param num 滚动的行数
         */
        bool focusDown(uint8_t num = 1);

        /**
         * @brief 获取焦点指针
         */
        lp getFocus();
        /**
         * @brief 获取光标指针
         * @note 如果为NULL，则隐藏了光标，可以使用```cursorHide(false);```恢复
         */
        lp getCursor();
        /**
         * @brief 获取头指针
         */
        lp getBegin();
        /**
         * @brief 获取尾指针
         * @note 尾指针始终为空，访问最后以一个元素应该使用```getEnd()-1```
         */
        lp getEnd();

        /**
         * @brief 获取上行
         */
        lp UP(lp h, size_t n = 1);

        /**
         * @brief 获取下行
         */
        lp DOWN(lp h, size_t n = 1);

        /**
         * @brief 获取样式
         */
        const font *style(lp p);
    };
}
#endif