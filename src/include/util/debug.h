#ifndef __DEBUG__
#define __DEBUG__

#ifdef DEBUG0
	#define DEXEC(a) a
	#define DASSERT(a) assert(a)
	#define DPRINT1(a)
	#define DPRINT2(a, b)
	#define DPRINT3(a, b, c)
	#define DPRINT4(a, b, c, d)
	#define DPRINT5(a, b, c, d, e)
	#define DPRINT6(a, b, c, d, e, f)
	#define DPRINT7(a, b, c, d, e, f, g)
	#define DPRINT8(a, b, c, d, e, f, g, h)
	#define DPRINT9(a, b, c, d, e, f, g, h, i)

#elif DEBUG1
	#include <iostream>

	#define DEXEC(a) a
	#define DASSERT(a) assert(a)
	#define DPRINT1(a) std::cout << a << std::endl
	#define DPRINT2(a, b) std::cout << a << b << std::endl
	#define DPRINT3(a, b, c) std::cout << a << b << c<< std::endl
	#define DPRINT4(a, b, c, d) std::cout << a << b << c << d << std::endl
	#define DPRINT5(a, b, c, d, e) std::cout << a << b << c << d << e << std::endl
	#define DPRINT6(a, b, c, d, e, f) std::cout << a << b << c << d << e << f << std::endl
	#define DPRINT7(a, b, c, d, e, f, g) std::cout << a << b << c << d << e << f << g << std::endl
	#define DPRINT8(a, b, c, d, e, f, g, h) std::cout << a << b << c << d << e << f << g << h << std::endl
	#define DPRINT9(a, b, c, d, e, f, g, h, i) std::cout << a << b << c << d << e << f << g << h << i << std::endl

#else
	#define DEXEC(a)
	#define DASSERT(a)
	#define DPRINT1(a)
	#define DPRINT2(a, b)
	#define DPRINT3(a, b, c)
	#define DPRINT4(a, b, c, d)
	#define DPRINT5(a, b, c, d, e)
	#define DPRINT6(a, b, c, d, e, f)
	#define DPRINT7(a, b, c, d, e, f, g)
	#define DPRINT8(a, b, c, d, e, f, g, h)
	#define DPRINT9(a, b, c, d, e, f, g, h, i)

#endif

#endif
