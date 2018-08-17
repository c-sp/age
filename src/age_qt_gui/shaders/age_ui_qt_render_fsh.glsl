
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform sampler2D texture;

uniform vec4 u_color; // used to blend frames

varying vec2 v_texcoord;


void main()
{
    gl_FragColor = u_color * texture2D(texture, v_texcoord);
}
