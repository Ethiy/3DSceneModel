#include "face.h"

#include <boost/range/combine.hpp>
#include <boost/range/sub_range.hpp>
#include <boost/foreach.hpp>

#include <algorithm>
#include <iterator>

namespace urban
{
    Face::Face(void){}
    Face::Face(const Face & other):points(other.points){}
    Face::Face(size_t first, size_t second, size_t third): points{{first, second, third}}{}
    Face::~Face(void){}

    void Face::swap(Face & other)
    {
        using std::swap;
        swap(vertices_number, other.vertices_number);
        swap(points, other.points);
    }

    Face & Face::operator=(Face other)
    {
        other.swap(*this);
        return *this;
    }

    size_t Face::operator[](size_t index)
    {
        return points[index];
    }

    std::vector<size_t> Face::get_indexes(void)
    {
        return points;
    }
    
    size_t Face::size(void)
    {
        return vertices_number;
    }

    void Face::invert_orientation(void)
    {
        std::vector<size_t> aux(vertices_number-1);
        std::reverse_copy( std::next(std::begin(points), 1), std::end(points), std::begin(aux));
        std::copy(std::begin(aux), std::end(aux), std::next(std::begin(points), 1) );
    }

    Lib3dsFace* Face::to_3ds()
    {
        Lib3dsFace* face = reinterpret_cast<Lib3dsFace*>(calloc(sizeof(Lib3dsFace), vertices_number-2));
        {
            std::vector<size_t> twos , threes;
            std::copy(std::next(std::begin(points), 1), std::prev(std::begin(points), 1), std::begin(twos));
            std::copy(std::next(std::begin(points), 2), std::end(points), std::begin(threes));
            size_t two,three;
            BOOST_FOREACH( boost::tie(two, three), boost::combine(twos, threes))
            {
                auto init = std::initializer_list<size_t>({points.at(0), two, three});
                std::copy(std::begin(init), std::end(init), face->points);;
            }
        }
        return face;
    }

    std::ostream& operator<<(std::ostream & os, const Face & face)
    {
        os << face.vertices_number << " ";
        std::copy(std::begin(face.points), std::end(face.points), std::ostream_iterator<size_t>(os, " "));
        return os;
    }

    void swap(Face & lhs, Face &rhs)
    {
        lhs.swap(rhs);
    }
}
