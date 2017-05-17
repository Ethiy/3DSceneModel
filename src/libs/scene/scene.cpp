#include "scene.h"

#include "../algorithms/utils/util_algorithms.h"

namespace urban
{
    /** 
     * Order a Vector of meshes by name
     * @param meshes vector of meshes.
     * @retrurn a map of name order meshes
     */
    std::map<std::string, urban::shadow::Mesh> order(std::vector<urban::shadow::Mesh> const& meshes);
    std::vector<urban::shadow::Mesh> select(std::set<std::string> const& names, std::map<std::string, urban::shadow::Mesh> const& ordered_meshes);

    namespace scene
    {
        Scene::Scene(void)
        {}
        Scene::Scene(urban::shadow::Point const& _pivot, unsigned short _epsg_code, std::map<std::size_t, BComposition> const& _structure)
            :pivot(_pivot), epsg_code(_epsg_code), structure(_structure)
        {}
        Scene::Scene(Scene const& other)
            :pivot(other.pivot), epsg_code(other.epsg_code), structure(other.structure)
        {}
        Scene::Scene(Scene && other)
            :pivot(std::move(other.pivot)), epsg_code(std::move(other.epsg_code)), structure(std::move(other.structure))
        {}
        Scene::~Scene(void)
        {}

        void Scene::swap(Scene & other)
        {
            using std::swap;
            swap(pivot, other.pivot);
            swap(epsg_code, other.epsg_code);
            swap(structure, other.structure);
        }

        Scene & Scene::operator=(Scene const& other)
        {
            pivot = other.pivot;
            epsg_code = other.epsg_code;
            structure = other.structure;

            return *this;
        }
        Scene & Scene::operator=(Scene && other)
        {
            pivot= std::move(other.pivot);
            epsg_code= std::move(other.epsg_code);
            structure= std::move(other.structure);

            return *this;
        }

        shadow::Point Scene::get_pivot(void) const noexcept
        {
            return pivot;
        }
        
        unsigned short Scene::get_epsg(void) const noexcept
        {
            return epsg_code;
        }

        std::vector<urban::shadow::Mesh> Scene::roofs(std::size_t identifier, std::vector<urban::shadow::Mesh> const& meshes) const
        {
            return roofs(identifier, order(meshes));
        }
        std::vector<urban::shadow::Mesh> Scene::roofs(std::size_t identifier, std::map<std::string, urban::shadow::Mesh> const& ordered_meshes) const
        {
            return select(structure.at(identifier).roofs, ordered_meshes);
        }
        std::vector<urban::shadow::Mesh> Scene::walls(std::size_t identifier, std::vector<urban::shadow::Mesh> const& meshes) const
        {
            return walls(identifier, order(meshes));
        }
        std::vector<urban::shadow::Mesh> Scene::walls(std::size_t identifier, std::map<std::string, urban::shadow::Mesh> const& ordered_meshes) const
        {
            return select(structure.at(identifier).walls, ordered_meshes);
        }

        std::map<std::size_t, std::pair<std::vector<urban::shadow::Mesh>, std::vector<urban::shadow::Mesh> > > Scene::cluster(std::vector<shadow::Mesh> const& meshes) const
        {
            std::map<std::size_t, std::pair<std::vector<urban::shadow::Mesh>, std::vector<urban::shadow::Mesh> > > buildings;

            auto ordered_meshes = order(meshes);

            for(auto const& _building : structure)
            {
                buildings.emplace(
                    _building.first,
                    std::make_pair(
                        roofs(_building.first, ordered_meshes),
                        walls(_building.first, ordered_meshes)
                    )
                );
            }

            return buildings;
        }

        void swap(Scene & lhs, Scene & rhs)
        {
            lhs.swap(rhs);
        }
    }

    std::map<std::string, urban::shadow::Mesh> order(std::vector<urban::shadow::Mesh> const& meshes)
    {
        std::map<std::string, urban::shadow::Mesh> ordered_meshes;
        for(auto const& mesh : meshes)
        {
            ordered_meshes.emplace(mesh.get_name(), mesh);
        }
        return ordered_meshes;
    }

    std::vector<urban::shadow::Mesh> select(std::set<std::string> const& names, std::map<std::string, urban::shadow::Mesh> const& ordered_meshes)
    {
        std::vector<urban::shadow::Mesh> selected;
        selected.reserve(names.size());
        for(auto const& mesh_name : names)
        {
            auto placeholder = ordered_meshes.find(mesh_name);
            if(placeholder != std::end(ordered_meshes))
                selected.push_back(placeholder->second);
        }
        return selected;
    }

}