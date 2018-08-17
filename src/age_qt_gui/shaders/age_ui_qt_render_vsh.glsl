
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform mat4 u_projection;

attribute vec4 a_vertex;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;


void main()
{
    gl_Position = u_projection * a_vertex;
    v_texcoord = a_texcoord;
}
