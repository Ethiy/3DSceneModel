#pragma once

#include "../geometry_definitions.h"

#include <vector>
#include <utility>

namespace urban
{
    class FaceProjection : Polygon
    {
    public:
        using Polygon::Polygon;

        FaceProjection(const FaceProjection &);

        Plane get_plane(void) const noexcept;
        Vector get_normal(void) const noexcept;
        double get_height(const Point_2 &);

        bool is_degenerate(void) const;

        bool contains(const Point_2 &);

        void set_plane(const Plane & _plane) noexcept;
    private:
        Plane original_plane;
    };
}