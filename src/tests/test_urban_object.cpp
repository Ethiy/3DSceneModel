#include <scene/unode.h>
#include <io/io_3ds.h>

#include <boost/filesystem.hpp>

#include <string>
#include <fstream>

#include <catch.hpp>

SCENARIO("Urban Node manipulation:")
{
    GIVEN("A 3ds file")
    {
        auto staff_mesh = city::io::T3DSHandler(
            boost::filesystem::path("../../ressources/3dModels/3DS/Toy/Toy Santa Claus N180816.3DS"),
            std::map<std::string,bool>{{"read", true}}
        ).mesh("Staff", std::set<char>{'S'});

        WHEN("the \"buildings\" are read into unodes")
        {
            city::scene::UNode staff(staff_mesh, city::shadow::Point(), 0);

            THEN("the output checks")
            {
                std::ostringstream auxilary;
                auxilary << staff;

                std::ifstream tmp("../../ressources/tests/staff_unstitched.off");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE( auxilary.str() == tmp_str );
            }
        }
        WHEN("the \"buildings\" are read into unodes and stitched")
        {
            city::scene::UNode staff(staff_mesh, city::shadow::Point(), 0);

            THEN("the output checks")
            {
                std::ostringstream auxilary;
                auxilary << staff.stitch_borders();

                std::ifstream tmp("../../ressources/tests/staff.off");
                std::string tmp_str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
                REQUIRE( auxilary.str() == tmp_str );
            }
        }
    }
}
