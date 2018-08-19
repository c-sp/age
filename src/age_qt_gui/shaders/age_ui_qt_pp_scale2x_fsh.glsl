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

// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

/*
  ###############################
  ##                           ##
  ##     SCALE2X ALGORITHM     ##
  ##                           ##
  ##  https://www.scale2x.it/  ##
  ##                           ##
  ###############################


  Pixel grid of texture being scaled
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  +---+---+---+
  |   | B |   |
  +---+---+---+
  | D | E | F |
  +---+---+---+
  |   | H |   |
  +---+---+---+


  Pixel being scaled (E)
  ~~~~~~~~~~~~~~~~~~~~~~

  +----+----+
  | E0 | E1 |
  +----+----+
  | E2 | E3 |
  +----+----+


  Scaling logic
  ~~~~~~~~~~~~~

  if (B != H && D != F)
  {
    E0 = D == B ? D : E;
    E1 = F == B ? F : E;
    E2 = D == H ? D : E;
    E3 = F == H ? F : E;
  }
  else
  {
    E0 = E;
    E1 = E;
    E2 = E;
    E3 = E;
  }

*/

uniform sampler2D texture;

uniform vec2 u_texture_size;
const float eq_threshold = 0.001;


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord) * 0.5;

    vec2 tex_coord_b = tex_coord + vec2( 0.0,  1.0);
    vec2 tex_coord_d = tex_coord + vec2(-1.0,  0.0);
    vec2 tex_coord_e = tex_coord;
    vec2 tex_coord_f = tex_coord + vec2( 1.0,  0.0);
    vec2 tex_coord_h = tex_coord + vec2( 0.0, -1.0);

    tex_coord_b /= u_texture_size;
    tex_coord_d /= u_texture_size;
    tex_coord_e /= u_texture_size;
    tex_coord_f /= u_texture_size;
    tex_coord_h /= u_texture_size;

    vec4 b = texture2D(texture, tex_coord_b);
    vec4 d = texture2D(texture, tex_coord_d);
    vec4 e = texture2D(texture, tex_coord_e);
    vec4 f = texture2D(texture, tex_coord_f);
    vec4 h = texture2D(texture, tex_coord_h);

    vec2 e_fract = fract(tex_coord);
    vec4 df = (e_fract.x < 0.5) ? d : f;
    vec4 hb = (e_fract.y < 0.5) ? h : b;

    bool df_eq_hb = distance(df, hb) < eq_threshold;
    bool b_neq_h = distance(b, h) > eq_threshold;
    bool d_neq_f = distance(d, f) > eq_threshold;

    // mix D-or-F and B-or-H if we consider them equal by threshold
    vec4 color = (df_eq_hb && b_neq_h && d_neq_f) ? mix(df, hb, 0.5) : e;

    gl_FragColor = color;
}
