#include "evlk_Terminal.h"

namespace _EVLK_TERMINAL_
{
    bool Terminal::replace(font *p, font f)
    {
        *p = f;
        return true;
    }
    bool Terminal::insert(font *p, font f, bool force)
    {
        font *last = end - 1; // 需要向后移动一位
        if (!endPush())
        {
            if (!force)
                return false;

            last--;
        }
        while (last >= p)
        {
            *(last + 1) = *last;
            last--;
        }
        replace(p + 1, f);
        return true;
    }
    bool Terminal::insertFollow(font *p, char c, bool force)
    {
        font f;
        if (p == log)
        {
            f.color = color;
            f.bgcolor = bgcolor;
        }
        else
            f = *p--;
        f.c = c;

        return insert(p, f, force);
    }
    bool Terminal::remove(font *p, size_t num)
    {
        num = num > 1 ? num : 1;
        if (p + num - 1 >= end)
            return false;
        if (p < cursor)
            cursor -= num;

        for (font *i = p; i < end - num; ++i)
        {
            *i = *(i + num);
        }
        while (num)
        {
            if (!endPop(true))
                return false;
            num--;
        }

        return true;
    }
    bool Terminal::remove(font *p, font *e)
    {
        if (e <= p)
            return false;
        size_t num = e - p;
        return remove(p, num);
    }
    bool Terminal::removeFormat(font *p, size_t num, font *head)
    {
        if (num < 1)
            return false;

        uint8_t row = getRow(p, head);
        font *e = lineEnd(head);
        uint8_t fillpos = width - rowFillNum(head);
        if (e == head && e->c == '\n') //'\n'在行首
            return true;

        num = row + num - 1 > width ? width - row + 1 : num;
        if (p + num > head + fillpos)
        {
            num = head + fillpos - p + 1;
            if (e->c == '\n')
                num--;
        }

        if (e == end - 1 && e->c != '\n') // 尾行
            num--;

        fillpos = fillpos - num + 1;

        char t = e->c;
        if (!remove(p, num))
            return false;

        if (t != '\n')
            if (!insertFollow(head + fillpos - 1, '\n', false))
                return false;

        return true;
    }
    bool Terminal::clear(font *p, size_t num)
    {
        while (num)
        {
            font f = *(p + num - 1);
            f.c = ' ';
            replace((p + num - 1), f);
            num--;
        }
        return true;
    }
    bool Terminal::clearFormat(font *p, size_t num, font *head)
    {
        font *e = lineEnd(head);
        if (num >= e - p + 1)
        {
            num = e - p + 1;
            if (e->c == '\n')
                num--;
        }
        return clear(p, num);
    }
    bool Terminal::clearFormatR(font *p, size_t num, font *head)
    {
        if (num >= p - head + 1)
        {
            num = p - head + 1;
            if (p->c == '\n')
                num--;
        }
        return clear(p - num + 1, num);
    }
    Terminal::font *Terminal::lineDown(font *head)
    {
        if (!head)
            return NULL;

        for (font *t = head; t < head + width; t++)
        {
            if (t->c == '\n')
            {
                if (t + 1 >= end)
                    return NULL;
                return t + 1;
            }
        }
        if (head + width >= end)
            return NULL;
        return head + width;
    }
    const Terminal::font *Terminal::lineDown(const font *head)
    {
        font *Head = const_cast<font *>(head);
        return lineDown(Head);
    }
    Terminal::font *Terminal::lineUp(font *head, font *stand, size_t num)
    {
        if (head < stand)
            return NULL;

        font *first = stand;
        font *idx = first;
        while (first < head)
        {
            first = lineDown(first);
            if (num)
                num--;
            else
                idx = lineDown(idx);
        }
        return idx;
    }
    Terminal::font *Terminal::lineEnd(font *head)
    {
        head = lineDown(head);
        if (!head)
            return end - 1;
        return head - 1;
    }
    uint8_t Terminal::getColumn(font *p, font *&head)
    {
        if (head > p)
            return 0;

        font *pre = NULL;
        font *idx = head;
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
    uint8_t Terminal::getRow(font *p, font *head)
    {
        if (head > p)
            return 0;

        return p - head + 1 <= width && p - head + 1 > 0 ? p - head + 1 : 0;
    }
    bool Terminal::getColumn(uint8_t column, size_t &owe, font *&head)
    {
        if (column <= 0)
            return false;
        owe = 0; // 只有在column>最低行的时候才会为正值
        font *next = head;
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
    bool Terminal::getRow(uint8_t row, uint8_t &owe, uint8_t &pos, font *head)
    {
        if (row <= 0 || row > width)
            return false;
        pos = head ? width - rowFillNum(head) : 0; // 行填充位置(如果head为NULL的话则为假行，假行为'\n'单字符行)
        owe = pos < row ? row - pos : 0;           // 行填充数

        return true;
    }
    Terminal::font *Terminal::getPos(uint8_t row, uint8_t column, font *stand, bool force)
    {
        font *head = stand;
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
            if (end + fillLine + fillRow >= size) // 溢出判断
                return NULL;

            font f = *(head + fillRowPos);
            while (fillLine)
            {
                f.c = '\n';
                insert(end - 1, f, false);
                head = end - 1;
                fillLine--;
            }

            while (fillRow)
            {
                f.c = ' ';
                insert(head + fillRowPos - 1, f, false);
                fillRow--;
            }

            return head + row - 1;
        }
        return NULL;
    }
    uint8_t Terminal::rowFillNum(font *head)
    {
        for (uint8_t i = 0; i < width; i++)
        {
            if ((head + i)->c == '\n')
                return width - i;
        }

        if (head + width >= end)
        {
            return width - (end - head);
        }
        return 0;
    }
    bool Terminal::cursorPop()
    {
        if (cursor <= log)
            return false;
        cursor--;
        return true;
    }
    bool Terminal::endPop(bool force)
    {
        if (end <= cursor && (!force || !cursorPop()))
            return false;
        font f;
        end--;
        *end = f;
        return true;
    }
    bool Terminal::cursorPush(bool force)
    {
        if (cursor < end - 1)
        {
            cursor++;
            return true;
        }

        if (force && endPush())
        {
            cursor++;
            return true;
        }

        return false;
    }
    bool Terminal::endPush()
    {
        if (end < size)
        {
            font f;
            if (end->c == '\0')
            {
                f.c = ' ';
                *end = f;
            }
            end++;
            f.c = '\0';
            *end = f;
            return true;
        }
        return false;
    }
    uint16_t Terminal::colorCode(uint8_t code)
    {
        uint16_t Color = 0;
        switch (code)
        {
        case 0:
            Color = T565_BLACK;
            break;
        case 1:
            Color = T565_RED;
            break;
        case 2:
            Color = T565_GREEN;
            break;
        case 3:
            Color = T565_YELLOW;
            break;
        case 4:
            Color = T565_BLUE;
            break;
        case 5:
            Color = T565_MAGENTA;
            break;
        case 6:
            Color = T565_CYAN;
            break;
        case 7:
            Color = T565_WHITE;
            break;
        default:
            break;
        }
        return Color;
    }
    void Terminal::cmdParser(char c)
    {
        auto cancel = [this]()
        {
            cmdlock = 0;
            write(cmdtemp);
            strcpy(cmdtemp, "");
        };

        auto strapp = [](char *str, char c)
        {
            size_t length = strlen(str);
            str[length] = c;
            str[length + 1] = '\0';
        };

        if (!cmdlock)
            return;
        strapp(cmdtemp, c);

        uint8_t len = strlen(cmdtemp);

        if (len >= _cmdTempBufferLen)
        {
            cancel();
            return;
        }

        if (c == '[')
        {
            if (cmdlock != 1 || len != 1)
                cancel();
            else
                cmdlock = 2;
            return;
        }

        if (c == '?')
        {
            if (cmdlock != 2 || len != 2)
                cancel();
            return;
        }

        char end[] = {'m', 'A', 'B', 'C', 'D', 'E', 'G', 'n', 's', 'u', 'l', 'h', 'X', 'K', 'J', 'M', 'P', '@', 'L', 'S', 'T'};

        for (char ch : end)
        {
            if (c == ch)
            {
                cmdlock = 0;
                char Temp[len + 2];
                Temp[0] = '\033';
                strcpy(Temp + 1, cmdtemp);
                if (cmdParser(Temp))
                    strcpy(cmdtemp, "");
                else
                    cancel();
                return;
            }
        }

        if (!(isdigit(c) || c == ';') || cmdlock != 2)
        {
            cancel();
            return;
        }
    }

    size_t Terminal::write(uint8_t c)
    {
        if (cmdlock)
        {
            cmdParser(c);
            return 0;
        }
        if (c == '\033')
        {
            cmdlock = 1;
            return 0;
        }

        font f;
        f.c = char(c);
        f.color = this->color;
        f.bgcolor = this->bgcolor;
        replace(cursor, f);
        cursorPush(true);
        return 1;
    }
    Terminal::Terminal(uint8_t width, size_t LogLen, const char *Log)
    {
        this->width = width;

        size_t strLen = strlen(Log);
        strLen++; // 留出空格给光标

        size_t size = LogLen > strLen ? LogLen : strLen;
        log = new font[size + 1];
        stand = log;
        this->size = log + size;

        strLen--;
        for (size_t i = 0; i < strLen; i++)
        {
            log[i].c = Log[i];
        }
        end = log + strLen;
        endPush();

        focus = log;
        cursor = end - 1;
        printLog = NULL;
        window = NULL;
    };
    Terminal::~Terminal()
    {
        delete log;
        if (printLog)
            delete printLog;
        if (window)
            delete window;
    }
    uint8_t Terminal::Width()
    {
        return width;
    }
    size_t Terminal::Length()
    {
        // font *i = log;
        // while (i->c)
        // {
        //     i++;
        // }
        return end - log;
    }
    size_t Terminal::Height(font *stand)
    {
        size_t h = 1;
        stand = stand == NULL ? log : stand;
        while (stand)
        {
            stand = lineDown(stand);
            h++;
        }

        return h;
    }
    bool Terminal::resize(uint8_t width)
    {
        this->width = width;
        return true;
    }
    bool Terminal::cmdParser(const char *str)
    {
        if (str[0] != '\033' || str[1] != '[')
            return false;
        uint8_t len = strlen(str);
        if (len < 3)
            return false;

        const char *begin = str + 2;
        char end = *(str + len - 1);
        len = len - 3;
        if (*(begin + 1) == '?')
        {
            begin++;
            len--;
        }

        int data[4] = {0};
        auto parser = [&data](const char *cmd, uint8_t width) -> uint8_t
        {
            uint8_t i = 0;
            char *str = new char[width + 1];
            strncpy(str, cmd, width);
            str[width] = '\0';
            for (char *ret = strtok(str, ";"); ret != NULL; ret = strtok(NULL, ";"))
            {
                data[i] = atoi(ret);
                i++;
            }
            return i;
            delete str;
        };
        uint8_t num = parser(begin, len);

        if (*(begin + 1) == '?')
        {
            if (num == 1 && data[0] == 25)
            {
                switch (end)
                {
                case 'l':
                    return cursorHide(true);
                    break;
                case 'h':
                    return cursorHide(false);
                    break;
                default:
                    return false;
                    break;
                }
            }
            else
                return false;
        }

        switch (end)
        {
        case 'H':
            if (num != 2)
                return false;
            return cursorPos(data[1], data[0], true);
        case 'A':
            if (num != 1)
                return false;
            return cursorDirect(0, data[0], true);
        case 'B':
            if (num != 1)
                return false;
            return cursorDirect(1, data[0], true);
        case 'C':
            if (num != 1)
                return false;
            return cursorDirect(2, data[0], true);
        case 'D':
            if (num != 1)
                return false;
            return cursorDirect(3, data[0], true);
        case 'E':
            if (num != 1)
                return false;
            return cursorDirectHead(0, data[0], true);
        case 'F':
            if (num != 1)
                return false;
            return cursorDirectHead(1, data[0], true);
        case 'G':
            if (num != 1)
                return false;
            return cursorRow(data[0], true);
        case 'n': // TODO
            if (num != 1 || data[0] != 6)
                return false;
            break;
        case 's':
            if (num != 1 || data[0] != 0)
                return false;
            return cursorSave();
        case 'u':
            if (num != 1 || data[0] != 0)
                return false;
            return cursorUse();
        case 'X':
            if (num != 1)
                return false;
            return clearDirect(0, data[0]);
        case 'K':
            if (num != 1)
                return false;
            switch (data[0])
            {
            case 0:
                return clearDirect(0, -1);
            case 1:
                return clearDirect(1, -1);
            case 2:
                return clearDirect(2, -1);
            default:
                return false;
            }
            return cursorDirectHead(1, data[0]);
        case 'J':
            if (num != 1)
                return false;
            switch (data[0])
            {
            case 0:
                return clearScreen(0);
            case 1:
                return clearScreen(1);
            case 2:
                return clearScreen(2);
            default:
                return false;
            }
        case 'M':
            if (num != 1)
                return false;
            return removeDirect(0, data[0]);
        case 'P':
            if (num != 1)
                return false;
            return removeDirect(1, data[0]);
        case '@': // TODO
            break;
        case 'L': // TODO
            break;
        case 'S':
            if (num != 1)
                return false;
            return focusUp(data[0]);
        case 'T':
            if (num != 1)
                return false;
            return focusDown(data[0]);
        case 'm':
            if (num != 1)
                return false;
            if (data[0] == 0)
            {
                return charStyle(T_default_fontColor, T_default_backgroundColor);
            }
            if (30 <= data[0] && data[0] <= 37)
                return color = colorCode(data[0] - 30);

            if (40 <= data[0] && data[0] <= 47)
                return bgcolor = colorCode(data[0] - 40);
            return false;
        default:
            break;
        }

        return false;
    }
    const char *Terminal::getLog()
    {
        if (printLog)
        {
            delete printLog;
            printLog = NULL;
        }
        size_t l = Length();
        printLog = new char[l + 1];
        printLog[l] = '\0';
        for (size_t i = 0; i < l; i++)
        {
            printLog[i] = log[i].c;
        }
        return printLog;
    }
    const Terminal::font *Terminal::getWindow(const font *&head)
    {
        if (window)
        {
            delete window;
            window = NULL;
        }

        if (head == NULL)
            head = focus;

        if (head == end)
            return NULL;
        const font *next = lineDown(head);
        if (!next)
            next = end;

        window = new font[width];
        uint8_t i = 0;
        while (head < next)
        {
            if (head->c == '\0')
                head->c == '?'; //! debug

            *(window + i) = *head;
            if (head == cursor && !cursor_Hide)
            {
                (window + i)->color = head->bgcolor;
                (window + i)->bgcolor = head->color;
            }
            head++;
            i++;
        }
        while (i < width)
        {
            (window + i)->c = '\0'; //!
            i++;
        }

        return window;
    }

    bool Terminal::cursorPos(uint8_t row, uint8_t column, bool force)
    {
        font *t = getPos(row, column, log, force);
        if (!t)
            return false;
        cursor = t;
        if (end <= cursor)
            end = cursor + 1;
        return true;
    }
    bool Terminal::cursorDirect(uint8_t direct, uint8_t num, bool force)
    {
        font *head = stand;
        uint8_t column = getColumn(cursor, head);
        uint8_t row = getRow(cursor, head);
        if (!column || !row)
            return false;

        /**Direction:
         * 0:UP
         * 1:DOWN
         * 2:RIGHT
         * 3:LEFT
         */
        switch (direct)
        {
        case 0:
            column = column - num > 1 ? column - num : 1;
            break;
        case 1:
            column = column + num;
            break;
        case 2:
            row = row + num < width ? row + num : width;
            break;
        case 3: // force not work
            row = row - num > 1 ? row - num : 1;
            break;
        default:
            return false;
            break;
        }

        return cursorPos(row, column, force);
    }
    bool Terminal::cursorDirectHead(uint8_t direct, uint8_t num, bool force)
    {
        font *head = stand;
        uint8_t column = getColumn(cursor, head);
        if (!column)
            return false;

        /**Direction:
         * 0:DOWN
         * 1:UP
         */
        switch (direct)
        {
        case 0:
            column += num;
            break;
        case 1: // force not work
            column = column - num > 1 ? column - num : 1;
            break;
        default:
            return false;
            break;
        }

        return cursorPos(1, column, force);
    }
    bool Terminal::cursorRow(uint8_t row, bool force)
    {
        font *head = stand;
        uint8_t column = getColumn(cursor, head);
        return cursorPos(row, column, force);
    }
    bool Terminal::cursorGet(uint8_t &row, uint8_t &column)
    {
        font *head = stand;
        column = getColumn(cursor, head);
        row = getRow(cursor, head);
        if (!column || !row)
            return false;
        return true;
    }
    bool Terminal::cursorSave()
    {
        font *head = log;
        uint8_t column = getColumn(cursor, head);
        uint8_t row = getRow(cursor, head);
        if (!column || !row)
            return false;

        cursor_Save_Column = column;
        cursor_Save_Column = row;
        return true;
    }
    bool Terminal::cursorUse()
    {
        font *head = log;
        font *pos = getPos(cursor_Save_Row, cursor_Save_Column, head, true);
        if (!pos)
            return false;
        cursor = pos;
        return true;
    }
    bool Terminal::cursorHide(bool enable)
    {
        cursor_Hide = enable;
        return true;
    }

    bool Terminal::clearDirect(uint8_t direct, uint8_t num)
    {
        font *head = log;
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
            clearFormat(cursor + 1, num, head);
            break;

        case 1:
            clearFormatR(cursor, num, head);
            break;

        case 2:
            clearFormat(cursor + 1, num, head);
            clearFormatR(cursor, num, head);
            break;

        default:
            return false;
            break;
        }
        return true;
    }
    bool Terminal::clearScreen(uint8_t direct)
    {
        font *head = log;
        uint8_t column = getColumn(cursor, head);
        uint8_t row = getRow(cursor, head);
        if (!column || !row)
            return false;

        font *idx;
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
                font *begin = idx + row;
                clearFormat(begin, -1, idx);
                idx = lineDown(idx);
            }
            break;
        case 1:
            idx = focus;
            if (idx > head)
                idx = NULL;
            while (idx)
            {
                font *begin = idx + row - 1;
                clearFormatR(begin, -1, idx);
                idx = lineDown(idx);
                if (idx > head)
                    idx = NULL;
            }
            clearFormatR(cursor, -1, head);
            break;
        case 2:
            idx = focus;
            idx->c = ' ';
            // while (idx)
            // {
            //     font *begin = idx;
            //     clearFormat(begin, -1, idx);
            //     idx = lineDown(idx);
            // }
            remove(idx + 1, end);
            cursor = focus;
            break;

