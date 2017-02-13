#include "shadow_mesh.h"

#include <stdexcept>

#include <iterator>
#include <algorithm>
#include <numeric>

namespace urban
{
    namespace shadow
    {
        void Mesh::compute_box(void)
        {
            auto xmin_itr = std::min_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.x()) < to_double(p2.second.x());
                }
            );

            auto xmax_itr = std::max_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.x()) < to_double(p2.second.x());
                }
            );

            auto ymin_itr = std::min_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.y()) < to_double(p2.second.y());
                }
            );

            auto ymax_itr = std::max_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.y()) < to_double(p2.second.y());
                }
            );

            auto zmin_itr = std::min_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.z()) < to_double(p2.second.z());
                }
            );

            auto zmax_itr = std::max_element(
                std::begin(points),
                std::end(points),
                [](const std::pair<size_t, Point> & p1, const std::pair<size_t, Point>& p2)
                {
                    return to_double(p1.second.z()) < to_double(p2.second.z());
                }
            );

            bounding_box = Bbox(to_double(xmin_itr->second.x()), to_double(xmax_itr->second.x()), to_double(ymin_itr->second.y()), to_double(ymax_itr->second.y()), to_double(zmin_itr->second.z()), to_double(zmax_itr->second.z()));
        }

        Mesh::Mesh(void):name("N/A"){}
        Mesh::Mesh(const Mesh & other):name(other.name), points(other.points), faces(other.faces), bounding_box(other.bounding_box){}
        Mesh::Mesh(Mesh && other):name(std::move(other.name)), points(std::move(other.points)), faces(std::move(other.faces)), bounding_box(std::move(other.bounding_box)){}
        Mesh::Mesh(Lib3dsMesh* lib3ds_mesh):name(lib3ds_mesh->name)
        {
            size_t it(0);
            std::for_each(
                lib3ds_mesh->pointL,
                lib3ds_mesh->pointL + lib3ds_mesh->points,
                [&](Lib3dsPoint _point)
                {
                    points[it++] = Point(_point.pos[0], _point.pos[1], _point.pos[2]);
                }
            );

            it = 0;
            std::for_each(
                lib3ds_mesh->faceL,
                lib3ds_mesh->faceL + lib3ds_mesh->faces,
                [&](Lib3dsFace _face)
                {
                    Point point_0 = points[_face.points[0]];
                    Point point_1 = points[_face.points[1]];
                    Point point_2 = points[_face.points[2]];

                    Vector v1 = Vector(point_0, point_1);
                    Vector v2 = Vector(point_1, point_2);
                    Vector n = Vector(_face.normal[0], _face.normal[1], _face.normal[2]);

                    if(CGAL::determinant(v1, v2, n)>0)
                        faces.emplace(std::make_pair(it++, std::move(Face(_face.points[0], _face.points[1], _face.points[2]))));
                    else
                        faces.emplace(std::make_pair(it++, std::move(Face(_face.points[0], _face.points[2], _face.points[1]))));
                }
            );
            compute_box();
        }

        Mesh::Mesh(const Polyhedron & polyhedron)
        {
            size_t it(0);
            std::for_each(
                polyhedron.points_begin(),
                polyhedron.points_end(),
                [&](const Point & point)
                {
                    points.emplace(std::make_pair(it++, point));
                }
            );

            it = 0;
            std::for_each(
                polyhedron.facets_begin(),
                polyhedron.facets_end(),
                [&](const Facet & facet)
                {
                    size_t face_degree(facet.facet_degree());
                    std::vector<size_t> face_points(face_degree);
                    face_points[0] = get_index(*(facet.facet_begin()));
                    std::transform(
                        std::next(facet.facet_begin(), 1),
                        std::next(facet.facet_begin(), face_degree),
                        std::next(std::begin(face_points), 1),
                        [&](const Polyhedron::Halfedge & halfedge)
                        {
                            return get_index(halfedge);
                        }
                    );
                    faces.emplace(std::make_pair(it++, std::move(Face(face_degree, face_points))));
                }
            );
            compute_box();
        }

        Mesh::Mesh(std::string _name, std::map<size_t, Point>_points, std::map<size_t, Face> _faces):name(_name), points(_points), faces(_faces)
        {
            compute_box();
        }
        Mesh::~Mesh(void){}

        void Mesh::swap(Mesh & other)
        {
            using std::swap;
            swap(name, other.name);
            swap(points, other.points);
            swap(faces, other.faces);
            swap(bounding_box, other.bounding_box);
        }

        Mesh & Mesh::operator=(const Mesh & other) noexcept
        {
            name = other.name;
            points = other.points;
            faces = other.faces;
            bounding_box = bounding_box;
            return *this;
        }

        Mesh & Mesh::operator=(Mesh && other) noexcept
        {
            name = std::move(other.name);
            points = std::move(other.points);
            faces = std::move(other.faces);
            bounding_box = std::move(bounding_box);
            return *this;
        }

        std::string Mesh::get_name(void) const noexcept
        {
            return name;
        }

        size_t Mesh::get_number_points(void) const noexcept
        {
            return points.size();
        }

        std::map<size_t, Point> Mesh::get_points(void) const noexcept
        {
            return points;
        }
        
        size_t Mesh::get_number_faces(void) const noexcept
        {
            return faces.size();
        }

        std::map<size_t, Face> Mesh::get_faces(void) const noexcept
        {
            return faces;
        }

        Bbox Mesh::bbox(void) const noexcept
        {
            return bounding_box;
        }

        size_t Mesh::get_index(const Polyhedron::Halfedge & halfedge)
        {
            size_t index;
            auto point_handle = std::find_if(
                std::begin(points),
                std::end(points),
                [&](const std::pair<size_t, Point> & p)
                {
                    return p.second == halfedge.vertex()->point();
                }
            );
            if(point_handle!= std::end(points))
                index = point_handle->first;
            else
                throw std::out_of_range("The face contains a non listed point");
            return index;
        }


        Lib3dsMesh* Mesh::to_3ds()
        {
            Lib3dsMesh* mesh = reinterpret_cast<Lib3dsMesh*>(calloc(sizeof(Lib3dsMesh), 1));
            strcpy(mesh->name, const_cast<char*>(name.c_str()));
            mesh->points = static_cast<Lib3dsWord>(points.size());
            mesh->pointL = reinterpret_cast<Lib3dsPoint*>(calloc(sizeof(Lib3dsPoint), mesh->points));

            std::transform(
                std::begin(points),
                std::end(points),
                mesh->pointL,
                [&](std::pair<size_t, Point> p)
                {
                    Lib3dsPoint point;
                    auto init = std::initializer_list<double>({to_double(p.second.x()), to_double(p.second.y()), to_double(p.second.z())});
                    std::copy(std::begin(init), std::end(init), point.pos);
                    return point;
                }
            );

            mesh->faces = static_cast<Lib3dsDword>(
                std::accumulate(
                    std::begin(faces), 
                    std::end(faces),
                    0,
                    [](int& size, const std::pair<const size_t, Face> & second)
                        {
                            return size + static_cast<int>(second.second.size()) - 2;
                        }
                    )
                );

            mesh->faceL = reinterpret_cast<Lib3dsFace*>(calloc(sizeof(Lib3dsFace), mesh->faces));

            std::transform(
                std::begin(faces),
                std::end(faces),
                mesh->faceL,
                [this](std::pair<size_t, Face> t)
                {
                    return *t.second.to_3ds(points);
                }
            );

            mesh->next = NULL;

            return mesh;
        }

        std::ostream& operator<<(std::ostream &os, const Mesh & mesh)
        {
            os << "Name: " << mesh.name << std::endl
            << "Points: " << std::endl;

            std::for_each(
                std::begin(mesh.points),
                std::end(mesh.points),
                [&](std::pair<size_t, Point> p)
                {
                    os << "Point " << p.first << " : " << to_double(p.second.x()) << " " << to_double(p.second.y()) << " " << to_double(p.second.z()) << std::endl;
                }
            );

            os << "Faces: " << std::endl;

            std::for_each(
                std::begin(mesh.faces),
                std::end(mesh.faces),
                [&](std::pair<size_t, Face> t)
                {
                    os << "Face " << t.first << " : " << t.second << std::endl;
                }
            );

            return os;
        }
    }

    void swap(shadow::Mesh & lhs, shadow::Mesh & rhs)
    {
        lhs.swap(rhs);
    }
}
