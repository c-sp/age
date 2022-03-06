//
// Copyright 2022 Christoph Sprenger
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

#include <age_debug.hpp>
#include <age_types.hpp>

#include <deque>

//! \todo maybe replace this with gb_logger
#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif



namespace age
{
    struct sp_pixel
    {
        sp_pixel() : sp_pixel(0, 0) {}

        sp_pixel(uint8_t id, uint8_t color)
            : m_id(id),
              m_color(color) {}

        uint8_t m_id;
        uint8_t m_color;
    };

    using gb_bg_fifo = std::deque<uint8_t>;
    using gb_sp_fifo = std::deque<sp_pixel>;



    class gb_lcd_fifo_fetcher
    {
        AGE_DISABLE_COPY(gb_lcd_fifo_fetcher);
        AGE_DISABLE_MOVE(gb_lcd_fifo_fetcher);

    public:
        gb_lcd_fifo_fetcher(const gb_device&              device,
                            const gb_lcd_renderer_common& common,
                            const uint8_t*                video_ram,
                            const gb_lcd_sprites&         sprites,
                            const gb_window_check&        window,
                            gb_bg_fifo&                   bg_fifo,
                            gb_sp_fifo&                   sp_fifo)
            : m_device(device),
              m_common(common),
              m_video_ram(video_ram),
              m_sprites(sprites),
              m_window(window),
              m_bg_fifo(bg_fifo),
              m_sp_fifo(sp_fifo)
        {
        }

        ~gb_lcd_fifo_fetcher() = default;



        void init_for_line(int line, bool is_line_zero)
        {
            m_line                       = line;
            m_next_step                  = fetcher_step::fetch_bg_tile_id;
            m_clks_next_step             = is_line_zero ? 87 : 85;
            m_fetching_window            = false;
            m_clks_last_bg_tile_id       = 0;
            m_fetched_bg_tile_id         = 0;
            m_fetched_bg_tile_attributes = 0;
            m_fetched_bg_bitplane        = {};
            m_clks_tile_data_change      = gb_no_line;
            m_clks_next_bg_fetch         = 0;
            m_clks_last_sprite_finished  = int_max;
            m_spx0_delay                 = 0;
            m_sprite_to_fetch            = no_sprite;
            m_fetched_sprite_tile_id     = 0;
            m_fetched_sprite_attributes  = 0;
            m_fetched_sprite_bitplane    = {};
        }

        void finish_rendering()
        {
            m_next_step      = fetcher_step::line_finished;
            m_clks_next_step = gb_clock_cycles_per_lcd_line;
        }

        void restart_bg_fetch(int start_fetch_cycle_clks, bool push_last_tile = false)
        {
            m_next_step      = fetcher_step::fetch_bg_tile_id;
            m_clks_next_step = start_fetch_cycle_clks;
            if (push_last_tile)
            {
                push_bg_bitplane();
            }
        }

        void trigger_sprite_fetch(uint8_t sprite_id, int current_line_clks, int spx0_delay)
        {
            AGE_ASSERT(m_clks_next_step > current_line_clks)
            AGE_ASSERT(m_clks_next_step <= current_line_clks + 4)
            AGE_ASSERT(m_clks_last_bg_tile_id <= current_line_clks)
            AGE_ASSERT(m_sprite_to_fetch == no_sprite)
            AGE_ASSERT(spx0_delay >= 0)
            AGE_ASSERT(spx0_delay <= 5)
            m_sprite_to_fetch           = sprite_id;
            m_clks_last_sprite_finished = int_max;
            m_clks_next_bg_fetch        = 9 - (current_line_clks - m_clks_last_bg_tile_id) + spx0_delay;
            m_spx0_delay                = spx0_delay;
            switch (m_next_step)
            {
                case fetcher_step::fetch_bg_tile_id:
                    m_next_step      = fetcher_step::fetch_sprite_tile_id;
                    m_clks_next_step = (m_clks_next_step - current_line_clks == 4)
                                           ? current_line_clks + 2
                                           : current_line_clks + 1;
                    break;

                case fetcher_step::fetch_bg_bitplane0:
                case fetcher_step::fetch_bg_bitplane1:
                    // delay sprite fetch until after the current bg tile has been fetched
                    break;

                case fetcher_step::fetch_sprite_tile_id:
                case fetcher_step::fetch_sprite_bitplane0:
                case fetcher_step::fetch_sprite_bitplane1:
                case fetcher_step::line_finished:
                    AGE_ASSERT(false)
                    break;
            }
        }

