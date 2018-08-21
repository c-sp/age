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
  | A | B | C |
  +---+---+---+
  | D | E | F |
  +---+---+---+
  | G | H | I |
  +---+---+---+


  Pixel being scaled (E)
  ~~~~~~~~~~~~~~~~~~~~~~

  +----+----+
  | E0 | E1 |
  +----+----+
  | E2 | E3 |
  +----+----+


  Scaling algorithm
  ~~~~~~~~~~~~~~~~~

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

uniform vec2 u_inv_texture_size;

// equality threshold:
// if the distance between two texels is smaller than this
// threshold we consider them equal
const float eq_threshold = 0.01;
const float eq_threshold2 = 2.0 * eq_threshold;
const vec3 eq_threshold_vec3 = vec3(eq_threshold);


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord) * 0.5;
    vec2 tex_coord_e = tex_coord * u_inv_texture_size;

    // Calculate the texture coordinate offset for reading
    // adjacent texels depending on which "sub-texel" of
    // E (E0, E1, ...) we are currently processing.
    vec3 offset = vec3(
                // subtract 0.4 to prevent sign(0)
                u_inv_texture_size * sign(fract(tex_coord) - vec2(0.4, 0.4)),
                0.0
                );

    // read E
    vec4 e = texture2D(texture, tex_coord_e);
    // read the to E_n horizontally adjacent texel: either D or F
    vec4 adj_horz = texture2D(texture, tex_coord_e + offset.xz);

    // if E matches it's adjacent texel we can skip any
    // further processing as the result will always be E
    if (distance(e, adj_horz) > eq_threshold)
    {
        // read the to E_n vertically adjacent texel: either H or B
        vec4 adj_vert = texture2D(texture, tex_coord_e + offset.zy);

        vec3 replace_e_vec = vec3(
                    // for E2 this would be: D == H
                    eq_threshold2 - distance(adj_horz, adj_vert),
                    // D != F (or F != D)
                    distance(adj_horz, texture2D(texture, tex_coord_e - offset.xz)),
                    // H != B (or B != H)
                    distance(adj_vert, texture2D(texture, tex_coord_e - offset.zy))
                    );

        bool replace_e = all(greaterThan(replace_e_vec, eq_threshold_vec3));
        if (replace_e)
        {
            e = mix(adj_horz, adj_vert, 0.5);
        }
    }

    gl_FragColor = e;
}



/*
    old, slightly slower code
    -------------------------

    vec2 tex_coord = vec2(gl_FragCoord) * 0.5;

    vec2 tex_coord_b = tex_coord + vec2( 0.0,  1.0);
    vec2 tex_coord_d = tex_coord + vec2(-1.0,  0.0);
    vec2 tex_coord_e = tex_coord;
    vec2 tex_coord_f = tex_coord + vec2( 1.0,  0.0);
    vec2 tex_coord_h = tex_coord + vec2( 0.0, -1.0);

    tex_coord_b *= u_inv_texture_size;
    tex_coord_d *= u_inv_texture_size;
    tex_coord_e *= u_inv_texture_size;
    tex_coord_f *= u_inv_texture_size;
    tex_coord_h *= u_inv_texture_size;

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
*/
