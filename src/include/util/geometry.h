
// Copyright 2021 Kyle Langendoen

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#define MAX_RECTANGLE_COUNT 5

#include <iostream>
#include <algorithm>
#include <queue>
#include <cassert>
#include <vector>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <limits>
#include <cfenv>
#include <iterator>
#include <functional>
#include <util/debug.h>
#include <globals/globals.h>
#include <variant>
#include <cstddef>
#include <storage/tree_node_allocator.h>


class Point
{
	public:

        double values[dimensions];

		static Point atInfinity;
		static Point atNegInfinity;
		static Point atOrigin;
        static Point closest_larger_point( const Point &existing_point ) {
            Point local_point( existing_point );
            for( unsigned d = 0; d < dimensions; d++ ) {
                local_point[d] = nextafter( local_point[d], DBL_MAX );
                if( not (local_point[d] > existing_point[d] ) ) {
                    std::cout << "existing point: " << existing_point[d]
                        << std::endl;
                }
                assert( local_point[d] > existing_point[d] );
            }
            return local_point;
        }
        static Point closest_smaller_point( const Point &existing_point ) {
            Point local_point( existing_point );
            for( unsigned d = 0; d < dimensions; d++ ) {
                local_point[d] = nextafter( local_point[d], -DBL_MAX );
                assert( local_point[d] < existing_point[d] );
            }
            return local_point;
        }


		Point();

		Point(double x, double y);
		Point(double value);
		Point(const Point &o) = default;

		bool orderedCompare(const Point &rhs, unsigned startingDimension) const;
		double distance(const Point &p) const;

		Point &operator-=(const Point &rhs);
		Point &operator+=(const Point &rhs);
		Point &operator/=(double scalar);
		Point &operator*=(double scalar);
		Point &operator*=(const Point &rhs);
		double &operator[](unsigned index);
		double operator[](unsigned index) const;
		Point &operator<<(const Point &p);
		Point &operator>>(const Point &p);

		friend Point operator-(const Point &lhs, const Point &rhs);
		friend Point operator+(const Point &lhs, const Point &rhs);
		friend Point operator*(const Point &lhs, const double scalar);
		friend Point operator/(const Point &lsh, const double scalar);
		friend Point operator*(const Point &lhs, const Point &rhs);
		friend bool operator<(const Point &lhs, const Point &rhs);
		friend bool operator>(const Point &lhs, const Point &rhs);
		friend bool operator<=(const Point &lhs, const Point &rhs);
		friend bool operator>=(const Point &lhs, const Point &rhs);
		friend bool operator==(const Point &lhs, const Point &rhs);
		friend bool operator!=(const Point &lhs, const Point &rhs);
		friend std::ostream& operator<<(std::ostream &os, const Point &point);

};

bool operator<(const Point &lhs, const Point &rhs);
bool operator>(const Point &lhs, const Point &rhs);
bool operator<=(const Point &lhs, const Point &rhs);
bool operator>=(const Point &lhs, const Point &rhs);
bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);

class Rectangle
{
	public:
		static Rectangle atInfinity;
		static Rectangle atNegInfinity;
		static Rectangle atOrigin;

        // Lower point is inclusive
		Point lowerLeft;
        // Upper point is exclusive
		Point upperRight;

		Rectangle();
		Rectangle(double x, double y, double xp, double yp);
		Rectangle(Point lowerLeft, Point upperRight);
		Rectangle(const Rectangle &o) = default;

		double area() const;
		double margin() const;
		double computeIntersectionArea(const Rectangle &givenRectangle) const;
		double computeExpansionArea(const Point &givenPoint) const;
		double computeExpansionMargin(const Point &givenPoint) const;
		double computeExpansionArea(const Rectangle &givenRectangle) const;
		double marginDelta(const Point &givenPoint, const Rectangle &givenRectangle) const;
		double areaDelta(const Point &givenPoint, const Rectangle &givenRectangle) const;
		void expand(const Point &givenPoint);
		void expand(const Rectangle &givenRectangle);
		bool alignedForMerging(const Rectangle &givenRectangle) const;
		bool alignedOpposingBorders(const Rectangle &givenRectangle) const;
		bool intersectsRectangle(const Rectangle &givenRectangle) const;
		bool containsPoint(const Point &givenPoint) const;
		bool containsRectangle(const Rectangle &givenRectangle) const;
		Point centrePoint() const;
		Rectangle copyExpand(const Point &givenPoint) const;
		Rectangle intersection(const Rectangle &clippingRectangle) const;
		std::vector<Rectangle> fragmentRectangle(const Rectangle &clippingRectangle) const;

