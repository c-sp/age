//
// © 2022 Christoph Sprenger <https://github.com/c-sp>
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

#ifndef AGE_GB_LCD_FIFO_FETCHER_HPP
#define AGE_GB_LCD_FIFO_FETCHER_HPP

//!
//! \file
//!

#include "../../common/age_gb_device.hpp"
#include "age_gb_lcd_renderer_common.hpp"
#include "age_gb_lcd_window_check.hpp"

#include <age_types.hpp>

#include <cassert>
#include <deque>
#include <span>



namespace age
{
    struct gb_sp_dot
    {
        gb_sp_dot() : gb_sp_dot(0, 0, 0) {}

        gb_sp_dot(uint16_t priority, uint8_t color, uint8_t attributes)
            : m_priority(priority),
              m_color(color),
              m_attributes(attributes) {}

        uint16_t m_priority;
        uint8_t  m_color;
        uint8_t  m_attributes;
    };

    using gb_sp_fifo = std::deque<gb_sp_dot>;



    class gb_lcd_fifo_fetcher
    {
        AGE_DISABLE_COPY(gb_lcd_fifo_fetcher);
        AGE_DISABLE_MOVE(gb_lcd_fifo_fetcher);

    public:
        gb_lcd_fifo_fetcher(const gb_device&              device,
                            const gb_lcd_renderer_common& common,
                            std::span<uint8_t const>      video_ram,
                            const gb_lcd_sprites&         sprites,
                            const gb_window_check&        window)
            : m_device(device),
              m_common(common),
              m_video_ram(video_ram),
              m_sprites(sprites),
              m_window(window)
        {
        }

        ~gb_lcd_fifo_fetcher() = default;



        void init_for_line(int line, bool is_line_zero)
        {
            m_line           = line;
            m_next_step      = fetcher_step::fetch_bg_name;
            m_next_step_clks = is_line_zero ? 87 : 85;

            m_bg_fetch_window             = false;
            m_bg_clks_last_tile_name      = 0;
            m_bg_clks_last_lcdc_tile_data = 0;
            m_bg_next_name                = 0;
            m_bg_next_attributes          = 0;
            m_bg_next_bitplane            = {};
            m_bg_cur_attributes           = 0;
            m_bg_cur_bitplane             = {};
            m_bg_cur_buffered_dots        = 8;

            m_spr_clks_next_bg_fetch        = 0;
            m_spr_clks_last_sprite_finished = int_max;
            m_spr_spx0_delay                = 0;
            m_spr_to_fetch_id               = no_sprite;
            m_spr_next_name                 = 0;
            m_spr_next_attributes           = 0;
            m_spr_next_bitplane             = {};
        }

        void finish_rendering()
        {
            m_next_step      = fetcher_step::line_finished;
            m_next_step_clks = gb_clock_cycles_per_lcd_line;
        }

        void restart_bg_fetch(int start_fetch_cycle_clks)
        {
            m_next_step      = fetcher_step::fetch_bg_name;
            m_next_step_clks = start_fetch_cycle_clks;
        }

        void trigger_sprite_fetch(uint8_t sprite_id, uint8_t spr_x, int current_line_clks, int spx0_delay)
        {
            assert(m_next_step_clks > current_line_clks);
            assert(m_spr_to_fetch_id == no_sprite);
            assert(spx0_delay >= 0);
            assert(spx0_delay <= 5);
            m_spr_to_fetch_id               = sprite_id;
            m_spr_to_fetch_x                = spr_x;
            m_spr_clks_last_sprite_finished = int_max;
            m_spr_clks_next_bg_fetch        = m_bg_cur_buffered_dots + 1 + spx0_delay;
            m_spr_spx0_delay                = spx0_delay;
            switch (m_next_step)
            {
                case fetcher_step::fetch_bg_name:
                    m_next_step      = fetcher_step::fetch_sprite_name;
                    m_next_step_clks = (m_next_step_clks - current_line_clks == 4)
                                           ? current_line_clks + 2
                                           : current_line_clks + 1;
                    break;

                case fetcher_step::fetch_bg_bitplane0:
                case fetcher_step::fetch_bg_bitplane1:
                    // delay sprite fetch until after the current bg tile has been fetched
                    break;

                case fetcher_step::fetch_sprite_name:
                case fetcher_step::fetch_sprite_bitplane0:
                case fetcher_step::fetch_sprite_bitplane1:
                case fetcher_step::line_finished:
                    assert(false);
            }
        }

