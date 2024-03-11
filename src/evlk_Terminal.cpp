#include "evlk_Terminal.h"
#include <cctype>
#include <cstring>
#include <stdlib.h>

namespace _EVLK_TERMINAL_
{
    Terminal::fontI::fontI(bool des) : destory(des)
    {
        size = 0;
        style = NULL;
    };
    Terminal::fontI::~fontI()
    {
        if (style && destory)
            delete style;
    }
    Terminal::fontI &Terminal::fontI::operator=(const fontI &f)
    {
        this->size = f.size;
        this->style = f.style;
        return *this;
    }
    Terminal::fp Terminal::style(lp p, lp &h, fp &hs)
    {
        if (!log.isIn(h, p) || !styles.isIn(hs))
            return NULL;

        size_t n = p - log.begin() + 1;
        size_t l = h - log.begin();

        while (hs < styles.end())
        {
            size_t L = l + hs->size;
            if (L >= n)
            {
                h = log.begin() + l;
                return hs;
            }
            l = L;
            hs++;
        }
        return NULL;
    }
    Terminal::fp Terminal::style_q(lp p)
    {
        if (!log.isIn(style_serch_lp, p))
        {
            style_serch_lp = log.begin();
            style_serch_fp = styles.begin();
        }
        return style(p, style_serch_lp, style_serch_fp);
    }
    Terminal::fp Terminal::style_q(lp p, lp &h)
    {
        fp S = style_q(p);
        if (S)
            h = style_serch_lp;
        return S;
    }
    void Terminal::style_q_r()
    {
        if (style_serch_fp > styles.begin())
        {
            style_serch_fp--;
            style_serch_lp -= style_serch_fp->size;
        }
    }
    Terminal::fp Terminal::style_resize(fp p, size_t num)
    {
        fontI temp(false);
        temp.style = p->style;
        temp.size = num;
        return styles.replace(p, 1, temp);
    }
    Terminal::fp Terminal::style_new(fp p, size_t size, font &f)
    {
        fontI temp(false);
        if (&f)
            temp.style = styleFactory->createFont();
        *temp.style = f;
        temp.size = size;
        return styles.replace(p, 1, temp, true);
    }
    void Terminal::style_diff(lp b, size_t n, size_t &h, size_t &m, size_t &e, fp &se)
    {
        h = m = e = 0;
        se = NULL;
        if (!log.isIn(b, b + n))
            return;

        lp fh = NULL;
        fp sh = style_q(b, fh);

        if (b + n <= fh + sh->size)
        {
            h = n;
            e = (fh + sh->size) - (b + n);
            se = sh;
            return;
        }
        h = sh->size - (b - fh);
        n -= h;
        for (fp i = sh + 1; i < styles.end(); i++)
        {
            if (n <= i->size)
            {
                e = n;
                se = i;
                return;
            }
            n -= i->size;
            m += i->size;
        }
    }

    bool Terminal::style_check()
    {
        fp fb = styles.begin();
        fp const fe = styles.end();
        size_t l = 0;
        while (fb < fe)
        {
            if (!fb->style)
                return 0;
            if (fb + 1 != fe && *fb->style == *(fb + 1)->style)
                return 0;
            l += fb->size;
            fb++;
        }
        if (fe->size != 0 || fe->style)
            return 0;
        if (l != log.length())
            return 0;
        return 1;
    }

