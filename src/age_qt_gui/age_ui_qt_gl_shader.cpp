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

#include <age_debug.hpp>

#include "age_ui_qt_gl_renderer.hpp"

#if 0
#define LOG(x) AGE_LOG(x)
#else
#define LOG(x)
#endif

constexpr const char *qt_shader_uniform_texture_size = "texture_size";
constexpr const char *qt_shader_scale2x_uniform_eq_threshold = "eq_threshold";



// scale2x shader source

constexpr const char *qt_shader_scale2x_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord_b;"
        "varying vec2 coord_d;"
        "varying vec2 coord_f;"
        "varying vec2 coord_h;"

        "void main ()"
        "{"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"
            "gl_Position = ftransform();"
            "vec2 pixel_ofs_x = vec2(1.0, 0.0) / texture_size;"
            "vec2 pixel_ofs_y = vec2(0.0, 1.0) / texture_size;"
            "coord_d = gl_TexCoord[0].st - pixel_ofs_x;"
            "coord_f = gl_TexCoord[0].st + pixel_ofs_x;"
            "coord_b = gl_TexCoord[0].st + pixel_ofs_y;"
            "coord_h = gl_TexCoord[0].st - pixel_ofs_y;"
        "}";

constexpr const char *qt_shader_scale2x_fss =
        "uniform vec2 texture_size;"
        "uniform float eq_threshold;"

        "uniform sampler2D texture;"

        "varying vec2 coord_b;"
        "varying vec2 coord_d;"
        "varying vec2 coord_f;"
        "varying vec2 coord_h;"

        "void main ()"
        "{"
            "vec4 b = texture2D(texture, coord_b);"
            "vec4 d = texture2D(texture, coord_d);"
            "vec4 e = texture2D(texture, gl_TexCoord[0].st);"
            "vec4 f = texture2D(texture, coord_f);"
            "vec4 h = texture2D(texture, coord_h);"

            "vec2 c_fract = fract(gl_TexCoord[0].st * texture_size);"
            "vec4 eq1 = (c_fract.x < 0.5) ? d : f;"
            "vec4 eq2 = (c_fract.y < 0.5) ? h : b;"

            "bool eq = distance(eq1, eq2) < eq_threshold;"
            "bool neq1 = distance(d, f) > eq_threshold;"
            "bool neq2 = distance(b, h) > eq_threshold;"

            "vec4 color = (eq && neq1 && neq2) ? (eq1 + eq2) * 0.5 : e;"

            "gl_FragColor = color;"
        "}";



// age-scale2x shader source

constexpr const char *qt_shader_age_scale2x_uniform_eq_threshold = "eq_threshold";

constexpr const char *qt_shader_age_scale2x_vss =
        "uniform vec2 texture_size;"
        "uniform float eq_threshold;"

        "varying vec2 pixel_offset_s;"
        "varying vec2 pixel_offset_t;"
        "varying vec2 small_offset;"
        "varying float eq_threshold2;"

        "void main ()"
        "{"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"
            "gl_Position = ftransform();"
            "pixel_offset_s = vec2(1.0, 0.0) / texture_size;"
            "pixel_offset_t = vec2(0.0, 1.0) / texture_size;"
            "small_offset = vec2(0.1, 0.1) / texture_size;"
            "eq_threshold2 = 2.0 * eq_threshold;"
        "}";

constexpr const char *qt_shader_age_scale2x_fss =
        "uniform vec2 texture_size;"
        "uniform float eq_threshold;"
        "uniform sampler2D texture;"

        "varying vec2 pixel_offset_s;"
        "varying vec2 pixel_offset_t;"
        "varying vec2 small_offset;"
        "varying float eq_threshold2;"

        "void main ()"
        "{"
            "vec2 offsetS = pixel_offset_s;"
            "vec2 offsetT = pixel_offset_t;"
            "vec2 f = fract(gl_TexCoord[0].st * texture_size + small_offset);"
            "offsetS.s *= -sign(f.s - 0.5);"
            "offsetT.t *= -sign(f.t - 0.5);"

            "vec4 E = texture2D(texture, gl_TexCoord[0].st);"
            "vec4 D = texture2D(texture, gl_TexCoord[0].st - offsetS);"
            "vec4 H = texture2D(texture, gl_TexCoord[0].st - offsetT);"
            "vec4 color = E;"

            "vec3 test3 = vec3(eq_threshold2 - distance(D, H),"
            "distance(D, texture2D(texture, gl_TexCoord[0].st + offsetS)),"
            "distance(H, texture2D(texture, gl_TexCoord[0].st + offsetT)));"

            "if (all(greaterThan(test3, vec3(eq_threshold))))"
            "{"
                "color = (D + H) * 0.5;"

                "D = texture2D(texture, gl_TexCoord[0].st - offsetS - offsetT);"
                "f = vec2(eq_threshold2 - distance(D, E), distance(H, E));"

                // test for E == G && H != E
                "if (all(greaterThan(f, vec2(eq_threshold))))"
                "{"
                    "vec4 test4 = vec4(distance(D, texture2D(texture, gl_TexCoord[0].st - 2.0 * offsetT)),"
                    "distance(E, texture2D(texture, gl_TexCoord[0].st + offsetS - offsetT)),"
                    "distance(D, texture2D(texture, gl_TexCoord[0].st - 2.0 * offsetS)),"
                    "distance(E, texture2D(texture, gl_TexCoord[0].st - offsetS + offsetT)));"

                    // both changed -> mix color
                    "if (all(greaterThan(test4, vec4(eq_threshold))))"
                    "color = (color + E) * 0.5;"
                "}"
            "}"

            "gl_FragColor = color;"
        "}";



