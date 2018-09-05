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

#include <algorithm>

#include <age_debug.hpp>
#include <pcm/age_pcm_ring_buffer.hpp>

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#define AGE_ASSERT_BUFFERED_SAMPLES \
    AGE_ASSERT( \
    /* buffer size matches */ \
    (m_buffer_size == static_cast<int>(m_buffer.size())) && (\
    /* buffer completely filled */ \
    ((m_buffered_samples == m_buffer_size) && (m_first_buffered_sample == m_first_new_sample)) \
    /* buffer empty */ \
    || ((m_buffered_samples == 0) && (m_first_buffered_sample == m_first_new_sample)) \
    /* samples not wrapping around */ \
    || ((m_buffered_samples == m_first_new_sample - m_first_buffered_sample) && (m_first_buffered_sample < m_first_new_sample)) \
    /* samples wrapping around */ \
    || ((m_buffered_samples == m_buffer_size - (m_first_buffered_sample - m_first_new_sample)) && (m_first_buffered_sample > m_first_new_sample)) \
    ))



//---------------------------------------------------------
//
//   constructor
//
//---------------------------------------------------------

age::pcm_ring_buffer::pcm_ring_buffer(int max_buffered_samples)
    : m_buffer_size(std::max(1, max_buffered_samples)),
      m_buffer(static_cast<unsigned>(m_buffer_size), pcm_sample())
{
}



//---------------------------------------------------------
//
//   public methods
//
//---------------------------------------------------------

int age::pcm_ring_buffer::get_max_buffered_samples() const
{
    return m_buffer_size;
}

int age::pcm_ring_buffer::get_buffered_samples() const
{
    return m_buffered_samples;
}

int age::pcm_ring_buffer::get_last_discarded_samples() const
{
    return m_last_discarded_samples;
}

const age::pcm_sample* age::pcm_ring_buffer::get_buffered_samples_ptr(int &available_stereo_samples) const
{
    const pcm_sample *result = nullptr;
    int available = 0;

    if (m_buffered_samples > 0)
    {
        AGE_ASSERT(m_buffered_samples <= m_buffer_size);

        // if the buffered samples wrap around to the buffer's beginning,
        // return only the samples available until the buffer's end
        available = (m_buffer_size - m_first_buffered_sample) < m_buffered_samples
                ? m_buffer_size - m_first_buffered_sample
                : m_buffered_samples;

        result = &m_buffer[static_cast<unsigned>(m_first_buffered_sample)];
    }

    available_stereo_samples = available;
    return result;
}



void age::pcm_ring_buffer::add_samples(const pcm_vector &samples_to_add)
{
    AGE_ASSERT(samples_to_add.size() <= int_max);

    int num_samples_to_add = static_cast<int>(samples_to_add.size());
    add_samples(samples_to_add, num_samples_to_add);
}

void age::pcm_ring_buffer::add_samples(const pcm_vector &samples_to_add, int num_samples_to_add)
{
    AGE_ASSERT_BUFFERED_SAMPLES;

    AGE_ASSERT(samples_to_add.size() <= int_max);
    num_samples_to_add = std::min(num_samples_to_add, static_cast<int>(samples_to_add.size()));

    auto first = std::begin(samples_to_add);
    auto last = first + num_samples_to_add;

    // if we add more samples than the buffer can hold in total, we just
    // fill the buffer with the last N samples to add
    if (num_samples_to_add >= m_buffer_size)
    {
        int sample_offset = num_samples_to_add - m_buffer_size;

        m_last_discarded_samples = m_buffered_samples + sample_offset; // set this before changing m_buffered_samples
        m_buffered_samples = m_buffer_size;
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
        auto samples_added = add_samples_with_offset(first, last);
        add_samples_with_offset(first + samples_added, last);
    }

    AGE_ASSERT(m_buffered_samples <= m_buffer_size);
    AGE_ASSERT(m_first_buffered_sample < m_buffer_size);
    AGE_ASSERT(m_first_new_sample < m_buffer_size);
    AGE_ASSERT_BUFFERED_SAMPLES;
}



void age::pcm_ring_buffer::set_to(pcm_sample sample)
{
    AGE_ASSERT_BUFFERED_SAMPLES;

    std::fill(std::begin(m_buffer), std::end(m_buffer), sample);
    m_last_discarded_samples = 0;

    m_buffered_samples = m_buffer_size;
    m_first_buffered_sample = 0;
    m_first_new_sample = 0;

    AGE_ASSERT_BUFFERED_SAMPLES;
}



void age::pcm_ring_buffer::discard_buffered_samples(int samples_to_discard)
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
        AGE_ASSERT((m_buffered_samples > 0) && (m_buffered_samples < m_buffer_size));
        AGE_ASSERT(m_first_buffered_sample != m_first_new_sample);
    }

    AGE_ASSERT(m_buffered_samples <= m_buffer_size);
    AGE_ASSERT(m_first_buffered_sample < m_buffer_size);
    AGE_ASSERT(m_first_new_sample < m_buffer_size);
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

int age::pcm_ring_buffer::add_samples_with_offset(pcm_vector::const_iterator begin, pcm_vector::const_iterator end)
{
    int samples_added = 0;

    std::ptrdiff_t samples_available = end - begin;
    AGE_ASSERT(samples_available < m_buffer_size);

    if (samples_available > 0)
    {
        // calculate the number of samples we can add to the buffer before wrapping around
        if (m_first_new_sample + samples_available > m_buffer_size)
        {
            samples_available = m_buffer_size - m_first_new_sample;
        }

        // copy the samples to the buffer
        std::copy(begin, begin + samples_available, std::begin(m_buffer) + m_first_new_sample);

        samples_added = samples_available;
        m_first_new_sample = add_check_wrap_around(m_first_new_sample, samples_added);
        m_buffered_samples += samples_added;

        // did we discard old samples?
        if (m_buffered_samples > m_buffer_size)
        {
            m_last_discarded_samples += m_buffered_samples - m_buffer_size;
            m_buffered_samples = m_buffer_size;
            m_first_buffered_sample = m_first_new_sample;
        }
    }

    LOG("added " << samples_added << " samples");
    return samples_added;
}



int age::pcm_ring_buffer::add_check_wrap_around(int v1, int v2) const
{
    int result = v1 + v2;

    if (result >= m_buffer_size)
    {
        result -= m_buffer_size;
    }

    AGE_ASSERT(result >= 0);
    AGE_ASSERT(result < m_buffer_size);
    return result;
}
