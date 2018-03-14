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
#include <cmath>
#include <ios> // std::hex
#include <limits>

#include <age_debug.hpp>

#include "age_ui_downsampler.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

#if 0
#define LOG_FIR(x) AGE_LOG(x)
#else
#define LOG_FIR(x)
#endif

constexpr double pi = 3.14159265358979323846;





//---------------------------------------------------------
//
//   downsampler
//
//---------------------------------------------------------

age::downsampler::downsampler(uint input_sampling_rate, uint output_sampling_rate)
    : m_input_output_ratio(calculate_ratio(input_sampling_rate, output_sampling_rate)),
      m_output_input_ratio(calculate_ratio(output_sampling_rate, input_sampling_rate))
{
    AGE_ASSERT(m_input_output_ratio > 0x10000); // we're downsampling
    AGE_ASSERT(m_output_input_ratio < 0x10000); // we're downsampling
}



const age::pcm_vector& age::downsampler::get_output_samples() const
{
    return m_output_samples;
}

void age::downsampler::clear_output_samples()
{
    m_output_samples.clear();
}

void age::downsampler::set_volume(float volume)
{
    m_volume = std::min(1.f, std::max(0.f, volume));
}

void age::downsampler::add_output_sample(int16 left, int16 right)
{
    add_output_sample(pcm_sample(left, right));
}

void age::downsampler::add_output_sample(pcm_sample sample)
{
    sample *= m_volume;
    m_output_samples.push_back(sample);
}



age::uint age::downsampler::calculate_ratio(uint value1, uint value2)
{
    AGE_ASSERT(value1 > 0);
    AGE_ASSERT(value2 > 0);

    double fp_ratio = value1;
    fp_ratio /= value2;
    uint ratio = static_cast<uint>(fp_ratio * 0x10000);

    LOG(value1 << " / " << value2 << " ratio: " << (ratio >> 16) << " + " << (ratio & 0xFFFF) << "/65536");
    LOG("floating point ratio was: " << fp_ratio << ", diff is " << (fp_ratio - ratio / 65536.0));

    return ratio;
}





//---------------------------------------------------------
//
//   downsampler_linear
//
//---------------------------------------------------------

void age::downsampler_linear::add_input_samples(const pcm_vector &samples)
{
    if (!samples.empty())
    {
        AGE_ASSERT(m_right_sample_index < (m_input_output_ratio >> 16) + 0x10000);
        AGE_ASSERT(m_right_sample_fraction < 0x10000);

        // next_right_sample_index == 0  ->  based on m_last_input_sample
        while (m_right_sample_index == 0)
        {
            add_output_sample(m_last_input_sample, samples[m_right_sample_index]);
        }

        // next_right_sample_index >= 1  ->  both samples taken from vector
        while (m_right_sample_index < samples.size())
        {
            add_output_sample(samples[m_right_sample_index - 1], samples[m_right_sample_index]);
        }

        // normalize
        AGE_ASSERT(m_right_sample_index >= samples.size());

        m_right_sample_index -= samples.size();
        m_last_input_sample = samples[samples.size() - 1];

        AGE_ASSERT(m_right_sample_index < (m_input_output_ratio >> 16) + 0x10000);
        AGE_ASSERT(m_right_sample_fraction < 0x10000);
    }
}



void age::downsampler_linear::add_output_sample(const pcm_sample &left_sample, const pcm_sample &right_sample)
{
    int32 fraction = static_cast<int32>(m_right_sample_fraction);
    int32 diff[2], interpolated[2];

    diff[0] = right_sample.m_samples[0];
    diff[1] = right_sample.m_samples[1];
    diff[0] -= left_sample.m_samples[0];
    diff[1] -= left_sample.m_samples[1];

    interpolated[0] = left_sample.m_samples[0] + ((diff[0] * fraction) >> 16);
    interpolated[1] = left_sample.m_samples[1] + ((diff[1] * fraction) >> 16);

    downsampler::add_output_sample(static_cast<int16>(interpolated[0]), static_cast<int16>(interpolated[1]));

    m_right_sample_fraction += m_input_output_ratio;
    m_right_sample_index += m_right_sample_fraction >> 16;
    m_right_sample_fraction &= 0xFFFF;
}





