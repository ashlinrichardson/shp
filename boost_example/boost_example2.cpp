
#include <iostream>
#include <vector>
#include <boost/geometry/geometry.hpp>

using namespace boost::geometry;

int main(void)
{
    // Define a polygons and fill the outer rings.
    polygon_2d a;
    {
        const double c[][2] = {
            {160, 330}, {60, 260}, {20, 150}, {60, 40}, {190, 20}, {270, 130}, {260, 250}, {160, 330}
        };
        assign(a, c);
    }

}




