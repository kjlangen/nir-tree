#include <rtree/rtree.h>
#include <nirtree/nirtree.h>
#include <rplustree/rplustree.h>
#include <rstartree/rstartree.h>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char*argv[])
{
	// Create unit test session
	Catch::Session session;

	// Run unit tests
	assert(dimensions == 2);
	return session.run(argc, argv);
}