        void set_clks_tile_data_change(gb_current_line at_line)
        {
            AGE_ASSERT(m_device.is_cgb_device())
            AGE_ASSERT(at_line.m_line == m_line)
            m_clks_tile_data_change = at_line;
        }



        [[nodiscard]] int next_step_clks() const
        {
            return m_clks_next_step;
        }

        [[nodiscard]] int next_step() const
        {
            return to_underlying(m_next_step);
        }

        [[nodiscard]] int last_sprite_finished_clks() const
        {
            return m_clks_last_sprite_finished;
        }

        [[nodiscard]] bool next_step_fetches_tile_id() const
        {
            return m_next_step == fetcher_step::fetch_bg_tile_id;
        }

        void execute_next_step(int x_pos, int x_pos_win_start)
        {
            AGE_ASSERT(m_clks_next_step < gb_clock_cycles_per_lcd_line)
            switch (m_next_step)
            {
                case fetcher_step::fetch_bg_tile_id:
                    LOG("(" << m_clks_next_step << "): fetch_bg_tile_id")
                    m_clks_last_bg_tile_id = m_clks_next_step;
                    fetch_bg_tile_id(x_pos, x_pos_win_start);
                    schedule_next_step(2, fetcher_step::fetch_bg_bitplane0);
                    break;

                case fetcher_step::fetch_bg_bitplane0:
                    LOG("(" << m_clks_next_step << "): fetch_bg_bitplane0")
                    fetch_bg_bitplane(0);
                    schedule_next_step(2, fetcher_step::fetch_bg_bitplane1);
                    break;

                case fetcher_step::fetch_bg_bitplane1:
                    LOG("(" << m_clks_next_step << "): fetch_bg_bitplane1")
                    fetch_bg_bitplane(1);
                    push_bg_bitplane();
                    if (m_sprite_to_fetch != no_sprite)
                    {
                        schedule_next_step(2, fetcher_step::fetch_sprite_tile_id);
                    }
                    else
                    {
                        schedule_next_step(4, fetcher_step::fetch_bg_tile_id);
                    }
                    break;

                case fetcher_step::fetch_sprite_tile_id:
                    LOG("(" << m_clks_next_step << "): fetch_sprite_tile_id")
                    fetch_sprite_tile_id();
                    schedule_next_step(2, fetcher_step::fetch_sprite_bitplane0);
                    break;

                case fetcher_step::fetch_sprite_bitplane0:
                    LOG("(" << m_clks_next_step << "): fetch_sprite_bitplane0")
                    fetch_sprite_bitplane(0);
                    schedule_next_step(2, fetcher_step::fetch_sprite_bitplane1);
                    break;

                case fetcher_step::fetch_sprite_bitplane1:
                    AGE_ASSERT(m_clks_last_sprite_finished == int_max)
                    AGE_ASSERT(m_clks_next_bg_fetch > 0)
                    AGE_ASSERT(m_clks_next_bg_fetch <= 10)
                    AGE_ASSERT(m_sprite_to_fetch != no_sprite)
                    LOG("(" << m_clks_next_step << "): fetch_sprite_bitplane1")
                    fetch_sprite_bitplane(1);
                    apply_sp_bitplane();
                    m_sprite_to_fetch           = no_sprite;
                    m_clks_last_sprite_finished = m_clks_next_step + m_spx0_delay;
                    schedule_next_step(m_clks_next_bg_fetch, fetcher_step::fetch_bg_tile_id);
                    m_clks_next_bg_fetch = 0;
                    break;

                case fetcher_step::line_finished:
                    break;
            }
        }



