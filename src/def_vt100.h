#ifndef _DEF_VT100_H_
#define _DEF_VT100_H_
// ANSI escape code
#define def_vt100_srg0 "\33[0m"
#define def_vt100_bold "\33[1m"
#define def_vt100_dim "\33[2m"
#define def_vt100_smso "\33[3m"
#define def_vt100_mul "\33[4m"
#define def_vt100_blink "\33[5m"
#define def_vt100_rev "\33[7m"
#define def_vt100_invis "\33[8m"
#define def_vt100_cup(r, c) "\33[" + r + ';' + c + 'H'
#define def_vt100_home "\33[H"
#define def_vt100_cur_up(n) "\33[" + n + 'A'
#define def_vt100_cur_down(n) "\33[" + n + 'B'
#define def_vt100_cur_right(n) "\33[" + n + 'C'
#define def_vt100_cur_left(n) "\33[" + n + 'D'
#define def_vt100_clean "\33[2J"
#define def_vt100_el "\33[K"
#define def_vt100_el1 "\33[1K"
#define def_vt100_el2 "\33[2K"
#define def_vt100_sc "\33[s"
#define def_vt100_rc "\33[u"
#define def_vt100_civis "\33[?25l"
#define def_vt100_cvvis "\33[?25h"

#define def_vt100_setaf0 "\33[30m" // black
#define def_vt100_setaf1 "\33[31m" // red
#define def_vt100_setaf2 "\33[32m" // green
#define def_vt100_setaf3 "\33[33m" // yellow
#define def_vt100_setaf4 "\33[34m" // blue
#define def_vt100_setaf5 "\33[35m" // magenta
#define def_vt100_setaf6 "\33[36m" // cyan
#define def_vt100_setaf7 "\33[37m" // white
#define def_vt100_setaf9 "\33[39m" // reset

#define def_vt100_setab0 "\33[40m" // black
#define def_vt100_setab1 "\33[41m" // red
#define def_vt100_setab2 "\33[42m" // green
#define def_vt100_setab3 "\33[43m" // yellow
#define def_vt100_setab4 "\33[44m" // blue
#define def_vt100_setab5 "\33[45m" // magenta
#define def_vt100_setab6 "\33[46m" // cyan
#define def_vt100_setab7 "\33[47m" // white
#define def_vt100_setab9 "\33[49m" // reset

#define def_vt100_lfc_black "\33[90m"
#define def_vt100_lfc_red "\33[91m"
#define def_vt100_lfc_green "\33[92m"
#define def_vt100_lfc_yellow "\33[93m"
#define def_vt100_lfc_blue "\33[94m"
#define def_vt100_lfc_magenta "\33[95m"
#define def_vt100_lfc_cyan "\33[96m"
#define def_vt100_lfc_white "\33[97m"

#define def_vt100_lbc_black "\33[100m"
#define def_vt100_lbc_red "\33[101m"
#define def_vt100_lbc_green "\33[102m"
#define def_vt100_lbc_yellow "\33[103m"
#define def_vt100_lbc_blue "\33[104m"
#define def_vt100_lbc_magenta "\33[105m"
#define def_vt100_lbc_cyan "\33[106m"
#define def_vt100_lbc_white "\33[107m"
#endif