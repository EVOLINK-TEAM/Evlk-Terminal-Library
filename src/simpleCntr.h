#ifndef _SIMPLECNTR_
#define _SIMPLECNTR_

template <class T>
class simpleCntr
{
private:
    T *const Begin;
    T *const Size;
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
    const T *insert(const T *pos, size_t num, const T &fill)
    {
        if (!num)
            return pos;
        if (Begin > pos || pos > End || End + num > Size)
            return nullptr;
        T *Pos = const_cast<T *>(pos);
        End += num;
        for (T *i = End - 1; i > pos + num - 1; i--)
            *i = *(i - num);
        for (T *i = Pos; i <= pos + num - 1; i++)
            *i = fill;
        return pos;
    };
    const T *insert(const T *pos, size_t num)
    {
        T f;
        return insert(pos, num, f);
    };
    const T *remove(const T *pos, size_t num, const T &fill, bool des = false)
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
        for (T *i = Pos; i < End - num; i++)
        {
            *i = *(i + num);
        }
        for (T *i = End - num; i < End; i++)
            *i = fill;
        End -= num;
        return pos;
    };
    const T *remove(const T *pos, size_t num, bool des = false)
    {
        T f;
        return remove(pos, num, f, des);
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
    const T *begin()
    {
        return this->Begin;
    };
    const T *size()
    {
        return this->Size;
    };
    const T *end()
    {
        return this->End;
    };
    simpleCntr(size_t size, T fill)
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