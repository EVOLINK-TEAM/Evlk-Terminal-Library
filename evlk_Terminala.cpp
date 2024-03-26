#include "evlk_Terminal.h"
#include <cctype>
#include <cstring>
#include <stdlib.h>

namespace _EVLK_TERMINAL_
{

    bool Terminal::clear(lp p, size_t num)
    {
        char f = ' ';
        return (bool)log.replace(p, num, f, false);
    }
    bool Terminal::clear_f(lp p, size_t num, lp head)
    {
        if (!isIn(p, head))
            return false;

        lp e = lineEnd(head);
        if (num >= e - p + 1)
        {
            num = e - p + 1;
            if (*e == '\n')
                num--;
        }
        return clear(p, num);
    }
    bool Terminal::clear_fR(lp p, size_t num, lp head)
    {
        if (!isIn(p, head))
            return false;

        if (num >= p - head + 1)
        {
            num = p - head + 1;
            if (*p == '\n')
                num--;
        }
        return clear(p - num + 1, num);
    }

    uint8_t Terminal::getColumn(lp p, lp &head)
    {
        if (!isIn(p, head))
            return 0;

        lp pre = NULL;
        lp idx = head;
        uint8_t column = 0;

        while (idx && idx <= p)
        {
            pre = idx;
            idx = lineDown(idx);
            column++;
        }
        head = pre;
        return column;
    }
    uint8_t Terminal::getRow(lp p, lp head)
    {
        if (!isIn(p, head))
            return 0;
        return p - head + 1;
    }
    bool Terminal::getColumn(uint8_t column, size_t &owe, lp &head)
    {
        if (column <= 0 || !isIn(head))
            return false;
        owe = 0; // 只有在column>最低行的时候才会为正值
        lp next = head;
        while (column-- > 1)
        {
            next = lineDown(head);

            if (next)
                head = next;
            else
                owe++;
        }
        return true;
    }
    bool Terminal::getRow(uint8_t row, uint8_t &owe, uint8_t &pos, lp head)
    {
        if (row <= 0 || row > width || !isIn(head))
            return false;
        pos = head ? width - rowFillNum(head) : 0; // 行填充位置(如果head为NULL的话则为假行，假行为'\n'单字符行)
        owe = pos < row ? row - pos : 0;           // 行填充数

        return true;
    }
    Terminal::lp Terminal::getPos(uint8_t row, uint8_t column, lp stand, bool force)
    {
        lp head = stand;
        size_t fillLine;
        uint8_t fillRow;
        uint8_t fillRowPos;
        if (!getColumn(column, fillLine, head))
            return NULL;

        if (fillLine)
        {
            if (!getRow(row, fillRow, fillRowPos, NULL))
                return NULL;
            if (rowFillNum(head)) // head为最后一行，如果最后一行不完整，则会换行
                fillLine++;
        }
        else if (!getRow(row, fillRow, fillRowPos, head))
            return NULL;

        if (fillRow)
        {
            if (fillRowPos == 0) // 如果'\n'是首字符，那么填充时会把'\n'挤出
                fillRow--;
            else if (fillRowPos == width - 1) //'\n'在最后不需要处理
                fillRow--;
        }

        if (!fillLine && !fillRow)
            return head + row - 1;

        if (force)
        {
            if (log.end() + fillLine + fillRow >= log.size()) // 溢出判断
                return NULL;

            fp r = style_q(head + fillRowPos);
            if (!r)
                return NULL;

            font *fe = (r->style);

            if (!insert_s(log.end(), fillLine, *fe, '\n'))
                return NULL;
            while (fillLine)
            {
                head = log.end() - 1;
                fillLine--;
            }

            if (!insert_s(head + fillRowPos, fillRow, *fe, ' '))
                return NULL;

            return head + row - 1;
        }
        return NULL;
    }
    uint8_t Terminal::rowFillNum(lp head)
    {
        lp e = lineEnd(head);
        if (!e)
            return 0;

        uint8_t f = width - (e - head + 1);
        if (*e == '\n')
            f++;
        return f;
    }

    bool Terminal::resize(uint8_t width)
    {
        this->width = width;
        return true;
    }

    bool Terminal::clearDirect(uint8_t direct, uint8_t num)
    {
        lp head = log.begin();
        uint8_t column = getColumn(cursor, head);
        if (!column)
            return false;

        /**Direction:
         * 0:RIGHT
         * 1:LEFT
         * 2:LINE*/
        switch (direct)
        {
        case 0:
            return clear_f(cursor + 1, num, head);
            break;

        case 1:
            return clear_f(cursor, num, head);
            break;

        case 2:
            return clear_f(cursor + 1, num, head) && clear_fR(cursor, num, head);
            break;

        default:
            return false;
            break;
        }
        return true;
    }
    bool Terminal::clearScreen(uint8_t direct)
    {
        lp head = log.begin();
        uint8_t column = getColumn(cursor, head);
        uint8_t row = getRow(cursor, head);
        if (!column || !row)
            return false;

        lp idx;
        /**Direction:
         * 0:RIGHT_DOWN
         * 1:LEFT_UP
         * 2:SCREEN*/
        switch (direct)
        {
        case 0:
            idx = head;
            idx = lineDown(idx);
            while (idx)
            {
                lp begin = idx + row;
                clear_f(begin, -1, idx);
                idx = lineDown(idx);
            }
            break;
        case 1:
            idx = focus;
            if (idx > head)
                idx = NULL;
            while (idx)
            {
                lp begin = idx + row - 1;
                clear_fR(begin, -1, idx);
                idx = lineDown(idx);
                if (idx > head)
                    idx = NULL;
            }
            clear_fR(cursor, -1, head);
            break;
        case 2:
        {
            idx = focus;
            clear(idx, 1);
            remove(idx + 1, log.end() - (idx + 1));
            cursor = focus;
        }
        break;

        default:
            return false;
            break;
        }
        return true;
    }
    bool Terminal::Insert(char c, uint8_t num, bool force)
    {
        return (bool)insert_s(cursor + 1, num, *pencil, c);
    }

    bool Terminal::focusUp(uint8_t num)
    {
        focus = lineUp(focus, log.begin(), num);
        return true;
    }
    bool Terminal::focusDown(uint8_t num)
    {
        while (num)
        {
            lp next = lineDown(focus);
            if (next)
                focus = next;
            num--;
        }
        return true;
    }
    Terminal::lp Terminal::getFocus()
    {
        return focus;
    }
    Terminal::lp Terminal::getCursor()
    {
        if (cursor_Hide)
            return NULL;
        return cursor;
    }
    Terminal::lp Terminal::getBegin()
    {
        return log.begin();
    }
    Terminal::lp Terminal::getEnd()
    {
        return log.end();
    }
    Terminal::lp Terminal::UP(lp h, size_t n)
    {
        return lineUp(h, log.begin(), n);
    }
    Terminal::lp Terminal::DOWN(lp h, size_t n)
    {
        while (n)
        {
            h = lineDown(h);
            n--;
        }
        return h;
    }
    const font *Terminal::style(lp p)
    {
        fp S = style_q(p);
        if (!S)
            return NULL;
        return S->style;
    }
}
