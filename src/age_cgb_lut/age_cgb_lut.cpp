//
// Copyright 2021 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <gfx/age_png.hpp>

#include <filesystem>
#include <iostream>



namespace
{
    age::pixel_vector read_lut_png(const std::filesystem::path& lut_png_path)
    {
        if (!std::filesystem::is_regular_file(lut_png_path))
        {
            return {};
        }

        FILE* file = fopen(lut_png_path.string().c_str(), "rb");
        if (!file)
        {
            return {};
        }
        age::pixel_vector result = age::read_png_file(file, 1024, 32);
        fclose(file);
        return result;
    }

} // namespace



int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "usage: " << argv[0] << " lut_file out_file" << std::endl;
        return 1;
    }

    // read the lut png
    std::filesystem::path lut_png_path = std::filesystem::absolute(argv[1]);
    auto color_lut = read_lut_png(lut_png_path);
    if (color_lut.empty())
    {
        std::cout << "could not read lookup table from: " << lut_png_path << std::endl;
        return 1;
    }
    std::cout << "read lookup table from: " << lut_png_path << std::endl;

    //! \todo create lookup table code
    return 0;
}
