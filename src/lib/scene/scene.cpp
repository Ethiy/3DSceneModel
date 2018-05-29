#include <scene/scene.h>

#include <algorithms/util_algorithms.h>
#include <projection/scene_projection.h>

namespace city
{
    namespace scene
    {
        Scene::Scene(void)
        {}
        Scene::Scene(
            std::vector<shadow::Mesh> const& building_meshes,
            shadow::Mesh const& terrain_mesh,
            Point_3 const& _pivot,
            unsigned short _epsg_index
        )
            : pivot(_pivot), epsg_index(_epsg_index), buildings(building_meshes.size())
        {
            std::transform(
                std::begin(building_meshes),
                std::end(building_meshes),
                std::begin(buildings),
                [this](shadow::Mesh const& building_mesh)
                {
                    return UNode(building_mesh, pivot, epsg_index);
                }
            );

            buildings.erase(
                std::remove_if(
                    std::begin(buildings),
                    std::end(buildings),
                    [](UNode const& building)
                    {
                        return building.empty();
                    }
                ),
                std::end(buildings)
            );

            terrain = UNode(terrain_mesh, pivot, epsg_index);
        }
        Scene::Scene(Scene const& other)
            : pivot(other.pivot), epsg_index(other.epsg_index), buildings(other.buildings), terrain(other.terrain)
        {}
        Scene::Scene(Scene && other)
            : pivot(std::move(other.pivot)), epsg_index(std::move(other.epsg_index)), buildings(std::move(other.buildings)), terrain(std::move(other.terrain))
        {}
        Scene::~Scene(void)
        {}

        void Scene::swap(Scene & other)
        {
            using std::swap;
            swap(pivot, other.pivot);
            swap(epsg_index, other.epsg_index);
            swap(buildings, other.buildings);
            swap(terrain, other.terrain);
        }

        Scene & Scene::operator =(Scene const& other)
        {
            pivot = other.pivot;
            epsg_index = other.epsg_index;
            buildings = other.buildings;
            terrain = other.terrain;
            
            return *this;
        }
        Scene & Scene::operator =(Scene && other)
        {
            pivot= std::move(other.pivot);
            epsg_index= std::move(other.epsg_index);
            buildings= std::move(other.buildings);
            terrain= std::move(other.terrain);
            
            return *this;
        }

        Point_3 Scene::get_pivot(void) const noexcept
        {
            return pivot;
        }
        unsigned short Scene::get_epsg(void) const noexcept
        {
            return epsg_index;
        }
        UNode const& Scene::get_terrain(void) const noexcept
        {
            return terrain;
        }
        std::vector<UNode> const& Scene::all_buildings(void) const noexcept
        {
            return buildings;
        }
        Bbox_3 Scene::bbox(void) const
        {
            return std::accumulate(
                std::begin(buildings),
                std::end(buildings),
                terrain.bbox(),
                [](Bbox_3 const& bb, scene::UNode const& building)
                {
                    return bb + building.bbox();
                }
            );
        }
        std::vector<std::string> Scene::get_identifiers(void) const
        {
            std::vector<std::string> ids(buildings.size());
            std::transform(
                std::begin(buildings),
                std::end(buildings),
                std::begin(ids),
                [](UNode const& _building)
                {
                    return _building.get_name();
                }
            );
            return ids;
        }


        Scene::iterator Scene::begin(void) noexcept
        {
            return buildings.begin();
        }
        Scene::iterator Scene::end(void) noexcept
        {
            return buildings.end();
        }
        Scene::const_iterator Scene::begin(void) const noexcept
        {
            return buildings.begin();
        }
        Scene::const_iterator Scene::end(void) const noexcept
        {
            return buildings.end();
        }
        Scene::const_iterator Scene::cbegin(void) const noexcept
        {
            return buildings.cbegin();
        }
        Scene::const_iterator Scene::cend(void) const noexcept
        {
            return buildings.cend();
        }

        std::size_t Scene::size(void) const noexcept
        {
            return buildings.size();
        }

        void swap(Scene & lhs, Scene & rhs)
        {
            lhs.swap(rhs);
        }

        Scene& Scene::prune(bool const _terrain)
        {
            std::transform(
                std::begin(buildings),
                std::end(buildings),
                std::begin(buildings),
                [](scene::UNode & building)
                {
                    try
                    {
                        return building.prune();
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                        return building;
                    }
                }
            );
            if(_terrain)
                terrain = terrain.prune();

            return *this;
        }

        std::vector<projection::FootPrint> Scene::orthoproject(bool const _terrain) const
        {
            std::cout << "Projecting... " << std::flush;
            std::vector<projection::FootPrint> ortho_projections(buildings.size() + static_cast<std::size_t>(_terrain));
            std::transform(
                std::begin(buildings),
                std::end(buildings),
                std::begin(ortho_projections),
                [](UNode const& building)
                {
                    return projection::FootPrint(building);
                }
            );
            ortho_projections.erase(
                std::remove_if(
                    std::begin(ortho_projections),
                    std::end(ortho_projections),
                    [](projection::FootPrint const& ortho_projection)
                    {
                        return ortho_projection.empty();
                    }
                ),
                std::end(ortho_projections)
            );
            if(_terrain)
                ortho_projections[buildings.size()] = projection::FootPrint(terrain);
            
            std::cout << "Done." << std::flush << std::endl;

            return ortho_projections;
        }

        bool Scene::empty(void) const noexcept
        {
            return buildings.empty() && terrain.empty();
        }

        Scene& Scene::operator +=(Scene const& other)
        {
            if(empty())
            {
                operator =(other);
            }
            else
            {
                if(epsg_index != other.epsg_index)
                    throw std::logic_error("Not yet implemented");

                buildings.insert(std::begin(buildings), std::begin(other.buildings), std::end(other.buildings));
                
                std::transform(
                    std::begin(buildings),
                    std::next(
                        std::begin(buildings),
                        static_cast<std::vector<UNode>::iterator::difference_type>(other.buildings.size())
                    ),
                    std::begin(buildings),
                    [this, &other](UNode & building)
                    {
                        return translate(building, other.pivot -  pivot);
                    }
                );
                auto o_terrain = other.terrain;
                terrain = UNode(
                    (
                    shadow::Mesh(terrain)
                    +
                    shadow::Mesh(
                            translate(o_terrain, other.pivot -  pivot)
                        )
                    ).set_name("terrain"),
                    terrain.get_reference_point(),
                    terrain.get_epsg()
                );
            }
            return *this;
        }

        Scene operator +(Scene const& lhs, Scene const& rhs)
        {
            auto result = lhs;
            return result += rhs;
        }
    }

    scene::Scene & prune(scene::Scene & scene, bool const terrain)
    {
        return scene.prune(terrain);
    }
}
