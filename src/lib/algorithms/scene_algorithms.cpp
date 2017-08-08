#include <algorithms/scene_algorithms.h>

#include <io/Adjacency_stream/adjacency_stream.h>
#include <io/io_gdal.h>

namespace urban
{
    void save_building_duals(boost::filesystem::path const& root_path, scene::Scene const& scene)
    {
        std::cout << "Saving brick duals... " << std::flush;
        boost::filesystem::path dual_dir(root_path / "dual_graphs");
        boost::filesystem::create_directory(dual_dir);
        for(auto const& building : scene)
        {
            std::fstream adjacency_file(
                boost::filesystem::path(dual_dir / (building.get_name() + ".txt")).string(),
                std::ios::out
            );
            io::Adjacency_stream as(adjacency_file);
            as << building;
        }
        std::cout << " Done." << std::flush << std::endl;
    }
    void save_building_prints(boost::filesystem::path const& root_path, std::vector<projection::FootPrint> const& projections, bool const labels)
    {
        std::cout << "Saving vector projections... " << std::flush;
        boost::filesystem::path vector_dir(root_path / "vectors");
        boost::filesystem::create_directory(vector_dir);
        for(auto const& projection : projections)
        {
            io::FileHandler<GDALDriver>(
                io::GdalFormat::gml,
                boost::filesystem::path(vector_dir / (projection.get_name() + ".gml")),
                std::map<std::string,bool>{{"write", true}}
            ).write(projection);
        }
        if(labels)
        {
            boost::filesystem::path label_dir(root_path / "labels");
            boost::filesystem::create_directory(label_dir);
            for(auto const& projection : projections)
            {
                io::FileHandler<GDALDriver>(
                    io::GdalFormat::shapefile,
                    boost::filesystem::path(label_dir / (projection.get_name() + ".shp")),
                    std::map<std::string,bool>{{"write", true}}
                ).write(projection, true);
            }
        }
        std::cout << "Done." << std::flush << std::endl;
    }
    void save_building_rasters(boost::filesystem::path const& root_path, std::vector<projection::RasterPrint> const& raster_projections)
    {
        std::cout << "Saving raster projections... " << std::flush;
        boost::filesystem::path raster_dir(root_path / "rasters");
        boost::filesystem::create_directory(raster_dir);
        for(auto const& rasta : raster_projections)
        {
            urban::io::FileHandler<GDALDriver>(
                urban::io::GdalFormat::geotiff,
                boost::filesystem::path(raster_dir / (rasta.get_name() + ".tiff")),
                std::map<std::string,bool>{{"write", true}}
            ).write(rasta);
        }
        std::cout << "Done." << std::flush << std::endl;
    }
    void save_scene_prints(boost::filesystem::path const& root_path, std::string const& filename, std::vector<projection::FootPrint> const& projections, bool const rasterize, double const pixel_size)
    {
        std::cout << "Summing , rasterizing and saving scene projections... " << std::flush;

        auto scene_projection = std::accumulate(
            std::begin(projections),
            std::end(projections),
            urban::projection::FootPrint()
        );

        urban::io::FileHandler<GDALDriver>(
            urban::io::GdalFormat::gml,
            boost::filesystem::path(root_path / (filename + ".gml")),
            std::map<std::string,bool>{{"write", true}}
        ).write(scene_projection);

        if(rasterize)
        {
            urban::projection::RasterPrint global_rasta(scene_projection, pixel_size);

            urban::io::FileHandler<GDALDriver>(
                urban::io::GdalFormat::geotiff,
                boost::filesystem::path(root_path / (filename + ".tiff")),
                std::map<std::string,bool>{{"write", true}}
            ).write(global_rasta);
        }

        std::cout << "Done." << std::flush << std::endl;        
    }

    std::vector<projection::FootPrint> orthoproject(scene::Scene const& scene)
    {
        std::cout << "Projecting... " << std::flush;
        std::vector<projection::FootPrint> ortho_projections(scene.building_size());
        std::transform(
            std::begin(scene),
            std::end(scene),
            std::begin(ortho_projections),
            [](scene::UNode const& building)
            {
                return projection::FootPrint(building);
            }
        );
        std::cout << "Done." << std::flush << std::endl;

        return ortho_projections;
    }
    std::vector<projection::RasterPrint> rasterize_scene(std::vector<projection::FootPrint> const& projections, double const  pixel_size)
    {
        std::cout << "rasterizing projections... " << std::flush;
        std::vector<projection::RasterPrint> raster_projections(projections.size());
        std::transform(
            std::begin(projections),
            std::end(projections),
            std::begin(raster_projections),
            [pixel_size](projection::FootPrint const& projection)
            {
                return projection::RasterPrint(projection, pixel_size);
            }
        );
        std::cout << "Done." << std::flush << std::endl;

        return raster_projections;
    }
}
