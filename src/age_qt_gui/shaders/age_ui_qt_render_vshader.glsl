
// Set default precision to medium
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_projection;
uniform vec4 u_color; // used to blend frames

attribute vec4 a_vertex;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
    gl_Position = u_projection * a_vertex;
    v_texcoord = a_texcoord;
    v_color = u_color;
}