        void set_clks_tile_data_change(int line_clks)
        {
            m_bg_clks_last_lcdc_tile_data = line_clks;
        }



        [[nodiscard]] uint8_t get_bg_attributes() const
        {
            return m_bg_cur_attributes;
        }

        uint8_t pop_bg_dot()
        {
            assert(m_bg_cur_buffered_dots > 0);

            uint8_t color_idx = m_bg_cur_attributes & gb_tile_attrib_cgb_palette;
            color_idx <<= 2;

            color_idx += (m_bg_cur_bitplane[0] & 1);
            color_idx += (m_bg_cur_bitplane[1] & 1) << 1;

            m_bg_cur_bitplane[0] >>= 1;
            m_bg_cur_bitplane[1] >>= 1;
            --m_bg_cur_buffered_dots;

            return color_idx;
        }

        age::gb_sp_dot pop_sp_dot()
        {
            if (m_sp_fifo.empty())
            {
                return {};
            }
            auto sp_px = m_sp_fifo[0];
            m_sp_fifo.pop_front();
            return sp_px;
        }



        [[nodiscard]] int next_step_clks() const
        {
            return m_next_step_clks;
        }

        [[nodiscard]] int clks_last_sprite_finished() const
        {
            return m_spr_clks_last_sprite_finished;
        }

        [[nodiscard]] int clks_last_bg_name_fetch() const
        {
            return m_bg_clks_last_tile_name;
        }

        void execute_next_step(int x_pos, int x_pos_win_start)
        {
            assert(m_next_step_clks < gb_clock_cycles_per_lcd_line);
            switch (m_next_step)
            {
                case fetcher_step::fetch_bg_name:
                    m_bg_clks_last_tile_name = m_next_step_clks;
                    fetch_bg_name(x_pos, x_pos_win_start);
                    schedule_next_step(2, fetcher_step::fetch_bg_bitplane0);
                    break;

                case fetcher_step::fetch_bg_bitplane0:
                    fetch_bg_bitplane(0);
                    schedule_next_step(2, fetcher_step::fetch_bg_bitplane1);
                    break;

                case fetcher_step::fetch_bg_bitplane1:
                    fetch_bg_bitplane(1);
                    if (m_spr_to_fetch_id != no_sprite)
                    {
                        schedule_next_step(2, fetcher_step::fetch_sprite_name);
                    }
                    else
                    {
                        schedule_next_step(4, fetcher_step::fetch_bg_name);
                    }
                    break;

                case fetcher_step::fetch_sprite_name:
                    fetch_sprite_name();
                    schedule_next_step(2, fetcher_step::fetch_sprite_bitplane0);
                    break;

                case fetcher_step::fetch_sprite_bitplane0:
                    fetch_sprite_bitplane(0);
                    schedule_next_step(2, fetcher_step::fetch_sprite_bitplane1);
                    break;

                case fetcher_step::fetch_sprite_bitplane1:
                    assert(m_spr_clks_last_sprite_finished == int_max);
                    assert(m_spr_clks_next_bg_fetch > 0);
                    assert(m_spr_clks_next_bg_fetch <= 10);
                    assert(m_spr_to_fetch_id != no_sprite);
                    fetch_sprite_bitplane(1);
                    apply_sp_bitplane();
                    m_spr_to_fetch_id               = no_sprite;
                    m_spr_clks_last_sprite_finished = m_next_step_clks + m_spr_spx0_delay;
                    schedule_next_step(m_spr_clks_next_bg_fetch, fetcher_step::fetch_bg_name);
                    m_spr_clks_next_bg_fetch = 0;
                    break;

                case fetcher_step::line_finished:
                    break;
            }
        }



    private:
        enum class fetcher_step
        {
            fetch_bg_name,
            fetch_bg_bitplane0,
            fetch_bg_bitplane1,
            fetch_sprite_name,
            fetch_sprite_bitplane0,
            fetch_sprite_bitplane1,
            line_finished,
        };

