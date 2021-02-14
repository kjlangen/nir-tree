#ifndef __GLOBALS__
#define __GLOBALS__

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef DIM
#pragma message "DIM" STR(DIM)
#define DIM 2
#endif

const unsigned dimensions = DIM;

#endif
