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
    Terminal::Terminal(size_t width, size_t height, size_t Log_Len, fontFactory &factory, size_t Style_Len)
        : log(Log_Len > 1 ? Log_Len : 1, '\0'),
          styles(Style_Len > 1 ? Style_Len : 1),
          editor(height > 1 ? height + 1 : 2),
          width(width > 1 ? width : 1), height(height > 1 ? height : 1),
          styleFactory(&factory)
    {
        focus = cursor = log.begin();
        style_editor_lp = log.begin();
        style_editor_fp = styles.begin();
        blank = styleFactory->createFont();
        blank->init();
        _insert(log.begin(), 1, *blank, '\n');
        pencil = styleFactory->createFont(); // 初始化样式画笔
        pencil->sgr0();
        editor.insert(editor.end(), 2);
        editor.replace(editor.begin(), 1, log.begin());
        editor.replace(editor.begin() + 1, 1, log.end());
    };
    Terminal::~Terminal()
    {
        delete pencil;
        delete blank;
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
            style_serch_lp = p < style_editor_lp ? log.begin() : style_editor_lp;
            style_serch_fp = p < style_editor_lp ? styles.begin() : style_editor_fp;
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
    void Terminal::style_diff(lp b, size_t n, size_t &h, size_t &m, size_t &e, fp &se)
    {
        h = m = e = 0;
        se = NULL;
        if (!log.isIn(b, b + n - 1))
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
    bool Terminal::isIn(size_t head_i)
    {
        return head_i + 1 < editor.length();
    }
    Terminal::lp Terminal::_insert(lp p, size_t n, font &s, char f) //!
    {
        if (!n)
            return p;

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
            style_resize(S, S->size + n);
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
    Terminal::lp Terminal::_remove(lp p, size_t n)
    {
        if (!n)
            return p;
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
        if (pB != pE && !styles.remove(pB + 1, pE - 1 - pB, true))
            return NULL;
        return r;
    }
    Terminal::lp Terminal::_replace(lp p, size_t n, font &s, char f)
    {
        lp r = _remove(p, n);
        if (r)
            r = _insert(r, n, s, f);
        return r;
    }
    Terminal::lp Terminal::_head(lp ep, size_t &index)
    {
        index = 0;
        if (ep < editor[0] || ep >= *(editor.end() - 1))
            return NULL;
        size_t len = editor.length() - 1;
        while (index < len)
        {
            if (ep < editor[index + 1])
                return editor[index];
            index++;
        }
        return NULL;
    }
    Terminal::lp Terminal::_end(size_t head_i)
    {
        if (!isIn(head_i))
            return NULL;
        return editor[head_i + 1] - 1;
    }
    Terminal::lp Terminal::_end(lp head)
    {
        if (!isIn(head))
            return NULL;

        size_t qi;
        if (this->_head(head, qi))
            return _end(qi);

        for (lp t = head; t < head + width; t++)
            if (*t == '\n')
                return t;
        if (head + width > log.end())
            return NULL;
        return head + width - 1;
    }
    Terminal::lp Terminal::_down(size_t &head_i, size_t &n)
    {
        if (!isIn(head_i))
        {
            head_i = editor.length();
            n = 0;
            return NULL;
        }
        size_t l = editor.length() - 1;
        if (head_i + 1 + n >= l)
            n = l - head_i - 1;
        head_i += n;
        return editor[head_i];
    }
    Terminal::lp Terminal::_down(lp head, size_t &n)
    {
        if (!isIn(head))
        {
            n = 0;
            return NULL;
        }

        size_t N = n;
        size_t qi;
        lp h = head;
        if (this->_head(head, qi))
            h = _down(qi, n);
        else
            n = 0;
        while (n < N)
        {
            lp second = _end(h) + 1;
            if (second == log.end())
                break;
            h = second;
            n++;
        }
        return h;
    }
    Terminal::lp Terminal::_up(size_t &head_i, size_t &n)
    {
        if (!isIn(head_i))
        {
            head_i = editor.length();
            n = 0;
            return NULL;
        }
        if (head_i < n)
            n = head_i;
        head_i -= n;
        return editor[head_i];
    }
    Terminal::lp Terminal::_up(lp head, size_t &n)
    {
        if (!isIn(head))
        {
            n = 0;
            return NULL;
        }
        if (!n)
            return head;

        auto UP = [&n, this](lp head, lp stand) -> lp
        {
            if (head < stand)
                return NULL;

            lp first, idx;
            first = idx = stand;
            while (first < head)
            {
                size_t t = 1;
                first = _down(first, t);
                if (n)
                    n--;
                else
                    idx = _down(idx, t);
            }
            return idx;
        };

        lp h = UP(head, editor[0]);
        if (!n)
            head = h;
        else
            head = UP(head, log.begin());
        return head;
    }
    Terminal::lp Terminal::_fillCol(size_t head_i, size_t Col, bool force)
    {
        if (!Col || Col > width || !isIn(head_i))
            return NULL;
        auto editor_off_r = [this, &head_i](size_t &n)
        {
            head_i++;
            size_t l = editor.length() - 1;
            while (head_i <= l)
            {
                lp p = editor[head_i] + n;
                editor.replace(editor.begin() + head_i, 1, p);
                head_i++;
            }
        };
        lp head = editor[head_i];
        lp end = this->_end(head_i);
        size_t col = end - head + 1;
        if (Col > col)
        {
            if (!force)
                return NULL;
            size_t n = Col - col;
            if (!_insert(end + 1, n, *blank, ' '))
                return NULL;
            char t = ' ';
            log.replace(end, 1, t);
            t = '\n';
            log.replace(end + n, 1, t);
            editor_off_r(n);
        }
        return head + Col - 1;
    }
    Terminal::lp Terminal::_fillRow(size_t Row, bool force)
    {
        if (!Row || Row > height)
            return NULL;
        size_t H = editor.length() - 1;
        if (Row > H)
        {
            if (!force)
                return NULL;
            size_t n = Row - H;
            lp last = _end(H - 1);
            if (!_insert(last + 1, n, *blank, '\n'))
                return NULL;
            editor.insert(editor.end(), n);
            for (size_t i = 0; i < n; i++)
                editor.replace(editor.begin() + H + i + 1, 1, (last + i + 2));
        }
        return editor[Row - 1];
    }
    Terminal::lp Terminal::_delCol(size_t head_i, size_t Col, size_t n)
    {
        auto editor_off_l = [this, head_i, Col](size_t n)
        {
            size_t i = head_i + 1; // 当行行头不移动
            size_t l = editor.length() - 1;
            while (i <= l)
            {
                lp p = editor[i] - n;
                editor.replace(editor.begin() + i, 1, p);
                i++;
            }
        };
        if (!Col || Col > width || !isIn(head_i))
            return NULL;
        lp head = editor[head_i];
        lp end = this->_end(head_i);
        size_t N = end - head + 1;
        if (!n || Col > N)
            return head + Col - 1;
        n = Col + n > N + 1 ? N + 1 - Col : n;
        if (head + Col - 1 + n >= log.end()) // 保留至少一个字符
            if (!n--)
                return head + Col - 1;
        bool broken = !(N == width && *end != '\n');
        bool whole = Col == 1 && n == N;
        lp r = _remove(head + Col - 1, n);
        if (!r)
            return NULL;
        if (!broken && !whole)
        {
            end -= n;
            _insert(end + 1, 1, *blank, '\n');
            n--;
        }

        lp p = editor[head_i] + Col - 1;
        if (cursor > p + n)
            cursor -= n;
        else if (cursor > p)
            cursor = p;
        editor_off_l(n);
        if (whole)
            editor.remove(editor.begin() + head_i, 1);
        return r;
    }
    Terminal::lp Terminal::_setPos(size_t row, size_t col, bool force)
    {
        lp p = _fillCol(_fillRow(row, force) ? row - 1 : height, col, force);
        if (p)
            cursor = p;
        return p;
    }
    Terminal::lp Terminal::_getPos(size_t &row, size_t &col)
    {
        lp p = cursor;
        row = col = 0;
        lp h = _head(p, row);
        if (!h)
            return NULL;
        row++;
        col = p - h + 1;
        return p;
    }
    Terminal::lp Terminal::_formPos(size_t row, size_t col)
    {
        return _fillCol(_fillRow(row, false) ? row - 1 : height, col, false);
    }
    size_t Terminal::write_f(const uint8_t *buffer, size_t size)
    {
        auto newLine = [this](size_t &row) -> lp
        {
            size_t l = editor.length() - 1;
            if (row < l)
                row++;
            else if (row > height)
                return NULL;
            else
            {
                if (row == height)
                {
                    size_t N = 1;
                    editorDown(N, true);
                    if (!N)
                        return NULL;
                    row--;
                }
                row++;
                if (!_fillRow(row))
                    return NULL;
            }
            return editor[row - 1];
        };
        auto fillLine = [this](size_t row, size_t col, const char *str, size_t n) -> lp
        {
            // col + n <= width + 1;
            size_t N = n;
            if (col + n - 1 < width)
                N++;
            if (!_fillCol(row - 1, col + N - 1))
                return NULL;
            lp p = editor[row - 1] + col - 1;
            for (size_t i = 0; i < n; i++)
                // log.replace(head + i, 1, *(str + i));
                _replace(p + i, 1, *pencil, *(str + i));
            return n ? p + n - 1 : p;
            // return p;
        };
        if (!size)
            return 0;
        lp p = cursor;
        size_t row, col, outsize = 0;
        if (!_getPos(row, col))
            return 0;
        for (const uint8_t *bufferEnd = buffer + size - 1, *i = buffer;
             buffer <= bufferEnd;
             i++)
        {
            auto fill = [&i, &buffer, row, &col, fillLine, this, &outsize]() -> bool
            {
                size_t n = i - buffer + 1;
                lp r = fillLine(row, col, (char *)buffer, n);
                if (!r)
                    return false;
                // cursor = r + n;
                cursor = r + 1;
                col += n;
                buffer += n;
                outsize += n;
                return true;
            };
            if (*i == '\n') // 换行符换行
            {
                if (i - buffer)
                {
                    i--;
                    if (!fill())
                        return outsize;
                    i++;
                }
                if (!(newLine(row)))
                    return outsize;
                cursor = editor[row - 1];
                col = 1;
                buffer++;
                outsize++;
            }
            else if (col + (i - buffer + 1) > width) // 一般换行
            {
                if (!(fill() && newLine(row)))
                    return outsize;

                cursor = editor[row - 1];
                col = 1;
            }
            else if (i >= bufferEnd)
                if (!fill())
                    return outsize;
        }
        focus = editor[0]; // 重定向
        return outsize;
    }
    Terminal::lp Terminal::editorDown(size_t &n, bool force)
    {
        for (size_t i = 0; i < n; i++)
        {
            size_t N = 1;
            lp down = this->_down(editor[editor.length() - 1], N);
            if (N || (force && editor.length() >= 2))
            {
                for (size_t i = 0; i < editor.length() - 1; i++)
                    editor.replace(editor.begin() + i, 1, *(editor.begin() + i + 1));
                editor.replace(editor.end() - 1, 1, down);
                if (!N && force)
                    editor.remove(editor.end() - 1, 1);
            }
            else
            {
                n = i;
                break;
            }
        }
        lp e_lp;
        fp e_fp = style_q(editor[0], e_lp);
        if (!e_fp)
            return NULL;
        style_editor_fp = e_fp;
        style_editor_lp = e_lp;

        return editor[0];
    }
    Terminal::lp Terminal::editorUp(size_t &n, bool force)
    {
        for (size_t i = 0; i < n; i++)
        {
            size_t N = 1;
            lp up = this->_up(editor[0], N);
            if (N || (force && editor.length() >= 2))
            {
                for (size_t i = 0; i < editor.length() - 1; i++)
                    editor.replace(editor.begin() + i + 1, 1, *(editor.begin() + i));
                editor.replace(editor.begin(), 1, up);
                if (!N && force)
                    editor.remove(editor.begin(), 1);
            }
            else
            {
                n = i;
                break;
            }
        }
        lp e_lp;
        fp e_fp = style_q(editor[0], e_lp);
        if (!e_fp)
            return NULL;
        style_editor_fp = e_fp;
        style_editor_lp = e_lp;

        return editor[0];
    }
    void Terminal::VT100(char c)
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
            if (cmdlock != 1 || len != 2)
                cancel();
            else
                cmdlock = 2;
            return;
        }

        if (c == '?')
        {
            if (cmdlock != 2 || len != 3)
                cancel();
            return;
        }

        char end[] = {'m', 'A', 'B', 'C', 'D', 'E', 'G', 'n', 's', 'u', 'l', 'h', 'X', 'K', 'J', 'M', 'P', '@', 'L', 'S', 'T'};

        for (char ch : end)
        {
            if (c == ch)
            {
                cmdlock = 0;
                if (VT100(cmdtemp, len))
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

    size_t Terminal::write(const uint8_t *buffer, size_t size)
    {
        const uint8_t *b = buffer;
        size_t n = 0;
        size_t N = 0;
        for (size_t i = 0; i < size; i++)
        {
            char c = *(buffer + i);
            if (cmdlock)
            {
                VT100(c);
                b++;
                continue;
            }
            if (c == '\033')
            {
                N += write_f(b, n);
                b += n + 1;
                n = 0;
                strcpy(cmdtemp, "\033");
                cmdlock = 1;
                continue;
            }
            n++;
        }
        N += write_f(b, n);
        return N;
    }
    size_t Terminal::write(uint8_t c)
    {
        return write(&c, 1);
    }
    uint8_t Terminal::Width()
    {
        return width;
    }
    size_t Terminal::Length()
    {
        return log.length();
    }
    size_t Terminal::Height()
    {
        size_t h = 1;
        lp stand = log.begin();
        while (stand)
        {
            stand = down(stand);
            h++;
        }
        return h;
    }
    bool Terminal::VT100(const char *str) { return VT100(str, strlen(str)); }
    bool Terminal::VT100(const char *str, size_t len)
    {
        if (str[0] != '\033' || str[1] != '[')
            return false;
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

        int data[8] = {0};
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
                    return hide(true);
                    break;
                case 'h':
                    return hide(false);
                    break;
                default:
                    return false;
                    break;
                }
            }
            else
                return false;
        }

#define VT100_param_limit(n) \
    if (num != n)            \
        return false;

        switch (end)
        {
        case 'H':
            VT100_param_limit(2);
            return cup(data[1], data[0], true);
        case 'A':
            VT100_param_limit(1);
            return move(0, data[0], true);
        case 'B':
            VT100_param_limit(1);
            return move(1, data[0], true);
        case 'C':
            VT100_param_limit(1);
            return move(2, data[0], true);
        case 'D':
            VT100_param_limit(1);
            return move(3, data[0], true);
        case 'E':
            VT100_param_limit(1);
            return move(4, data[0], true);
        case 'F':
            VT100_param_limit(1);
            return move(5, data[0], true);
        case 'G':
            VT100_param_limit(1);
            return move(6, data[0], true);
        case 'n': // TODO
            break;
        case 's': // TODO
            break;
        case 'u': // TODO
            break;
        case 'X':
            VT100_param_limit(1);
            return clear(0, data[0]);
        case 'K':
            VT100_param_limit(1);
            switch (data[0])
            {
            case 0:
                return clear(1);
            case 1:
                return clear(2);
            case 2:
                return clear(3);
            default:
                return false;
            }
        case 'J':
            VT100_param_limit(1);
            switch (data[0])
            {
            case 0:
                return clear(4);
            case 1:
                return clear(5);
            case 2:
                return clear(6);
            default:
                return false;
            }
        case 'M':
            VT100_param_limit(1);
            return remove(0, data[0]);
        case 'P':
            VT100_param_limit(1);
            return remove(1, data[0]);
        case '@': // TODO
            break;
        case 'L': // TODO
            break;
        case 'S':
            VT100_param_limit(1);
            return Focus(0, data[0]);
        case 'T':
            VT100_param_limit(1);
            return Focus(1, data[0]);
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
                bool c = Pencil(*f);
                delete f;
                return c;
            }
        default:
            break;
        }
        return false;
    }

    bool Terminal::Pos(size_t &row, size_t &col) { return _getPos(row, col); }
    bool Terminal::cup(size_t row, size_t column, bool force) { return _setPos(row, column, force); }
    bool Terminal::move(uint8_t direct, uint8_t num, bool force)
    {
        size_t row, col;
        if (!Pos(row, col))
            return false;

        /**Direction:
         * 0:UP
         * 1:DOWN
         * 2:RIGHT
         * 3:LEFT
         * 4:DOWN_HEAD
         * 5:UP_HAED
         * 6:COL
         */
        switch (direct)
        {
        case 0:
            row = row > num + 1 ? row - num : 1;
            break;
        case 1:
            row = row + num > height ? height : row + num;
            break;
        case 2:
            col = col + num > width ? width : col + num;
            break;
        case 3: // force not work
            col = col > num + 1 ? col - num : 1;
            break;
        case 4:
            row = row + num > height ? height : row + num;
            col = 1;
            break;
        case 5:
            row = row > num + 1 ? row - num : 1;
            col = 1;
        case 6:
            col = num && num <= width ? num : col;
        default:
            return false;
            break;
        }
        return cup(row, col, force);
    }
    bool Terminal::hide(bool enable)
    {
        cursor_Hide = enable;
        return true;
    }
    bool Terminal::clear(uint8_t direct, uint8_t num)
    {
        size_t row, col, head_i;
        if (!Pos(row, col))
            return false;

        lp head = _head(cursor, head_i);
        size_t l = editor.length() - 1;

        auto Clear = [this, &head_i](size_t col, size_t num) -> bool
        {
            lp end = _end(head_i);
            if (*end == '\n')
                end--;
            size_t End = end + 1 - editor[head_i];
            if (col > End)
                return true;
            num = col + num > End + 1 ? End - col + 1 : num;
            // if (col + num > End + 1)
            //     return _delCol(head_i, col, num);
            return _replace(editor[head_i] + col - 1, num, *blank, ' ');
        };

        /**Direction:0:向右 - 1:向右 - 2:向左 - 3:整行 - 4:右下 - 5:左上 - 6:整屏(光标移至左上角)
         * 0:RIGHT
         * 1:RIGHT
         * 2:LEFT
         * 3:LINE
         * 4:RIGHT_DOWN
         * 5:LEFT_UP
         * 6:ALL*/
        switch (direct)
        {
        case 0:
            return Clear(col, num);

        case 1:
            return Clear(col, width);

        case 2:
            return Clear(1, col);

        case 3:
            return Clear(1, width);

        case 4:
            while (head_i < l)
            {
                Clear(col, width);
                head_i++;
            }
            break;
        case 5:
            while (head_i > 0)
            {
                Clear(1, col);
                head_i--;
            }
            Clear(1, col);
            break;
        case 6:
            head_i = 0;
            while (head_i < l)
            {
                Clear(1, width);
                head_i++;
            }
            cursor = editor[0];
            break;
        default:
            return false;
        }
        return true;
    }
    bool Terminal::remove(uint8_t direct, uint8_t num)
    {
        size_t row, col, head_i;
        if (!Pos(row, col))
            return false;

        lp head = _head(cursor, head_i);
        size_t l = editor.length() - 1;

        /**Direction:
         * 0:DOWN
         * 1:RIGHT*/
        switch (direct)
        {
        case 0:
        {
            size_t i = head_i;
            while (num && i < l)
            {
                _delCol(head_i, 1, width);
                num--;
                i++;
            }
            break;
        }
        case 1:
            _delCol(head_i, col, num);
            break;
        default:
            return false;
        }
        return true;
    }
    bool Terminal::Pencil(_EVLK_TERMINAL_::font &pencil)
    {
        if (!&pencil)
            return false;
        *this->pencil = pencil;
        return true;
    }
    _EVLK_TERMINAL_::font &Terminal::Pencil() { return *pencil; }
    bool Terminal::Focus(bool direct, size_t num) { return direct ? (focus = _down(focus, num)) : (focus = _up(focus, num)); }
    Terminal::lp Terminal::Focus() { return focus; }
    Terminal::lp Terminal::Cursor() { return cursor; }
    Terminal::lp Terminal::Begin() { return log.begin(); }
    Terminal::lp Terminal::End() { return log.end(); }
    Terminal::lp Terminal::up(lp h, size_t n)
    {
        lp d = _up(h, n);
        return !n ? NULL : d;
    }
    Terminal::lp Terminal::down(lp h, size_t n)
    {
        lp d = _down(h, n);
        return !n ? NULL : d;
    }
    Terminal::lp Terminal::end(lp h) { return _end(h); }
    const font *Terminal::style(lp p)
    {
        fp S = style_q(p);
        if (!S)
            return NULL;
        return S->style;
    }

}
