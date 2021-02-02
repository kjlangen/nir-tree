#ifndef __STATISTICS__
#define __STATISTICS__

#ifdef STAT
	#include <iostream>

	#define STATEXEC(e) e
	#define STATMEM(mem) std::cout << "Memory Usage: " << (mem / 1024) << "KB, " << (mem / (1024 * 1024)) << "MB, " << (mem / (1024 * 1024 * 1024)) << "GB" << std::endl
	#define STATHEIGHT(height) std::cout << "Tree Height: " << height << std::endl
	#define STATSIZE(n) std::cout << "Tree Nodes: " << n << std::endl
	#define STATSINGULAR(n) std::cout << "Tree Nodes w/fanout=1: " << n << std::endl
	#define STATLEAF(n) std::cout << "Tree Leaves: " << n << std::endl
	#define STATBRANCH(branches) std::cout << "Tree Branches: " << branches << std::endl
	#define STATFANHIST() std::cout << "Histogram of Fanout Follows: " << std::endl
	#define STATLINES(n) std::cout << "Bounding Lines: " << n << std::endl
	#define STATTOTALPOLYSIZE(n) std::cout << "Total Polygon Size: " << n << std::endl
	#define STATPOLYHIST() std::cout << "Histogram of Polygon Sizes Follows:" << std::endl
	#define STATSEARCHHIST() std::cout << "Histogram of Searched Nodes Follows:" << std::endl
	#define STATLEAVESHIST() std::cout << "Histogram of Searched Leaves Follows:" << std::endl
	#define STATRANGESEARCHHIST() std::cout << "Histogram of Range Searched Nodes Follows:" << std::endl
	#define STATRANGELEAVESHIST() std::cout << "Histogram of Range Searched Leaves Follows:" << std::endl
	#define STATHIST(bucket, count) std::cout << "  " << bucket << " : " << count << std::endl
#else
	#define STATEXEC(e)
	#define STATMEM(mem)
	#define STATHEIGHT(height)
	#define STATSIZE(n)
	#define STATSINGULAR(n)
	#define STATLEAF(n)
	#define STATBRANCH(branches)
	#define STATFANHIST()
	#define STATLINES(n)
	#define STATTOTALPOLYSIZE(n)
	#define STATPOLYHIST()
	#define STATSEARCHHIST()
	#define STATLEAVESHIST()
	#define STATRANGESEARCHHIST()
	#define STATRANGELEAVESHIST()
	#define STATHIST(bucket, count)
#endif

#endif
