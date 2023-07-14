//
// © 2020 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_gb_lcd_palettes.hpp"



namespace
{
    void set_colors(std::array<age::pixel, 4>& colors, int color00, int color01, int color10, int color11)
    {
        colors = {age::pixel(color00), age::pixel(color01), age::pixel(color10), age::pixel(color11)};
    }

    bool is_nintendo_rom(std::span<age::uint8_t const> rom_header)
    {
        int licensee = rom_header[0x14B];
        if (licensee == 0x33)
        {
            return (rom_header[0x144] == '0') && (rom_header[0x145] == '1');
        }
        return licensee == 0x01;
    }

    //!
    //! Initialize Game Boy Color palettes for classic roms.
    //! Based on:
    //! https://tcrf.net/Notes:Game_Boy_Color_Bootstrap_ROM#Assigned_Palette_Configurations
    //! https://web.archive.org/web/20170830061747/http://www.vcfed.org/forum/showthread.php?19247-Disassembling-the-GBC-Boot-ROM&p=128734
    //!
    void init_dmg_colors(std::array<age::pixel, 4>&    bgp,
                         std::array<age::pixel, 4>&    obp0,
                         std::array<age::pixel, 4>&    obp1,
                         std::span<age::uint8_t const> rom_header)
    {
        uint8_t rom_name_hash = 0;
        if (is_nintendo_rom(rom_header))
        {
            for (int i = 0x134; i <= 0x143; ++i)
            {
                rom_name_hash += rom_header[i];
            }
        }
        age::uint8_t rom_name_char4 = rom_header[0x134 + 3];

        switch (rom_name_hash)
        {
            case 0x00: // dummy entry: no Nintendo rom
                break;

            case 0x01:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x0C:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x0D:
                if (rom_name_char4 == 0x45)
                {
                    set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                    set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x52)
                {
                    set_colors(bgp, 0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000);
                    obp0 = bgp;
                    set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                    return;
                }
                break;

            case 0x10:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x14:
                set_colors(bgp, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                obp1 = bgp;
                return;

            case 0x15:
                set_colors(bgp, 0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x16:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x17:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0x18:
                if (rom_name_char4 == 0x49)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = obp0;
                    return;
                }
                if (rom_name_char4 == 0x4B)
                {
                    set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                    set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                    set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                    return;
                }
                break;

            case 0x19:
                set_colors(bgp, 0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                obp1 = bgp;
                return;

            case 0x1D:
                set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                set_colors(obp0, 0xFF6352, 0xD60000, 0x630000, 0x000000);
                obp1 = bgp;
                return;

            case 0x27:
                if (rom_name_char4 == 0x42)
                {
                    set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                    set_colors(obp0, 0xFF6352, 0xD60000, 0x630000, 0x000000);
                    set_colors(obp1, 0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF);
                    return;
                }
                if (rom_name_char4 == 0x4E)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    return;
                }
                break;

            case 0x28:
                if (rom_name_char4 == 0x41)
                {
                    set_colors(bgp, 0x000000, 0x008484, 0xFFDE00, 0xFFFFFF);
                    obp0 = bgp;
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x46)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x4A0000);
                    obp1 = obp0;
                    return;
                }
                break;

            case 0x29:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x34:
                set_colors(bgp, 0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                obp1 = obp0;
                return;

            case 0x35:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x36:
                set_colors(bgp, 0x52DE00, 0xFF8400, 0xFFFF00, 0xFFFFFF);
                set_colors(obp0, 0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF);
                set_colors(obp1, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                return;

            case 0x39:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x3C:
                set_colors(bgp, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                return;

            case 0x3D:
                set_colors(bgp, 0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                obp1 = bgp;
                return;

            case 0x3E:
                set_colors(bgp, 0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0x3F:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                obp1 = obp0;
                return;

            case 0x43:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x46:
                if (rom_name_char4 == 0x45)
                {
                    set_colors(bgp, 0xB5B5FF, 0xFFFF94, 0xAD5A42, 0x000000);
                    set_colors(obp0, 0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A);
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x52)
                {
                    set_colors(bgp, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    set_colors(obp0, 0xFFFF00, 0xFF0000, 0x630000, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                    return;
                }
                break;

            case 0x49:
                set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                set_colors(obp0, 0xFF6352, 0xD60000, 0x630000, 0x000000);
                set_colors(obp1, 0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF);
                return;

            case 0x4B:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x4E:
                set_colors(bgp, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0xFFFF7B, 0x0084FF, 0xFF0000);
                return;

            case 0x52:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x58:
                set_colors(bgp, 0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x59:
                set_colors(bgp, 0xFFFFFF, 0xADAD84, 0x42737B, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF7300, 0x944200, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0x5C:
                set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                set_colors(obp0, 0xFF6352, 0xD60000, 0x630000, 0x000000);
                set_colors(obp1, 0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF);
                return;

            case 0x5D:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x61:
                if (rom_name_char4 == 0x41)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    return;
                }
                if (rom_name_char4 == 0x45)
                {
                    set_colors(bgp, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = bgp;
                    return;
                }
                break;

            case 0x66:
                if (rom_name_char4 == 0x45)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = obp0;
                    return;
                }
                if (rom_name_char4 == 0x4C)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = obp0;
                    return;
                }
                break;

            case 0x67:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x68:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x69:
                set_colors(bgp, 0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0x6A:
                if (rom_name_char4 == 0x49)
                {
                    set_colors(bgp, 0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x4B)
                {
                    set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                    set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                    set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                    return;
                }
                break;

            case 0x6B:
                set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0x6D:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0x6F:
                set_colors(bgp, 0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x70:
                set_colors(bgp, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x00FF00, 0x318400, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0x71:
                set_colors(bgp, 0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x75:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x86:
                set_colors(bgp, 0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A);
                set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                set_colors(obp1, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                return;

            case 0x88:
                set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x8B:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0x8C:
                set_colors(bgp, 0xFFFFFF, 0xADAD84, 0x42737B, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF7300, 0x944200, 0x000000);
                obp1 = bgp;
                return;

            case 0x90:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x92:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x95:
                set_colors(bgp, 0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0x97:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x99:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0x9A:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x4A0000);
                obp1 = obp0;
                return;

            case 0x9C:
                set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                return;

            case 0x9D:
                set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                return;

            case 0xA2:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0xA5:
                if (rom_name_char4 == 0x41)
                {
                    set_colors(bgp, 0x000000, 0x008484, 0xFFDE00, 0xFFFFFF);
                    obp0 = bgp;
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x52)
                {
                    set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0x7BFF31, 0x008400, 0x4A0000);
                    obp1 = obp0;
                    return;
                }
                break;

            case 0xA8:
                set_colors(bgp, 0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A);
                set_colors(obp0, 0xFFC542, 0xFFD600, 0x943A00, 0x4A0000);
                set_colors(obp1, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                return;

            case 0xAA:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                obp1 = bgp;
                return;

            case 0xB3:
                if (rom_name_char4 == 0x42)
                {
                    set_colors(bgp, 0xA59CFF, 0xFFFF00, 0x006300, 0x000000);
                    set_colors(obp0, 0xFF6352, 0xD60000, 0x630000, 0x000000);
                    set_colors(obp1, 0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF);
                    return;
                }
                if (rom_name_char4 == 0x52)
                {
                    set_colors(bgp, 0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000);
                    obp0 = bgp;
                    set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                    return;
                }
                if (rom_name_char4 == 0x55)
                {
                    set_colors(bgp, 0xFFFFFF, 0xADAD84, 0x42737B, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF7300, 0x944200, 0x4A0000);
                    obp1 = obp0;
                    return;
                }
                break;

            case 0xB7:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0xBD:
                set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x4A0000);
                obp1 = obp0;
                return;

            case 0xBF:
                if (rom_name_char4 == 0x20)
                {
                    set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = bgp;
                    return;
                }
                if (rom_name_char4 == 0x43)
                {
                    set_colors(bgp, 0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF);
                    set_colors(obp1, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                    return;
                }
                break;

            case 0xC6:
                if (rom_name_char4 == 0x20)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = obp0;
                    return;
                }
                if (rom_name_char4 == 0x41)
                {
                    set_colors(bgp, 0xFFFFFF, 0xADAD84, 0x42737B, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF7300, 0x944200, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                    return;
                }
                break;

            case 0xC9:
                set_colors(bgp, 0xFFFFCE, 0x63EFEF, 0x9C8431, 0x5A5A5A);
                set_colors(obp0, 0xFFFFFF, 0xFF7300, 0x944200, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0xCE:
            case 0xD1:
                set_colors(bgp, 0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF);
                set_colors(obp1, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                return;

            case 0xD3:
                if (rom_name_char4 == 0x49)
                {
                    set_colors(bgp, 0xFFFFFF, 0xADAD84, 0x42737B, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    return;
                }
                if (rom_name_char4 == 0x52)
                {
                    set_colors(bgp, 0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = bgp;
                    return;
                }
                break;

            case 0xDB:
                set_colors(bgp, 0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0xE0:
                set_colors(bgp, 0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0xE8:
                set_colors(bgp, 0x000000, 0x008484, 0xFFDE00, 0xFFFFFF);
                obp0 = bgp;
                obp1 = bgp;
                return;

            case 0xF0:
                set_colors(bgp, 0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF);
                set_colors(obp1, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                return;

            case 0xF2:
                set_colors(bgp, 0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000);
                obp0 = bgp;
                set_colors(obp1, 0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF);
                return;

            case 0xF4:
                if (rom_name_char4 == 0x20)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    obp1 = obp0;
                    return;
                }
                if (rom_name_char4 == 0x2D)
                {
                    set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
                    set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
                    set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                    return;
                }
                break;

            case 0xF6:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                return;

            case 0xF7:
                set_colors(bgp, 0xFFFFFF, 0xFFAD63, 0x843100, 0x000000);
                set_colors(obp0, 0xFFFFFF, 0x7BFF31, 0x008400, 0x000000);
                set_colors(obp1, 0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000);
                return;

            case 0xFF:
                set_colors(bgp, 0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000);
                obp0 = bgp;
                obp1 = bgp;
                return;

            default:
                break;
        }

        // default colors
        // set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000);
        // set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000);
        //
        // minor adjustments to be compatible to dmg-acid2.gb on CGB
        set_colors(bgp, 0xFFFFFF, 0x7BFF31, 0x0063C6, 0x000000);
        set_colors(obp0, 0xFFFFFF, 0xFF8484, 0x943939, 0x000000);
        obp1 = obp0;
    }

} // namespace





void age::gb_lcd_palettes::init_dmg_colors(std::span<uint8_t const> rom_header)
{
    ::init_dmg_colors(m_bgp_colors, m_obp0_colors, m_obp1_colors, rom_header);
}