		friend bool operator==(const Rectangle &lr, const Rectangle &rr);
		friend bool operator!=(const Rectangle &lr, const Rectangle &rr);
		friend std::ostream& operator<<(std::ostream& os, const Rectangle &rectangle);
};

bool operator==(const Rectangle &lhs, const Rectangle &rhs);
bool operator!=(const Rectangle &lhs, const Rectangle &rhs);

class RectangleIterator {
    struct RectangleIteratorBase {
        virtual ~RectangleIteratorBase();
    };
};

class IsotheticPolygon
{
	public:
		struct OptimalExpansion
		{
			unsigned index;
			double area;
		};

		Rectangle boundingBox;
		std::vector<Rectangle> basicRectangles;

		IsotheticPolygon();
        void reset();
		explicit IsotheticPolygon(const Rectangle &baseRectangle);
		IsotheticPolygon(const IsotheticPolygon &basePolygon);

		double area() const;
		double computeIntersectionArea(const Rectangle &givenRectangle) const;
		OptimalExpansion computeExpansionArea(const Point &givenPoint) const;
		OptimalExpansion computeExpansionArea(const Rectangle &givenRectangle) const;
        std::pair<double, std::vector<OptimalExpansion>>
                computeExpansionArea(const IsotheticPolygon &poly) const;
		void expand(const Point &givenPoint);
		void expand(const Point &givenPoint, const OptimalExpansion &expansion);
		bool intersectsRectangle(const Rectangle &givenRectangle) const;
		bool intersectsPolygon(const IsotheticPolygon &givenPolygon) const;
		bool borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool containsPoint(const Point &givenPoint) const ;
		bool disjoint(const IsotheticPolygon &givenPolygon) const;
		std::vector<Rectangle> intersection(const Rectangle &givenRectangle) const;
		void intersection(const IsotheticPolygon &constraintPolygon);
		void increaseResolution(const Point &givenPoint, const Rectangle &clippingRectangle);
		void increaseResolution(const Point &givenPoint, const IsotheticPolygon &clippingPolygon);

		void maxLimit(double limit, unsigned d=0);
		void minLimit(double limit, unsigned d=0);
		void merge(const IsotheticPolygon &mergePolygon);
		void remove(unsigned basicRectangleIndex);
		void deduplicate();
		void refine();
        void recomputeBoundingBox();

        void shrink( const std::vector<Point> &pinPoints ) {
            // Early exit
            if( basicRectangles.size() == 0 or pinPoints.size() == 0 ) {
                return;
            }

            std::vector<Rectangle> rectangleSetShrunk;
            for (const Rectangle &basicRectangle : basicRectangles) {
                bool addRectangle = false;
                Rectangle shrunkRectangle = Rectangle(Point::atInfinity, Point::atNegInfinity);
                for( const auto &pinPoint : pinPoints ) {
                    if( basicRectangle.containsPoint(pinPoint) ) {
                        shrunkRectangle.expand( pinPoint );
                        addRectangle = true;
                        assert( shrunkRectangle.containsPoint(pinPoint) );
                    }
                }

                if( addRectangle ) {
                    rectangleSetShrunk.emplace_back( std::move(shrunkRectangle) );
                }
            }

            assert(rectangleSetShrunk.size() > 0);

            basicRectangles.swap(rectangleSetShrunk);

        }


		bool exists() const;
		bool valid() const;
		bool unique() const;
		bool lineFree() const;
		bool infFree() const;

		friend bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
        IsotheticPolygon &operator=(const IsotheticPolygon &other) = default;
		friend std::ostream& operator<<(std::ostream &os, const IsotheticPolygon &polygon);
};

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);

class InlineBoundedIsotheticPolygon {
	public:

