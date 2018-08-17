
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform sampler2D texture;

uniform vec2 u_texture_size;
const vec2 c_pixel_center = vec2(0.5, 0.5);


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord);// + c_pixel_center;

    vec2 tex_coord1 = tex_coord + vec2(-2.0, 2.0);
    vec2 tex_coord2 = tex_coord + vec2(-1.0, 1.0);
    vec2 tex_coord3 = tex_coord;
    vec2 tex_coord4 = tex_coord + vec2(1.0, -1.0);
    vec2 tex_coord5 = tex_coord + vec2(2.0, -2.0);

    tex_coord1 /= u_texture_size;
    tex_coord2 /= u_texture_size;
    tex_coord3 /= u_texture_size;
    tex_coord4 /= u_texture_size;
    tex_coord5 /= u_texture_size;

    vec4 c1 = texture2D(texture, tex_coord1);
    vec4 c2 = texture2D(texture, tex_coord2);
    vec4 c3 = texture2D(texture, tex_coord3);
    vec4 c4 = texture2D(texture, tex_coord4);
    vec4 c5 = texture2D(texture, tex_coord5);

    vec4 color = -c1 - 2.0 * c2 + 7.0 * c3 + 2.0 * c4 + c5;
    color /= 7.0;

    gl_FragColor = color;
}
