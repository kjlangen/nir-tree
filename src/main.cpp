#include <iostream>
#include <rtree/rtree.h>
#include <nirtree/nirtree.h>
#include <rplustree/rplustree.h>
#include <bench/randomPoints.h>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

enum TreeType { R_TREE, R_PLUS_TREE, NIR_TREE };

void parameters(TreeType type, BenchType bench, unsigned a, unsigned b, unsigned n, unsigned l, unsigned s) {
	std::string treeTypes[] = {"R_TREE", "R_PLUS_TREE", "NIR_TREE"};
	std::string benchTypes[] = { "UNIFORM", "SKEW", "CLUSTER" };
	std::cout << "### TEST PARAMETERS ###" << std::endl;
	std::cout << "  tree = " << treeTypes[type] << std::endl;
	std::cout << "  bench = " << benchTypes[bench] << std::endl;
	std::cout << "  min/max branches = " << a << "/" << b << std::endl;
	std::cout << "  n = " << n << std::endl;
	std::cout << "  log frequency = " << l << std::endl;
	std::cout << "  seed = " << s << std::endl;
	std::cout << "### ### ### ### ### ###" << std::endl;
}

int main(int argc, char *argv[])
{
	Catch::Session session;
#ifdef UNIT_TESTING
	return session.run(argc, argv);
#else
	// Process command line options
	int option, a = 250, b = 500, n = 10000, l = 1000, s = 3141;
	TreeType type = NIR_TREE;
	BenchType bench = UNIFORM;
	while ((option = getopt(argc, argv, "t:m:a:b:n:l:s:")) != -1)
	{
		switch (option)
		{
			case 't': // Tree type
			{
				type = (TreeType)atoi(optarg);
				break;
			}
			case 'm': // Benchmark type
			{
				bench = (BenchType)atoi(optarg);
				break;
			}
			case 'a': // Minimum branch factor
			{
				a = atoi(optarg);
				break;
			}
			case 'b': // Maximum branch factor
			{
				b = atoi(optarg);
				break;	
			}
			case 'n': // Benchmark size
			{
				n = atoi(optarg);
				break;
			}
			case 'l': // Log frequency
			{
				l = atoi(optarg);
				break;
			}
			case 's': // Benchmark seed
			{
				s = atoi(optarg);
				break;
			}
			default:
			{
				std::cout << "Bad option. Exiting." << std::endl;
				return 1;
			}
		}
	}

	// Print test parameters
	parameters(type, bench, a, b, n, l, s);

	// Run benchmarking
	switch (type) {
		case R_TREE:
		{
			rtree::RTree rt(a, b);
			randomPoints(rt, bench, n, s, l);
			break;
		}
		case R_PLUS_TREE:
		{
			rplustree::RPlusTree rpt(a, b);
			randomPoints(rpt, bench, n, s, l);
			break;
		}
		case NIR_TREE:
		{
			nirtree::NIRTree nt(a, b);
			randomPoints(nt, bench, n, s, l);
			break;
		}
		default:
		{
			assert(false);
		}
	}
#endif
}