        InlineBoundedIsotheticPolygon() :
            summary_rectangle_(
                    Point(std::numeric_limits<double>::infinity()),
                    Point(-std::numeric_limits<double>::infinity())
            )
        {
            rectangle_count_ = 0;
        }

        InlineBoundedIsotheticPolygon( const Rectangle &rect ) {
            rectangle_count_ = 1;
            basicRectangles[0] = rect;
            summary_rectangle_ = basicRectangles[0];
        }

        InlineBoundedIsotheticPolygon( const
                InlineBoundedIsotheticPolygon &other ) = default;

        InlineBoundedIsotheticPolygon &operator=( const
                InlineBoundedIsotheticPolygon &other ) = default;

    
        using iterator = Rectangle *;
        using const_iterator = Rectangle const *;

        iterator begin() {
            return basicRectangles.begin();
        }

        iterator end() {
            return basicRectangles.begin() + rectangle_count_;
        }

        const_iterator begin() const {
            return basicRectangles.cbegin();
        }

        const_iterator end() const {
            return basicRectangles.cbegin() + rectangle_count_;
        }

        double area() const {
            double area = 0.0;

            for (const Rectangle &basicRectangle : basicRectangles)
            {
                area += basicRectangle.area();
            }

            return area;
        }

        IsotheticPolygon materialize_polygon() {
            IsotheticPolygon polygon;
            std::copy( this->begin(), this->end(),
                    std::back_inserter(polygon.basicRectangles) );
            polygon.boundingBox = summary_rectangle_;
            return polygon;
        }

        void push_polygon_to_disk( const IsotheticPolygon &polygon ) {
            assert( polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT );
            std::copy( polygon.basicRectangles.begin(),
                    polygon.basicRectangles.end(),
                    basicRectangles.begin() );
            rectangle_count_ = polygon.basicRectangles.size();
            summary_rectangle_ = polygon.boundingBox;
        }

        unsigned get_rectangle_count() const {
            return rectangle_count_;
        }

        Rectangle &get_summary_rectangle() {
            return summary_rectangle_;

        }

        bool containsPoint( const Point &p ) {
            if( not summary_rectangle_.containsPoint( p ) ) {
                return false;
            }
            for( auto iter = begin(); iter != end(); iter++ ) {
                if( iter->containsPoint( p ) ) {
                    return true;
                }
            }
            return false;
        }

        bool intersectsRectangle( const Rectangle &rect ) {
            if( not summary_rectangle_.intersectsRectangle( rect ) ) {
                return false;
            }
            for( auto iter = begin(); iter != end(); iter++ ) {
                if( iter->intersectsRectangle( rect ) ) {
                    return true;
                }
            }
            return false;
        }

		friend bool operator==(const InlineBoundedIsotheticPolygon &lhs,
                const InlineBoundedIsotheticPolygon &rhs);
		friend bool operator!=(const InlineBoundedIsotheticPolygon&lhs,
                const InlineBoundedIsotheticPolygon &rhs);

private:
        unsigned rectangle_count_;
        Rectangle summary_rectangle_;
		std::array<Rectangle, MAX_RECTANGLE_COUNT> basicRectangles;


};

bool operator==(const InlineBoundedIsotheticPolygon &lhs, const
        InlineBoundedIsotheticPolygon &rhs);

bool operator!=(const InlineBoundedIsotheticPolygon &lhs, const
        InlineBoundedIsotheticPolygon &rhs);

struct PageableIsotheticPolygon {

    // Rectangles present in this polygon chunk
    unsigned rectangle_count_;

    // The logical pointer to the next chunk of polygon data
    tree_node_handle next_;

    // Flexible array member
    Rectangle basicRectangles[1];

    static constexpr size_t get_max_rectangle_count_per_page() {
        return ((PAGE_DATA_SIZE -
                sizeof(PageableIsotheticPolygon))/sizeof(Rectangle))+1;
    }

    static constexpr size_t compute_node_size( unsigned rect_count ) {
        return sizeof(PageableIsotheticPolygon) + (rect_count-1) *
            sizeof(Rectangle);
    }

};


// DO NOT MATERIALIZE ON THE STACK
class InlineUnboundedIsotheticPolygon {
	public:

        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            // Meaningless, do not compare
            using difference_type = void;
            using value_type = Rectangle;
            using pointer = Rectangle *;
            using reference = Rectangle &;

