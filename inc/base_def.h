#ifndef __ECE_LIB_BASE_DEFINE_H_
#define __ECE_LIB_BASE_DEFINE_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define static
#define inline
#define app_printf(format, args...) printf("\033[1m\033[45;37m[WLOC]"format"\033[0m\n", ##args)
#define lib_printf(format, args...) printf("\033[1m\033[41;37m[EDGE]"format"\033[0m\n", ##args)
#define srv_printf(format, args...) printf("\033[1m\033[46;37m[OASI]"format"\033[0m\n", ##args)

#endif