    private:
        enum class fetcher_step
        {
            fetch_bg_tile_id,
            fetch_bg_bitplane0,
            fetch_bg_bitplane1,
            fetch_sprite_tile_id,
            fetch_sprite_bitplane0,
            fetch_sprite_bitplane1,
            line_finished,
        };

        void schedule_next_step(int clks_offset, fetcher_step step)
        {
            AGE_ASSERT(clks_offset > 0)
            m_clks_next_step += clks_offset;
            m_next_step = step;
            AGE_ASSERT(m_clks_next_step < gb_clock_cycles_per_lcd_line)
        }



        void fetch_bg_tile_id(int x_pos, int x_pos_win_start)
        {
            m_fetching_window = x_pos_win_start <= x_pos;

            int tile_vram_ofs = 0;
            if (m_fetching_window)
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

            m_fetched_bg_tile_id         = m_video_ram[tile_vram_ofs];          // bank 0
            m_fetched_bg_tile_attributes = m_video_ram[tile_vram_ofs + 0x2000]; // bank 1 (always zero for DMG)

            // LOG("line " << m_line.m_line << " (" << m_clks_next_step << "): fetched "
            //             << (m_fetching_window ? "window" : "bg")
            //             << " tile id 0x" << std::hex << (int) m_fetched_bg_tile_id
            //             << " with attributes 0x" << (int) m_fetched_bg_tile_attributes << std::dec)
        }

        void fetch_bg_bitplane(int bitplane_offset)
        {
            AGE_ASSERT((bitplane_offset == 0) || (bitplane_offset == 1))

            // CGB tile data glitch
            if (m_clks_tile_data_change.m_line_clks == m_clks_next_step)
            {
                AGE_ASSERT(m_device.is_cgb_device())
                if (!(m_common.get_lcdc() & gb_lcdc_bg_win_data))
                {
                    m_fetched_bg_bitplane[bitplane_offset] = m_common.m_xflip_cache[m_fetched_bg_tile_id];
                    return;
                }
            }

            int tile_line = m_fetching_window
                                ? m_window.current_window_line() & 0b111
                                : (m_common.m_scy + m_line) & 0b111;
            // y-flip
            if (m_fetched_bg_tile_attributes & gb_tile_attrib_flip_y)
            {
                tile_line = 7 - tile_line;
            }

            // read tile data
            int tile_data_ofs = (m_fetched_bg_tile_id ^ m_common.m_tile_xor) << 4; // 16 bytes per tile
            tile_data_ofs += m_common.m_tile_data_offset;
            tile_data_ofs += (m_fetched_bg_tile_attributes & gb_tile_attrib_vram_bank) << 10;
            tile_data_ofs += tile_line * 2; // 2 bytes per line

            auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

            // x-flip
            // (we invert the x-flip for easier rendering:
            // this way bit 0 is used for the leftmost pixel)
            m_fetched_bg_bitplane[bitplane_offset] = (m_fetched_bg_tile_attributes & gb_tile_attrib_flip_x)
                                                         ? bitplane
                                                         : m_common.m_xflip_cache[bitplane];
        }

        void push_bg_bitplane()
        {
            unsigned bitplane0 = m_fetched_bg_bitplane[0];
            unsigned bitplane1 = m_fetched_bg_bitplane[1];

            uint8_t palette_ofs = m_fetched_bg_tile_attributes & gb_tile_attrib_cgb_palette;
            palette_ofs <<= 2;

            bitplane1 <<= 1;
            for (int i = 0; i < 8; ++i)
            {
                m_bg_fifo.push_back((bitplane0 & 0b01U) + (bitplane1 & 0b10U) + palette_ofs);
                bitplane0 >>= 1;
                bitplane1 >>= 1;
            }
        }



        void fetch_sprite_tile_id()
        {
            m_fetched_sprite_tile_id    = m_sprites.read_oam(m_sprite_to_fetch * 4 + gb_oam_ofs_tile);
            m_fetched_sprite_attributes = m_sprites.read_oam(m_sprite_to_fetch * 4 + gb_oam_ofs_attributes);

            if (m_sprites.get_sprite_size() == 16)
            {
                m_fetched_sprite_tile_id &= 0xFE;
            }
        }

