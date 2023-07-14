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

#include <cmath>



namespace
{
    constexpr std::array<float, 16> mult_4x4(std::array<float, 16> m1, std::array<float, 16> m2)
    {
        return {
            m1[0] * m2[0] + m1[1] * m2[4] + m1[2] * m2[8] + m1[3] * m2[12],
            m1[0] * m2[1] + m1[1] * m2[5] + m1[2] * m2[9] + m1[3] * m2[13],
            m1[0] * m2[2] + m1[1] * m2[6] + m1[2] * m2[10] + m1[3] * m2[14],
            m1[0] * m2[3] + m1[1] * m2[7] + m1[2] * m2[11] + m1[3] * m2[15],

            m1[4] * m2[0] + m1[5] * m2[4] + m1[6] * m2[8] + m1[7] * m2[12],
            m1[4] * m2[1] + m1[5] * m2[5] + m1[6] * m2[9] + m1[7] * m2[13],
            m1[4] * m2[2] + m1[5] * m2[6] + m1[6] * m2[10] + m1[7] * m2[14],
            m1[4] * m2[3] + m1[5] * m2[7] + m1[6] * m2[11] + m1[7] * m2[15],

            m1[8] * m2[0] + m1[9] * m2[4] + m1[10] * m2[8] + m1[11] * m2[12],
            m1[8] * m2[1] + m1[9] * m2[5] + m1[10] * m2[9] + m1[11] * m2[13],
            m1[8] * m2[2] + m1[9] * m2[6] + m1[10] * m2[10] + m1[11] * m2[14],
            m1[8] * m2[3] + m1[9] * m2[7] + m1[10] * m2[11] + m1[11] * m2[15],

            m1[12] * m2[0] + m1[13] * m2[4] + m1[14] * m2[8] + m1[15] * m2[12],
            m1[12] * m2[1] + m1[13] * m2[5] + m1[14] * m2[9] + m1[15] * m2[13],
            m1[12] * m2[2] + m1[13] * m2[6] + m1[14] * m2[10] + m1[15] * m2[14],
            m1[12] * m2[3] + m1[13] * m2[7] + m1[14] * m2[11] + m1[15] * m2[15],
        };
    }

    //
    // the following color-correction code is based on
    // https://forums.libretro.com/t/real-gba-and-ds-phat-colors/1540/190
    //

    constexpr float target_gamma   = 2.2;
    constexpr float display_gamma  = 2.2;
    constexpr float lighten_screen = 1.0;
    constexpr float contrast       = 1.0;
    constexpr float lum            = 0.94;
    constexpr float sat            = 1.0;

    // (note that GLSL matrices are filled column by column)
    //
    //                   r    g    b    black
    // mat4 color = mat4(r,   rg,  rb,  0.0,  //red channel    <- column 1
    //                   gr,  g,   gb,  0.0,  //green channel  <- column 2
    //                   br,  bg,  b,   0.0,  //blue channel   <- column 3
    //                   blr, blg, blb, 0.0); //alpha channel; these numbers do nothing for our purposes.
    //
    // mat4 adjust = mat4((1.0 - sat) * 0.3086 + sat, (1.0 - sat) * 0.3086,       (1.0 - sat) * 0.3086,       1.0,
    //                    (1.0 - sat) * 0.6094,       (1.0 - sat) * 0.6094 + sat, (1.0 - sat) * 0.6094,       1.0,
    //                    (1.0 - sat) * 0.0820,       (1.0 - sat) * 0.0820,       (1.0 - sat) * 0.0820 + sat, 1.0,
    //                    0.0,                        0.0,                        0.0,                        1.0);
    // color *= adjust;
    constexpr auto color = mult_4x4({0.82, 0.24, -0.06, 0,
                                     0.125, 0.665, 0.21, 0,
                                     0.195, 0.075, 0.73, 0,
                                     0, 0, 0, 0},
                                    {(1.0 - sat) * 0.3086 + sat, (1.0 - sat) * 0.6094, (1.0 - sat) * 0.0820, 0,
                                     (1.0 - sat) * 0.3086, (1.0 - sat) * 0.6094 + sat, (1.0 - sat) * 0.0820, 0,
                                     (1.0 - sat) * 0.3086, (1.0 - sat) * 0.6094, (1.0 - sat) * 0.0820 + sat, 0,
                                     1, 1, 1, 1});

    float gb_to_float(unsigned gb)
    {
        return static_cast<float>(gb & 0x1FU) / 31.0F;
    }

    float clamp(float value)
    {
        return std::min(std::max(value, 0.F), 1.F);
    }

    int clamp_to_uint8(float value)
    {
        // return zero in case of NaN
        return std::min(std::max(static_cast<int>(value * 255), 0), 255);
    }

} // namespace



age::pixel age::gb_lcd_palettes::lookup_cgb_color(unsigned cgb_rgb15)
{
    if (m_cgb_color_lut.empty())
    {
        m_cgb_color_lut.reserve(0x8000);
        for (int gb_rgb15 = 0; gb_rgb15 < 0x8000; ++gb_rgb15)
        {
            m_cgb_color_lut.emplace_back(cgb_color_correction(gb_rgb15));
        }
    }
    return m_cgb_color_lut[cgb_rgb15 & 0x7FFFU];
}



age::pixel age::cgb_color_correction(unsigned cgb_rgb15)
{
    float r = gb_to_float(cgb_rgb15);
    float g = gb_to_float(cgb_rgb15 >> 5);
    float b = gb_to_float(cgb_rgb15 >> 10);
    // float a = 1.0;

    // vec4 screen = pow(texture(Source, vTexCoord), vec4(target_gamma + (lighten_screen * -1.0))).rgba;
    r = std::pow(r, target_gamma + (lighten_screen * -1.0F));
    g = std::pow(g, target_gamma + (lighten_screen * -1.0F));
    b = std::pow(b, target_gamma + (lighten_screen * -1.0F));
    // a = std::pow(a, target_gamma + (lighten_screen * -1.0F));

    // vec4 avglum = vec4(0.5);
    // screen = mix(screen, avglum, (1.0 - contrast));
    r = r * contrast + 0.5F * (1 - contrast);
    g = g * contrast + 0.5F * (1 - contrast);
    b = b * contrast + 0.5F * (1 - contrast);
    // a = a * contrast + 0.5F * (1 - contrast);

    // screen = clamp(screen * lum, 0.0, 1.0);
    b = clamp(b * lum);
    g = clamp(g * lum);
    r = clamp(r * lum);
    // a = clamp(a * lum);

    // screen = color * screen;
    r = color[0] * r + color[1] * g + color[2] * b;  // + color[3] * a;
    g = color[4] * r + color[5] * g + color[6] * b;  // + color[7] * a;
    b = color[8] * r + color[9] * g + color[10] * b; // + color[11] * a;
    // a = color[12] * r + color[13] * g + color[14] * b + color[15] * a;

    // FragColor = pow(screen, vec4(1.0 / display_gamma));
    r = std::pow(r, 1.0F / display_gamma);
    g = std::pow(g, 1.0F / display_gamma);
    b = std::pow(b, 1.0F / display_gamma);
    // a = std::pow(a, 1.0F / display_gamma);

    return age::pixel{clamp_to_uint8(r), clamp_to_uint8(g), clamp_to_uint8(b)};
}
