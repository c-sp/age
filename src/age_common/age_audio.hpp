
#ifndef AGE_AUDIO_HPP
#define AGE_AUDIO_HPP

//!
//! \file
//!

#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age {

struct lpcm_stereo_sample
{
    lpcm_stereo_sample()
        : lpcm_stereo_sample(0, 0)
    {}

    lpcm_stereo_sample(int16 left_sample, int16 right_sample)
    {
        m_samples[0] = left_sample;
        m_samples[1] = right_sample;
    }

    lpcm_stereo_sample& operator*=(float factor)
    {
        AGE_ASSERT(factor <= 1);
        AGE_ASSERT(factor >= 0);
        m_samples[0] = static_cast<int16>(m_samples[0] * factor);
        m_samples[1] = static_cast<int16>(m_samples[1] * factor);
        return *this;
    }

    lpcm_stereo_sample& operator-=(const lpcm_stereo_sample &other)
    {
        m_samples[0] -= other.m_samples[0];
        m_samples[1] -= other.m_samples[1];
        return *this;
    }

    lpcm_stereo_sample& operator+=(const lpcm_stereo_sample &other)
    {
        m_samples[0] += other.m_samples[0];
        m_samples[1] += other.m_samples[1];
        return *this;
    }

    bool operator==(const lpcm_stereo_sample &other) const
    {
        return m_stereo_sample == other.m_stereo_sample;
    }

    bool operator!=(const lpcm_stereo_sample &other) const
    {
        return m_stereo_sample != other.m_stereo_sample;
    }

    union
    {
        uint32 m_stereo_sample;
        int16 m_samples[2];
    };
};

typedef std::vector<lpcm_stereo_sample> pcm_vector;

} // namespace age



#endif // AGE_AUDIO_HPP
