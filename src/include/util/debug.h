#ifndef __DEBUG__
#define __DEBUG__

#include <iostream>

#ifdef DEBUG
	#define DEXEC(a) a
	#define DASSERT(a) assert(a)
	#define DPRINT1(a) std::cout << a << std::endl
	#define DPRINT2(a, b) std::cout << a << b << std::endl
	#define DPRINT3(a, b, c) std::cout << a << b << c<< std::endl
	#define DPRINT4(a, b, c, d) std::cout << a << b << c << d << std::endl
	#define DPRINT5(a, b, c, d, e) std::cout << a << b << c << d << e << std::endl
	#define DPRINT6(a, b, c, d, e, f) std::cout << a << b << c << d << e << f << std::endl
#endif

#ifndef DEBUG
	#define DEXEC(a)
	#define DASSERT(a)
	#define DPRINT1(a)
	#define DPRINT2(a, b)
	#define DPRINT3(a, b, c)
	#define DPRINT4(a, b, c, d)
	#define DPRINT5(a, b, c, d, e)
	#define DPRINT6(a, b, c, d, e)
#endif

#endif
