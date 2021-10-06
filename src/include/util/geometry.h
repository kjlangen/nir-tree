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

#ifndef __GEOMETRY__
#define __GEOMETRY__

#include <iostream>
#include <algorithm>
#include <queue>
#include <cassert>
#include <vector>
#include <cmath>
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
		const double operator[](unsigned index) const;
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

		Point lowerLeft;
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
		bool strictIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool containsPoint(const Point &givenPoint) const;
		bool strictContainsPoint(const Point &givenPoint) const;
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

        void shrink(const std::vector<Point> &pinPoints);

		bool exists() const;
		bool valid() const;
		bool unique() const;
		bool lineFree() const;
		bool infFree() const;

		friend bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend std::ostream& operator<<(std::ostream &os, const IsotheticPolygon &polygon);
};

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);

class DiskBackedIsotheticPolygon {
public:
    virtual IsotheticPolygon materialize_polygon() = 0;
    virtual void push_polygon_to_disk( IsotheticPolygon &polygon ) = 0;
};

#define MAX_RECTANGLE_COUNT 5 
class InlineBoundedIsotheticPolygon : DiskBackedIsotheticPolygon {
	public:

        InlineBoundedIsotheticPolygon() {
            rectangle_count_ = 0;
        }

        InlineBoundedIsotheticPolygon( const
                InlineBoundedIsotheticPolygon &other ) = default;

        InlineBoundedIsotheticPolygon &operator=( const
                InlineBoundedIsotheticPolygon &other ) = default;

        unsigned rectangle_count_;
		std::array<Rectangle, MAX_RECTANGLE_COUNT> basicRectangles;

    
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

        IsotheticPolygon materialize_polygon() override {
            IsotheticPolygon polygon;
            std::copy( this->begin(), this->end(),
                    std::back_inserter(polygon.basicRectangles) );
            polygon.boundingBox = Rectangle(Point::atInfinity,
                    Point::atNegInfinity);
            for( const auto &rectangle : polygon.basicRectangles ) {
                polygon.boundingBox.expand( rectangle );
            }

            return polygon;
        }

        void push_polygon_to_disk( IsotheticPolygon &polygon ) override {
            assert( polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT );
            std::copy( polygon.basicRectangles.begin(),
                    polygon.basicRectangles.end(),
                    basicRectangles.begin() );
            rectangle_count_ = polygon.basicRectangles.size();
        }

};

bool operator==(const InlineBoundedIsotheticPolygon &lhs, const InlineBoundedIsotheticPolygon &rhs);
bool operator!=(const InlineBoundedIsotheticPolygon &lhs, const InlineBoundedIsotheticPolygon &rhs);

struct PageableIsotheticPolygon {

    // Rectangles present in this polygon chunk
    unsigned rectangle_count_;

    // The logical pointer to the next chunk of polygon data
    tree_node_handle next_;

    // Flexible array member
    Rectangle basicRectangles[1];

    static size_t get_max_rectangle_count_per_page() {
        return ((PAGE_DATA_SIZE -
                sizeof(PageableIsotheticPolygon))/sizeof(Rectangle))+1;
    }

    static size_t compute_node_size( unsigned rect_count ) {
        return sizeof(PageableIsotheticPolygon) + (rect_count-1) *
            sizeof(Rectangle);
    }

};

// DO NOT MATERIALIZE ON THE STACK
class InlineUnboundedIsotheticPolygon : public DiskBackedIsotheticPolygon {
	public:

        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            // Meaningless, do not compare
            using difference_type = std::ptrdiff_t;
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
                cur_poly_offset_++;
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

        // Total rectangle count across all of the polygons
        unsigned total_rectangle_count_;
        unsigned cur_overflow_pages_;
        tree_node_allocator *allocator_;

        // First page of polygon data, we'll have to chase pointers for
        // the rest
        // N.B., you must malloc this struct with the appropriate size
        // for the first polygon's data, the rest will be taken care of
        // by the constructor
        PageableIsotheticPolygon poly_data_;

        InlineUnboundedIsotheticPolygon( tree_node_allocator *allocator ) {
            poly_data_.next_ = tree_node_handle( nullptr );
            poly_data_.rectangle_count_ = 0;
            total_rectangle_count_ = 0;
            cur_overflow_pages_ = 0;
            allocator_ = allocator;
        }

        Iterator begin() {
            return Iterator( this, allocator_ );
        }

        Iterator end() {
            Iterator iter( this, allocator_ );
            iter.at_end_ = true;
            return iter;
        }

        IsotheticPolygon materialize_polygon() override {
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

        void push_polygon_to_disk( IsotheticPolygon &in_memory_polygon ) override {

            size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
                    sizeof(InlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
                + 1;

            size_t max_rectangles_per_page =
                PageableIsotheticPolygon::get_max_rectangle_count_per_page();


            // We are always at least one page.
            unsigned new_rectangle_count =
                in_memory_polygon.basicRectangles.size();
            unsigned copy_count = std::min( new_rectangle_count,
                    (unsigned) max_rects_on_first_page);
            std::copy( in_memory_polygon.basicRectangles.begin(),
                    in_memory_polygon.basicRectangles.begin() + copy_count,
                    std::begin( poly_data_.basicRectangles ) );
            poly_data_.rectangle_count_ = copy_count;

            if( copy_count == new_rectangle_count ) {
                total_rectangle_count_ = new_rectangle_count;
                poly_data_.next_ = tree_node_handle( nullptr );
                return;
            }

            tree_node_handle next_poly_handle = poly_data_.next_;
            pinned_node_ptr<PageableIsotheticPolygon> poly_pin(
                    allocator_->buffer_pool_, nullptr, nullptr );

            while( copy_count < new_rectangle_count ) {
                //Check if we already have a page alloc'd
                if( next_poly_handle == nullptr ) {
                    auto alloc_data =
                        allocator_->create_new_tree_node<PageableIsotheticPolygon>(
                                PAGE_DATA_SIZE );
                    new (&(*(alloc_data.first)))
                        PageableIsotheticPolygon();
                    next_poly_handle = alloc_data.second;
                    if( poly_pin == nullptr ) {
                        poly_data_.next_ = next_poly_handle;
                    } else {
                        poly_pin->next_ = next_poly_handle;
                    }
                    poly_pin = alloc_data.first;
                    next_poly_handle = tree_node_handle(nullptr);
                } else {
                    poly_pin = allocator_->get_tree_node<PageableIsotheticPolygon>(
                            next_poly_handle );
                    cur_overflow_pages_++;
                    next_poly_handle = poly_pin->next_;
                }

                unsigned entries_to_copy = std::min( new_rectangle_count
                        - copy_count, (unsigned) max_rectangles_per_page );
                std::copy( in_memory_polygon.basicRectangles.begin() + copy_count,
                        in_memory_polygon.basicRectangles.begin() + copy_count +
                        entries_to_copy, std::begin(
                            poly_pin->basicRectangles ) );
                copy_count += entries_to_copy;
                poly_pin->rectangle_count_ = entries_to_copy;
            }

            poly_pin->next_ = tree_node_handle( nullptr );
            total_rectangle_count_ = new_rectangle_count;

        }
};

unsigned compute_sizeof_inline_unbounded_polygon( unsigned num_rects ); 

#endif