//---------------------------------------------------------
//
//   downsampler_low_pass
//
//---------------------------------------------------------

void age::downsampler_low_pass::add_input_samples(const pcm_vector &samples)
{
    // inefficient: we copy all samples into a single buffer before filtering
    uint num_old_samples = m_prev_samples.size();
    m_prev_samples.resize(num_old_samples + samples.size());
    std::copy(std::begin(samples), std::end(samples), std::begin(m_prev_samples) + num_old_samples);

    // resample
    if (m_prev_samples.size() >= m_fir_values.size())
    {
        uint max = m_prev_samples.size() - m_fir_values.size();

        while (m_next_output_index <= max)
        {
            int64 result0 = 0, result1 = 0;

            for (uint i = 0; i < m_fir_values.size(); ++i)
            {
                int64 sample0 = m_prev_samples[m_next_output_index + i].m_samples[0];
                int64 sample1 = m_prev_samples[m_next_output_index + i].m_samples[1];

                int32 fir_value = m_fir_values[i];
                sample0 *= fir_value;
                sample1 *= fir_value;

                result0 += sample0;
                result1 += sample1;
            }

            result0 >>= 31;
            result1 >>= 31;
            add_output_sample(static_cast<int16>(result0), static_cast<int16>(result1));

            m_next_output_fraction += m_input_output_ratio;
            m_next_output_index += m_next_output_fraction >> 16;
            m_next_output_fraction &= 0xFFFF;
        }

        // discard processed samples
        size_t samples_to_discard = std::min(m_next_output_index, m_prev_samples.size());

        auto begin = std::begin(m_prev_samples);
        m_prev_samples.erase(begin, begin + samples_to_discard);

        m_next_output_index -= samples_to_discard;
    }
}



age::uint age::downsampler_low_pass::get_fir_size() const
{
    return m_fir_values.size();
}



void age::downsampler_low_pass::create_windowed_sinc(double transition_frequency, uint filter_order, std::function<double (double, uint)> window_weight)
{
    AGE_ASSERT(transition_frequency > 0.0);
    AGE_ASSERT(!(transition_frequency > 0.5));
    AGE_ASSERT(filter_order > 0);

    for (uint n = 0; n <= filter_order; ++n)
    {
        double dn = n;

        double sinc = calculate_sinc(dn, filter_order, transition_frequency);
        double weight = window_weight(dn, filter_order);
        double windowed = sinc * weight;

        AGE_ASSERT(!(windowed > 1.0));
        AGE_ASSERT(!(windowed < -1.0));

        int32 i = static_cast<int32>(windowed * std::numeric_limits<int32>::max());
        int32 entry = i;

        LOG_FIR("n " << dn << ":   " << sinc << " * " << weight << " = " << windowed
                << "   (0x" << std::hex << entry << std::dec << ")");

        m_fir_values.push_back(entry);
    }
}



double age::downsampler_low_pass::calculate_sinc(double n, uint filter_order, double transition_frequency)
{
    double result = 2 * transition_frequency;

    double v = n - (filter_order / 2.0);
    if ((v > 0) || (v < 0))
    {
        v *= pi;
        result = std::sin(result * v) / v;
    }

    return result;
}





//---------------------------------------------------------
//
//   downsampler_kaiser_low_pass
//
//---------------------------------------------------------

//
// low pass filter with kaiser window:
//      http://www.labbookpages.co.uk/audio/firWindowing.html
//      https://www.youtube.com/watch?v=2C5ueijU4qg
//
//  - We don't use the approach of interpolation followed by decimation since
//    that would require very big filter kernels depending on the input/output
//    sampling rates (e.g. gameboy 2097152 -> audio device 44100: interpolate
//    by 11025, decimate by 524288).
//
//  - Current approach: use continuous lowpass FIR simulated by interpolating
//    between samples of the FIR stored in a table.
//

