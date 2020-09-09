#ifndef __STATISTICS__
#define __STATISTICS__

#ifdef STAT
	#include <iostream>

	#define STATEXEC(e) e
	#define STATMEM(m) std::cout << "MEM " << m << std::endl;
	#define STATSPLIT() std::cout << "SPLIT" << std::endl;
	#define STATSHRINK() std::cout << "SHRINK" << std::endl;
	#define STATHEIGHT(h) std::cout << "HEIGHT " << h << std::endl;
	#define STATPSIZE(n) std::cout << "SIZE " << n << std::endl;
	#define STATBRANCH(b) std::cout << "BRANCH " << b << std::endl;
#else
	#define STATEXEC(e)
	#define STATMEM(m)
	#define STATSPLIT()
	#define STATSHRINK()
	#define STATHEIGHT(h)
	#define STATPSIZE(n)
	#define STATBRANCH(b)
#endif

#endif
