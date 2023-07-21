//
// © 2023 Christoph Sprenger <https://github.com/c-sp>
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

#include <gtest/gtest.h>

#include "age_tr_module.hpp"

namespace
{
    constexpr auto cgb_abcd = age::gb_device_type::cgb_abcd;
    constexpr auto cgb_e    = age::gb_device_type::cgb_e;
    constexpr auto dmg      = age::gb_device_type::dmg;
} // namespace



// mooneye-test-suite file names

TEST(AgeTestRunnerParseDeviceTypes, AddSpETiming)
{
    EXPECT_EQ(
        age::tr::parse_device_types("add_sp_e_timing.gb"),
        std::unordered_set({cgb_abcd, cgb_e, dmg}));
}

TEST(AgeTestRunnerParseDeviceTypes, BootDivCgbABCDE)
{
    EXPECT_EQ(
        age::tr::parse_device_types("boot_div-cgbABCDE.gb"),
        std::unordered_set({cgb_abcd, cgb_e}));
}

TEST(AgeTestRunnerParseDeviceTypes, BootDivDmgABCMgb)
{
    EXPECT_EQ(
        age::tr::parse_device_types("boot_div-dmgABCmgb.gb"),
        std::unordered_set({dmg}));
}

//! \todo fix parse_device_types for "boot_div-S.gb"
//TEST(AgeTestRunnerParseDeviceTypes, BootDivS)
//{
//    EXPECT_EQ(
//        age::tr::parse_device_types("boot_div-S.gb"),
//        std::unordered_set<age::gb_device_type>());
//}

TEST(AgeTestRunnerParseDeviceTypes, BootRegsCgb)
{
    EXPECT_EQ(
        age::tr::parse_device_types("boot_regs-cgb.gb"),
        std::unordered_set({cgb_abcd, cgb_e}));
}

TEST(AgeTestRunnerParseDeviceTypes, UnusedHwioC)
{
    EXPECT_EQ(
        age::tr::parse_device_types("unused_hwio-C.gb"),
        std::unordered_set({cgb_abcd, cgb_e}));
}

TEST(AgeTestRunnerParseDeviceTypes, UnusedHwioGS)
{
    EXPECT_EQ(
        age::tr::parse_device_types("unused_hwio-GS.gb"),
        std::unordered_set({dmg}));
}



// age-test-suite file names

TEST(AgeTestRunnerParseDeviceTypes, EiHaltDmgCCgbBCE)
{
    EXPECT_EQ(
        age::tr::parse_device_types("ei-halt-dmgC-cgbBCE.gb"),
        std::unordered_set({dmg, cgb_abcd, cgb_e}));
}

TEST(AgeTestRunnerParseDeviceTypes, LcdAlignLyCgbBC)
{
    EXPECT_EQ(
        age::tr::parse_device_types("lcd-align-ly-cgbBC.gb"),
        std::unordered_set({cgb_abcd}));
}

TEST(AgeTestRunnerParseDeviceTypes, LyNcmBC)
{
    EXPECT_EQ(
        age::tr::parse_device_types("ly-ncmBC.gb"),
        std::unordered_set({cgb_abcd}));
}

TEST(AgeTestRunnerParseDeviceTypes, VramReadNcmBCE)
{
    EXPECT_EQ(
        age::tr::parse_device_types("vram-read-ncmBCE.gb"),
        std::unordered_set({cgb_abcd, cgb_e}));
}



// made up file names

TEST(AgeTestRunnerParseDeviceTypes, MadeUpCGS)
{
    EXPECT_EQ(
        age::tr::parse_device_types("made-up-SCG.gb"),
        std::unordered_set({dmg, cgb_abcd, cgb_e}));
}

TEST(AgeTestRunnerParseDeviceTypes, MadeUpDmgABCCgbABC)
{
    EXPECT_EQ(
        age::tr::parse_device_types("made-up-dmgABC-cgbABC.gb"),
        std::unordered_set({dmg, cgb_abcd}));
}

TEST(AgeTestRunnerParseDeviceTypes, MadeUpDmg)
{
    EXPECT_EQ(
        age::tr::parse_device_types("made-up-dmg.gb"),
        std::unordered_set({dmg}));
}

TEST(AgeTestRunnerParseDeviceTypes, MadeUpNcm)
{
    EXPECT_EQ(
        age::tr::parse_device_types("made-up-ncm.gb"),
        std::unordered_set({cgb_abcd, cgb_e}));
}
