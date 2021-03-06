#pragma once

#include <shadow/mesh.h>

#include <io/Line/line.h>

#include <algorithms/io_algorithms.h>

#include <boost/range/combine.hpp>

#include <ostream>

#include <stdexcept>

#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>
#include <ios>


namespace city
{
    namespace io
    {
        /**
         * @ingroup io
         * @brief formats an output stream to the OFF
         */
        class Obj_stream
        {
        public:
            /**
            * Reference constructor
            * @param _ios reference to input/output stream
            * @see Obj_stream(std::ostream && _ios)
            * @see Obj_stream(Obj_stream & _ios)
            * @see Obj_stream(Obj_stream && _ios)
            */
            Obj_stream(std::iostream & _ios): ios(_ios) {}
            /**
            * Copy constructor
            * @param other reference to Adjacency stream
            * @see Obj_stream(std::iostream & _ios)
            * @see Obj_stream(std::iostream && _ios)
            * @see Obj_stream(Obj_stream && other)
            */
            Obj_stream(Obj_stream & other): ios(other.ios) {}
            /**
            * Defines operator<< for this stream.
            * @tparam T output value type
            * @param value value to output
            * @return reference to the Obj_stream
            */
            template<typename T>
            Obj_stream & operator <<(const T & value)
            {
                ios << value; 
                return *this;
            }

            /**
            * Defines operator>> for this stream.
            * @tparam T input value type
            * @param value value to input
            * @return reference to the Obj_stream
            */
            template<typename T>
            Obj_stream & operator >>(const T & value)
            {
                ios >> value; 
                return *this;
            }

            /**
            * Defines operator<< for this stream.
            * @param func function applied to input/output stream
            * @return reference to the Obj_stream
            */
            Obj_stream & operator <<(std::iostream& (*func)(std::iostream&) )
            {
                func(ios);
                return *this;
            }

            /** 
             * Writes Mesh to output obj stream.
             * @param meshes the Mesh to write
             * @return the obj output stream
             */
            Obj_stream & operator <<(std::vector<shadow::Mesh> const& meshes)
            {
                std::vector<std::size_t> shifts(meshes.size());
                std::transform(
                    std::begin(meshes),
                    std::prev(std::end(meshes)),
                    std::next(std::begin(shifts)),
                    [](shadow::Mesh const& mesh)
                    {
                        return mesh.points_size();
                    }
                );
                std::partial_sum(
                    std::begin(shifts),
                    std::end(shifts),
                    std::begin(shifts)
                );

                print_points(meshes);
                print_faces(meshes, shifts);

                return *this;
            }
            /** 
             * Reads Mesh from obj stream.
             * @param meshes the Mesh to write
             * @return the output stream
             */
            Obj_stream & operator >>(std::vector<shadow::Mesh> & meshes)
            {
                auto lines = parse();

                if (lines.empty())
                    throw std::out_of_range("The stream contains only comments and/or empty lines!");

                auto cursor = std::begin(lines);

                auto points = read_points(lines, cursor);
                
                meshes = read_objects(lines, cursor, points);

                return *this;
            }

        private:
            /** reference to a stream */
            std::iostream & ios;

            void print_points(std::vector<shadow::Mesh> const& meshes)
            {
                for(auto const& mesh : meshes)
                    print_mesh_points(mesh);
            }
            void print_mesh_points(shadow::Mesh const& mesh)
            {
                std::for_each(
                    mesh.points_cbegin(),
                    mesh.points_cend(),
                    [this](shadow::Point const& points)
                    {
                        ios << "v " << points << std::endl;
                    }
                );
            }
            void print_faces(std::vector<shadow::Mesh> const& meshes, std::vector<std::size_t> const& shifts)
            {
                for(auto const& mesh_shift : boost::combine(meshes, shifts))
                    print_mesh_faces(mesh_shift.get<0>(), mesh_shift.get<1>());
            }
            void print_mesh_faces(shadow::Mesh const& mesh, std::size_t const shift)
            {
                ios << std::endl;
                ios << "o " << mesh.get_name() << std::endl;

                std::for_each(
                    mesh.faces_cbegin(),
                    mesh.faces_cend(),
                    [this, shift](shadow::Face const& facet)
                    {
                        ios << "f ";
                        for(auto const index: facet)
                            ios << index + shift + 1 << " ";
                        ios << std::endl;
                    }
                );
            }

            std::deque<std::string> parse(void)
            {
                std::deque<std::string> lines;
                readlines(ios, std::back_inserter(lines));

                /*Ignore Comments*/
                lines.erase(
                    std::remove_if(
                        std::begin(lines),
                        std::end(lines),
                        [](std::string const& line)
                        {
                            return line.empty() || line.at(0) == '#';
                        }
                    ),
                    std::end(lines)
                );

                return lines;
            }