// 3x3 gauss shader source

constexpr const char *qt_shader_gauss_3x3_first_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"

        "void main ()"
        "{"
            "gl_Position = ftransform();"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"

            "coord1 = gl_TexCoord[0].st + vec2(0.0,  1.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st + vec2(0.0, -1.0) / texture_size;"
        "}";

constexpr const char *qt_shader_gauss_3x3_second_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"

        "void main ()"
        "{"
            "gl_Position = ftransform();"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"

            "coord1 = gl_TexCoord[0].st + vec2( 1.0, 0.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st + vec2(-1.0, 0.0) / texture_size;"
        "}";

constexpr const char *qt_shader_gauss_3x3_fss =
        "uniform sampler2D texture;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"

        "void main ()"
        "{"
            "vec4 color = texture2D(texture, coord1) * 1.0"
                       "+ texture2D(texture, gl_TexCoord[0].st) * 4.0"
                       "+ texture2D(texture, coord2) * 1.0;"
            "gl_FragColor = color / 6.0;"
        "}";



// 5x5 gauss shader source

constexpr const char *qt_shader_gauss_5x5_first_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"
        "varying vec2 coord4;"

        "void main ()"
        "{"
            "gl_Position = ftransform();"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"

            "coord1 = gl_TexCoord[0].st + vec2(0.0,  2.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st + vec2(0.0,  1.0) / texture_size;"
            "coord3 = gl_TexCoord[0].st + vec2(0.0, -1.0) / texture_size;"
            "coord4 = gl_TexCoord[0].st + vec2(0.0, -2.0) / texture_size;"
        "}";

constexpr const char *qt_shader_gauss_5x5_second_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"
        "varying vec2 coord4;"

        "void main ()"
        "{"
            "gl_Position = ftransform();"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"

            "coord1 = gl_TexCoord[0].st + vec2( 2.0, 0.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st + vec2( 1.0, 0.0) / texture_size;"
            "coord3 = gl_TexCoord[0].st + vec2(-1.0, 0.0) / texture_size;"
            "coord4 = gl_TexCoord[0].st + vec2(-2.0, 0.0) / texture_size;"
        "}";

constexpr const char *qt_shader_gauss_5x5_fss =
        "uniform sampler2D texture;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"
        "varying vec2 coord4;"

        "void main ()"
        "{"
            "vec4 color = texture2D(texture, coord1) * 1.0"
                       "+ texture2D(texture, coord2) * 4.0"
                       "+ texture2D(texture, gl_TexCoord[0].st) * 7.0"
                       "+ texture2D(texture, coord3) * 4.0"
                       "+ texture2D(texture, coord4) * 1.0;"
            "gl_FragColor = color / 17.0;"
        "}";



// 3x3 emboss shader source

constexpr const char *qt_shader_emboss_3x3_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"

        "void main ()"
        "{"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"
            "gl_Position    = ftransform();"

            "coord1 = gl_TexCoord[0].st + vec2(-1.0, 1.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st;"
            "coord3 = gl_TexCoord[0].st + vec2(1.0, -1.0) / texture_size;"
        "}";

constexpr const char *qt_shader_emboss_3x3_fss =
        "uniform sampler2D texture;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"

        "void main ()"
        "{"
            "vec4 p1 = texture2D(texture, coord1);"
            "vec4 p2 = texture2D(texture, coord2);"
            "vec4 p3 = texture2D(texture, coord3);"

            "vec4 color = -p1 + 4.0 * p2 + p3;"
            "color /= 4.0;"

            "gl_FragColor = color;"
        "}";



// 5x5 emboss shader source

constexpr const char *qt_shader_emboss_5x5_vss =
        "uniform vec2 texture_size;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"
        "varying vec2 coord4;"
        "varying vec2 coord5;"

        "void main ()"
        "{"
            "gl_TexCoord[0] = gl_MultiTexCoord0;"
            "gl_Position    = ftransform();"

            "coord1 = gl_TexCoord[0].st + vec2(-2.0, 2.0) / texture_size;"
            "coord2 = gl_TexCoord[0].st + vec2(-1.0, 1.0) / texture_size;"
            "coord3 = gl_TexCoord[0].st;"
            "coord4 = gl_TexCoord[0].st + vec2(1.0, -1.0) / texture_size;"
            "coord5 = gl_TexCoord[0].st + vec2(2.0, -2.0) / texture_size;"
        "}";

