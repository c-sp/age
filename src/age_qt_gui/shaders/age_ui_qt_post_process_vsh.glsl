
// version directive and OpenGL ES precision qualifiers
// are added by the shader loader

attribute vec4 a_vertex;

void main()
{
    gl_Position = a_vertex;
}
