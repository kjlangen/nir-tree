#ifndef __GRAPH__
#define __GRAPH__

#include <algorithm>
#include <utility>
#include <cstring>
#include <vector>
#include <stack>
#include <iostream>
#include <chrono>
#include <util/geometry.h>

class Graph
{
	private:
		struct TaggedRectangle
		{
			Rectangle r;
			unsigned tag;
		};

		unsigned long vertices;
		bool *g;

		// Rectangles
		void weightSubtree(float *weights, TaggedRectangle *vTagged, unsigned lBound, unsigned rBound);
		void querySubtree(TaggedRectangle queryRectangle, float *weights, TaggedRectangle *vTagged, unsigned lBound, unsigned rBound);

	public:
		Graph(const unsigned n);
		Graph(std::vector<Rectangle> &v);
		Graph(std::vector<IsotheticPolygon> &v);
		~Graph();

		bool *operator[](unsigned i);	
};
#endif
