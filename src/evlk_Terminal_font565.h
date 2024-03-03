#ifndef _EVLK_TERMINAL_FONT565_H_
#define _EVLK_TERMINAL_FONT565_H_

#include "evlk_Terminal_fontImpl.h"

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
#endif

namespace _EVLK_TERMINAL_
{

    struct font565 : public font
    {
        uint16_t color;
        uint16_t bgcolor;
        bool Bold;
        bool Dim;
        bool Smso;
        bool Smul;
        bool Blink;
        bool Rev;
        bool Invis;

        uint16_t colorCode(uint8_t code);
        font565();

        void init() override;
        bool color_8(uint8_t c) override;
        bool bgColor_8(uint8_t c) override;

        bool sgr0() override;
        bool bold() override;
        bool dim() override;
        bool smso() override;
        bool smul() override;
        bool blink() override;
        bool rev() override;
        bool invis() override;
        bool operator==(const font &f) const override;
        bool operator==(const font565 &f) const;
        void operator=(const font &f) override;
        font565 &operator=(const font565 &f);
    };

    struct fontFactory565 : public fontFactory
    {
        font *createFont() override;
    };
}
#endif