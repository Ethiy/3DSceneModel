#pragma once

#include "urban_object.h"

#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <algorithm>
#include <iterator>


namespace urban
{
    template <class HDS>
    class SurfaceBuilder: public CGAL::Modifier_base<HDS>
    {
    public:
        SurfaceBuilder(std::vector<Lib3dsFace> mesh_faces, std::map<size_t,Point> mesh_points): faces(mesh_faces), points(mesh_points){}
        void operator()(HDS & target)
        {
            CGAL::Polyhedron_incremental_builder_3<HDS> incremental_builder( target, true);
            incremental_builder.begin_surface(points.size(), faces.size());
            std::for_each( std::begin(points), std::end(points), [&](std::pair<size_t, Point> p)
                                                                    {
                                                                        incremental_builder.add_vertex( p.second);
                                                                    }
                        );
            std::for_each( std::begin(faces), std::end(faces), [&](Lib3dsFace face)
                                                                {
                                                                    Point point_0 = points[face.points[0]];
                                                                    Point point_1 = points[face.points[1]];
                                                                    Point point_2 = points[face.points[2]];

                                                                    Vector v1 = Vector(point_0, point_1);
                                                                    Vector v2 = Vector(point_1, point_2);
                                                                    Vector n = Vector(face.normal[0], face.normal[1], face.normal[2]);

                                                                    std::cout << CGAL::determinant(v1, v2, n) << std::endl;

                                                                    incremental_builder.begin_facet();
                                                                    if(CGAL::determinant(v1, v2, n)>0)
                                                                    {
                                                                        incremental_builder.add_vertex_to_facet( face.points[0]);
                                                                        incremental_builder.add_vertex_to_facet( face.points[1]);
                                                                        incremental_builder.add_vertex_to_facet( face.points[2]);
                                                                    }
                                                                    else
                                                                    {
                                                                        incremental_builder.add_vertex_to_facet( face.points[0]);
                                                                        incremental_builder.add_vertex_to_facet( face.points[2]);
                                                                        incremental_builder.add_vertex_to_facet( face.points[1]);
                                                                    }
                                                                    incremental_builder.end_facet();
                                                                }
                                                                );
            incremental_builder.end_surface();
        }
    private:
        std::vector<Lib3dsFace> faces;
        std::map<size_t,Point> points;
    };
}
