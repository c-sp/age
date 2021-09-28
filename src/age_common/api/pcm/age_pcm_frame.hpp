//
// Copyright 2020 Christoph Sprenger
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

#ifndef AGE_PCM_FRAME_HPP
#define AGE_PCM_FRAME_HPP

//!
//! \file
//!

#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age
{

    struct pcm_frame
    {
        pcm_frame()
            : pcm_frame(0, 0)
        {}

        pcm_frame(int16_t left_sample, int16_t right_sample)
            : m_left_sample(left_sample),
              m_right_sample(right_sample)
        {}

        pcm_frame& operator*=(float factor)
        {
            AGE_ASSERT(factor <= 1);
            AGE_ASSERT(factor >= 0);
            float s0       = m_left_sample;
            float s1       = m_right_sample;
            m_left_sample  = static_cast<int16_t>(s0 * factor);
            m_right_sample = static_cast<int16_t>(s1 * factor);
            return *this;
        }

        pcm_frame& operator-=(const pcm_frame& other)
        {
            m_left_sample  = static_cast<int16_t>(m_left_sample - other.m_left_sample);
            m_right_sample = static_cast<int16_t>(m_right_sample - other.m_right_sample);
            return *this;
        }

        pcm_frame& operator+=(const pcm_frame& other)
        {
            m_left_sample  = static_cast<int16_t>(m_left_sample + other.m_left_sample);
            m_right_sample = static_cast<int16_t>(m_right_sample + other.m_right_sample);
            return *this;
        }

        bool operator==(const pcm_frame& other) const
        {
            return get_32bits() == other.get_32bits();
        }

        bool operator!=(const pcm_frame& other) const
        {
            return !(*this == other);
        }


        [[nodiscard]] uint32_t get_32bits() const
        {
            uint32_t frame = 0;
            memcpy(&frame, &m_left_sample, sizeof(uint32_t));
            return frame;
        }

        void set_32bits(uint32_t frame)
        {
            memcpy(&m_left_sample, &frame, sizeof(pcm_frame));
        }

        int16_t m_left_sample;
        int16_t m_right_sample;
    };

    static_assert(sizeof(pcm_frame) == 4, "expected pcm_frame size of 4 bytes (16 bit stereo)");

    using pcm_vector = std::vector<pcm_frame>;

} // namespace age



#endif // AGE_PCM_FRAME_HPP