        void schedule_next_step(int clks_offset, fetcher_step step)
        {
            assert(clks_offset > 0);
            m_next_step_clks += clks_offset;
            m_next_step = step;
            assert(m_next_step_clks < gb_clock_cycles_per_lcd_line);
        }



        void fetch_bg_name(int x_pos, int x_pos_win_start)
        {
            bool bg_enabled        = m_device.cgb_mode() || ((m_common.get_lcdc() & gb_lcdc_bg_enable) != 0);
            m_bg_cur_attributes    = bg_enabled ? m_bg_next_attributes : 0;
            m_bg_cur_bitplane      = bg_enabled ? m_bg_next_bitplane : uint8_array<2>{};
            m_bg_cur_buffered_dots = 8;

            m_bg_fetch_window = x_pos_win_start <= x_pos;
            int tile_vram_ofs = 0;
            if (m_bg_fetch_window)
            {
                int tile_line_ofs = (x_pos - x_pos_win_start) >> 3;
                tile_vram_ofs     = m_common.m_win_tile_map_offset + ((m_window.current_window_line() & 0b11111000) << 2) + tile_line_ofs;
            }
            else
            {
                int bg_y          = m_common.m_scy + m_line;
                int px_ofs        = ((x_pos < gb_x_pos_first_px) || m_device.is_cgb_device()) ? 0 : 1;
                int tile_line_ofs = ((m_common.m_scx + x_pos + px_ofs) >> 3) & 0b11111;
                tile_vram_ofs     = m_common.m_bg_tile_map_offset + ((bg_y & 0b11111000) << 2) + tile_line_ofs;
            }

            m_bg_next_name       = m_video_ram[tile_vram_ofs];          // bank 0
            m_bg_next_attributes = m_video_ram[tile_vram_ofs + 0x2000]; // bank 1 (always zero for DMG)

            // LOG("line " << m_line.m_line << " (" << m_next_step_clks << "): fetched "
            //             << (m_bg_fetch_window ? "window" : "bg")
            //             << " tile id 0x" << std::hex << (int) m_bg_next_name
            //             << " with attributes 0x" << (int) m_bg_next_attributes << std::dec)
        }

        void fetch_bg_bitplane(int bitplane_offset)
        {
            assert((bitplane_offset == 0) || (bitplane_offset == 1));

            // CGB tile data glitch
            if (m_bg_clks_last_lcdc_tile_data == m_next_step_clks)
            {
                assert(m_device.is_cgb_device());
                if (!(m_common.get_lcdc() & gb_lcdc_bg_win_data))
                {
                    m_bg_next_bitplane[bitplane_offset] = m_common.m_xflip_cache[m_bg_next_name];
                    return;
                }
            }

            int tile_line = m_bg_fetch_window
                                ? m_window.current_window_line() & 0b111
                                : (m_common.m_scy + m_line) & 0b111;
            // y-flip
            if (m_bg_next_attributes & gb_tile_attrib_flip_y)
            {
                tile_line = 7 - tile_line;
            }

            // read tile data
            int tile_data_ofs = (m_bg_next_name ^ m_common.m_tile_xor) << 4; // 16 bytes per tile
            tile_data_ofs += m_common.m_tile_data_offset;
            tile_data_ofs += (m_bg_next_attributes & gb_tile_attrib_vram_bank) << 10;
            tile_data_ofs += tile_line * 2; // 2 bytes per line

            auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

            // x-flip
            // (we invert the x-flip for easier rendering:
            // this way bit 0 is used for the leftmost pixel)
            m_bg_next_bitplane[bitplane_offset] = (m_bg_next_attributes & gb_tile_attrib_flip_x)
                                                      ? bitplane
                                                      : m_common.m_xflip_cache[bitplane];
        }



        void fetch_sprite_name()
        {
            m_spr_next_name       = m_sprites.read_oam(m_spr_to_fetch_id * 4 + gb_oam_ofs_tile);
            m_spr_next_attributes = m_sprites.read_oam(m_spr_to_fetch_id * 4 + gb_oam_ofs_attributes);

            if (m_sprites.get_sprite_size() == 16)
            {
                m_spr_next_name &= 0xFE;
            }
        }

