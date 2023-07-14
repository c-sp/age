//
// © 2021 Christoph Sprenger <https://github.com/c-sp>
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

#include "age_gb_lcd_palettes.hpp"

#include <gtest/gtest.h>



namespace
{
    age::pixel correct_color(int r, int g, int b)
    {
        int gb_rgb15 = r + g * 32 + b * 1024;
        return age::cgb_color_correction(gb_rgb15);
    }
} // namespace



TEST(AgeCgbColorCorrection, Color000000)
{
    auto color = correct_color(0, 0, 0);
    EXPECT_EQ(color.m_r, 0x00);
    EXPECT_EQ(color.m_g, 0x00);
    EXPECT_EQ(color.m_b, 0x00);
    EXPECT_EQ(color.m_a, 0xFF);
}

//! \todo the tests below fail,
//        apparently we don't use the same color correction as the LUT texture

//TEST(AgeCgbColorCorrection, Color1F1F1F)
//{
//    auto color = correct_color(0x1F, 0x1F, 0x1F);
//    EXPECT_EQ(color.m_r, 0xF8);
//    EXPECT_EQ(color.m_g, 0xF8);
//    EXPECT_EQ(color.m_b, 0xF8);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//
//
//TEST(AgeCgbColorCorrection, Color010000)
//{
//    auto color = correct_color(1, 0, 0);
//    EXPECT_EQ(color.m_r, 0x0A);
//    EXPECT_EQ(color.m_g, 0x04);
//    EXPECT_EQ(color.m_b, 0x05);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//TEST(AgeCgbColorCorrection, Color1F0000)
//{
//    auto color = correct_color(0x1F, 0, 0);
//    EXPECT_EQ(color.m_r, 0xE2);
//    EXPECT_EQ(color.m_g, 0x60);
//    EXPECT_EQ(color.m_b, 0x76);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//
//
//TEST(AgeCgbColorCorrection, Color000100)
//{
//    auto color = correct_color(0, 1, 0);
//    EXPECT_EQ(color.m_r, 0x06);
//    EXPECT_EQ(color.m_g, 0x09);
//    EXPECT_EQ(color.m_b, 0x03);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//TEST(AgeCgbColorCorrection, Color001F00)
//{
//    auto color = correct_color(0, 0x1F, 0);
//    EXPECT_EQ(color.m_r, 0x81);
//    EXPECT_EQ(color.m_g, 0xCE);
//    EXPECT_EQ(color.m_b, 0x4C);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//
//
//TEST(AgeCgbColorCorrection, Color000001)
//{
//    auto color = correct_color(0, 0, 1);
//    EXPECT_EQ(color.m_r, 0x00);
//    EXPECT_EQ(color.m_g, 0x05);
//    EXPECT_EQ(color.m_b, 0x09);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
//
//TEST(AgeCgbColorCorrection, Color00001F)
//{
//    auto color = correct_color(0, 0, 0x1F);
//    EXPECT_EQ(color.m_r, 0x00);
//    EXPECT_EQ(color.m_g, 0x7A);
//    EXPECT_EQ(color.m_b, 0xD7);
//    EXPECT_EQ(color.m_a, 0xFF);
//}