constexpr const char *qt_shader_emboss_5x5_fss =
        "uniform sampler2D texture;"

        "varying vec2 coord1;"
        "varying vec2 coord2;"
        "varying vec2 coord3;"
        "varying vec2 coord4;"
        "varying vec2 coord5;"

        "void main ()"
        "{"
            "vec4 p1 = texture2D(texture, coord1);"
            "vec4 p2 = texture2D(texture, coord2);"
            "vec4 p3 = texture2D(texture, coord3);"
            "vec4 p4 = texture2D(texture, coord4);"
            "vec4 p5 = texture2D(texture, coord5);"

            "vec4 color = -p1 - 2.0 * p2 + 7.0 * p3 + 2.0 * p4 + p5;"
            "color /= 7.0;"

            "gl_FragColor = color;"
        "}";





//---------------------------------------------------------
//
//   shader_program
//
//---------------------------------------------------------

age::qt_gl_renderer::shader_program::shader_program(const char *vertex_shader_source, const char *fragment_shader_source, qt_filter filter)
    : m_scale_factor(get_qt_filter_factor(filter))
{
    AGE_ASSERT(m_scale_factor > 0);

    m_shader_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertex_shader_source);
    m_shader_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_shader_source);
    m_shader_program.link();

    m_uniform_texture_size = m_shader_program.uniformLocation(qt_shader_uniform_texture_size);
    LOG("uniform texture_size id is " << m_uniform_texture_size);
}

age::qt_gl_renderer::shader_program::~shader_program()
{
}

age::uint age::qt_gl_renderer::shader_program::get_scale_factor() const
{
    return m_scale_factor;
}

void age::qt_gl_renderer::shader_program::prepare(GLfloat texture_width, GLfloat texture_height)
{
    m_shader_program.bind();
    m_shader_program.setUniformValue(m_uniform_texture_size, texture_width, texture_height);
    set_uniforms();
}



//---------------------------------------------------------
//
//   shader_program_scale2x
//
//---------------------------------------------------------

age::qt_gl_renderer::shader_program_scale2x::shader_program_scale2x()
    : shader_program(qt_shader_scale2x_vss, qt_shader_scale2x_fss, qt_filter::scale2x)
{
    m_uniform_eq_threshold = m_shader_program.uniformLocation(qt_shader_scale2x_uniform_eq_threshold);
    LOG("uniform eq_threshold id is " << m_uniform_eq_threshold);
}

void age::qt_gl_renderer::shader_program_scale2x::set_uniforms()
{
    m_shader_program.setUniformValue(m_uniform_eq_threshold, 0.01f);
}



//---------------------------------------------------------
//
//   shader_program_age_scale2x
//
//---------------------------------------------------------

age::qt_gl_renderer::shader_program_age_scale2x::shader_program_age_scale2x()
    : shader_program(qt_shader_age_scale2x_vss, qt_shader_age_scale2x_fss, qt_filter::age_scale2x)
{
    m_uniform_eq_threshold = m_shader_program.uniformLocation(qt_shader_age_scale2x_uniform_eq_threshold);
    LOG("uniform eq_threshold id is " << m_uniform_eq_threshold);
}

void age::qt_gl_renderer::shader_program_age_scale2x::set_uniforms()
{
    m_shader_program.setUniformValue(m_uniform_eq_threshold, 0.01f);
}



//---------------------------------------------------------
//
//   shader_program_gauss
//
//---------------------------------------------------------

age::qt_gl_renderer::shader_program_gauss3x3_first_pass::shader_program_gauss3x3_first_pass()
    : shader_program(qt_shader_gauss_3x3_first_vss, qt_shader_gauss_3x3_fss, qt_filter::gauss3x3)
{
}

age::qt_gl_renderer::shader_program_gauss3x3_second_pass::shader_program_gauss3x3_second_pass()
    : shader_program(qt_shader_gauss_3x3_second_vss, qt_shader_gauss_3x3_fss, qt_filter::gauss3x3)
{
}

age::qt_gl_renderer::shader_program_gauss5x5_first_pass::shader_program_gauss5x5_first_pass()
    : shader_program(qt_shader_gauss_5x5_first_vss, qt_shader_gauss_5x5_fss, qt_filter::gauss5x5)
{
}

age::qt_gl_renderer::shader_program_gauss5x5_second_pass::shader_program_gauss5x5_second_pass()
    : shader_program(qt_shader_gauss_5x5_second_vss, qt_shader_gauss_5x5_fss, qt_filter::gauss5x5)
{
}



//---------------------------------------------------------
//
//   shader_program_emboss
//
//---------------------------------------------------------

age::qt_gl_renderer::shader_program_emboss3x3::shader_program_emboss3x3()
    : shader_program(qt_shader_emboss_3x3_vss, qt_shader_emboss_3x3_fss, qt_filter::emboss3x3)
{
}

age::qt_gl_renderer::shader_program_emboss5x5::shader_program_emboss5x5()
    : shader_program(qt_shader_emboss_5x5_vss, qt_shader_emboss_5x5_fss, qt_filter::emboss5x5)
{
}
