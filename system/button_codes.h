//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_system
{

enum button_code //from X11 codes
{
    key_return    =0xff0d,
    key_escape    =0xff1b,
    key_space     =0x0020,
    key_tab       =0xff09,
    key_pause     =0xff13,

    key_shift     =0xffe1,
    key_shift_r   =0xffe2,
    key_control   =0xffe3,
    key_control_r =0xffe4,
    key_alt       =0xffe9,
    key_alt_r     =0xffea,
    key_capital   =0xffe5,

    key_up        =0xff52,
    key_down      =0xff54,
    key_left      =0xff51,
    key_right     =0xff53,

    key_end       =0xff57,
    key_home      =0xff50,
    key_insert    =0xff63,
    key_delete    =0xffff,
    key_backspace =0xff08,

    key_a         =0x0061,
    key_0         =0x00300,
    key_1         =0x0031,
    key_f1        =0xffbe,

/*
    #define KEY_TAB 0xff09
    #define KEY_ALT  0xffe9
    #define KEY_ALT_R 0xffea
    #define KEY_RETURN 0xff0d
    #define KEY_ESCAPE 0xff1b

    #define KEY_SHIFT 0xffe1
    #define KEY_SHIFT_R 0xffe2
    #define KEY_CONTROL 0xffe3
    #define KEY_CONTROL_R 0xffe4
    #define KEY_PAUSE 0xff13
    #define KEY_CAPITAL 0xffe5

    #define KEY_SPACE 0x0020
    #define KEY_END 0xff57
    #define KEY_HOME 0xff50
    #define KEY_LEFT 0xff51
    #define KEY_UP 0xff52
    #define KEY_RIGHT 0xff53
    #define KEY_DOWN 0xff54
    #define KEY_INSERT 0xff63
    #define KEY_DELETE 0xffff
    #define KEY_BACKSPACE 0xff08

    //small, capital = 0x0041
    #define KEY_A 0x0061
    #define KEY_0 0x00300
    #define KEY_1 0x0031
    #define KEY_F1 0xffbe
*/
};

}
