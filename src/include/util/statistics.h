#ifndef __STATISTICS__
#define __STATISTICS__

#ifdef STAT
	#include <iostream>

	#define STATEXEC(e) e
	#define STATMEM(m) std::cout << "MEM " << m << std::endl
	#define STATSPLIT() std::cout << "SPLIT" << std::endl
	#define STATSHRINK() std::cout << "SHRINK" << std::endl
	#define STATHEIGHT(h) std::cout << "HEIGHT " << h << std::endl
	#define STATSIZE(n) std::cout << "SIZE " << n << std::endl
	#define STATPSIZE(n) std::cout << "PSIZE " << n << std::endl
	#define STATBRANCH(b) std::cout << "BRANCH " << b << std::endl
	#define STATBRSR(b) std::cout << "BRANCH SEARCH " << b << std::endl
	#define STATFAN(f) std::cout << "FANOUT " << f << std::endl
#else
	#define STATEXEC(e)
	#define STATMEM(m)
	#define STATSPLIT()
	#define STATSHRINK()
	#define STATHEIGHT(h)
	#define STATSIZE(n)
	#define STATPSIZE(n)
	#define STATBRANCH(b)
	#define STATBRSR(b)
	#define STATFAN(f)
#endif

#endif
