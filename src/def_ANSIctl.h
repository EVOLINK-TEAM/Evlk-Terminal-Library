#ifndef _DEF_ANSIec_H_
#define _DEF_ANSIec_H_
// ANSI escape code
#define def_ansiec_normal "\33[0m"
#define def_ansiec_bright "\33[1m"
#define def_ansiec_underline "\33[4m"
#define def_ansiec_flash "\33[5m"
#define def_ansiec_inverse "\33[7m"
#define def_ansiec_invisable "\33[8m"
#define def_ansiec_cur_up(n) "\33[" + n + 'A'
#define def_ansiec_cur_down(n) "\33[" + n + 'B'
#define def_ansiec_cur_right(n) "\33[" + n + 'C'
#define def_ansiec_cur_left(n) "\33[" + n + 'D'
#define def_ansiec_cur_pos(x, y) "\33[" + y + ';' + x + 'H'
#define def_ansiec_clean "\33[2J"
#define def_ansiec_clean_line "\33[K"
#define def_ansiec_cur_save "\33[s"
#define def_ansiec_cur_recover "\33[u"
#define def_ansiec_cur_hide "\33[?25l"
#define def_ansiec_cur_show "\33[?25h"

#define def_ansiec_fc_black "\33[30m"
#define def_ansiec_fc_red "\33[31m"
#define def_ansiec_fc_green "\33[32m"
#define def_ansiec_fc_yellow "\33[33m"
#define def_ansiec_fc_blue "\33[34m"
#define def_ansiec_fc_magenta "\33[35m"
#define def_ansiec_fc_cyan "\33[36m"
#define def_ansiec_fc_white "\33[37m"

#define def_ansiec_bc_black "\33[40m"
#define def_ansiec_bc_red "\33[41m"
#define def_ansiec_bc_green "\33[42m"
#define def_ansiec_bc_yellow "\33[43m"
#define def_ansiec_bc_blue "\33[44m"
#define def_ansiec_bc_magenta "\33[45m"
#define def_ansiec_bc_cyan "\33[46m"
#define def_ansiec_bc_white "\33[47m"

#define def_ansiec_lfc_black "\33[90m"
#define def_ansiec_lfc_red "\33[91m"
#define def_ansiec_lfc_green "\33[92m"
#define def_ansiec_lfc_yellow "\33[93m"
#define def_ansiec_lfc_blue "\33[94m"
#define def_ansiec_lfc_magenta "\33[95m"
#define def_ansiec_lfc_cyan "\33[96m"
#define def_ansiec_lfc_white "\33[97m"

#define def_ansiec_lbc_black "\33[100m"
#define def_ansiec_lbc_red "\33[101m"
#define def_ansiec_lbc_green "\33[102m"
#define def_ansiec_lbc_yellow "\33[103m"
#define def_ansiec_lbc_blue "\33[104m"
#define def_ansiec_lbc_magenta "\33[105m"
#define def_ansiec_lbc_cyan "\33[106m"
#define def_ansiec_lbc_white "\33[107m"
#endif