            Iterator( InlineUnboundedIsotheticPolygon *outer_poly,
                    tree_node_allocator *allocator ) :
                outer_poly_( outer_poly ),
                allocator_( allocator ),
                poly_pin_( allocator->buffer_pool_, nullptr, nullptr ),
                cur_poly_depth_( 0 ),
                cur_poly_offset_( 0 )
                {
                    at_end_ = (outer_poly_->poly_data_.rectangle_count_ == 0 );
                
                }

            reference operator*() const {
                if( cur_poly_depth_ == 0 ) {
                    return
                        outer_poly_->poly_data_.basicRectangles[cur_poly_offset_];
                }
                // Otherwise, something needs to be pinned
                assert( poly_pin_ != nullptr );
                return
                    poly_pin_->basicRectangles[cur_poly_offset_];
            }

            pointer operator->() {
                if( cur_poly_depth_ == 0 ) {
                    return
                        &(outer_poly_->poly_data_.basicRectangles[cur_poly_offset_]);
                }
                // Otherwise, something needs to be pinned
                assert( poly_pin_ != nullptr );
                return
                    &(poly_pin_->basicRectangles[cur_poly_offset_]);

            }

            Iterator &operator++() {
                if( at_end_ ) {
                    return *this;
                }
                cur_poly_offset_++;
                if( cur_poly_depth_ == 0 ) {
                    // If we are still on the same page, carry on
                    if( cur_poly_offset_ < 
                            outer_poly_->poly_data_.rectangle_count_ ) {
                        return *this;
                    }
                    // Overflowed the current page, read next
                    if( !outer_poly_->poly_data_.next_ ) {
                        at_end_ = true;
                        return *this;
                    }

                    poly_pin_ =
                        allocator_->get_tree_node<PageableIsotheticPolygon>(
                                outer_poly_->poly_data_.next_ );

                    cur_poly_depth_++;
                    cur_poly_offset_ = 0;

                    return *this;
                }

                // Still on same poly page
                if( cur_poly_offset_ < poly_pin_->rectangle_count_ ) {
                    return *this;
                }

                if( !poly_pin_->next_ ) {
                    at_end_ = true;

                    return *this;
                }

                poly_pin_ =
                    allocator_->get_tree_node<PageableIsotheticPolygon>(
                            poly_pin_->next_ );
                cur_poly_depth_++;
                cur_poly_offset_ = 0;

                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(
                const Iterator &a,
                const Iterator &b 
            ) {
                if( a.at_end_ or b.at_end_ ) {
                    return a.at_end_ and b.at_end_;
                }
                return a.outer_poly_ == b.outer_poly_ and 
                        a.cur_poly_depth_ == b.cur_poly_depth_ and
                    a.cur_poly_offset_ == b.cur_poly_offset_;
            }
            friend bool operator!=(
                const Iterator &a,
                const Iterator &b 
            ) {
                return !(a == b);
            }

            InlineUnboundedIsotheticPolygon *outer_poly_;
            tree_node_allocator *allocator_;
            pinned_node_ptr<PageableIsotheticPolygon> poly_pin_;

            unsigned cur_poly_depth_;
            unsigned cur_poly_offset_;
            bool at_end_;
        };


        InlineUnboundedIsotheticPolygon(
                tree_node_allocator *allocator,
                unsigned max_rectangle_count_on_first_page ) :
            max_rectangle_count_on_first_page_(
                    max_rectangle_count_on_first_page ),
            total_rectangle_count_( 0 ),
            cur_overflow_pages_( 0 ),
            allocator_( allocator )
        {
            poly_data_.next_ = tree_node_handle( nullptr );
            poly_data_.rectangle_count_ = 0;
        }

        Iterator begin() {
            return Iterator( this, allocator_ );
        }

        Iterator end() {
            Iterator iter( this, allocator_ );
            iter.at_end_ = true;
            return iter;
        }

        double area() {
            double area = 0.0;

            for (auto it = begin(); it != end(); ++it)
            {
                Rectangle &basicRectangle = *it;
                area += basicRectangle.area();
            }

            return area;
        }

