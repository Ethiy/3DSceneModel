#include "../UrbanObject/Mesh/mesh.h"

#include "catch.hpp"

#include <initializer_list>
#include <iterator>
#include <algorithm>

SCENARIO("Mesh manipulation:")
{
    GIVEN( "A Lib3ds mesh:")
    {
        Lib3dsMesh* test_mesh = reinterpret_cast<Lib3dsMesh*>(calloc(sizeof(Lib3dsMesh),1));
        test_mesh->faces = 1;
        auto init0 = std::initializer_list<Lib3dsWord>({0, 1, 2});
        test_mesh->faceL = reinterpret_cast<Lib3dsFace*>(calloc(sizeof(Lib3dsFace),3));
        std::copy(std::begin(init0), std::end(init0), (test_mesh->faceL)->points);
        test_mesh->points = 3;
        test_mesh->pointL = reinterpret_cast<Lib3dsPoint*>(calloc(sizeof(Lib3dsPoint),3));
        auto init1 = std::initializer_list<float>({15.5343f, -13.4504f, 60.8789f});
        std::copy(std::begin(init1), std::end(init1), (test_mesh->pointL)->pos);
        init1 = std::initializer_list<float>({15.7204f, -13.188f, 60.8789f});
        std::copy(std::begin(init1), std::end(init1), (test_mesh->pointL + 1)->pos) ;
        init1 = std::initializer_list<float>({15.7204f, -13.188f, 61.1764f});
        std::copy(std::begin(init1), std::end(init1), (test_mesh->pointL + 2)->pos);

        WHEN( "the mesh is created:")
        {
            urban::Mesh u_mesh(*test_mesh);
            THEN("the output checks:")
            {
                std::ostringstream auxilary;
                auxilary << u_mesh;
                REQUIRE( auxilary.str() == "Points: \nPoint 0 : 15.5343 -13.4504 60.8789\nPoint 1 : 15.7204 -13.188 60.8789\nPoint 2 : 15.7204 -13.188 61.1764\nTriangles: \nTriangle 0 : 0 2 1 \n" );
            }
        }
        WHEN( "mesh points and triangles are accessed:")
        {
            urban::Mesh u_mesh(*test_mesh);
            std::map<size_t, urban::Point> points = u_mesh.get_points();
            std::map<size_t, urban::Triangle> triangles = u_mesh.get_triangles();
            THEN("the output checks:")
            {
                std::ostringstream auxilary, _auxilary;
                auxilary << u_mesh;
                _auxilary << "Points: " << std::endl;
                std::for_each(std::begin(points), std::end(points), [&](std::pair<size_t, urban::Point> p)
                                                                                {
                                                                                    _auxilary << "Point " << p.first << " : " << p.second << std::endl;
                                                                                }
                            );
                _auxilary << "Triangles: " << std::endl;
                std::for_each(std::begin(triangles), std::end(triangles), [&](std::pair<size_t, urban::Triangle> t)
                                                                                        {
                                                                                            _auxilary << "Triangle " << t.first << " : " << t.second << std::endl;
                                                                                        }
                            );
                REQUIRE( auxilary.str() == _auxilary.str() );
            }
        }
        WHEN( "Mesh is converted back to lib3ds format and to \'urban::Mesh\' again: ")
        {
            urban::Mesh u_mesh(*test_mesh);
            THEN("the output checks:")
            {
                std::ostringstream auxilary;
                Lib3dsMesh* _mesh = u_mesh.to_3ds();
                urban::Mesh _u_mesh(*_mesh);
                auxilary << _u_mesh;
                REQUIRE( auxilary.str() == "Points: \nPoint 0 : 15.5343 -13.4504 60.8789\nPoint 1 : 15.7204 -13.188 60.8789\nPoint 2 : 15.7204 -13.188 61.1764\nTriangles: \nTriangle 0 : 0 1 2 \n" );
            }
        }
    }
}
