
// Set default precision to medium
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;

varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
    gl_FragColor = v_color * texture2D(texture, v_texcoord);
}
