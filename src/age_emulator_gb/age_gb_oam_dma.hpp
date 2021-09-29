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

#ifndef AGE_GB_OAM_DMA_HPP
#define AGE_GB_OAM_DMA_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"
#include "common/age_gb_device.hpp"
#include "common/age_gb_events.hpp"

#include "lcd/age_gb_lcd.hpp"

#include "age_gb_memory.hpp"



namespace age
{
    class gb_oam_dma
    {
        AGE_DISABLE_COPY(gb_oam_dma);
        AGE_DISABLE_MOVE(gb_oam_dma);

    public:
        gb_oam_dma(const gb_device& device,
                   const gb_clock&  clock,
                   const gb_memory& memory,
                   gb_events&       events,
                   gb_lcd&          lcd);

        ~gb_oam_dma() = default;

        [[nodiscard]] bool dma_active() const
        {
            return m_oam_dma_active;
        }

        [[nodiscard]] uint8_t read_dma_reg() const
        {
            return m_oam_dma_reg;
        }

        void set_back_clock(int clock_cycle_offset);
        void write_dma_reg(uint8_t value);
        void handle_start_dma_event();
        void continue_dma();

    private:
        const gb_clock&  m_clock;
        const gb_memory& m_memory;
        gb_events&       m_events;
        gb_lcd&          m_lcd;

        uint8_t m_oam_dma_reg;
        bool    m_oam_dma_active     = false;
        int     m_oam_dma_address    = 0;
        int     m_oam_dma_offset     = 0;
        int     m_oam_dma_last_cycle = gb_no_clock_cycle;
    };

} // namespace age



#endif // AGE_GB_OAM_DMA_HPP
