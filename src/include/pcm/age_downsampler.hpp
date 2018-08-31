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

#ifndef AGE_DOWNSAMPLER_HPP
#define AGE_DOWNSAMPLER_HPP

//!
//! \file
//!

#include <functional>
#include <vector>

#include <age_non_copyable.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

class downsampler : public non_copyable
{
public:

    downsampler(int input_sampling_rate, int output_sampling_rate);
    virtual ~downsampler() = default;

    const pcm_vector& get_output_samples() const;

    void clear_output_samples();
    void set_volume(float volume);
    virtual void add_input_samples(const pcm_vector &samples) = 0;

protected:

    void add_output_sample(int16_t left, int16_t right);
    void add_output_sample(pcm_sample sample);

    //!
    //! This variable contains the number of input samples required for
    //! a single output sample. The unsigned integer is treated as fixed
    //! point value with the lower 16 bits treated as fraction.
    //!
    const int m_input_output_ratio;

private:

    static int calculate_ratio(int input_sampling_rate, int output_sampling_rate);

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
    virtual ~downsampler_linear() override = default;

    void add_input_samples(const pcm_vector &samples) override;

private:

    void add_output_sample(const pcm_sample &left_sample, const pcm_sample &right_sample);

    int m_right_sample_index = 1; // 0 = use last sample as left, 1 = use first sample of new samples as left
    int m_right_sample_fraction = 0;

    pcm_sample m_last_input_sample;
};



class downsampler_low_pass : public downsampler
{
public:

    using downsampler::downsampler;
    virtual ~downsampler_low_pass() override = default;

    void add_input_samples(const pcm_vector &samples) override;

    size_t get_fir_size() const;

protected:

    void create_windowed_sinc(double transition_frequency, int filter_order, std::function<double (double, int)> window_weight);

private:

    double calculate_sinc(double n, int filter_order, double transition_frequency);

    std::vector<int32_t> m_fir_values;
    pcm_vector m_prev_samples;

    int m_next_output_index = 0;
    int m_next_output_fraction = 0;
};



class downsampler_kaiser_low_pass : public downsampler_low_pass
{
public:

    downsampler_kaiser_low_pass(int input_sampling_rate, int output_sampling_rate, double ripple);
    virtual ~downsampler_kaiser_low_pass() = default;

private:

    static int calculate_filter_order(double A, double tw);
    static double calculate_beta(double A);
    static double calculate_bessel(double value);
};

} // namespace age



#endif // AGE_DOWNSAMPLER_HPP
