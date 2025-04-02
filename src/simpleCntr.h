#ifndef _SIMPLECNTR_
#define _SIMPLECNTR_
#include "string.h"

template <class T>
class simpleCntr
{
private:
    T *Begin;
    T *Size;
    T *End;

public:
    size_t length()
    {
        return End - Begin;
    }
    bool empty()
    {
        return Begin == End;
    }
    size_t avilable()
    {
        return Size - End;
    }
    bool isIn(const T *pos)
    {
        return Begin <= pos && pos < End;
    };
    bool isIn(const T *first, const T *secend)
    {

        return Begin <= first && first <= secend && secend < End;
    }
    bool resize(size_t num)
    {
        size_t s = Size - Begin;
        size_t e = End - Begin > num ? num : End - Begin;
        if (num == s)
            return true;
        T *New = new T[s + 1];
        memcpy(New, Begin, (num > s ? s : num) * sizeof(T));
        memset(Begin, 0, sizeof(T) * (s + 1));
        delete[] Begin;
        Begin = New;
        Size = Begin + num;
        End = Begin + e;
        return true;
    };
    bool resize(size_t num, const T &fill)
    {
        bool s = resize(num);
        if (!s)
            return s;
        for (T *i = End; i <= Size; i++)
            *i = fill;
        return true;
    }
    const T *insert(const T *pos, size_t num, const T &fill)
    {
        const T *p = insert(pos, num);
        if (!p)
            return p;
        T *Pos = const_cast<T *>(pos);
        for (T *i = Pos; i <= pos + num - 1; i++)
            *i = fill;
        return p;
    };
    const T *insert(const T *pos, size_t num)
    {
        if (!num)
            return pos;
        if (Begin > pos || pos > End || End + num > Size)
            return nullptr;
        T *Pos = const_cast<T *>(pos);
        memmove(Pos + num, pos, sizeof(T) * (End - pos));
        End += num;
        return pos;
    };
    const T *remove(const T *pos, size_t num, const T &fill, bool des = false)
    {
        const T *p = remove(pos, num, des);
        if (!p)
            return p;
        for (T *i = End; i < End + num; i++)
            *i = fill;
        return p;
    };
    const T *remove(const T *pos, size_t num, bool des = false)
    {
        if (!num)
            return pos;
        if (Begin > pos || pos + num - 1 > End)
            return nullptr;
        T *Pos = const_cast<T *>(pos);
        if (des)
            for (T *i = Pos; i < Pos + num; i++)
            {
                i->~T();
                *i = T();
            }
        memmove(Pos, pos + num, sizeof(T) * (End - pos));
        End -= num;
        return pos;
    };
    const T *replace(const T *pos, size_t num, const T &fill, bool des = false)
    {
        if (!num)
            return pos;
        if (Begin > pos || pos + num > End)
            return nullptr;
        T *Pos = const_cast<T *>(pos);
        while (num)
        {
            if (des)
            {
                Pos->~T();
                *Pos = T();
            }
            *Pos = fill;
            Pos += num - 1;
            num--;
        }
        return pos;
    };
    const T &operator[](size_t p)
    {
        if (Begin + p <= End)
            return *(Begin + p);
        return *Begin;
    };
    size_t size()
    {
        return Size - Begin;
    }
    const T *begin()
    {
        return this->Begin;
    }
    const T *end()
    {
        return this->End;
    };
    simpleCntr(size_t size, const T &fill)
        : simpleCntr(size)
    {
        for (T *i = Begin; i <= Size; i++)
            *i = fill;
    };
    simpleCntr(size_t size)
        : Begin(new T[size + 1]),
          Size(Begin + size),
          End(Begin){};
    ~simpleCntr()
    {
        delete[] Begin;
    };
};

#endif