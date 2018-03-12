#include "assert.h"
#include "stdio.h"

#define DBG(format, ...) {printf("%s(%*d): ",__FILE__,4,__LINE__);printf(format, ##__VA_ARGS__);printf("\n");}
#define MAKE_COLOR(r,g,b,a) ((uint32_t)((a << 24) | (b << 16) | (g << 8) | r))
#define COL_WHITE 0xffffffff
#define COL_BLACK 0xff000000
#define WITH_B(color, b) ((color & ~0x00ff0000) | (b << 16))
#define WITH_G(color, g) ((color & ~0x0000ff00) | (g << 8))
#define WITH_R(color, r) ((color & ~0x000000ff) | (r << 0))
#define WITH_A(color, a) ((color & ~0xff000000) | (a << 24))
#define KILOBYTES(Number) (1024L*Number)
#define MEGABYTES(Number) (1024L*KILOBYTES(Number))
#define GIGABYTES(Number) (1024L*MEGABYTES(Number))
#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
#define PI 3.1415f
#define global_variable static
#define internal_function static
#define local_persistent static

#define SET_FLAG(VAR, FLAG) (VAR |= FLAG)
#define UNSET_FLAG(VAR, FLAG) (VAR &= ~FLAG)
