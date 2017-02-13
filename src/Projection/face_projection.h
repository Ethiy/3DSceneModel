#pragma once

#include "../geometry_definitions.h"

#include <vector>
#include <utility>

namespace urban
{
    class FaceProjection
    {
    public:
        FaceProjection(void);
        FaceProjection(const Polygon_with_holes & _projected_polygon, const Plane & _supporting_plane);
        FaceProjection(const FaceProjection & other);
        FaceProjection(FaceProjection && other);
        ~FaceProjection(void);

        void swap(FaceProjection & other);

        FaceProjection & operator=(const FaceProjection & other);

        FaceProjection & operator=(FaceProjection && other);

        Polygon_with_holes get_polygon(void) const noexcept;
        Plane get_plane(void) const noexcept;
        Vector get_normal(void) const noexcept;

        double get_plane_height(const Point_2 &) const;
        double get_height(const Point_2 &) const;

        typedef Polygon_with_holes::Hole_const_iterator Hole_const_iterator;
        Hole_const_iterator holes_begin(void) const;
        Hole_const_iterator holes_end(void) const;
        Polygon outer_boundary(void) const;
        
        /*! In our case, two edges are coinciding means that all edges are so:
         *  - Due to the fact that all edges are supported by the same plane;
         *  - This means that we should just check if the area is null to assert
         *      that the projection is degenarate.
         */
        bool is_degenerate(void) const;
        bool is_perpendicular(void) const;

        bool contains(const Point_2 & point) const;

    private:
        Polygon_with_holes projected_polygon;
        Plane supporting_plane;
    };

    void swap(FaceProjection & lhs, FaceProjection & rhs);
}
