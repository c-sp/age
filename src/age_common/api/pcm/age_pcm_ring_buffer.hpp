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

#ifndef AGE_PCM_RING_BUFFER_HPP
#define AGE_PCM_RING_BUFFER_HPP

//!
//! \file
//!

#include <age_types.hpp>
#include <pcm/age_pcm_frame.hpp>



namespace age
{

    //!
    //! \brief Provides a simple ring buffer to store a fixed amount of {@link pcm_frame}s.
    //!
    //! While samples can be constantly added to the buffer even if it is completely filled, only the last \c C
    //! will be kept, where \c C is the buffer's capacity.
    //! The main methods for interacting with this ring buffer are add_samples(), get_buffered_samples_ptr() and
    //! discard_buffered_samples().
    //!
    class pcm_ring_buffer
    {
    public:
        //!
        //! \brief Create a ring buffer of the specified capacity.
        //!
        //! \param max_buffered_samples The ring buffer's capacity.
        //! This is the maximal number of {@link pcm_frame}s this ring buffer can store.
        //! Values smaller than 1 are treated as 1.
        //!
        explicit pcm_ring_buffer(int max_buffered_samples);

        //!
        //! \brief Get the ring buffer's capacity.
        //!
        //! \return The ring buffer's capacity.
        //! This is the maximal number of {@link pcm_frame}s this ring buffer can store.
        //! The capacity is guaranteed to be greater than or equal to 1.
        //!
        [[nodiscard]] int get_max_buffered_samples() const;

        //!
        //! \brief Get the actual number of {@link pcm_frame}s currently buffered.
        //!
        //! \return The number of {@link pcm_frame}s currently buffered.
        //! This value is always greater than or equal to 0 (zero).
        //!
        [[nodiscard]] int get_buffered_samples() const;

        //!
        //! \brief Get a pointer to the currently buffered {@link pcm_frame}s.
        //!
        //! Since buffered samples might wrap around from the internal buffer's end to the buffer's beginning, calling
        //! this method immediately after discarding the returned samples using discard_buffered_samples()
        //! might return even more buffered samples.
        //! If you want to be sure to get all buffered samples, call the combination of get_buffered_samples_ptr()
        //! and discard_buffered_samples() twice in a row with the appropriate parameters.
        //!
        //! \param available_stereo_samples This variable will be set to the number of samples available at the
        //! returned pointer.
        //! If no samples are available, it will be set to 0 (zero).
        //!
        //! \return A pointer to the currently buffered {@link pcm_frame}s.
        //! If no samples are buffered, \c nullptr will be returned.
        //!
        const pcm_frame* get_buffered_samples_ptr(int& available_stereo_samples) const;



        //!
        //! \brief Add the specified {@link pcm_frame}s to this ring buffer.
        //!
        //! This convenience method simply calls
        //! add_samples(const pcm_vector &samples_to_add, int num_samples_to_add)
        //! with \c num_samples_to_add set to the size of \c samples_to_add.
        //!
        //! \param samples_to_add The {@link pcm_frame}s to be added to this ring buffer.
        //!
        void add_samples(const pcm_vector& samples_to_add);

        //!
        //! \brief Add the specified {@link pcm_frame}s to this ring buffer.
        //!
        //! Add the first \c num_samples_to_add samples of \c samples_to_add to this ring buffer.
        //! If \c num_samples_to_add is bigger than the size of \c samples_to_add, the whole contents
        //! of \c samples_to_add will be added to this ring buffer.
        //!
        //! If not all samples can be buffered because this ring buffer does not have enough free space
        //! available, older samples will be discarded.
        //! If more samples shall be buffered than this ring buffer can hold, only the last \c C samples
        //! will be stored into the ring buffer, where \c C is the ring buffer's capacity.
        //!
        //! \param samples_to_add The {@link pcm_frame}s to be added to this ring buffer.
        //! \param num_samples_to_add The number of samples to copy from \c samples_to_add into
        //! this ring buffer.
        //!
        void add_samples(const pcm_vector& samples_to_add, int num_samples_to_add);

        //!
        //! \brief Replace the buffered samples with the specified pcm_frame.
        //!
        //! This will discard the buffer's current content and fill the whole buffer with the specified sample.
        //! Thus, when this method finishes, get_buffered_samples() will be equal to get_max_buffered_samples().
        //!
        //! \param sample The pcm_frame to fill the buffer with.
        //!
        void set_to(pcm_frame sample);

        //!
        //! \brief Discard the specified amount of buffered {@link pcm_frame}s.
        //!
        //! Calling this method causes the pointer returned by get_buffered_samples_ptr() to move forward by
        //! \c samples_to_discard samples.
        //! Usually this method is called after get_buffered_samples_ptr() was called and the returned samples have
        //! been processed.
        //! If \c samples_to_discard is greater than the number of currently buffered samples, the outcome will be
        //! the same as if \c samples_to_discard was the exact number of buffered samples.
        //!
        //! \param samples_to_discard The number of buffered {@link pcm_frame}s to discard.
        //!
        void discard_buffered_samples(int samples_to_discard);

        //!
        //! \brief Clear the whole ring buffer.
        //!
        //! Discard all currently buffered {@link pcm_frame}s.
        //!
        void clear();



    private:
        int               add_samples_with_offset(pcm_vector::const_iterator begin, pcm_vector::const_iterator end);
        [[nodiscard]] int add_check_wrap_around(int v1, int v2) const;

        int        m_buffer_size;
        pcm_vector m_buffer;
        int        m_last_discarded_samples = 0;

        int m_buffered_samples      = 0;
        int m_first_buffered_sample = 0;
        int m_first_new_sample      = 0;
    };

} // namespace age



#endif // AGE_PCM_RING_BUFFER_HPP
