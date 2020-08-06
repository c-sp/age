//
// Copyright 2020 Christoph Sprenger
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

  Pixel grid of texture being scaled
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      +---+---+---+
      | A | B |   |
  +---+---+---+---+
  |D_2| D | E | F |
  +---+---+---+---+
      | G | H | I |
      +---+---+---+
          |H_2|
          +---+


  Pixel being scaled (E)
  ~~~~~~~~~~~~~~~~~~~~~~

  +----+----+
  | E0 | E1 |
  +----+----+
  | E2 | E3 |
  +----+----+


  Scaling algorithm example
  ~~~~~~~~~~~~~~~~~~~~~~~~~

  original texture
  +----+----+----+----+    +----+
  |    |####|####|####|    |####|  black texel
  |    |####|####|####|    |####|  (not scaled)
  +----+----+----+----+    +----+
  |    |    |####|####|    |    |  white texel
  |    |    |####|####|    |    |  (not scaled)
  +----+----+----+----+    +----+
  |    |####|    |    |
  |    |####|    |    |    scaled:
  +----+----+----+----+      ## black texel
  |    |####|    |    |      ** interpolated texel
  |    |####|    |    |         white texel
  +----+----+----+----+

  scale2x                  scale2x-age
  +----+----+----+----+    +----+----+----+----+
  |    |####|####|####|    |    |####|####|####|
  |    |####|####|####|    |    |####|####|####|
  +----+----+----+----+    +----+----+----+----+
  |    |    |####|####|    |    |    |####|####|
  |    |    |  ##|####|    |    |    |**##|####|
  +----+----+----+----+    +----+----+----+----+
  |    |####|##  |    |    |    |####|**  |    |
  |    |####|    |    |    |    |####|    |    |
  +----+----+----+----+    +----+----+----+----+
  |    |####|    |    |    |    |####|    |    |
  |    |####|    |    |    |    |####|    |    |
  +----+----+----+----+    +----+----+----+----+

*/

uniform sampler2D texture;

uniform vec2 u_inv_texture_size;

// equality threshold:
// if the distance between two texels is smaller than this
// threshold we consider them equal
const float eq_threshold = 0.01;
const float eq_threshold2 = 2.0 * eq_threshold;
const vec3 eq_threshold_vec3 = vec3(eq_threshold);
const vec4 eq_threshold_vec4 = vec4(eq_threshold);


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
            vec4 e_tmp = mix(adj_horz, adj_vert, 0.5);

            // Instead of just replacing E we check if any adjacent
            // texel would be replaced too.
            // E.g. for E2 we check if D3 or H1 would be replaced.
            // If that is the case we interpolate E2 and G.

            vec4 adj_diag = texture2D(texture, tex_coord_e + offset.xy);

            // check if we can skip the expensive if-clause
            // (for E2 we can skip if G != E)
            if (distance(adj_diag, e) < eq_threshold)
            {
                vec2 offset_diag = vec2(-offset.x, offset.y);

                vec4 interpolate_vec = vec4(
                            // for E2 this would be E != D_2
                            distance(e, texture2D(texture, tex_coord_e + offset.xz * 2.0)),
                            // for E2 this would be G != A
                            distance(adj_diag, texture2D(texture, tex_coord_e - offset_diag)),
                            // for E2 this would be E != H_2
                            distance(e, texture2D(texture, tex_coord_e + offset.zy * 2.0)),
                            // for E2 this would be G != I
                            distance(adj_diag, texture2D(texture, tex_coord_e + offset_diag))
                            );
                bvec4 interpolate = greaterThan(interpolate_vec, eq_threshold_vec4);

                // for E2:
                //      all(xy) -> D3 replaced
                //      all(zw) -> H1 replaced
                if (all(interpolate.xy) || all(interpolate.zw))
                {
                    e_tmp = mix(e_tmp, e, 0.5);
                }
            }

            e = e_tmp;
        }
    }

    gl_FragColor = e;
}