    bool Terminal::isIn(lp p)
    {
        return log.isIn(p);
    }
    bool Terminal::isIn(lp p, lp h)
    {
        return log.isIn(h, p) || (p - h) < width;
    }
    Terminal::lp Terminal::insert(lp p, size_t n, font &s, char f) //!
    {
        bool isEnd = (p == log.end()); // 插入干扰
        lp r = log.insert(p, n, f);
        if (!r)
            return NULL;

        if (styles.empty()) // 初始化样式表
        {
            fp b = styles.insert(styles.end(), 1); // 构造函数预留了至少一个
            style_new(b, n, s);
            return r;
        }

        lp h = NULL;
        fp S = isEnd ? styles.end() - 1 : style_q(p, h);
        if (!S)
            return NULL;

        auto style_insertError = [r, n, S, this]() -> lp
        {
            style_resize(S, S->size + n);
            return r;
        };

        if (*S->style == s) // 样式相同时
        {
            style_resize(S, S->size + n);
        }
        else
        {
            if (isEnd)
            {
                fp temp = styles.insert(styles.end(), 1);
                if (!temp)
                    return style_insertError();
                style_new(temp, n, s);
            }
            else
            {
                if (p == h) // 头
                {
                    if (!(p == log.begin()) && *(S - 1)->style == s) // 样式与上一个样式相同
                    {
                        style_q_r();
                        style_resize(S - 1, n + (S - 1)->size);
                    }
                    else
                    {
                        fp temp = styles.insert(S, 1);
                        if (!temp)
                            return style_insertError();
                        style_new(temp, n, s);
                    }
                }
                else // 中
                {
                    fp temp = styles.insert(S + 1, 2);
                    if (!temp)
                        return style_insertError();
                    size_t S_size = S->size;

                    style_resize(S, p - h);
                    style_new(temp, n, s);
                    style_new(++temp, S_size - (p - h), *(S->style));
                }
            }
        }
        return r;
    }
    Terminal::lp Terminal::insert_s(lp p, size_t n, font &s, char f)
    {
        lp S = insert(p, n, s, f);
        if (S)
        {
            stand = p < stand ? stand - n : stand;
            cursor = p < cursor ? cursor - n : cursor;
            focus = p < focus ? focus - n : focus;
        }
        return S;
    }
    Terminal::lp Terminal::remove(lp p, size_t n)
    {
        lp fh = NULL;
        fp sh = style_q(p, fh);
        if (!sh)
            return NULL;

        size_t dh, dm, de;
        fp se = NULL;
        style_diff(p, n, dh, dm, de, se);
        if (!se)
            return NULL;

        bool headOver = !(se == sh);             // 是否超过头
        size_t headRemain = !headOver ? de : 0;  // 未超过头时的剩余
        bool hasHead = !(p == fh);               // 是否在头的最前
        bool hasEnd = headOver && de < se->size; // 是否在尾的最后

        char f = '\0';
        lp r = log.remove(p, n, f);
        if (!r)
            return NULL;

        style_resize(sh, sh->size - dh);
        if (headRemain)
            return r;
        else
            style_resize(se, se->size - de);

        bool B = !hasHead && sh > styles.begin();
        bool E = !hasEnd && sh < styles.end() - 1;
        fp pB = B ? sh - 1 : sh;
        fp pE = E ? se + 1 : se;

        if (B)
            style_q_r();

        if (pB != pE && *pB->style == *pE->style)
        {
            style_resize(pB, pB->size + pE->size);
            pE++;
        }
        if (!styles.remove(pB + 1, pE - 1 - pB, true))
            return NULL;
        return r;
    }
    Terminal::lp Terminal::remove_s(lp p, size_t n)
    {
        lp S = remove(p, n);
        if (S)
        {
            stand = p < stand ? stand - n : stand;
            cursor = p < cursor ? cursor - n : cursor;
            focus = p < focus ? focus - n : focus;
        }
        return S;
    }
    Terminal::lp Terminal::remove_f(lp p, size_t n, lp head)
    {
        uint8_t row = getRow(p, head);
        if (!row)
            return NULL;

        lp e = lineEnd(head);
        uint8_t fillpos = width - rowFillNum(head);

        if (e == head && *e == '\n') //'\n'在行首
            return p;

        n = row + n - 1 > width ? width - row + 1 : n;
        if (p + n > head + fillpos)
        {
            n = head + fillpos - p + 1;
            if (*e == '\n')
                n--;
        }

        if (e == log.end() - 1 && *e != '\n') // 尾行
            n--;

        fillpos = fillpos - n + 1;

        lp S = remove_s(p, n);
        if (!S)
            return NULL;

        fp s = style_q(S);
        if (!s)
            return NULL;

        if (*e != '\n')
            if (!insert_s(head + fillpos, 1, *s->style, '\n'))
                return NULL;
        return p;
    }
    Terminal::lp Terminal::replace(lp p, size_t n, font &s, char f)
    {
        lp r = remove(p, n);
        if (r)
            r = insert(r, n, s, f);
        return r;
    }
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
    Terminal::lp Terminal::lineDown(lp head)
    {
        if (!isIn(head))
            return NULL;

        for (lp t = head; t < head + width; t++)
        {
            if (*t == '\n')
            {
                if (t + 1 >= log.end())
                    return NULL;
                return t + 1;
            }
        }
        if (head + width >= log.end())
            return NULL;
        return head + width;
    }
    Terminal::lp Terminal::lineUp(lp head, lp stand, size_t num)
    {
        if (!isIn(head) || !isIn(stand) || head < stand)
            return NULL;

        lp first = stand;
        lp idx = first;
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
    Terminal::lp Terminal::lineEnd(lp head)
    {
        if (!isIn(head))
            return NULL;

        head = lineDown(head);
        if (!head)
            return log.end() - 1;
        return head - 1;
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
    void Terminal::cmdParser(char c)
    {
        auto cancel = [this]()
        {
            cmdlock = 0;
            write((uint8_t *)cmdtemp, strlen(cmdtemp));
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
        return write(&c, 1);
    }
    size_t Terminal::write(const uint8_t *buffer, size_t size)
    {
        auto Write = [this](const uint8_t *buffer, size_t size) -> size_t
        {
            if (!size)
                return true;
            const char *Buffer = (const char *)buffer;
            bool S = true;

            size_t rs = size;
            if (cursor + rs > log.end())
            {
                rs = log.end() - cursor;
                S = insert_s(log.end(), size - rs + 1, *pencil, ' ');
            }
            if (S)
                S = replace(cursor, rs, *pencil, ' ');
            if (S)
            {
                for (size_t i = 0; i < size; i++)
                    log.replace(cursor + i, 1, *(Buffer + i));
                cursor += size;
            }
            return S ? size : 0;
        };

        const uint8_t *b = buffer;
        size_t n = 0;
        size_t N = 0;
        for (size_t i = 0; i < size; i++)
        {
            char c = *(buffer + i);
            if (cmdlock)
            {
                cmdParser(c);
                b++;
                continue;
            }
            if (c == '\033')
            {
                N += Write(b, n);
                b += n + 1;
                n = 0;
                cmdlock = 1;
                continue;
            }
            n++;
        }
        N += Write(b, n);
        return N;
    }
    Terminal::Terminal(uint8_t width, size_t Log_Len, fontFactory &factory, size_t Style_Len)
        : log(Log_Len > 1 ? Log_Len : 1, '\0'),
          styles(Style_Len > 1 ? Style_Len : 1),
          width(width),
          styleFactory(&factory)
    {
        stand = focus = cursor = log.begin();
        pencil = styleFactory->createFont(); // 初始化样式画笔
        pencil->init();
        insert(log.begin(), 1, *pencil, ' ');
    };
    Terminal::~Terminal()
    {
        delete pencil;
    }
    uint8_t Terminal::Width()
    {
        return width;
    }
    size_t Terminal::Length()
    {
        return log.length();
    }
    size_t Terminal::Height(lp stand)
    {
        size_t h = 1;
        stand = stand == NULL ? log.begin() : stand;
        while (stand)
        {
            stand = DOWN(stand);
            h++;
        }
        return h;
    }
    bool Terminal::resize(uint8_t width)
    {
        this->width = width;
        return true;
    }
    bool Terminal::cmdParser(lp str)
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
            if (num < 1)
                return false;

            {
                _EVLK_TERMINAL_::font *f = this->styleFactory->createFont();
                *f = *pencil;
                for (size_t i = 0; i < num; i++)
                {
                    if (0 <= data[i] && data[i] <= 8)
                    {
                        switch (data[i])
                        {
                        case 0:
                            f->sgr0();
                            break;
                        case 1:
                            f->bold();
                            break;
                        case 2:
                            f->dim();
                            break;
                        case 3:
                            f->smso();
                            break;
                        case 4:
                            f->smul();
                            break;
                        case 5:
                            f->blink();
                            break;
                        case 7:
                            f->rev();
                            break;
                        case 8:
                            f->invis();
                            break;
                        default:
                            break;
                        }
                    }

                    if (30 <= data[0] && data[0] <= 37)
                        f->color_8(data[0] - 30);

                    if (40 <= data[0] && data[0] <= 47)
                        f->bgColor_8(data[0] - 40);
                }
                bool c = charStyle(*f);
                delete f;
                return c;
            }
        default:
            break;
        }
        return false;
    }

    bool Terminal::cursorPos(uint8_t row, uint8_t column, bool force) //!
    {
        lp t = getPos(row, column, log.begin(), force);
        if (!t)
            return false;
        cursor = t;
        //! if (end <= cursor)
        //     end = cursor + 1;
        return true;
    }
    bool Terminal::cursorDirect(uint8_t direct, uint8_t num, bool force)
    {
        lp head = stand;
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
        lp head = stand;
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
        lp head = stand;
        uint8_t column = getColumn(cursor, head);
        return cursorPos(row, column, force);
    }
    bool Terminal::cursorGet(uint8_t &row, uint8_t &column)
    {
        lp head = stand;
        column = getColumn(cursor, head);
        row = getRow(cursor, head);
        if (!column || !row)
            return false;
        return true;
    }
    bool Terminal::cursorSave()
    {
        lp head = log.begin();
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
        lp head = log.begin();
        lp pos = getPos(cursor_Save_Row, cursor_Save_Column, head, true);
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
    bool Terminal::removeDirect(uint8_t direct, uint8_t num)
    {
        lp head = log.begin();
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
            lp begin = lineDown(head);
            lp End = begin;
            if (!begin)
                return true;

            while (num)
            {
                lp next = lineDown(End);
                End = next;
                num--;
            }
            if (!End)
                End = log.end();
            remove(begin, End - begin);
            break;
        }
        case 1:
            remove_f(cursor, num, head);
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

    bool Terminal::charStyle(_EVLK_TERMINAL_::font &pencil)
    {
        if (!&pencil)
            return false;
        *this->pencil = pencil;
        return true;
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