            std::vector<shadow::Point> read_points(std::deque<std::string> const& lines, std::deque<std::string>::iterator & cursor)
            {
                /*
                   It is initialized by the origin:
                    - it corresponds to the pivot
                    - and `obj' point indexes starts at 1.
                */
                std::vector<shadow::Point> points(1);
                points.reserve(lines.size() / 2);

                for(; cursor != std::end(lines) && cursor->front() == 'v'; ++cursor)
                    points.push_back(
                        ::city::line2pt(cursor->substr(1))
                    );

                return points;
            }

            std::vector<shadow::Mesh> read_objects(std::deque<std::string> const& lines, std::deque<std::string>::iterator & cursor, std::vector<shadow::Point> const& points)
            {
                auto objects = object_names(lines, cursor);

                std::vector<shadow::Mesh> meshes(objects.size());

                std::transform(
                    std::begin(objects),
                    std::end(objects),
                    std::begin(meshes),
                    [&lines, &points, this](std::pair<std::string, std::pair<std::size_t, size_t> > const& object_id)
                    {
                        return read_object(lines, points, object_id.first, object_id.second.first, object_id.second.second);
                    }
                );

                return meshes;
            }

            shadow::Mesh read_object(std::deque<std::string> const& lines, std::vector<shadow::Point> const& points, std::string const& name, std::size_t const index, std::size_t const number_of_facets)
            {
                std::map<std::size_t, std::size_t> index_map;
                std::vector<shadow::Face> facets = read_facets(lines, index, number_of_facets, index_map);

                return shadow::Mesh(name, ::city::select(points, index_map), facets);
            }

            std::vector<shadow::Face> read_facets(std::deque<std::string> const& lines, std::size_t const index, std::size_t const number_of_facets, std::map<std::size_t, std::size_t> & index_map)
            {
                std::vector<shadow::Face> object_faces(number_of_facets);

                std::transform(
                    std::next(std::begin(lines), static_cast<long>(index)),
                    std::next(std::begin(lines), static_cast<long>(index) + static_cast<long>(number_of_facets)),
                    std::begin(object_faces),
                    [&index_map, this](std::string const& facet_line)
                    {
                        return read_facet(facet_line.substr(1), index_map);
                    }
                );

                return object_faces;
            }

            shadow::Face read_facet(std::string const& line, std::map<std::size_t, std::size_t> & index_map)
            {
                std::istringstream buffer_line(line);

                std::deque<std::size_t> indexes;

                std::copy(
                    std::istream_iterator<double>(buffer_line),
                    std::istream_iterator<double>(),
                    std::back_inserter(indexes)
                );

                auto maped_index = index_map.size();
                for(auto index : indexes)
                    if(!static_cast<bool>(index_map.count(index)))
                        index_map.emplace(std::make_pair(index, maped_index++));

                std::vector<std::size_t> new_indexes(indexes.size());
                std::transform(
                    std::begin(indexes),
                    std::end(indexes),
                    std::begin(new_indexes),
                    [&index_map](std::size_t const index)
                    {
                        return index_map[index];
                    }
                );
                return shadow::Face(new_indexes);
            }

            std::map<std::string, std::pair<std::size_t, std::size_t> > object_names(std::deque<std::string> const& lines, std::deque<std::string>::iterator & cursor)
            {
                std::map<std::string, std::pair<std::size_t, std::size_t> > objects;

                std::stringstream buffer;

                std::deque<std::string> names;
                std::deque<std::size_t> indexes;
                for(; cursor != std::end(lines); ++cursor)
                {
                    if(cursor->front() == 'o')
                    {
                        buffer.str(cursor->substr(1));
                        std::string name("");
                        buffer >> name;
                        names.push_back(name);
                        indexes.push_back(
                            static_cast<std::size_t>(
                                std::distance<std::deque<std::string>::const_iterator>(
                                    std::begin(lines),
                                    cursor
                                )
                            ) + 1
                        );
                        buffer.clear();
                    }
                }
                indexes.push_back(lines.size() + 1);

                std::vector< std::size_t > sizes(indexes.size());

                std::adjacent_difference(
                    indexes.rbegin(),
                    indexes.rend(),
                    sizes.rbegin(),
                    [](std::size_t const lhs, std::size_t const rhs)
                    {
                        return rhs - lhs -1;
                    }
                );

                sizes.pop_back();
                indexes.pop_back();

                for(auto const& name_size_index : boost::combine(names, sizes, indexes))
                    objects.emplace( 
                        std::make_pair(
                            name_size_index.get<0>(),
                            std::make_pair(
                                name_size_index.get<2>(),
                                name_size_index.get<1>()
                            )
                        )
                    );
                    
                return objects;
            }
        };
    }
}
