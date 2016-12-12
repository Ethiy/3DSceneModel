#include "../IO/io_off.h"

#include <boost/filesystem.hpp>

#include <string>
#include <fstream>
#include <streambuf>

#include "catch.hpp"

SCENARIO("Input/Output from 3dsMAX file:")
{
    GIVEN("An existing 3dsMAX file")
    {
        boost::filesystem::path filepath("../../ressources/3dModels/OFF/hammerhead.off");
        
        WHEN("the reading mode is chosen")
        {
            std::map<std::string,bool> modes{{"read", true}};
            urban::io::FileHandler<std::fstream> handler(filepath, modes);
            std::vector<urban::ShadowMesh> meshes;
            int exit_code = handler.read(meshes);

            THEN("the output checks")
            {
                std::ostringstream auxilary;
                std::copy(std::begin(meshes), std::end(meshes), std::ostream_iterator<urban::ShadowMesh>(auxilary, "\n"));

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE( auxilary.str() == tmp_str );
            }
        }
        
        WHEN("the reading mode is not chosen")
        {
            std::map<std::string,bool> modes;
            urban::io::FileHandler<std::fstream> handler(filepath, modes);
            std::vector<urban::ShadowMesh> meshes;
            int exit_code = handler.read(meshes);

            THEN("the output checks")
            {
                REQUIRE( exit_code == boost::system::errc::io_error );
            }
        }
    }
    
    GIVEN("A wrong file path")
    {
        boost::filesystem::path filepath("../ressources/3dModels/OFF/hammerhead.off");
        
        WHEN("the reading mode is chosen")
        {
            std::map<std::string,bool> modes{{"read", true}};
            urban::io::FileHandler<std::fstream> handler(filepath, modes);
            std::vector<urban::ShadowMesh> meshes;
            int exit_code = handler.read(meshes);

            THEN("the output checks")
            {
                REQUIRE( exit_code == boost::system::errc::no_such_file_or_directory );
            }
        }
        
        WHEN("the reading mode is not chosen")
        {
            std::map<std::string,bool> modes;
            urban::io::FileHandler<std::fstream> handler(filepath, modes);
            std::vector<urban::ShadowMesh> meshes;
            int exit_code = handler.read(meshes);

            THEN("the output checks")
            {
                REQUIRE( exit_code == boost::system::errc::io_error );
            }
        }
    }

    GIVEN("A A urban::ShadowMesh object")
    {
        boost::filesystem::path _filepath("../ressources/3dModels/OFF/hammerhead.off");
        std::map<std::string,bool> _modes{{"read", true}};
        urban::io::FileHandler<std::fstream> reader(_filepath, _modes);
        std::vector<urban::ShadowMesh> meshes;
        int exit_code = reader.read(meshes);

        WHEN("the writing mode is chosen")
        {
            boost::filesystem::path filepath("../../ressources/3dModels/OFF/hammerhead.off");
            std::map<std::string,bool> modes{{"write", true}};
            urban::io::FileHandler<std::fstream> writer(filepath, modes);
            exit_code = writer.write(meshes);

            std::map<std::string,bool> __modes{{"read", true}};
            urban::io::FileHandler<std::fstream> handler(_filepath, __modes);
            std::vector<urban::ShadowMesh> _meshes;
            exit_code = handler.read(_meshes);


            THEN("the output checks")
            {
                std::ostringstream auxilary;
                std::copy(std::begin(_meshes), std::end(_meshes), std::ostream_iterator<urban::ShadowMesh>(auxilary, "\n"));

                std::ifstream tmp("../../ressources/tests/hammerhead_shadow_mesh.txt");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                //REQUIRE( auxilary.str() == tmp_str );
            }
        }
        
        WHEN("the writing mode is not chosen")
        {
            //Not specified!! The writer is not of interest for now
        }
    }
}
