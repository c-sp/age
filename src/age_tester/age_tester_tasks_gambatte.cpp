//
// Copyright 2021 Christoph Sprenger
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

#include "age_tester_tasks.hpp"



namespace
{
    //!
    //! gambatte tests run for 15 frames
    //! (see gambatte/test/testrunner.cpp)
    //!
    constexpr int test_frames = 15;

    //!
    //! gambatte "outaudio" tests run for 15 frames,
    //! the last 35112 samples are then checked
    //! (see gambatte/test/testrunner.cpp)
    //!
    constexpr int samples_per_frame = 35112;

    void run_test(age::gb_emulator &emulator)
    {
        AGE_ASSERT(emulator.get_pcm_sampling_rate() == emulator.get_cycles_per_second() / 2);
        for (int i = test_frames; i >= 0; --i)
        {
            emulator.emulate(samples_per_frame * 2);
        }
    }



    std::optional<bool> parse_audio_string(const std::string &str,
                                           const std::string &audio_prefix)
    {
        auto pos = str.find(audio_prefix);
        if ((pos == std::string::npos) || (pos + audio_prefix.length() >= str.length()))
        {
            return {};
        }
        auto out_char = str.at(pos + audio_prefix.length());
        switch (out_char)
        {
            case '0':
                return {false};
            case '1':
                return {true};
            default:
                return {};
        }
    }

    age::tester::run_test_t audio_test(bool expect_audio_out)
    {
        return [=](age::gb_emulator &emulator) {
            run_test(emulator);

            // evaluate test result by checking the first X samples of the last emulation iteration
            // (similar to gambatte/test/testrunner.cpp)
            const age::pcm_vector &audio_buffer = emulator.get_audio_buffer();
            AGE_ASSERT(audio_buffer.size() >= samples_per_frame);

            const age::pcm_sample first_sample = emulator.get_audio_buffer()[0];
            auto count = std::count(begin(audio_buffer),
                                    begin(audio_buffer) + samples_per_frame,
                                    first_sample);

            return expect_audio_out == (count != samples_per_frame);
        };
    }

} // namespace



bool age::tester::schedule_rom_gambatte(const std::filesystem::path &rom_path,
                                        const schedule_test_t &schedule)
{
    auto rom_contents = load_rom_file(rom_path);
    auto filename = rom_path.filename().string();

    auto out = parse_audio_string(filename, "_dmg08_cgb04c_outaudio");
    if (out.has_value())
    {
        schedule(rom_contents, gb_hardware::cgb, audio_test(out.value()));
        schedule(rom_contents, gb_hardware::dmg, audio_test(out.value()));
        return true;
    }
    out = parse_audio_string(filename, "_cgb04c_outaudio");
    if (out.has_value())
    {
        schedule(rom_contents, gb_hardware::cgb, audio_test(out.value()));
        return true;
    }
    out = parse_audio_string(filename, "_dmg08_outaudio");
    if (out.has_value())
    {
        schedule(rom_contents, gb_hardware::dmg, audio_test(out.value()));
        return true;
    }

    return false;
}
