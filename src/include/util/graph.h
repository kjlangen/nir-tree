#ifndef __GRAPH__
#define __GRAPH__

#include <algorithm>
#include <utility>
#include <cstring>
#include <vector>
#include <set>
#include <stack>
#include <iostream>
#include <chrono>
#include <unordered_set>
#include <util/geometry.h>
#include <util/debug.h>
#include <nirtree/pencilPrinter.h>

class Graph
{
	private:
		struct SchedulePoint
		{
			float coord;
			unsigned index;
			bool start;
		};

		unsigned vertices;
		std::vector<unsigned> *g;
		bool *explored;

	public:
		// Constructors and destructors
		Graph(const unsigned n);
		Graph(std::vector<Rectangle> &v);
		Graph(std::vector<IsotheticPolygon> &p);
		~Graph();

		// High-level graph operations
		bool contiguous();
		bool contiguous(unsigned skipVertex);
		bool *findBalancedSeparator(unsigned *weights);
};
#endif
