
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform sampler2D texture;

varying vec2 v_texture_size;


void main()
{
    vec2 tex_coord = vec2(gl_FragCoord) / v_texture_size;
    gl_FragColor = texture2D(texture, tex_coord);
}
