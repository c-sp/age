//
// Copyright (c) 2010-2018 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AGE_PCM_SAMPLE_HPP
#define AGE_PCM_SAMPLE_HPP

//!
//! \file
//!

#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age {



struct pcm_sample
{
    pcm_sample()
        : pcm_sample(0, 0)
    {}

    pcm_sample(int16 left_sample, int16 right_sample)
    {
        m_samples[0] = left_sample;
        m_samples[1] = right_sample;
    }

    pcm_sample& operator*=(float factor)
    {
        AGE_ASSERT(factor <= 1);
        AGE_ASSERT(factor >= 0);
        m_samples[0] = static_cast<int16>(m_samples[0] * factor);
        m_samples[1] = static_cast<int16>(m_samples[1] * factor);
        return *this;
    }

    pcm_sample& operator-=(const pcm_sample &other)
    {
        m_samples[0] -= other.m_samples[0];
        m_samples[1] -= other.m_samples[1];
        return *this;
    }

    pcm_sample& operator+=(const pcm_sample &other)
    {
        m_samples[0] += other.m_samples[0];
        m_samples[1] += other.m_samples[1];
        return *this;
    }

    bool operator==(const pcm_sample &other) const
    {
        return m_stereo_sample == other.m_stereo_sample;
    }

    bool operator!=(const pcm_sample &other) const
    {
        return m_stereo_sample != other.m_stereo_sample;
    }

    union
    {
        uint32 m_stereo_sample;
        int16 m_samples[2];
    };
};

constexpr uint sizeof_pcm_sample = sizeof(pcm_sample);
static_assert(4 == sizeof_pcm_sample, "expected pcm_sample size of 4 bytes (16 bit stereo)");

typedef std::vector<pcm_sample> pcm_vector;



} // namespace age

#endif // AGE_PCM_SAMPLE_HPP
