#include "io_3ds.h"

#include <sstream>

namespace urban
{
    namespace io
    {
        FileHandler<Lib3dsFile>::FileHandler(void) {}

        FileHandler<Lib3dsFile>::FileHandler(boost::filesystem::path _filepath, std::map<std::string, bool> _modes)
            : filepath(_filepath), modes(_modes) {}

        FileHandler<Lib3dsFile>::~FileHandler(void) {}

        std::vector<urban::ShadowMesh> FileHandler<Lib3dsFile>::read(void)
        {
            std::ostringstream error_message;
            std::vector<urban::ShadowMesh> meshes;
            
            if (modes["read"])
            {
                if (boost::filesystem::is_regular_file(filepath))
                {
                    file = lib3ds_file_load(filepath.string().c_str());
                    Lib3dsMesh *p_meshes = file->meshes;
                    while (p_meshes)
                    {
                        meshes.push_back(urban::ShadowMesh(p_meshes));
                        p_meshes = p_meshes->next;
                    }
                }
                else
                {
                    error_message << "This file: " << filepath.string() << " cannot be found! You should check the file path.";
                    boost::system::error_code ec(boost::system::errc::no_such_file_or_directory, boost::system::system_category());
                    throw boost::filesystem::filesystem_error(error_message.str(), ec);
                }
            }
            else
            {
                error_message << std::boolalpha << "The read mode is set to:" << modes["read"] << "! You should set it as follows: \'modes[\"read\"] = true\'";
                boost::system::error_code ec(boost::system::errc::io_error, boost::system::system_category());
                throw boost::filesystem::filesystem_error(error_message.str(), ec);
            }
            return meshes;
        }

        void FileHandler<Lib3dsFile>::write(std::vector<urban::ShadowMesh> meshes)
        {
            std::ostringstream error_message;

            if (modes["write"])
            {
                file = reinterpret_cast<Lib3dsFile*>(calloc(sizeof(Lib3dsFace), 1));
                file->meshes = meshes[0].to_3ds();
                Lib3dsMesh *current = file->meshes;
                size_t it(1);

                do
                {
                    current->next = meshes[it++].to_3ds();
                    current = current->next;
                } while(it<meshes.size());
              
                // std::for_each(
                //     std::begin(meshes),
                //     std::end(meshes),
                //     [&](urban::ShadowMesh mesh) {
                //         current = mesh.to_3ds();
                //         //current->next = current++;
                //         current = current->next;
                //     });
                lib3ds_file_save(file, filepath.string().c_str());
            }
            else
            {
                error_message << std::boolalpha << "The write mode is set to:" << modes["write"] << "! You should set it as follows: \'modes[\"write\"] = true\'";
                boost::system::error_code ec(boost::system::errc::io_error, boost::system::system_category());
                throw boost::filesystem::filesystem_error(error_message.str(), ec);
            }
        }
    }
}
