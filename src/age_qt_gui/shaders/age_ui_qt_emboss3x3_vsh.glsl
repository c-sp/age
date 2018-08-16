
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

uniform mat4 u_projection;
uniform vec2 u_texture_size;

attribute vec4 a_vertex;

varying vec2 v_texture_size;


void main()
{
    gl_Position = u_projection * a_vertex;
    v_texture_size = u_texture_size;
}
