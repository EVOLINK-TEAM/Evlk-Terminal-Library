#ifndef _EVLK_TERMINAL_FONTIMPL_H_
#define _EVLK_TERMINAL_FONTIMPL_H_

#include "stdint.h"

namespace _EVLK_TERMINAL_
{
    struct font
    {
        virtual void init() = 0;

        virtual bool color_8(uint8_t) { return true; };
        virtual bool bgColor_8(uint8_t) { return true; };
        virtual bool color_256(uint8_t) { return true; };
        virtual bool bgcolor_256(uint8_t) { return true; };
        virtual bool color_true(uint8_t r, uint8_t g, uint8_t b) { return true; };
        virtual bool bgcolor_true(uint8_t r, uint8_t g, uint8_t b) { return true; };

        virtual bool sgr0() { return true; };
        virtual bool bold() { return true; };  // 粗体
        virtual bool dim() { return true; };   // 加深
        virtual bool smso() { return true; };  // 突出
        virtual bool smul() { return true; };  // 下划线
        virtual bool blink() { return true; }; // 闪烁
        virtual bool rev() { return true; };   // 倒叙
        virtual bool invis() { return true; }; // 隐藏
        virtual bool operator==(const font &) const = 0;
        virtual void operator=(const font &) = 0;

        virtual ~font() = default;
    };

    struct fontFactory
    {
        virtual font *createFont() = 0;
    };

}
#endif