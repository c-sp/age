
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform sampler2D texture;

uniform vec2 u_texture_size;
const vec2 c_pixel_center = vec2(0.5, 0.5);


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord);// + c_pixel_center;

    vec2 tex_coord1 = tex_coord + vec2(-1.0, 1.0);
    vec2 tex_coord2 = tex_coord;
    vec2 tex_coord3 = tex_coord + vec2(1.0, -1.0);

    tex_coord1 /= u_texture_size;
    tex_coord2 /= u_texture_size;
    tex_coord3 /= u_texture_size;

    vec4 c1 = texture2D(texture, tex_coord1);
    vec4 c2 = texture2D(texture, tex_coord2);
    vec4 c3 = texture2D(texture, tex_coord3);

    vec4 color = -c1 + 4.0 * c2 + c3;
    color /= 4.0;

    gl_FragColor = color;
}
