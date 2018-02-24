#include "assert.h"

#define DBG(format, ...) {printf("%s(%*d): ",__FILE__,4,__LINE__);printf(format, ##__VA_ARGS__);printf("\n");}

#define MAKE_COLOR(r,g,b,a) ((uint32_t)((a << 24) | (b << 16) | (g << 8) | r))