        IsotheticPolygon materialize_polygon() {
            // Copy all the sequentially arranged polygon stuff into an
            // in-memory buffer
            // Do operations there

            IsotheticPolygon polygon;
            for( auto &rectangle : *this ) {
                polygon.basicRectangles.push_back( rectangle );
            }

            polygon.boundingBox = Rectangle(Point::atInfinity,
                    Point::atNegInfinity);
            for( const auto &rectangle : polygon.basicRectangles ) {
                polygon.boundingBox.expand( rectangle );
            }

            return polygon;
        }

        void push_polygon_to_disk(
                const IsotheticPolygon &in_memory_polygon ) {
            size_t max_rectangles_per_page =
                PageableIsotheticPolygon::get_max_rectangle_count_per_page();

            summary_rectangle_ = in_memory_polygon.boundingBox;

            unsigned new_rectangle_count =
                in_memory_polygon.basicRectangles.size();
            unsigned copy_count = std::min( new_rectangle_count,
                     max_rectangle_count_on_first_page_ );
            std::copy( in_memory_polygon.basicRectangles.begin(),
                    in_memory_polygon.basicRectangles.begin() + copy_count,
                    std::begin( poly_data_.basicRectangles ) );
            poly_data_.rectangle_count_ = copy_count;

            if( copy_count == new_rectangle_count ) {
                total_rectangle_count_ = new_rectangle_count;
                poly_data_.next_ = tree_node_handle( nullptr );
                return;
            }

            assert( copy_count == max_rectangle_count_on_first_page_ );

            tree_node_handle next_poly_handle = poly_data_.next_;
            pinned_node_ptr<PageableIsotheticPolygon> poly_pin(
                    allocator_->buffer_pool_, nullptr, nullptr );

            while( copy_count < new_rectangle_count ) {
                //Check if we already have a page alloc'd
                if( next_poly_handle == nullptr ) {
                    auto alloc_data =
                        allocator_->create_new_tree_node<PageableIsotheticPolygon>(
                                PAGE_DATA_SIZE, NodeHandleType(0) );
                    new (&(*(alloc_data.first)))
                        PageableIsotheticPolygon();
                    if( poly_pin == nullptr ) {
                        poly_data_.next_ = alloc_data.second;
                    } else {
                        poly_pin->next_ = alloc_data.second;
                    }
                    poly_pin = alloc_data.first;
                    cur_overflow_pages_++;
                } else {
                    poly_pin = allocator_->get_tree_node<PageableIsotheticPolygon>(
                            next_poly_handle );
                    next_poly_handle = poly_pin->next_;
                }

                unsigned entries_to_copy = std::min( new_rectangle_count
                        - copy_count, (unsigned) max_rectangles_per_page );
                auto copy_loc =
                    in_memory_polygon.basicRectangles.begin() +
                    copy_count;
                for( unsigned i = 0; i < entries_to_copy; i++ ) {
                    poly_pin->basicRectangles[i] = *copy_loc;
                    copy_loc++;
                }
                copy_count += entries_to_copy;
                poly_pin->rectangle_count_ = entries_to_copy;
            }

            poly_pin->next_ = tree_node_handle( nullptr );
            total_rectangle_count_ = new_rectangle_count;
            assert( copy_count == total_rectangle_count_ );
        }

        void free_subpages( tree_node_allocator *allocator ) {
            /*
            tree_node_handle next_ptr = poly_data_.next_;
            while( next_ptr != nullptr ) {
                // Crab to get next ptr.
                auto next_pin =
                    allocator->get_tree_node<PageableIsotheticPolygon>( next_ptr );
                auto tmp_ptr = next_pin->next_;
                allocator->free( next_ptr, PAGE_DATA_SIZE );
                next_ptr = tmp_ptr;
            }
            */
        }

        unsigned get_total_rectangle_count() const {
            return total_rectangle_count_;
        }

        unsigned get_cur_overflow_pages() const {
            return cur_overflow_pages_;
        }

        unsigned get_max_rectangle_count_on_first_page() const {
            return max_rectangle_count_on_first_page_;
        }

        Rectangle &get_summary_rectangle() {
            return summary_rectangle_;
        }

