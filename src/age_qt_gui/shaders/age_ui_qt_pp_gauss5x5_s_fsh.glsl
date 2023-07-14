//
// © 2018 Christoph Sprenger <https://github.com/c-sp>
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

uniform sampler2D texture;

uniform vec2 u_inv_texture_size;


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord);

    vec2 tex_coord1 = tex_coord + vec2(-2.0, 0.0);
    vec2 tex_coord2 = tex_coord + vec2(-1.0, 0.0);
    vec2 tex_coord3 = tex_coord;
    vec2 tex_coord4 = tex_coord + vec2( 1.0, 0.0);
    vec2 tex_coord5 = tex_coord + vec2( 2.0, 0.0);

    tex_coord1 *= u_inv_texture_size;
    tex_coord2 *= u_inv_texture_size;
    tex_coord3 *= u_inv_texture_size;
    tex_coord4 *= u_inv_texture_size;
    tex_coord5 *= u_inv_texture_size;

    vec4 c1 = texture2D(texture, tex_coord1) * 0.06136;
    vec4 c2 = texture2D(texture, tex_coord2) * 0.24477;
    vec4 c3 = texture2D(texture, tex_coord3) * 0.38774;
    vec4 c4 = texture2D(texture, tex_coord4) * 0.24477;
    vec4 c5 = texture2D(texture, tex_coord5) * 0.06136;

    vec4 color = c1 + c2 + c3 + c4 + c5;

    gl_FragColor = color;
}