age::downsampler_kaiser_low_pass::downsampler_kaiser_low_pass(uint input_sampling_rate, uint output_sampling_rate, double ripple)
    : downsampler_low_pass(input_sampling_rate, output_sampling_rate)
{
    //
    //           lowpass filter using a kaiser window
    //                         based on
    //   http://www.labbookpages.co.uk/audio/firWindowing.html
    // ---------------------------------------------------------
    //
    // - Depending on the filter order the transition band around the
    //   transition frequency may be wider or smaller, but it's
    //   always there.
    //      -> A smaller transition band requires more calculations
    //         since the FIR grows.
    //
    // - If possible, we include all frequencies up to 15 Khz in the
    //   passband and use the gap between 15 Khz and the Nyquist
    //   Frequency as transition band.
    //      -> Attenuated frequencies in the transition band are kept
    //         in the not audible range.
    //      -> The stopband (beginning right after the transition band)
    //         contains all frequencies that would add aliasing.
    //

    // Use the maximal audible frequency to limit the passband,
    // if the output sampling rate allows it.
    // (According to Wikipedia this should actually be 20000 hz,
    // but we use 15000 hz for a smaller FIR)
    constexpr uint max_audible_frequency_hz = 15000;
    LOG("using " << max_audible_frequency_hz << " hz as maximal audible frequency");

    // calculate the Nyquist Frequency
    uint nyquist_frequency_hz = output_sampling_rate >> 1;
    LOG("output sampling rate " << output_sampling_rate << ", Nyquist Frequency " << nyquist_frequency_hz << " hz");

    // calculate the transition width:
    //  - not smaller than 10% of the Nyquist Frequency
    //  - for higher output sampling rates use the range [max_audible;nyquist]
    uint transition_width_hz = nyquist_frequency_hz / 10;
    if (max_audible_frequency_hz + transition_width_hz < nyquist_frequency_hz)
    {
        transition_width_hz = nyquist_frequency_hz - max_audible_frequency_hz;
    }

    double transition_width = transition_width_hz;
    transition_width /= input_sampling_rate; // we're creating the FIR for the input signal

    LOG("transition width " << transition_width << " (" << transition_width_hz << " hz)");

    // calculate transition frequency that marks the
    // center of the transition band
    uint transition_frequency_hz = nyquist_frequency_hz - transition_width_hz / 2;
    double transition_frequency = transition_frequency_hz;
    transition_frequency /= input_sampling_rate; // we're creating the FIR for the input signal

    LOG("transition frequency " << transition_frequency << " (" << transition_frequency_hz << " hz)");

    // calculate Kaiser Window parameters
    double A = -20 * std::log10(ripple);
    double tw = 2 * pi * transition_width;

    uint M = calculate_filter_order(A, tw);
    M += M & 1; // make even
    double beta = calculate_beta(A);

    LOG("ripple " << ripple << ", A " << A << ", tw " << tw << ", beta " << beta << ", filter order " << M);

    // window weight value calculation method
    double beta_bessel = calculate_bessel(beta);

    auto window_weight = [=](double n, uint filter_order)
    {
        double v = 2 * n / filter_order - 1;
        v *= v;
        v = beta * std::sqrt(1 - v);

        double v_bessel = calculate_bessel(v);
        return v_bessel / beta_bessel;
    };

    // calculate filter values
    create_windowed_sinc(transition_frequency, M, window_weight);

    LOG("");
    LOG("----------------------------------------------------------------------------");
    LOG("");
}



age::uint age::downsampler_kaiser_low_pass::calculate_filter_order(double A, double tw)
{
    double dM;

    if (A > 21.0)
    {
        dM = (A - 7.95) / (2.285 * tw);
    }
    else
    {
        dM = 5.79 / tw;
    }

    uint M = static_cast<uint>(dM) + 1;
    return M;
}



double age::downsampler_kaiser_low_pass::calculate_beta(double A)
{
    double beta;

    if (A > 50)
    {
        beta = 0.1102 * (A - 8.7);
    }
    else if (A > 21)
    {
        beta = 0.5842 * std::pow(A - 21, 0.4);
        beta += 0.07886 * (A - 21);
    }
    else
    {
        beta = 0;
    }

    return beta;
}



double age::downsampler_kaiser_low_pass::calculate_bessel(double value)
{
    double result = 1;

    value /= 2;
    double factorial = 1;

    for (uint i = 1; i <= 10; ++i)
    {
        double v1 = std::pow(value, 2 * i);
        double v2 = factorial * factorial;

        result += v1 / v2;

        factorial *= i;
    }

    return result;
}