        default:
            return false;
            break;
        }
        return true;
    }
    bool Terminal::removeDirect(uint8_t direct, uint8_t num)
    {
        font *head = log;
        uint8_t column = getColumn(cursor, head);
        if (!column)
            return false;

        /**Direction:
         * 0:DOWN
         * 1:RIGHT*/
        switch (direct)
        {
        case 0:
        {
            font *begin = lineDown(head);
            font *End = begin;
            if (!begin)
                return true;

            while (num)
            {
                font *next = lineDown(End);
                End = next;
                num--;
            }
            if (End == NULL)
                End = this->end;
            remove(begin, End);
            break;
        }
        case 1:
            removeFormat(cursor, num, head);
        default:
            return false;
            break;
        }
        return true;
    }
    bool Terminal::Insert(char c, uint8_t num, bool force)
    {
        if (num < 1)
            return true;
        font f;
        f.c = c;
        f.color = color;
        f.bgcolor = bgcolor;
        num--;
        replace(cursor, f);
        while (num)
        {
            insert(cursor, f, force);
            num--;
        }
        return true;
    }
    bool Terminal::focusUp(uint8_t num)
    {
        focus = lineUp(focus, log, num);
        return true;
    }
    bool Terminal::focusDown(uint8_t num)
    {
        while (num)
        {
            font *next = lineDown(focus);
            if (next)
                focus = next;
            num--;
        }
        return true;
    }

    bool Terminal::charStyle(uint16_t color, uint16_t bgColor)
    {
        this->color = color;
        this->bgcolor = bgColor;
        return true;
    }
    bool Terminal::charColorReverse()
    {
        uint16_t c = color;
        color = bgcolor;
        bgcolor = c;
        return true;
    }
}