        void fetch_sprite_bitplane(int bitplane_offset)
        {
            assert((bitplane_offset == 0) || (bitplane_offset == 1));

            int tile_line_mask = (m_sprites.get_sprite_size() == 16) ? 0b1111 : 0b111;
            int tile_line      = (m_line + 16 - m_sprites.read_oam(m_spr_to_fetch_id * 4 + gb_oam_ofs_y)) & tile_line_mask;

            // y-flip
            if (m_spr_next_attributes & gb_tile_attrib_flip_y)
            {
                tile_line = tile_line_mask - tile_line;
            }

            // read tile data
            int tile_data_ofs = m_spr_next_name << 4; // 16 bytes per tile
            tile_data_ofs += (m_spr_next_attributes & gb_tile_attrib_vram_bank) << 10;
            tile_data_ofs += tile_line * 2;           // 2 bytes per line

            auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

            // x-flip
            // (we invert the x-flip for easier rendering:
            // this way bit 0 is used for the leftmost pixel)
            m_spr_next_bitplane[bitplane_offset] = (m_spr_next_attributes & gb_tile_attrib_flip_x)
                                                       ? bitplane
                                                       : m_common.m_xflip_cache[bitplane];
        }

        void apply_sp_bitplane()
        {
            assert(m_sp_fifo.size() <= 8);
            m_sp_fifo.resize(8);

            uint16_t priority = m_device.cgb_mode()
                                    ? m_spr_to_fetch_id
                                    : (m_spr_to_fetch_x << 8) + m_spr_to_fetch_id;

            unsigned bitplane0 = m_spr_next_bitplane[0];
            unsigned bitplane1 = m_spr_next_bitplane[1];

            uint8_t palette_ofs = gb_get_sprite_palette_idx(m_spr_next_attributes, m_device.cgb_mode());
            palette_ofs <<= 2;

            bitplane1 <<= 1;
            for (int i = 0; i < 8; ++i)
            {
                uint8_t color = are_sprites_enabled(m_common.get_lcdc())
                                    ? (bitplane0 & 0b01U) + (bitplane1 & 0b10U) + palette_ofs
                                    : 0;
                bitplane0 >>= 1;
                bitplane1 >>= 1;

                if (((m_sp_fifo[i].m_color & 0b11) == 0) || (m_sp_fifo[i].m_priority > priority))
                {
                    m_sp_fifo[i] = {priority, color, m_spr_next_attributes};
                }
            }
        }



        const gb_device&              m_device;
        const gb_lcd_renderer_common& m_common;
        std::span<uint8_t const>      m_video_ram;
        const gb_lcd_sprites&         m_sprites;
        const gb_window_check&        m_window;
        gb_sp_fifo                    m_sp_fifo{};

        int          m_line           = gb_no_line.m_line;
        fetcher_step m_next_step      = fetcher_step::fetch_bg_name;
        int          m_next_step_clks = gb_no_clock_cycle;

        int            m_bg_clks_last_tile_name      = 0;
        int            m_bg_clks_last_lcdc_tile_data = 0;
        bool           m_bg_fetch_window             = false;
        uint8_t        m_bg_next_name                = 0;
        uint8_t        m_bg_next_attributes          = 0;
        uint8_array<2> m_bg_next_bitplane{};
        uint8_t        m_bg_cur_attributes = 0;
        uint8_array<2> m_bg_cur_bitplane{};
        int8_t         m_bg_cur_buffered_dots = 0;

        int            m_spr_clks_next_bg_fetch        = 0;
        int            m_spr_clks_last_sprite_finished = int_max;
        int            m_spr_spx0_delay                = 0;
        uint8_t        m_spr_to_fetch_id               = no_sprite;
        uint8_t        m_spr_to_fetch_x                = no_sprite;
        uint8_t        m_spr_next_name                 = 0;
        uint8_t        m_spr_next_attributes           = 0;
        uint8_array<2> m_spr_next_bitplane{};

        constexpr static uint8_t no_sprite = std::numeric_limits<uint8_t>::max();
    };

} // namespace age



#undef LOG

#endif // AGE_GB_LCD_FIFO_FETCHER_HPP
