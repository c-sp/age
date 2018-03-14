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

#ifndef AGE_UI_DOWNSAMPLER_HPP
#define AGE_UI_DOWNSAMPLER_HPP

//!
//! \file
//!

#include <functional>
#include <vector>

#include <age_audio.hpp>
#include <age_non_copyable.hpp>
#include <age_types.hpp>



namespace age
{

class downsampler : public non_copyable
{
public:

    downsampler(uint input_sampling_rate, uint output_sampling_rate);
    virtual ~downsampler() = default;

    const pcm_vector& get_output_samples() const;

    void clear_output_samples();
    void set_volume(float volume);
    virtual void add_input_samples(const pcm_vector &samples) = 0;

protected:

    void add_output_sample(int16 left, int16 right);
    void add_output_sample(lpcm_stereo_sample sample);

    //!
    //! This variable contains the number of input samples required for
    //! a single output sample. The unsigned integer is treated as fixed
    //! point value with the lower 16 bits treated as fraction.
    //!
    const uint m_input_output_ratio;
    const uint m_output_input_ratio;

private:

    static uint calculate_ratio(uint input_sampling_rate, uint output_sampling_rate);

    pcm_vector m_output_samples;
    float m_volume = 1;
};



//!
//! \brief A downsampler_linear resamples audio data by interpolating an output
//! sample from the two nearest input samples.
//!
class downsampler_linear : public downsampler
{
public:

    using downsampler::downsampler;
    virtual ~downsampler_linear() = default;

    void add_input_samples(const pcm_vector &samples) override;

private:

    void add_output_sample(const lpcm_stereo_sample &left_sample, const lpcm_stereo_sample &right_sample);

    uint m_right_sample_index = 1; // 0 = use last sample as left, 1 = use first sample of new samples as left
    uint m_right_sample_fraction = 0;

    lpcm_stereo_sample m_last_input_sample;
};



class downsampler_low_pass : public downsampler
{
public:

    using downsampler::downsampler;
    virtual ~downsampler_low_pass() = default;

    void add_input_samples(const pcm_vector &samples) override;

    uint get_fir_size() const;

protected:

    void create_windowed_sinc(double transition_frequency, uint filter_order, std::function<double (double, uint)> window_weight);

private:

    double calculate_sinc(double n, uint filter_order, double transition_frequency);

    std::vector<int32> m_fir_values;
    pcm_vector m_prev_samples;

    uint m_next_output_index = 0;
    uint m_next_output_fraction = 0;
};



class downsampler_kaiser_low_pass : public downsampler_low_pass
{
public:

    downsampler_kaiser_low_pass(uint input_sampling_rate, uint output_sampling_rate, double ripple);
    virtual ~downsampler_kaiser_low_pass() = default;

private:

    static uint calculate_filter_order(double A, double tw);
    static double calculate_beta(double A);
    static double calculate_bessel(double value);
};

} // namespace age



#endif // AGE_UI_DOWNSAMPLER_HPP
