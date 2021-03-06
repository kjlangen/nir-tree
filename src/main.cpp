#include <iostream>
#include <map>
#include <string>
#include <globals/globals.h>
#include <rtree/rtree.h>
#include <rplustree/rplustree.h>
#include <rstartree/rstartree.h>
#include <nirtree/nirtree.h>
#include <bench/randomPoints.h>
#include <unistd.h>

void parameters(std::map<std::string, unsigned> &configU, std::map<std::string, double> configD)
{
	std::string treeTypes[] = {"R_TREE", "R_PLUS_TREE", "R_STAR_TREE", "NIR_TREE"};
	std::string benchTypes[] = {"UNIFORM", "SKEW", "CLUSTER", "CALIFORNIA", "BIOLOGICAL", "FOREST", "CANADA"};

	std::cout << "### BENCHMARK PARAMETERS ###" << std::endl;
	std::cout << "  tree = " << treeTypes[configU["tree"]] << std::endl;
	std::cout << "  benchmark = " << benchTypes[configU["distribution"]] << std::endl;
	std::cout << "  min/max branches = " << configU["minfanout"] << "/" << configU["maxfanout"] << std::endl;
	std::cout << "  n = " << configU["size"] << std::endl;
	std::cout << "  dimensions = " << dimensions << std::endl;
	std::cout << "  seed = " << configU["seed"] << std::endl;
	std::cout << "  search rectangles = " << configU["rectanglescount"] << std::endl;
	std::cout << "  visualization = " << (configU["visualization"] ? "on" : "off") << std::endl;
	std::cout << "### ### ### ### ### ###" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
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
	configU.emplace("visualization", false);

	std::map<std::string, double> configD;

	while ((option = getopt(argc, argv, "t:m:a:b:n:s:r:v:")) != -1)
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
			case 'v': // Visualization
			{
				configU["visualization"] = true;
				break;
			}
			default:
			{
				std::cout << "Bad option. Usage:" << std::endl;
				std::cout << "    -t  Specifies tree type {0 = R-Tree, 1 = R+-Tree, 2 = R*-Tree, 3 = NIR-Tree}" << std::endl;
				std::cout << "    -m  Specifies benchmark type {0 = Uniform, 1 = Skew, 2 = Clustered, 3 = California, 4 = Biological, 5 = Forest}" << std::endl;
				std::cout << "    -a  Minimum fanout for nodes in the selected tree" << std::endl;
				std::cout << "    -b  Maximum fanout for nodes in the selected tree" << std::endl;
				std::cout << "    -n  Specified benchmark size if size is not constant for benchmark type" << std::endl;
				std::cout << "    -s  Specifies benchmark seed if benchmark type is randomly generated" << std::endl;
				std::cout << "    -r  Specifies number of rectangles to search in benchmark if size is not constant for benchmark type" << std::endl;
				std::cout << "    -v  Turns visualization on or off for first two dimensions of the selected tree" << std::endl;
				return 1;
			}
		}
	}

	// Print test parameters
	parameters(configU, configD);

	// Run the benchmark
	randomPoints(configU, configD);
}
