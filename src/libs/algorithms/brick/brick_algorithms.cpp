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
            [](Facet & facet)
            {
                Facet::Halfedge_handle halfedge = facet.halfedge();
                return Facet::Plane_3(
                    halfedge->vertex()->point(),
                    halfedge->next()->vertex()->point(),
                    halfedge->next()->next()->vertex()->point()
                );
            }
        );
    }

    Brick prune(Brick & brick)
    {
        Brick::Halfedge_iterator h_it = brick.halfedges_end();
        bool prunable = brick.joinable(h_it);

        Brick cooling_recipient;
        size_t pair(0);

        while(prunable)
        {
            std::cout << h_it->facet()->plane() << std::endl;
            Brick::Halfedge_handle h = h_it->opposite()->opposite();
            pair ++;
            switch(pair%2)
            {
                case 1:
                    cooling_recipient = brick.join_facet(h);
                    h_it = cooling_recipient.halfedges_end();
                    std::cout << "Cooling" << std::endl;
                    prunable = cooling_recipient.joinable(h_it);
                    std::cout << std::boolalpha << prunable << std::endl;
                    break;
                default:
                    brick = cooling_recipient.join_facet(h);
                    h_it = brick.halfedges_end();
                    prunable = brick.joinable(h_it);
                    std::cout << std::boolalpha << prunable << std::endl;
                    break;
            }
        }
        
        if(pair%2)
            brick = cooling_recipient;
        return brick;
    }

    double area(Brick& brick)
    {
        return std::accumulate(
            brick.facets_begin(),
            brick.facets_end(),
            .0,
            [](double & area, Facet & facet)
            {
                Polyhedron::Halfedge_around_facet_circulator h = facet.facet_begin();
                Vector_3 normal = CGAL::normal(h->vertex()->point(), h->next()->vertex()->point(), h->next()->next()->vertex()->point());
                return area 
                + to_double(CGAL::cross_product(h->vertex()->point() - CGAL::ORIGIN, h->next()->vertex()->point() - CGAL::ORIGIN) * normal/2.)
                + std::accumulate(
                    std::next(facet.facet_begin(), 1),
                    std::next(facet.facet_begin(), static_cast<long>(facet.facet_degree())),
                    .0,
                    [normal](double & surface_area, const Polyhedron::Halfedge & halfedge)
                    {
                        return surface_area + to_double(CGAL::cross_product(halfedge.vertex()->point() - CGAL::ORIGIN, halfedge.next()->vertex()->point() - CGAL::ORIGIN) * normal/2.);
                    }
                );
            }
        );
    }
}