        bool containsPoint( const Point &p ) {
            if( not summary_rectangle_.containsPoint( p ) ) {
                return false;
            }
            for( auto iter = begin(); iter != end(); iter++ ) {
                if( iter->containsPoint( p ) ) {
                    return true;
                }
            }
            return false;
        }

        bool intersectsRectangle( const Rectangle &rect ) {
            if( not summary_rectangle_.intersectsRectangle( rect ) ) {
                return false;
            }
            for( auto iter = begin(); iter != end(); iter++ ) {
                if( iter->intersectsRectangle( rect ) ) {
                    return true;
                }
            }
            return false;
        }

        static pinned_node_ptr<InlineUnboundedIsotheticPolygon> read_polygon_from_disk(
            tree_node_allocator *allocator,
            tree_node_handle poly_handle
        ) {
            auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                poly_handle
            );
            poly_pin->allocator_ = allocator;
            return poly_pin;
        }

        static constexpr size_t maximum_possible_rectangles_on_first_page() {
            size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
                    sizeof(InlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
                + 1;
            return max_rects_on_first_page;
        }

        size_t repack( char *buffer, unsigned cut_off_inline_rect_count, tree_node_allocator *new_allocator ) {
            // Doesn't need to be, but simplifies things for now
            assert( total_rectangle_count_ <=
                    maximum_possible_rectangles_on_first_page() );

            // Inline oversize polygon into repacked node directly.
            if( total_rectangle_count_ <= cut_off_inline_rect_count ) {
                size_t sz = 0;
                * (unsigned *) buffer = total_rectangle_count_;
                sz += sizeof( total_rectangle_count_ );
                for( auto iter = begin(); iter != end(); iter++ ) {
                    * (Rectangle *) (buffer + sz) = (*iter);
                    sz += sizeof( Rectangle );
                }
                return sz;
            }

            // Write polygon out of band. 

            // Write magic to signify out of band polygon
            size_t sz = 0;
            * (unsigned *) buffer =
                std::numeric_limits<unsigned>::max();
            sz += sizeof( unsigned );

            // Compute exact size needed to represent this out of band
            // polygon, allocate it using the new allocator.
            size_t precise_size_needed = sizeof(InlineUnboundedIsotheticPolygon) +
                    (total_rectangle_count_-1)*sizeof(Rectangle);
            auto alloc_data = new_allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                    precise_size_needed, NodeHandleType( 3 /* BIG POLYGON */ ) );

            // N.B., our current polygon can actually be
            // overprovisioned, so we use total_rectangle_count to count
            // down size.
            new (&(*alloc_data.first)) InlineUnboundedIsotheticPolygon(
                    new_allocator, total_rectangle_count_ );
            IsotheticPolygon cur_poly = materialize_polygon();
            alloc_data.first->push_polygon_to_disk( cur_poly );

            * (tree_node_handle *) (buffer + sz) = alloc_data.second;
            sz += sizeof(tree_node_handle);
            return sz;
        }

protected:
        // Total rectangle count across all of the polygons
        unsigned max_rectangle_count_on_first_page_;
        unsigned total_rectangle_count_;
        unsigned cur_overflow_pages_;
        tree_node_allocator *allocator_;
        Rectangle summary_rectangle_;

        // First page of polygon data, we'll have to chase pointers for
        // the rest
        // N.B., you must malloc this struct with the appropriate size
        // for the first polygon's data, the rest will be taken care of
        // by the constructor
        PageableIsotheticPolygon poly_data_;

};

constexpr unsigned compute_sizeof_inline_unbounded_polygon( unsigned num_rects ) { 
    return sizeof(InlineUnboundedIsotheticPolygon) +
        (num_rects-1)*sizeof(Rectangle);
}

// If this changes, then you need to change the remaining count in
// tree_node_allocator to match.
// 216 for 3
// 280 for 5
//FIXME broken
/*(
static_assert( compute_sizeof_inline_unbounded_polygon(
            MAX_RECTANGLE_COUNT + 1 ) == 272 );
            */


