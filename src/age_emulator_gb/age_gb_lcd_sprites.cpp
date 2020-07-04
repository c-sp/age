//
// Copyright 2019 Christoph Sprenger
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

#include <age_debug.hpp>

#include "age_gb_lcd_render.hpp"



age::gb_lcd_sprites::gb_lcd_sprites()
{
}



age::uint8_t age::gb_lcd_sprites::read_oam(int offset) const
{
    return m_oam[offset];
}

void age::gb_lcd_sprites::write_oam(int offset, uint8_t value)
{
    m_oam[offset] = value;
    m_dirty = true;
}
