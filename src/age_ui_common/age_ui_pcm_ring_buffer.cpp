//
// Copyright (c) 2010-2017 Christoph Sprenger
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

#include <algorithm>

#include <age_debug.hpp>

#include "age_ui_pcm_ring_buffer.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#define AGE_ASSERT_BUFFERED_SAMPLES \
    AGE_ASSERT( \
    /* buffer completely filled */ \
    ((m_buffered_samples == m_buffer.size()) && (m_first_buffered_sample == m_first_new_sample)) \
    /* buffer empty */ \
    || ((m_buffered_samples == 0) && (m_first_buffered_sample == m_first_new_sample)) \
    /* samples not wrapping around */ \
    || ((m_buffered_samples == m_first_new_sample - m_first_buffered_sample) && (m_first_buffered_sample < m_first_new_sample)) \
    /* samples wrapping around */ \
    || ((m_buffered_samples == m_buffer.size() - (m_first_buffered_sample - m_first_new_sample)) && (m_first_buffered_sample > m_first_new_sample)) \
    )



//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::pcm_ring_buffer::pcm_ring_buffer(uint max_buffered_samples)
    : m_buffer(max_buffered_samples, lpcm_stereo_sample())
{
}


//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

age::uint age::pcm_ring_buffer::get_max_buffered_samples() const
{
    return m_buffer.size();
}

age::uint age::pcm_ring_buffer::get_buffered_samples() const
{
    return m_buffered_samples;
}

age::uint age::pcm_ring_buffer::get_last_discarded_samples() const
{
    return m_last_discarded_samples;
}

const age::lpcm_stereo_sample* age::pcm_ring_buffer::get_buffered_samples_ptr(uint &available_stereo_samples) const
{
    const lpcm_stereo_sample *result = nullptr;
    uint available = 0;

    if (m_buffered_samples > 0)
    {
        // if the buffered samples wrap around to the buffer's beginning,
        // return only the samples available until the buffer's end
        available = (m_first_buffered_sample + m_buffered_samples > m_buffer.size())
                ? m_buffer.size() - m_first_buffered_sample
                : m_buffered_samples;

        result = &m_buffer[m_first_buffered_sample];
    }

    available_stereo_samples = available;
    return result;
}



void age::pcm_ring_buffer::add_samples(const pcm_vector &samples_to_add)
{
    uint num_samples_to_add = samples_to_add.size();
    add_samples(samples_to_add, num_samples_to_add);
}

void age::pcm_ring_buffer::add_samples(const pcm_vector &samples_to_add, uint num_samples_to_add)
{
    AGE_ASSERT_BUFFERED_SAMPLES;

    num_samples_to_add = std::min(num_samples_to_add, samples_to_add.size());
    auto first = std::begin(samples_to_add);
    auto last = first + num_samples_to_add;

    // if we add more samples than the buffer can hold in total, we just
    // fill the buffer with the last N samples to add
    if (num_samples_to_add >= m_buffer.size())
    {
        uint sample_offset = num_samples_to_add - m_buffer.size();

        m_last_discarded_samples = m_buffered_samples + sample_offset; // set this before changing m_buffered_samples
        m_buffered_samples = m_buffer.size();
        m_first_buffered_sample = 0;
        m_first_new_sample = 0;

        std::copy(first + sample_offset, last, std::begin(m_buffer));
    }

    // if we add less samples than the buffer can hold in total, we may
    // still overwrite currently buffered samples
    else
    {
        m_last_discarded_samples = 0;

        // call add_samples_with_offset(...) twice to handle potential wrap around
        uint samples_added = add_samples_with_offset(first, last);
        add_samples_with_offset(first + samples_added, last);
    }

    AGE_ASSERT(m_buffered_samples <= m_buffer.size());
    AGE_ASSERT(m_first_buffered_sample < m_buffer.size());
    AGE_ASSERT(m_first_new_sample < m_buffer.size());
    AGE_ASSERT_BUFFERED_SAMPLES;
}



void age::pcm_ring_buffer::set_to(lpcm_stereo_sample sample)
{
    AGE_ASSERT_BUFFERED_SAMPLES;

    std::fill(std::begin(m_buffer), std::end(m_buffer), sample);
    m_last_discarded_samples = 0;

    m_buffered_samples = m_buffer.size();
    m_first_buffered_sample = 0;
    m_first_new_sample = 0;

    AGE_ASSERT_BUFFERED_SAMPLES;
}



void age::pcm_ring_buffer::discard_buffered_samples(uint samples_to_discard)
{
    AGE_ASSERT_BUFFERED_SAMPLES;

    if (samples_to_discard >= m_buffered_samples)
    {
        clear();
        LOG("clearing");
    }
    else
    {
        AGE_ASSERT(samples_to_discard < m_buffered_samples);
        AGE_ASSERT(m_buffered_samples > 0);

        m_first_buffered_sample = add_check_wrap_around(m_first_buffered_sample, samples_to_discard);
        m_buffered_samples -= samples_to_discard;

        LOG("samples_to_discard " << samples_to_discard
            << ", m_first_buffered_sample " << m_first_buffered_sample
            << ", m_first_new_sample " << m_first_new_sample
            << ", m_buffered_samples " << m_buffered_samples);

        // we did not discard all buffered samples, but more than zero
        AGE_ASSERT((m_buffered_samples > 0) && (m_buffered_samples < m_buffer.size()));
        AGE_ASSERT(m_first_buffered_sample != m_first_new_sample);
    }

    AGE_ASSERT(m_buffered_samples <= m_buffer.size());
    AGE_ASSERT(m_first_buffered_sample < m_buffer.size());
    AGE_ASSERT(m_first_new_sample < m_buffer.size());
    AGE_ASSERT_BUFFERED_SAMPLES;
}



void age::pcm_ring_buffer::clear()
{
    // no need to adjust m_buffer
    // don't change m_last_discarded_samples

    m_first_buffered_sample = 0;
    m_first_new_sample = 0;
    m_buffered_samples = 0;
}



//---------------------------------------------------------
//
//   private methods
//
//---------------------------------------------------------

age::uint age::pcm_ring_buffer::add_samples_with_offset(pcm_vector::const_iterator begin, pcm_vector::const_iterator end)
{
    uint samples_available = end - begin;
    AGE_ASSERT(samples_available < m_buffer.size());
    uint samples_added = 0;

    if (samples_available > 0)
    {
        // calculate the number of samples we can add to the buffer before wrapping around
        if (m_first_new_sample + samples_available > m_buffer.size())
        {
            samples_available = m_buffer.size() - m_first_new_sample;
        }

        // copy the samples to the buffer
        std::copy(begin, begin + samples_available, std::begin(m_buffer) + m_first_new_sample);

        samples_added = samples_available;
        m_first_new_sample = add_check_wrap_around(m_first_new_sample, samples_added);
        m_buffered_samples += samples_added;

        // did we discard old samples?
        if (m_buffered_samples > m_buffer.size())
        {
            m_last_discarded_samples += m_buffered_samples - m_buffer.size();
            m_buffered_samples = m_buffer.size();
            m_first_buffered_sample = m_first_new_sample;
        }
    }

    LOG("added " << samples_added << " samples");
    return samples_added;
}



age::uint age::pcm_ring_buffer::add_check_wrap_around(uint v1, uint v2) const
{
    uint result = v1 + v2;

    if (result >= m_buffer.size())
    {
        result -= m_buffer.size();
    }

    AGE_ASSERT(result < m_buffer.size());
    return result;
}
