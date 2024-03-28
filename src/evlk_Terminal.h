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
        struct fontI // 样式与索引，应包含一个font实例
        {
            size_t size;
            font *style;
            bool destory;
            fontI(bool des = true);
            ~fontI();
            fontI &operator=(const fontI &);
        };

        typedef const fontI *fp; // 样式索引器
        typedef const char *lp;  // 记录索引器

    private:
        // VT100缓存
#define _cmdTempBufferLen 16
        char cmdtemp[_cmdTempBufferLen] = "";
        uint8_t cmdlock = 0; // 0-未标识
                             // 1-头检测('[')
                             // 2-尾检测

        simpleCntr<fontI> styles;
        simpleCntr<char> log;
        simpleCntr<lp> editor;

        fontFactory *const styleFactory; // 样式生成器
        font *pencil;                    // 画笔
        font *blank;                     // 空样式

        size_t width;  // 编辑框宽度
        size_t height; // 编辑框高度
        lp focus;      // 焦点位置
        lp cursor;     // 光标

        bool cursor_Hide = false;

        // 快速样式查找器，适用于连续查找
        //(只能被style_q调用)
        lp style_serch_lp = NULL;
        fp style_serch_fp = NULL;

        // 编辑框首字符的样式
        lp style_editor_lp = NULL;
        fp style_editor_fp = NULL;

    private:
        fp style_resize(fp, size_t);
        fp style_new(fp, size_t, font &); //! 应接在styles.insert以后，否则内存不会被释放
        fp style(lp, lp &, fp &);         // 样式查找器
        fp style_q(lp);                   // 快速样式查找器
        fp style_q(lp, lp &h);            // 快速样式查找器
        void style_q_r();                 // 左移快速样式查找器

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
        bool style_check(); //! test

        bool isIn(lp p);
        bool isIn(lp p, lp h);
        bool isIn(size_t head_i);                     // editor
        lp _insert(lp p, size_t n, font &s, char f);  // 前插,分配空间
        lp _remove(lp p, size_t n);                   // 删除字符
        lp _replace(lp p, size_t n, font &s, char f); // 替换字符

        // head_i = row-1;
        // editor[head_i]访问行头结点
        lp _head(lp ep, size_t &head_i); // !只能在editor内查找
        lp _end(size_t head_i);
        lp _end(lp head);
        lp _down(size_t &head_i, size_t &n);
        lp _down(lp head, size_t &n);
        lp _up(size_t &head_i, size_t &n);
        lp _up(lp head, size_t &n);

        lp _fillCol(size_t head_i, size_t Col, bool force = true); // 分配到指定Col需要的空间
        lp _fillRow(size_t Row, bool force = true);                // 分配到指定Row需要的空间
        lp _delCol(size_t head_i, size_t Col, size_t n);           // 删除单行的字符,含col,n会自动调整为最大删除量
        lp _setPos(size_t row, size_t col, bool force = true);
        lp _getPos(size_t &row, size_t &col);
        lp _formPos(size_t row, size_t col);
        size_t write_f(const uint8_t *buffer, size_t size); // 自动换行
        lp editorUp(size_t &n, bool force = false);         // todo
        lp editorDown(size_t &n, bool force = false);

        void VT100(char c); // 控制符解析

    public:
        size_t write(uint8_t);
        size_t write(const uint8_t *buffer, size_t size);

        /**
         * @param width 指定窗口宽度
         * @param height 指定窗口宽度
         * @param LogLen 存储的最大字符数
         * @param Log 初始化字符数组，当字符串长度大于LogLen时，以LogLen大小以Log长度为准
         */
        Terminal(fontFactory &factory, size_t width, size_t height, size_t Log_Len, size_t Style_Len);
        ~Terminal();

        /**
         * @brief 返回当前设定的宽度
         */
        uint8_t Width();

        /**
         * @brief 返回当前设定的高度
         */
        size_t Height();

        /**
         * @brief 返回存储的字符数组的长度
         */
        size_t Length();

        bool resize(size_t width, size_t height);

        /**
         * @brief ANSI转义控制符解析
         * @param 控制码，以"\033["开头
         */
        bool VT100(const char *str, size_t len);
        bool VT100(const char *str);

        //**************************************************************** 光标控制

        /** @brief 获取光标所在位置*/
        bool Pos(size_t &row, size_t &column);

        /**
         * @brief 设置光标位置
         * @param row 列
         * @param column 行
         * @param force 当需要填充空字符时是否填充
         */
        bool cup(size_t row, size_t column, bool force = false);

        /**
         * @brief 光标移动
         * @param direct 0:向上 - 1:向下 - 2:向右 - 3:向左 - 4:向下到行头 -5:向上到行头 -6:移动指定列
         * @param num 移动数
         * @param force 当需要填充空字符时是否填充,在direct为3,5时无效
         */
        bool move(uint8_t direct, uint8_t num, bool force = false);

        /**
         * @brief 光标隐藏
         * @param enable true:隐藏 - false:显示
         */
        bool hide(bool enable = false);

        //**************************************************************** 屏幕控制

        /**
         * @brief 清空(含光标)
         * @param direct 0:向右 - 1:向右 - 2:向左 - 3:整行 - 4:右下 - 5:左上 - 6:整屏(光标移至左上角)
         * @param num 清空个数,direct为0时有效
         * @note direct为1时，清空向右全部字符
         */
        bool clear(uint8_t direct, uint8_t num = 1);

        /**
         * @brief 删除字符
         * @param direct 0:删除光标之下n行(包含所在行)，剩下行往上移 - 1:删除光标右边n个字符，剩下部分左移
         * @param num 删除的行数/个数
         */
        bool remove(uint8_t direct, uint8_t num);

        //**************************************************************** 字体控制

        /**
         * @brief 设置字体画笔
         * @param pencil 画笔对象
         */
        bool Pencil(_EVLK_TERMINAL_::font &pencil);

        /**
         * @brief 获取字体画笔
         */
        _EVLK_TERMINAL_::font &Pencil();

        //************************************************************** 打印接口
        /**
         * @brief 焦点滚动
         * @param direct 0:上滚 - 1:下滚
         * @param num 滚动的行数
         * @attention 单行时间复杂度较高，不宜频繁使用
         */
        bool Focus(bool direct, size_t num = 1);
        /**
         * @brief 获取焦点指针
         */
        lp Focus();
        /**
         * @brief 获取光标指针
         * @note 如果为NULL，则隐藏了光标，可以使用```cursorHide(false);```恢复
         */
        lp Cursor();
        /**
         * @brief 获取头指针
         */
        lp Begin();
        /**
         * @brief 获取尾指针
         * @note 尾指针始终为空，访问最后以一个元素应该使用```getEnd()-1```
         */
        lp End();

        /**
         * @brief 获取上行
         */
        lp up(lp h, size_t n = 1);
        /**
         * @brief 获取下行
         */
        lp down(lp h, size_t n = 1);
        /**
         * @brief 获取结尾
         */
        lp end(lp head);

        /**
         * @brief 获取样式
         */
        const font *style(lp p);
    };
}
#endif