        void fetch_sprite_bitplane(int bitplane_offset)
        {
            AGE_ASSERT((bitplane_offset == 0) || (bitplane_offset == 1))

            int tile_line_mask = (m_sprites.get_sprite_size() == 16) ? 0b1111 : 0b111;
            int tile_line      = (m_line + 16 - m_sprites.read_oam(m_sprite_to_fetch * 4 + gb_oam_ofs_y)) & tile_line_mask;

            // y-flip
            if (m_fetched_sprite_attributes & gb_tile_attrib_flip_y)
            {
                tile_line = tile_line_mask - tile_line;
            }

            // read tile data
            int tile_data_ofs = m_fetched_sprite_tile_id << 4; // 16 bytes per tile
            tile_data_ofs += (m_fetched_sprite_attributes & gb_tile_attrib_vram_bank) << 10;
            tile_data_ofs += tile_line * 2; // 2 bytes per line

            auto bitplane = m_video_ram[tile_data_ofs + bitplane_offset];

            // x-flip
            // (we invert the x-flip for easier rendering:
            // this way bit 0 is used for the leftmost pixel)
            m_fetched_sprite_bitplane[bitplane_offset] = (m_fetched_sprite_attributes & gb_tile_attrib_flip_x)
                                                             ? bitplane
                                                             : m_common.m_xflip_cache[bitplane];
        }

        void apply_sp_bitplane()
        {
            AGE_ASSERT(m_sp_fifo.size() <= 8)
            m_sp_fifo.resize(8);

            unsigned bitplane0 = m_fetched_sprite_bitplane[0];
            unsigned bitplane1 = m_fetched_sprite_bitplane[1];

            uint8_t palette_ofs = gb_get_sprite_palette_idx(m_fetched_sprite_attributes, m_device.cgb_mode());
            palette_ofs <<= 2;

            bitplane1 <<= 1;
            for (int i = 0; i < 8; ++i)
            {
                uint8_t color = (bitplane0 & 0b01U) + (bitplane1 & 0b10U) + palette_ofs;
                bitplane0 >>= 1;
                bitplane1 >>= 1;

                //! \todo sprite priority
                if ((m_sp_fifo[i].m_color & 0b11) == 0)
                {
                    m_sp_fifo[i] = {m_sprite_to_fetch, color};
                }
            }
        }



        const gb_device&              m_device;
        const gb_lcd_renderer_common& m_common;
        const uint8_t*                m_video_ram;
        const gb_lcd_sprites&         m_sprites;
        const gb_window_check&        m_window;
        gb_bg_fifo&                   m_bg_fifo;
        gb_sp_fifo&                   m_sp_fifo;

        int          m_line           = gb_no_line.m_line;
        fetcher_step m_next_step      = fetcher_step::fetch_bg_tile_id;
        int          m_clks_next_step = gb_no_clock_cycle;

        bool            m_fetching_window            = false;
        int             m_clks_last_bg_tile_id       = 0;
        uint8_t         m_fetched_bg_tile_id         = 0;
        uint8_t         m_fetched_bg_tile_attributes = 0;
        uint8_array<2>  m_fetched_bg_bitplane{};
        gb_current_line m_clks_tile_data_change = gb_no_line;

        int            m_clks_next_bg_fetch        = 0;
        int            m_clks_last_sprite_finished = int_max;
        int            m_spx0_delay                = 0;
        uint8_t        m_sprite_to_fetch           = no_sprite;
        uint8_t        m_fetched_sprite_tile_id    = 0;
        uint8_t        m_fetched_sprite_attributes = 0;
        uint8_array<2> m_fetched_sprite_bitplane{};

        constexpr static uint8_t no_sprite = std::numeric_limits<uint8_t>::max();
    };

} // namespace age



#undef LOG

#endif // AGE_GB_LCD_FIFO_FETCHER_HPP
