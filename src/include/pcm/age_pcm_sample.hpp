//
// Copyright 2018 Christoph Sprenger
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

static_assert(sizeof(pcm_sample) == 4, "expected pcm_sample size of 4 bytes (16 bit stereo)");

typedef std::vector<pcm_sample> pcm_vector;



} // namespace age

#endif // AGE_PCM_SAMPLE_HPP
