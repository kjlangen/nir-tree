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
#include <util/geometry.h>
#include <util/debug.h>

class Graph
{
	private:
		struct SchedulePoint
		{
			unsigned coord;
			unsigned index;
			bool start;
		};

		unsigned vertices;
		bool *g;
		unsigned removedVertices;
		bool *gRemoved;

	public:
		// Constructors and destructors
		Graph(const unsigned n);
		Graph(std::vector<Rectangle> &v);
		Graph(std::vector<IsotheticPolygon> &v);
		~Graph();

		// High-level graph operations
		bool contiguous();
		bool contiguous(unsigned skipVertex);
		unsigned components(unsigned *labels);
		void remove(unsigned givenVertex);

		// Graph access
		bool *operator[](unsigned i);	
};
#endif
