#include "brick_algorithms.h"
#include "../utils/util_algorithms.h"

#include <CGAL/aff_transformation_tags.h>

#include <stdexcept>

#include <iterator>
#include <algorithm>
#include <numeric>

#include <cmath>

namespace urban
{
    double border_length(Brick & brick)
    {
        return std::accumulate(
            brick.border_halfedges_begin(),
            brick.halfedges_end(),
            .0,
            [](double & length, const Polyhedron::Halfedge & halfedge)
            {
                return length + std::sqrt(to_double(Vector_3(halfedge.next()->vertex()->point(), halfedge.vertex()->point()) * Vector_3(halfedge.next()->vertex()->point(), halfedge.vertex()->point())));
            }
        );
    }

    Brick & affine_transform(Brick & brick, const Affine_transformation_3 & affine_transformation)
    {
        std::transform(
            brick.points_begin(),
            brick.points_end(),
            brick.points_begin(),
            [& affine_transformation](Point_3 & point)
            {
                return affine_transformation.transform(point);
            }
        );
        return brick;
    }

    Brick & translate(Brick & brick, const Vector_3 & offset)
    {
        Affine_transformation_3 translation(CGAL::TRANSLATION, offset);
        return affine_transform(brick, translation);
    }

    Brick & scale(Brick & brick, double scale)
    {
        Affine_transformation_3 scaling(CGAL::SCALING, scale);
        return affine_transform(brick, scaling);
    }

    Brick & rotate(Brick & brick, const Vector_3 & axis, double angle)
    {
        std::map<double, Vector_3> _rotation{{angle, axis}};
        Affine_transformation_3 rotation(rotation_transform(_rotation));
        return affine_transform(brick, rotation);
    }

    Brick & rotate(Brick & brick, const std::map<double, Vector_3> & _rotations)
    {
        Affine_transformation_3 rotation(rotation_transform(_rotations));
        return affine_transform(brick, rotation);
    }


    void plane_equations(Brick& brick)
    {
        std::transform(
            brick.facets_begin(),
            brick.facets_end(),
            brick.planes_begin(),
            [](Brick::Facet & facet)
            {
                Brick::Halfedge_handle halfedge = facet.halfedge();
                return Brick::Facet::Plane_3(
                    halfedge->vertex()->point(),
                    halfedge->next()->vertex()->point(),
                    halfedge->next()->next()->vertex()->point()
                );
            }
        );
    }

    Brick & prune(Brick & brick)
    {
        std::vector<Brick::Halfedge_handle> prunables = brick.pruning_halfedges();
        for(auto prunable : prunables)
        {
            brick = brick.join_facet(prunable);
        }
        return brick;
    }

    double area(Brick& brick)
    {
        return std::accumulate(
            brick.facets_begin(),
            brick.facets_end(),
            .0,
            [&brick](double & area, Brick::Facet & facet)
            {
                return area + brick.area(facet);
            }
        );
    }
}