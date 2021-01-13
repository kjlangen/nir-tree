#include <iostream>
#include <map>
#include <string>
#include <globals/globals.h>
#include <rtree/rtree.h>
#include <nirtree/nirtree.h>
#include <rplustree/rplustree.h>
#include <bench/randomPoints.h>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

void parameters(std::map<std::string, unsigned> &configU, std::map<std::string, double> configD)
{
	std::string treeTypes[] = {"R_TREE", "R_PLUS_TREE", "NIR_TREE"};
	std::string benchTypes[] = {"UNIFORM", "SKEW", "CLUSTER", "CALIFORNIA", "BIOLOGICAL" };

	std::cout << "### BENCHMARK PARAMETERS ###" << std::endl;
	std::cout << "  tree = " << treeTypes[configU["tree"]] << std::endl;
	std::cout << "  benchmark = " << benchTypes[configU["distribution"]] << std::endl;
	std::cout << "  min/max branches = " << configU["minfanout"] << "/" << configU["maxfanout"] << std::endl;
	std::cout << "  n = " << configU["size"] << std::endl;
	std::cout << "  seed = " << configU["seed"] << std::endl;
	std::cout << "  search rectangles = " << configU["rectanglescount"] << std::endl;
	std::cout << "  skew factor = " << configD["skewfactor"] << std::endl;
	std::cout << "  cluster count = " << configU["clustercount"] << std::endl;
	std::cout << "### ### ### ### ### ###" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
	// Create unit test session
	Catch::Session session;

#ifdef UNIT_TESTING
	// Run unit tests
	assert(dimensions == 2);
	return session.run(argc, argv);

#else
	// Process command line options
	int option;

	// Benchmark default configuration
	std::map<std::string, unsigned> configU;
	configU.emplace("tree", NIR_TREE);
	configU.emplace("minfanout", 25);
	configU.emplace("maxfanout", 50);
	configU.emplace("size", 10000);
	configU.emplace("distribution", UNIFORM);
	configU.emplace("seed", 3141);
	configU.emplace("rectanglescount", 5000);
	configU.emplace("clustercount", 20);

	std::map<std::string, double> configD;
	configD.emplace("skewfactor", 9.0);

	while ((option = getopt(argc, argv, "t:m:a:b:n:s:r:c:f:")) != -1)
	{
		switch (option)
		{
			case 't': // Tree
			{
				configU["tree"] = (TreeType)atoi(optarg);
				break;
			}
			case 'm': // Benchmark type
			{
				configU["distribution"] = (BenchType)atoi(optarg);
				break;
			}
			case 'a': // Minimum fanout
			{
				configU["minfanout"] = atoi(optarg);
				break;
			}
			case 'b': // Maximum fanout
			{
				configU["maxfanout"] = atoi(optarg);
				break;	
			}
			case 'n': // Benchmark size
			{
				configU["size"] = atoi(optarg);
				break;
			}
			case 's': // Benchmark seed
			{
				configU["seed"] = atoi(optarg);
				break;
			}
			case 'r': // Number of search rectangles
			{
				configU["rectanglescount"] = atoi(optarg);
				break;
			}
			case 'c': // Number of clusters
			{
				configU["clustercount"] = atoi(optarg);
				break;
			}
			case 'f': // Skew factor
			{
				configD["skewfactor"] = (double) atof(optarg);
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
	parameters(configU, configD);

	// Run the benchmark
	randomPoints(configU, configD);
#endif
}
