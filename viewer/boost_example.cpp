/*
Copyright 2008 Intel Corporation

Use, modification and distribution are subject to the Boost Software License,
Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt).
*/
#include <boost/polygon/polygon.hpp>
#include <cassert>
namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

int main() {
    //lets construct a 10x10 rectangle shaped polygon
    typedef gtl::polygon_data<int> Polygon;
    typedef gtl::polygon_traits<Polygon>::point_type Point;
    Point pts[] = {gtl::construct<Point>(0, 0),
    gtl::construct<Point>(10, 0),
    gtl::construct<Point>(10, 10),
    gtl::construct<Point>(0, 10) };
    Polygon poly;
    gtl::set_points(poly, pts, pts+4);

    //now lets see what we can do with this polygon
    assert(gtl::area(poly) == 100.0f);
    assert(gtl::contains(poly, gtl::construct<Point>(5, 5)));
    assert(!gtl::contains(poly, gtl::construct<Point>(15, 5)));
    gtl::rectangle_data<int> rect;
    assert(gtl::extents(rect, poly)); //get bounding box of poly
    assert(gtl::equivalence(rect, poly)); //hey, that's slick
    assert(gtl::winding(poly) == gtl::COUNTERCLOCKWISE);
    assert(gtl::perimeter(poly) == 40.0f);

    //add 5 to all coords of poly
    gtl::convolve(poly, gtl::construct<Point>(5, 5));
    //multiply all coords of poly by 2
    gtl::scale_up(poly, 2);
    gtl::set_points(rect, gtl::point_data<int>(10, 10),
    gtl::point_data<int>(30, 30));
    assert(gtl::equivalence(poly, rect));
    return 0;
}