template <class InlinePolyType, class InlinePolygonIter>
IsotheticPolygon::OptimalExpansion computeExpansionArea(InlinePolyType &poly, InlinePolygonIter begin, InlinePolygonIter end, Point &givenPoint)
{
	// Early exit
	if (poly.containsPoint(givenPoint))
	{
		return {0, -1.0};
	}

	// Take the minimum expansion area, defaulting to the first rectangle in the worst case
	IsotheticPolygon::OptimalExpansion expansion = {0, std::numeric_limits<double>::infinity()};
	double evalArea;

    unsigned i = 0;
	for (auto it = begin; it != end; it++)
	{
		evalArea = it->computeExpansionArea(givenPoint);

		if( evalArea < expansion.area and it->area() != 0.0 ) {
			expansion.index = i;
			expansion.area = evalArea;
		}

        i++;
	}

	return expansion;
}

template <class InlinePolygonIter>
IsotheticPolygon::OptimalExpansion computeExpansionArea(InlinePolygonIter begin, InlinePolygonIter end, const Rectangle &givenRectangle)
{
	// Take the minimum expansion area
	IsotheticPolygon::OptimalExpansion expansion = {0, begin->computeExpansionArea(givenRectangle)};
	double evalArea;

    auto it = begin;
    ++it;
    unsigned i = 0;
	for (; it != end; ++it)
	{
		evalArea = it->computeExpansionArea(givenRectangle);

		if (evalArea < expansion.area)
		{
			expansion.index = i;
			expansion.area = evalArea;
		}
        i++;
	}

	return expansion;
}

typedef pinned_node_ptr<InlineUnboundedIsotheticPolygon> unbounded_poly_pin;
typedef std::variant<unbounded_poly_pin, InlineBoundedIsotheticPolygon> inline_poly;

static
std::pair<double, std::vector<IsotheticPolygon::OptimalExpansion>>
computeExpansionArea( const inline_poly &this_poly, const inline_poly &other_poly ) {
    std::vector<IsotheticPolygon::OptimalExpansion> expansions;
    double totalAreas = 0.0;

    bool this_poly_unbounded = std::holds_alternative<unbounded_poly_pin>(this_poly);
    bool other_poly_unbounded = std::holds_alternative<unbounded_poly_pin>(other_poly);


    if (other_poly_unbounded) {
        unbounded_poly_pin o_poly_pin = std::get<unbounded_poly_pin>(other_poly);
        for (auto it = o_poly_pin->begin(); it != o_poly_pin->end(); it++) {
            const Rectangle &rect = *it;

            IsotheticPolygon::OptimalExpansion exp;
            if (this_poly_unbounded) {
                unbounded_poly_pin t_poly_pin = std::get<unbounded_poly_pin>(this_poly);
                exp = computeExpansionArea<InlineUnboundedIsotheticPolygon::Iterator>( t_poly_pin->begin(), t_poly_pin->end(), rect );
            } else{
                InlineBoundedIsotheticPolygon t_poly = std::get<InlineBoundedIsotheticPolygon>(this_poly);
                exp = computeExpansionArea<InlineBoundedIsotheticPolygon::iterator>( t_poly.begin(), t_poly.end(), rect );
            }

            if( exp.area != 0.0 and exp.area != -1.0 ) {
                totalAreas += exp.area;
            }
            expansions.push_back( exp );
        }
    } else {
        InlineBoundedIsotheticPolygon o_poly = std::get<InlineBoundedIsotheticPolygon>(other_poly);

        for (auto it = o_poly.begin(); it != o_poly.end(); it++) {
            const Rectangle &rect = *it;

            IsotheticPolygon::OptimalExpansion exp;
            if (this_poly_unbounded) {
                unbounded_poly_pin t_poly_pin = std::get<unbounded_poly_pin>(this_poly);
                exp = computeExpansionArea<InlineUnboundedIsotheticPolygon::Iterator>( t_poly_pin->begin(), t_poly_pin->end(), rect );
            } else{
                InlineBoundedIsotheticPolygon t_poly = std::get<InlineBoundedIsotheticPolygon>(this_poly);
                exp = computeExpansionArea<InlineBoundedIsotheticPolygon::iterator>( t_poly.begin(), t_poly.end(), rect );
            }

            if( exp.area != 0.0 and exp.area != -1.0 ) {
                totalAreas += exp.area;
            }
            expansions.push_back( exp );
        }
    }

    return std::make_pair( totalAreas == 0.0 ? -1.0 : totalAreas, expansions );
}
