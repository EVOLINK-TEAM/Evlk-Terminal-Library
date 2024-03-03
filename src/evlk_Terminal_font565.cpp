#include "evlk_Terminal_font565.h"

namespace _EVLK_TERMINAL_
{
    uint16_t font565::colorCode(uint8_t code)
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
    font565::font565()
    {
        init();
    }

    void font565::init()
    {
        color = T565_WHITE;
        bgcolor = T565_BLACK;
        Bold = false;
        Dim = false;
        Smso = false;
        Smul = false;
        Blink = false;
        Rev = false;
        Invis = false;
    };
    bool font565::color_8(uint8_t c)
    {
        if (c > 8)
            return false;
        color = colorCode(c);
        return true;
    }
    bool font565::bgColor_8(uint8_t c)
    {
        if (c > 8)
            return false;
        bgcolor = colorCode(c);
        return true;
    }

    bool font565::sgr0()
    {
        init();
    }
    bool font565::bold()
    {
        Bold = true;
        return true;
    }
    bool font565::dim()
    {
        Dim = true;
        return true;
    }
    bool font565::smso()
    {
        Smso = true;
        return true;
    }
    bool font565::smul()
    {
        Smul = true;
        return true;
    }
    bool font565::blink()
    {
        bool blink = true;
        return true;
    }
    bool font565::rev()
    {
        Rev = true;
        return true;
    }
    bool font565::invis()
    {
        Invis = true;
        return true;
    }

    bool font565::operator==(const font &f) const
    {
        const font565 *F = dynamic_cast<const font565 *>(&f);
        if (F && *this == *F)
            return true;
        return false;
    };
    bool font565::operator==(const font565 &f) const
    {
        return (color == f.color &&
                bgcolor == f.bgcolor &&
                Bold == f.Bold &&
                Dim == f.Dim &&
                Smso == f.Smso &&
                Smul == f.Smul &&
                Blink == f.Blink &&
                Rev == f.Rev &&
                Invis == f.Invis);
    };
    void font565::operator=(const font &f)
    {
        const font565 *F = dynamic_cast<const font565 *>(&f);
        if (F)
            *this = *F;
    }
    font565 &font565::operator=(const font565 &f)
    {
        color = f.color;
        bgcolor = f.bgcolor;
        Bold = f.Bold;
        Dim = f.Dim;
        Smso = f.Smso;
        Smul = f.Smul;
        Blink = f.Blink;
        Rev = f.Rev;
        Invis = f.Invis;
        return *this;
    };

    font *fontFactory565::createFont()
    {
        return new font565();
    };

}